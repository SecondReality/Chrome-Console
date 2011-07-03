#include <stdafx.h>  
#include "ConsoleAccess.h"
#include<windows.h>
#include <iostream>

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
#include <sstream>
void ConsoleAccess::run()
{
	  //MessageBoxA(NULL, "ConsoleAccess::run function", "Commander", MB_OK);
	  //DisplayError(L"just a test - ignore");

	   //std::cout << "Hello";
      HANDLE hOutputReadTmp,hOutputRead,hOutputWrite;
      HANDLE hInputWriteTmp,hInputRead,hInputWrite;
      HANDLE hErrorWrite;
      HANDLE hThread;
      DWORD ThreadId;
      SECURITY_ATTRIBUTES sa;


      // Set up the security attributes struct.
      sa.nLength= sizeof(SECURITY_ATTRIBUTES);
      sa.lpSecurityDescriptor = NULL;
      sa.bInheritHandle = TRUE;


      // Create the child output pipe.
      if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0))
         DisplayError(L"CreatePipe");


      // Create a duplicate of the output write handle for the std error
      // write handle. This is necessary in case the child application
      // closes one of its std output handles.
      if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite,
                           GetCurrentProcess(),&hErrorWrite,0,
                           TRUE,DUPLICATE_SAME_ACCESS))
         DisplayError(L"DuplicateHandle");


      // Create the child input pipe.
      if (!CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0))
         DisplayError(L"CreatePipe");


      // Create new output read handle and the input write handles. Set
      // the Properties to FALSE. Otherwise, the child inherits the
      // properties and, as a result, non-closeable handles to the pipes
      // are created.
      if (!DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
                           GetCurrentProcess(),
                           &hOutputRead, // Address of new handle.
                           0,FALSE, // Make it uninheritable.
                           DUPLICATE_SAME_ACCESS))
         DisplayError(L"DupliateHandle");

      if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
                           GetCurrentProcess(),
                           &hInputWrite, // Address of new handle.
                           0,FALSE, // Make it uninheritable.
                           DUPLICATE_SAME_ACCESS))
      DisplayError(L"DupliateHandle");


      // Close inheritable copies of the handles you do not want to be
      // inherited.
      if (!CloseHandle(hOutputReadTmp)) DisplayError(L"CloseHandle");
      if (!CloseHandle(hInputWriteTmp)) DisplayError(L"CloseHandle");


      // Get std input handle so you can close it and force the ReadFile to
      // fail when you want the input thread to exit.
      if ( (hStdIn = GetStdHandle(STD_INPUT_HANDLE)) ==
                                                INVALID_HANDLE_VALUE )
         DisplayError(L"GetStdHandle");

      PrepAndLaunchRedirectedChild(hOutputWrite,hInputRead,hErrorWrite);


      // Close pipe handles (do not continue to modify the parent).
      // You need to make sure that no handles to the write end of the
      // output pipe are maintained in this process or else the pipe will
      // not close when the child process exits and the ReadFile will hang.
      if (!CloseHandle(hOutputWrite)) DisplayError(L"CloseHandle");
      if (!CloseHandle(hInputRead )) DisplayError(L"CloseHandle");
      if (!CloseHandle(hErrorWrite)) DisplayError(L"CloseHandle");


	  writeHandle_ = hInputWrite;
      // Launch the thread that gets the input and sends it to the child.
	  /*
      hThread = CreateThread(NULL,0,GetAndSendInputThread,
                              (LPVOID)hInputWrite,0,&ThreadId);
      if (hThread == NULL) DisplayError("CreateThread");
	  */


      // Read the child's output.
	  while(true)
	  {
		ReadAndHandleOutput(hOutputRead);
	  }
      // Redirection is complete


      // Force the read on the input to return by closing the stdin handle.
      if (!CloseHandle(hStdIn)) DisplayError(L"CloseHandle");


      // Tell the thread to exit and wait for thread to die.
      bRunThread = FALSE;

      if (WaitForSingleObject(hThread,INFINITE) == WAIT_FAILED)
         DisplayError(L"WaitForSingleObject");

      if (!CloseHandle(hOutputRead)) DisplayError(L"CloseHandle");
      if (!CloseHandle(hInputWrite)) DisplayError(L"CloseHandle");
   }

