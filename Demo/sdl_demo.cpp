/**
\file   sdl_demo.cpp
\author Andrew Baxter
\date   March 14, 2016

Boots up a window and clears it a solid color each frame
\todo Rework smart pointer management. At the moment, I can't rely on them deconstructing in the required order without nasty brackets.

*/

#include <SDL2/include/SDL.h>
#include <SDL2/include/SDL_syswm.h> //platform info
#include <iostream>

#include "../include/basilisk.h"
#include <Windows.h>

#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "Basilisk.lib")


HWND hWnd;
HINSTANCE hInstance;
constexpr const char *appName = "Hello World";
constexpr uint32_t appVersion = 1;


int Dump()
{
	OutputDebugString("Errors:\n");
	while (Basilisk::errors.size() > 0)
	{
		OutputDebugString((Basilisk::errors.front() + "\n").c_str());
		Basilisk::errors.pop();
	}

	OutputDebugString("Warnings:\n");
	while (Basilisk::warnings.size() > 0)
	{
		OutputDebugString((Basilisk::warnings.front() + "\n").c_str());
		Basilisk::warnings.pop();
	}

	return 1;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	const int w = 720, h = 480;
	
	SDL_DisplayMode monitor;
	if (SDL_GetDesktopDisplayMode(0, &monitor) != 0) {
		SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
		return 1;
	}

	//get window handle to initialize with
	SDL_Window *window = SDL_CreateWindow(appName, monitor.w/2-w/2, monitor.h/2-h/2, w, h, 0); //Center in the monitor
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	hWnd = wmInfo.info.win.window;
	hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));


	auto instance = Vulkan::Initialize(appName, appVersion);
	if (!instance) return Dump();
	uint32_t numGpus = instance->FindGpus();
	if (!numGpus) return Dump();
	if (!instance->HookWin32Window(0, hWnd, hInstance)) return Dump();

	auto device = instance->CreateDevice(0);
	if (!device) return Dump();

	{ //Make sure device outlives the objects it creates
		auto cmdSetup = device->CreateCommandBuffer(Vulkan::graphicsIndex);
		if (!cmdSetup) return Dump();
		cmdSetup->Begin(false);

		auto swapChain = device->CreateSwapChain(cmdSetup, { w, h }, 2); //Double-buffered 720x480 window
		if (!swapChain) return Dump();
		auto frameBuffer = device->CreateFrameBuffer({ w, h }, { VK_FORMAT_R8G8B8A8_UNORM }, false); //A single 24-bit, 720x480 render target (no depth buffer)
		if (!frameBuffer) return Dump();

		//auto pipelineLayout = device->CreatePipelineLayout({}); //Empty pipeline layout
		//if (!pipelineLayout) return Dump();
		//auto pipeline = device->CreateGraphicsPipeline(frameBuffer, pipelineLayout, {}); //Bare-bones graphics pipeline
		//if (!pipeline) return Dump();

		cmdSetup->End();
		device->ExecuteCommands({ cmdSetup });
		device->Join();

		auto cmdDraw = device->CreateCommandBuffer(Vulkan::graphicsIndex);
		if (!cmdDraw) return Dump();

		SDL_Event windowEvent;
		unsigned long long fence = 0;
		SDL_PollEvent(&windowEvent);

		while (windowEvent.type != SDL_QUIT)
		{
			device->Join();

			uint32_t currentBuffer = swapChain->GetBufferIndex();

			if (!device->PostPresent(swapChain, currentBuffer)) return Dump();

			//Fill the draw command buffer
			if (!cmdDraw->Begin(true)) return Dump();
			std::vector<VkClearValue> clearValues(1);
			clearValues[0].color = { 0.0f, 0.0f, 0.2f, 0.0f };
			cmdDraw->BeginRendering(frameBuffer, clearValues, false);
			cmdDraw->EndRendering();
			cmdDraw->Blit(frameBuffer, swapChain, 0, currentBuffer); //Can't be baked because of the way I've set up frame buffers. Consider changing.
			if (!cmdDraw->End()) return Dump();

			if (!device->ExecuteCommands({ cmdDraw })) return Dump();

			if (!device->PrePresent(swapChain, currentBuffer)) return Dump();
			device->Present(swapChain, currentBuffer);

			device->Join();
			SDL_PollEvent(&windowEvent);
		}
	}
	
	Dump();
	return 0;
}


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