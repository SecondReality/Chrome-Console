#include <assert.h>
#include <npfunctions.h>
#include "ConsoleAccess.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void OutputHandler(const ConsoleAccess::ConsoleString&);

class PluginInstance
{
public:
	PluginInstance(NPP instance): window_(0), consoleAccess_(&OutputHandler)
	{
	}

	void startConsole()
	{
		// Start the console io thread:
		boost::thread thread(boost::bind(&ConsoleAccess::run, &consoleAccess_));
	}

	void sendKey(int keyCode)
	{
		ConsoleAccess::ConsoleString consoleString;
		consoleString.push_back(keyCode);
		consoleAccess_.sendString(consoleString);
	}

	void sendString(const ConsoleAccess::ConsoleString& consoleString)
	{
		/*
		ConsoleAccess::ConsoleString consoleString;
		for(int i=0; i<str.length(); i++)
		{
			// TODO: a conversion may be needed
			consoleString.push_back(str[i]);
		}
		*/
		consoleAccess_.sendString(consoleString);
	}

	void set_window(HWND window)
	{

		//assert(window_==0);
//		assert(window!=0);

		window_=window;
		if(window_==0)
		{
			// MessageBoxA(NULL, "Window is null", "Commander", MB_OK);
		}
		else
		{
			// MessageBoxA(NULL, "Window is being set", "Commander", MB_OK);
			// Set the window procedure:
			SetLastError(0);
			LONG result = SetWindowLong(window_, GWL_WNDPROC, (LONG)&WindowProc);
			assert(result!=0);
		}

	}
	static const int WM_NewConsoleData = WM_USER;
private:
	HWND window_;
	ConsoleAccess consoleAccess_;
};