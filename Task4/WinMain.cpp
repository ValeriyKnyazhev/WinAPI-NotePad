#include <Windows.h>
#include "COverlappedWindow.h"
#include <iostream>

int WINAPI wWinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	LPWSTR commandLine,
	int cmdShow)
{
	COverlappedWindow cWindow;
	if (!cWindow.RegisterClass())
	{
		return -1;
	}

	if (!cWindow.Create())
	{
		return -1;
	}
	cWindow.Show(cmdShow);

	MSG message;
	BOOL getMessageResult = 0;
	while ((getMessageResult = ::GetMessage(&message, 0, 0, 0)) != 0) {
		if (getMessageResult == -1) {
			return -1;
		}
		if (!TranslateAccelerator(cWindow.GetHandle(), // handle to receiving window
			cWindow.GetHaccel(), // handle to active accelerator table
			&message// message data
			) | !::IsDialogMessage(cWindow.GetHandleDialog(), &message)) {
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
	}

	return 0;
}