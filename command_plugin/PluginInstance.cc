#include "stdafx.h"
#include "PluginInstance.h"

ConsoleAccess::ConsoleString consoleString;
HWND test;
void OutputHandler(const ConsoleAccess::ConsoleString& consoleStringIn)
{
	//MessageBoxA(NULL, "OutputHandler function", "Commander", MB_OK);
	consoleString = consoleStringIn;
	SendMessage(test, PluginInstance::WM_NewConsoleData,0, 0);
}
extern NPObject * outputHandler;

void pushString(ConsoleAccess::ConsoleString& cs, const std::string& toAppend)
{
	for(int i=0; i<toAppend.length(); i++)
	{
		cs.push_back(toAppend[i]);
	}
}

// COnvert a string so it can be put in html:
ConsoleAccess::ConsoleString transformString(ConsoleAccess::ConsoleString& inStr)
{
	ConsoleAccess::ConsoleString resultString;
	for(int i=0; i<inStr.size(); i++)
	{
		if(inStr[i]=='<')
		{
			pushString(resultString, "&lt;");
		}
		else if(inStr[i]=='>')
		{
			pushString(resultString, "&gt;");
		}
		else if(inStr[i]=='&')
		{
			pushString(resultString, "&amp;");
		}
		else if(inStr[i]=='\r')
		{
		}
		else if(inStr[i]=='\n')
		{
			pushString(resultString, "<br />");
		}
		else
		{
			resultString.push_back(inStr[i]);
		}
	}
	resultString.push_back('\0');
	return resultString;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	test=hWnd;
	if(message==PluginInstance::WM_NewConsoleData)
	{
		// MessageBoxA(NULL, "New Console Data received", "Commander", MB_OK);
		// Invoke the stored object with the consoleString parameter:
		ConsoleAccess::ConsoleString consoleStringCopy=transformString(consoleString);
		//consoleStringCopy.push_back('\0');

		NPVariant consoleLine[1];
		STRINGZ_TO_NPVARIANT(&consoleStringCopy[0], consoleLine[0]);

		NPVariant result;
		outputHandler->_class->invokeDefault(outputHandler, consoleLine, 1, &result);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}