#include <string>

   /////////////////////////////////////////////////////////////////////// 
   // PrepAndLaunchRedirectedChild
   // Sets up STARTUPINFO structure, and launches redirected child.
   /////////////////////////////////////////////////////////////////////// 
   void PrepAndLaunchRedirectedChild(HANDLE hChildStdOut,
                                     HANDLE hChildStdIn,
                                     HANDLE hChildStdErr)
   {
	   /*
      PROCESS_INFORMATION pi;
      STARTUPINFOA si;

      // Set up the start up info struct.
      ZeroMemory(&si,sizeof(STARTUPINFOA));
      si.cb = sizeof(STARTUPINFOA);
      si.dwFlags = STARTF_USESTDHANDLES;
      si.hStdOutput = hChildStdOut;
      si.hStdInput  = hChildStdIn;
      si.hStdError  = hChildStdErr;

      // Use this if you want to hide the child:
	//si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
//      si.wShowWindow = SW_HIDE;
	  		si.dwFlags		= STARTF_USEPOSITION;
		si.dwX			= 0x7FFF;
		si.dwY			= 0x7FFF;
      // Note that dwFlags must include STARTF_USESHOWWINDOW if you want to
      // use the wShowWindow flags.

	  DWORD dwStartupFlags = CREATE_NEW_CONSOLE|CREATE_SUSPENDED|CREATE_UNICODE_ENVIRONMENT;

      // Launch the process that you want to redirect (in this case,
      // Child.exe). Make sure Child.exe is in the same directory as
      // redirect.c launch redirect from a command line to prevent location
      // confusion.
	  LPWSTR command = L"cmd.exe";
      if (!CreateProcessA(NULL,"cmd.exe",NULL,NULL,TRUE,CREATE_NEW_CONSOLE|CREATE_SUSPENDED|CREATE_UNICODE_ENVIRONMENT,NULL,NULL,&si,&pi))
	  {
         DisplayError("CreateProcess");
	  }


      // Set global child process handle to cause threads to exit.
      hChildProcess = pi.hProcess;


      // Close any unnecessary handles.
      if (!CloseHandle(pi.hThread)) DisplayError("CloseHandle");*/

	   std::wstring pszCmdLine = L"cmd.exe";
	   	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	// Set up the start up info struct.
	::ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
      si.hStdOutput = hChildStdOut;
      si.hStdInput  = hChildStdIn;
      si.hStdError  = hChildStdErr;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

	// Note that dwFlags must include STARTF_USESHOWWINDOW if we
	// use the wShowWindow flags. This also assumes that the
	// CreateProcess() call will use CREATE_NEW_CONSOLE.

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
	// Close any unuseful handles
	::CloseHandle(pi.hThread);

   }


   /////////////////////////////////////////////////////////////////////// 
   // ReadAndHandleOutput
   // Monitors handle for input. Exits when child exits or pipe breaks.
   /////////////////////////////////////////////////////////////////////// 
   void ConsoleAccess::ReadAndHandleOutput(HANDLE hPipeRead)
   {
      CHAR lpBuffer[256];
      DWORD nBytesRead;

      while(TRUE)
      {
		  
		DWORD dwAvail = 0;
		if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL,
			&dwAvail, NULL))			// error
			break;

		if (!dwAvail)					// not data available
			break;
			

         if (!ReadFile(hPipeRead,lpBuffer,sizeof(lpBuffer),
                                          &nBytesRead,NULL))
         {
			 nBytesRead;
            if (GetLastError() == ERROR_BROKEN_PIPE)
               break; // pipe done - normal exit path.
            else
               DisplayError(L"ReadFile"); // Something bad happened.
         }

         // Display the character read on the screen.
		 /*
         if (!WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),lpBuffer,
                           nBytesRead,&nCharsWritten,NULL))
            DisplayError("WriteConsole");
			*/
		 //printf
		 //std::cout.write(lpBuffer, nBytesRead);
		 //MessageBoxA(NULL, "ReadAndHandleOutput -> Output is being sent", "Commander", MB_OK);
		 //MessageBoxA(NULL, "GOT!", "Commander", MB_OK);
		 ConsoleString consoleString(nBytesRead);
		 std::copy(&lpBuffer[0], &lpBuffer[nBytesRead], consoleString.begin());
		 callback_(consoleString);
      }
   }


   /////////////////////////////////////////////////////////////////////// 
   // GetAndSendInputThread
   // Thread procedure that monitors the console for input and sends input
   // to the child process through the input pipe.
   // This thread ends when the child application exits.
   /////////////////////////////////////////////////////////////////////// 
void ConsoleAccess::sendString(const ConsoleString& consoleString)
{
		  //std::stringstream ss;
	  //ss << "WRite handle 2: " << writeHandle_;
	  //MessageBoxA(NULL,ss.str().c_str(), "Commander", MB_OK);

  //CHAR read_buff[256];
  DWORD nBytesWrote;
  //HANDLE hPipeWrite = (HANDLE)lpvThreadParam;

  // Get input from our console and send it to child through the pipe.

	  	 /*
     if(!ReadConsole(hStdIn,read_buff,1,&nBytesRead,NULL))
        DisplayError("ReadConsole");

     read_buff[nBytesRead] = '\0'; // Follow input with a NULL.


     if (!WriteFile(hPipeWrite,read_buff,nBytesRead,&nBytesWrote,NULL))
     {
        if (GetLastError() == ERROR_NO_DATA)
		{
          // break; // Pipe was closed (normal exit path).
		}
        else
        DisplayError("WriteFile");
     }
	 */
  //MessageBoxA(NULL, "sendString", "Commander", MB_OK);
	ConsoleString consoleStringCopy = consoleString;
	// this null needed?
	//consoleStringCopy.push_back('\0');
	
	if (!WriteFile(writeHandle_, &consoleStringCopy[0], consoleStringCopy.size(), &nBytesWrote, NULL))
	//if (!WriteFile(writeHandle_, L"tesing\r\n", 8, &nBytesWrote, NULL))
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
	// MessageBoxA(NULL, "seasdandString", "Commander", MB_OK);
}

#include <strsafe.h>
   /////////////////////////////////////////////////////////////////////// 
   // DisplayError
   // Displays the error number and corresponding message.
   /////////////////////////////////////////////////////////////////////// 
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

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)pszAPI) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        pszAPI, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}