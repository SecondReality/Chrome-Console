#include <stdafx.h>  
#include <windows.h>
#include <iostream>
#include "ConsoleAccess.h"
#include <sstream>
#include <strsafe.h>

#pragma comment(lib, "User32.lib")

void DisplayError(LPCTSTR pszAPI);
void ReadAndHandleOutput(HANDLE hPipeRead);
void PrepAndLaunchRedirectedChild(HANDLE hChildStdOut,
                                 HANDLE hChildStdIn,
                                 HANDLE hChildStdErr);
DWORD WINAPI GetAndSendInputThread(LPVOID lpvThreadParam);

HANDLE hChildProcess = NULL;
HANDLE hStdIn = NULL; // Handle to parents std input.
BOOL bRunThread = TRUE;

ConsoleAccess::ConsoleAccess(outputHandler callback): callback_(callback)
{
}

ConsoleAccess::~ConsoleAccess()
{
}

void ConsoleAccess::run()
{
	HANDLE hOutputReadTmp, hOutputRead, hOutputWrite;
	HANDLE hInputWriteTmp, hInputRead, hInputWrite;
	HANDLE hErrorWrite;
	SECURITY_ATTRIBUTES sa;

	// Set up the security attributes struct.
	sa.nLength= sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// Create the child output pipe.
	if(!CreatePipe(&hOutputReadTmp, &hOutputWrite,&sa,0))
	{
		DisplayError(L"CreatePipe");
	}

	// Create a duplicate of the output write handle for the std error
	// write handle. This is necessary in case the child application closes one of its std output handles.
	if(!DuplicateHandle(GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		DisplayError(L"DuplicateHandle");
	}

	// Create the child input pipe
	if(!CreatePipe(&hInputRead, &hInputWriteTmp,&sa,0))
	{
		DisplayError(L"CreatePipe");
	}

	// Create new output read handle and the input write handles. Set
	// the Properties to FALSE. Otherwise, the child inherits the
	// properties and, as a result, non-closeable handles to the pipes
	// are created.
	if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(), &hOutputRead, 0,FALSE, DUPLICATE_SAME_ACCESS))
	{
		DisplayError(L"DuplicateHandle");
	}

	if(!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp, GetCurrentProcess(), &hInputWrite, 0,FALSE, DUPLICATE_SAME_ACCESS))
	{
		DisplayError(L"DuplicateHandle");
	}

	// Close inheritable copies of the handles you do not want to be inherited.
	if(!CloseHandle(hOutputReadTmp))
	{
		DisplayError(L"CloseHandle");
	}

	if(!CloseHandle(hInputWriteTmp))
	{
		DisplayError(L"CloseHandle");
	}

	// Get std input handle so you can close it and force the ReadFile to
	// fail when you want the input thread to exit.
	if( (hStdIn = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE )
	{
		DisplayError(L"GetStdHandle");
	}

	PrepAndLaunchRedirectedChild(hOutputWrite, hInputRead, hErrorWrite);

	// Close pipe handles (do not continue to modify the parent).
	// You need to make sure that no handles to the write end of the
	// output pipe are maintained in this process or else the pipe will
	// not close when the child process exits and the ReadFile will hang.
	if (!CloseHandle(hOutputWrite)) DisplayError(L"CloseHandle");
	if (!CloseHandle(hInputRead )) DisplayError(L"CloseHandle");
	if (!CloseHandle(hErrorWrite)) DisplayError(L"CloseHandle");

	writeHandle_ = hInputWrite;

	// Read the child's output
	while(true)
	{
		ReadAndHandleOutput(hOutputRead);
		Sleep(100);
	}

	// Force the read on the input to return by closing the stdin handle.
	//if (!CloseHandle(hStdIn)) DisplayError(L"CloseHandle");

	// Tell the thread to exit and wait for thread to die.
	bRunThread = FALSE;
}

// PrepAndLaunchRedirectedChild
// Sets up STARTUPINFO structure, and launches redirected child.
void PrepAndLaunchRedirectedChild(HANDLE hChildStdOut,
                                 HANDLE hChildStdIn,
                                 HANDLE hChildStdErr)
{
	std::wstring pszCmdLine = L"cmd.exe";
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	// Set up the start up info struct.
	// Note that dwFlags must include STARTF_USESHOWWINDOW if we use the wShowWindow flags.
	// This also assumes that the CreateProcess() call will use CREATE_NEW_CONSOLE.
	::ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.hStdOutput = hChildStdOut;
	si.hStdInput  = hChildStdIn;
	si.hStdError  = hChildStdErr;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

	// Launch the child process.
	::CreateProcess(
		NULL,
		const_cast<wchar_t*>(pszCmdLine.c_str()),
		NULL, NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL, NULL,
		&si,
		&pi);

	hChildProcess = pi.hProcess;
	::CloseHandle(pi.hThread);
}

// ReadAndHandleOutput
void ConsoleAccess::ReadAndHandleOutput(HANDLE hPipeRead)
{
  CHAR lpBuffer[256];
  DWORD nBytesRead;

  while(TRUE)
  {
	  
	DWORD dwAvail = 0;
	if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
	{
		break;
	}

	if (!dwAvail)
	{
		break;
	}
	
     if (!ReadFile(hPipeRead,lpBuffer,sizeof(lpBuffer), &nBytesRead,NULL))
     {
		 nBytesRead;
        if (GetLastError() == ERROR_BROKEN_PIPE)
           break; // pipe done - normal exit path.
        else
           DisplayError(L"ReadFile"); // Something bad happened.
     }

	 ConsoleString consoleString(nBytesRead);
	 std::copy(&lpBuffer[0], &lpBuffer[nBytesRead], consoleString.begin());
	 callback_(consoleString);
  }
}
 
// GetAndSendInputThread
// Thread procedure that monitors the console for input and sends input
// to the child process through the input pipe.
// This thread ends when the child application exits.
void ConsoleAccess::sendString(const ConsoleString& consoleString)
{
	DWORD nBytesWrote;
	ConsoleString consoleStringCopy = consoleString;

	if (!WriteFile(writeHandle_, &consoleStringCopy[0], consoleStringCopy.size(), &nBytesWrote, NULL))
	{
		if (GetLastError() == ERROR_NO_DATA)
		{
			// Pipe was closed (normal exit path).
			MessageBoxA(NULL, "sendString1a", "Commander", MB_OK);
		}
		else
		{
			DisplayError(L"WriteFile");
			MessageBoxA(NULL, "sendString1b", "Commander", MB_OK);
		}
	}
}

// DisplayError
// Displays the error number and corresponding message.
void DisplayError(LPCTSTR pszAPI)
{
 LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)pszAPI) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), pszAPI, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}