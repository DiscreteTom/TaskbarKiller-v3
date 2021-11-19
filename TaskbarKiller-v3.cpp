#include <Windows.h>
#include <WinUser.h>
#include <wingdi.h>

#ifdef _DEBUG
#include <stdio.h>
#endif

#ifndef _DEBUG
// hide command line window
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

const int HOTKEY_ID = 1;

double horizontalScale;
double verticalScale;
HWND taskbar;
RECT position;

// Ref: https://stackoverflow.com/questions/54912038/querying-windows-display-scaling
void get_display_scale() {
	auto activeWindow = GetActiveWindow();
	HMONITOR monitor = MonitorFromWindow(activeWindow, MONITOR_DEFAULTTONEAREST);

	// Get the logical width and height of the monitor
	MONITORINFOEX monitorInfoEx;
	monitorInfoEx.cbSize = sizeof(monitorInfoEx);
	GetMonitorInfo(monitor, &monitorInfoEx);
	auto cxLogical = monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left;
	auto cyLogical = monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top;

	// Get the physical width and height of the monitor
	DEVMODE devMode;
	devMode.dmSize = sizeof(devMode);
	devMode.dmDriverExtra = 0;
	EnumDisplaySettings(monitorInfoEx.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
	auto cxPhysical = devMode.dmPelsWidth;
	auto cyPhysical = devMode.dmPelsHeight;

	// Calculate the scaling factor
	horizontalScale = ((double)cxPhysical / (double)cxLogical);
	verticalScale = ((double)cyPhysical / (double)cyLogical);

#ifdef _DEBUG
	printf("horizontalScale: %f\n", horizontalScale);
	printf("verticalScale: %f\n", verticalScale);
#endif
}

void register_hot_key() {
#ifdef _DEBUG
	printf("registering hot key...");
#endif
	if (RegisterHotKey(
		NULL, // this thread will process the hotkey
		HOTKEY_ID,
		MOD_WIN, // win
		VK_OEM_3  // ~
	)) {
#ifdef _DEBUG
		printf("done.\n");
#endif
	}
}

void unregister_hot_key() {
	UnregisterHotKey(NULL, HOTKEY_ID);
}

void hide_taskbar() {
#ifdef _DEBUG
	printf("hide\n");
#endif
	auto empty = CreateRectRgn(0, 0, 0, 0);
	SetWindowRgn(taskbar, empty, true);
	DeleteObject(empty);
}

void show_taskbar() {
#ifdef _DEBUG
	printf("show\n");
#endif
	auto region = CreateRectRgn(position.left, position.top, position.right * horizontalScale, position.bottom * verticalScale);
	SetWindowRgn(taskbar, region, true);
	DeleteObject(region);
}

void run() {
	MSG msg = { 0 };
	bool hidden = false;
	while (GetMessage(&msg, NULL, 0, 0) != 0) // while msg is not WM_QUIT
	{
		if (msg.message == WM_HOTKEY) // process hot key
		{
			hidden = !hidden;
			if (hidden)
				hide_taskbar();
			else
				show_taskbar();
		}
	}
}

void format_position() {
	if (position.left < 0) {
		// taskbar is on the left, move right
#ifdef _DEBUG
		printf("taskbar is on the left.\n");
		printf("original left: %d\n", position.left);
		printf("original right: %d\n", position.right);
#endif
		position.right -= position.left;
		position.left = 0;
#ifdef _DEBUG
		printf("corrected left: %d\n", position.left);
		printf("corrected right: %d\n", position.right);
#endif
	}
	if (position.bottom < 0) {
		// taskbar is on the bottom, move up
#ifdef _DEBUG
		printf("taskbar is on the bottom.\n");
		printf("original top: %d\n", position.top);
		printf("original bottom: %d\n", position.bottom);
#endif
		position.top -= position.bottom;
		position.bottom = 0;
#ifdef _DEBUG
		printf("corrected top: %d\n", position.top);
		printf("corrected bottom: %d\n", position.bottom);
#endif
	}
}

int main()
{
	get_display_scale();
	register_hot_key();

	// get taskbar handle
	taskbar = FindWindowExA(NULL, NULL, "Shell_TrayWnd", NULL);
#ifdef _DEBUG
	printf("taskbar handle: %d\n", taskbar);
#endif

	// get taskbar position
	GetWindowRect(taskbar, &position);
	format_position();

	// handle hot key, hide/show taskbar
	run();

	// clean up
	show_taskbar();
	unregister_hot_key();
}

