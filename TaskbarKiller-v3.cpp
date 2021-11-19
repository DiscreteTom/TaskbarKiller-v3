#include <Windows.h>
#include <WinUser.h>
#include <wingdi.h>
#include <stdio.h>
#include <iostream>

using namespace std;

double horizontalScale;
double verticalScale;

// Ref: https://stackoverflow.com/questions/54912038/querying-windows-display-scaling
void get_scale() {
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

int main()
{
	get_scale();

	// get taskbar handle
	auto taskbar = FindWindowExA(NULL, NULL, "Shell_TrayWnd", NULL);
	cout << taskbar << endl;

	// store taskbar region
	RECT rc;
	GetWindowRect(taskbar, &rc);
	auto region = CreateRectRgn(rc.left, rc.top, rc.right * horizontalScale, rc.bottom * verticalScale);

	// hide taskbar
	auto empty = CreateRectRgn(0, 0, 0, 0);
	if (SetWindowRgn(taskbar, empty, true) != 0) {
		getchar();
		// restore taskbar
		SetWindowRgn(taskbar, region, true);
	}

	// clean up
	DeleteObject(empty);
	DeleteObject(region);
}

