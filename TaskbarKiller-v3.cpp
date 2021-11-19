#include <Windows.h>
#include <WinUser.h>
#include <wingdi.h>

// hide command line window
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

const int HOTKEY_ID = 1;

double horizontalScale;
double verticalScale;
HWND taskbar;
HRGN region;
HRGN empty;

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
}

void register_hot_key() {
	RegisterHotKey(
		NULL, // this thread will process the hotkey
		HOTKEY_ID,
		MOD_WIN, // win
		VK_OEM_3  // ~
	);
}

void unregister_hot_key() {
	UnregisterHotKey(NULL, HOTKEY_ID);
}

void hide_taskbar() {
	SetWindowRgn(taskbar, empty, true);
}

void show_taskbar() {
	SetWindowRgn(taskbar, region, true);
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

int main()
{
	get_display_scale();
	register_hot_key();

	// get taskbar handle
	taskbar = FindWindowExA(NULL, NULL, "Shell_TrayWnd", NULL);

	// store taskbar region
	RECT rc;
	GetWindowRect(taskbar, &rc);
	region = CreateRectRgn(rc.left, rc.top, rc.right * horizontalScale, rc.bottom * verticalScale);

	// create an empty region to hide taskbar
	empty = CreateRectRgn(0, 0, 0, 0);

	// handle hot key, hide/show taskbar
	run();

	// clean up
	DeleteObject(empty);
	DeleteObject(region);
	unregister_hot_key();
}

