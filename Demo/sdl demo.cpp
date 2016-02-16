#include <SDL2/include/SDL.h>
#include <SDL2/include/SDL_syswm.h> //platform info
#include <stdio.h>
#include <iostream>
#include <string>
#include <chrono>
//#include <Windows.h>

#include "../include/basilisk.h"

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "Basilisk.lib")

HWND hWnd;
const char *appName = "Basilisk";
HINSTANCE hInstance = GetModuleHandle(NULL);
constexpr bool FULL_SCREEN = false;

/*
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
	}
}

void initWindow(int screenW, int screenH)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Give the application a name.

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = appName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsHeight = (unsigned long)screenH;
		dmScreenSettings.dmPelsWidth = (unsigned long)screenW;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenW) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenH) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	hWnd = CreateWindowEx(WS_EX_APPWINDOW, appName, appName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenW, screenH, NULL, NULL, hInstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	// Hide the mouse cursor.
	ShowCursor(false);
}

void closeWindow()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(hWnd);
	hWnd = NULL;

	// Remove the application instance.
	UnregisterClass(appName, hInstance);
	hInstance = NULL;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	unsigned int screenHeight = 480;// GetSystemMetrics(SM_CYSCREEN);
	unsigned int screenWidth = 720;// GetSystemMetrics(SM_CXSCREEN);

	initWindow(screenWidth, screenHeight);
	ChangeDisplaySettings(NULL, 0);

	Basilisk::Device device;
	if (!device.initialize(hWnd, screenWidth, screenHeight, true, false))
	{
		OutputDebugString(Basilisk::error.getDetails());
		return 0;
	}

	unsigned long long fence = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	start = std::chrono::high_resolution_clock::now();
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	done = false;
	while (!done)
	{
		// Handle the windows messages.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			if (!(device.beginFrame(fence) &&
				device.execute(nullptr) &&
				(fence = device.present())))
			{
				OutputDebugString(Basilisk::error.getDetails());
				done = true;
			}
		}

	}
	end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> secondsPassed = end - start;
	float fps = fence / secondsPassed.count();
	std::string sFps = std::to_string(fps);
	OutputDebugString(sFps.c_str());
	OutputDebugString(" avg fps\n");

	device.shutdown();

	closeWindow();
	return 0;
}


*/


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	const int w = 720, h = 480;
	
	SDL_DisplayMode monitor;
	if (SDL_GetDesktopDisplayMode(0, &monitor) != 0) {
		SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
		return 1;
	}

	//get window handle to initialize Vulkan with

	SDL_Window *window = SDL_CreateWindow("Hello World", monitor.w/2-w/2, monitor.h/2-h/2, w, h, 0);
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version); // initialize info structure with SDL version info
	SDL_GetWindowWMInfo(window, &wmInfo);
#ifdef SDL_VIDEO_DRIVER_WINDOWS
	HWND hwnd = wmInfo.info.win.window;


	Basilisk::Device device;
	if (!device.initialize(hwnd, w, h, false, false))
	{
		OutputDebugString(Basilisk::error.getDetails());
		return 0;
	}

#else
#error Only coded for Windows (for now)!
#endif

	SDL_Event windowEvent;
	unsigned long long fence = 0;
	SDL_PollEvent(&windowEvent);
	std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> begin, execute, present, other;
	begin = std::chrono::duration<float>(0);
	execute = std::chrono::duration<float>(0);
	present = std::chrono::duration<float>(0);
	other = std::chrono::duration<float>(0);
	while (windowEvent.type != SDL_QUIT)
	{
		if (windowEvent.type == SDL_KEYDOWN)
			if (windowEvent.key.keysym.sym == SDLK_f)
				device.setFullscreen(false);

		other += std::chrono::high_resolution_clock::now() - start;
		start = std::chrono::high_resolution_clock::now();
		if (device.beginFrame(fence))
		{
			begin += std::chrono::high_resolution_clock::now() - start;
			start = std::chrono::high_resolution_clock::now();
			if (device.execute(nullptr))
			{
				execute += std::chrono::high_resolution_clock::now() - start;
				start = std::chrono::high_resolution_clock::now();

				fence = device.present();

				present += std::chrono::high_resolution_clock::now() - start;
				start = std::chrono::high_resolution_clock::now();
			}
		}
		//{
		//	OutputDebugString(Basilisk::error.getDetails());
		//	device.shutdown();
		//	return 0;
		//}
		SDL_PollEvent(&windowEvent);
	}

	std::string toString = std::to_string(other.count());
	OutputDebugString(toString.c_str());
	OutputDebugString("\n");
	toString = std::to_string(begin.count());
	OutputDebugString(toString.c_str());
	OutputDebugString("\n");
	toString = std::to_string(execute.count());
	OutputDebugString(toString.c_str());
	OutputDebugString("\n");
	toString = std::to_string(present.count());
	OutputDebugString(toString.c_str());
	OutputDebugString("\n");
	//std::chrono::duration<float> secondsPassed = std::chrono::high_resolution_clock::now() - start;
	//float fps = fence / secondsPassed.count();
	//std::string sFps = std::to_string(fps);
	//OutputDebugString(sFps.c_str());
	//OutputDebugString(" avg fps\n");

	device.release();
	return 0;
}
