/**
\file   rainbow.cpp
\author Andrew Baxter
\date   March 17, 2016

Boots up a window and clears it a different color of the rainbow, cycling back through every 20 seconds

*/

#include <chrono>
#include <Windows.h>

#include <basilisk.h>
#pragma comment(lib, "Basilisk.lib")


HWND hWnd = NULL;
HINSTANCE hInstance = NULL;
constexpr const char *appName = "Rainbow Demo";
constexpr uint32_t appVersion = 1;
constexpr bool fullScreen = true;


int Dump()
{
	OutputDebugString("Errors:\n");
	if (Basilisk::errors.size() == 0)
		OutputDebugString("None\n");
	while (Basilisk::errors.size() > 0)
	{
		OutputDebugString((Basilisk::errors.front() + "\n").c_str());
		Basilisk::errors.pop();
	}

	OutputDebugString("Warnings:\n");
	if (Basilisk::warnings.size() == 0)
		OutputDebugString("None\n");
	while (Basilisk::warnings.size() > 0)
	{
		OutputDebugString((Basilisk::warnings.front() + "\n").c_str());
		Basilisk::warnings.pop();
	}

	return 1;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		//Check if the window is being destroyed or closed manually
		case WM_DESTROY:
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_KEYDOWN:
		{
			if (wparam == VK_ESCAPE)
				PostQuitMessage(0);

			return 0;
		}
		default:
		{
			return DefWindowProc(hwnd, umessage, wparam, lparam);
		}
	}
}

void initWindow(int screenW, int screenH)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings = {};

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = appName;
	wc.cbSize = sizeof(WNDCLASSEX);
	RegisterClassEx(&wc);

	if (fullScreen)
	{
		//If full screen, set the screen to maximum size of the user's desktop and use a 32-bit presentation surface
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsHeight = static_cast<unsigned long>(screenH);
		dmScreenSettings.dmPelsWidth = static_cast<unsigned long>(screenW);
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	}

	//Create the window with the screen settings and get the handle to it
	hWnd = CreateWindowEx(WS_EX_APPWINDOW, appName, appName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		(GetSystemMetrics(SM_CXSCREEN) - screenW) / 2, (GetSystemMetrics(SM_CYSCREEN) - screenH) / 2, //Plant the window in the center of the screen
		screenW, screenH, NULL, NULL, hInstance, NULL);

	//Bring the window up on the screen and set it as main focus
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	
	ShowCursor(FALSE);
}

void closeWindow()
{
	ShowCursor(TRUE);

	if (fullScreen) //Fix display settings on exit
		ChangeDisplaySettings(NULL, 0);

	//Kill the window
	DestroyWindow(hWnd);
	hWnd = NULL;

	//Unhook it from the application instance
	UnregisterClass(appName, hInstance);
	hInstance = NULL;
}

glm::vec3 HueToRGB(float h)
{
	glm::vec3 rgb;
	rgb.r = glm::clamp(std::abs(h * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
	rgb.g = glm::clamp(2.0f - std::abs(h * 6.0f - 2.0f), 0.0f, 1.0f);
	rgb.b = glm::clamp(2.0f - std::abs(h * 6.0f - 4.0f), 0.0f, 1.0f);
	return rgb;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pScmdline, int nCmdshow)
{
	unsigned int w = 720, h = 480;
	if (fullScreen)
	{
		w = GetSystemMetrics(SM_CXSCREEN);
		h = GetSystemMetrics(SM_CYSCREEN);
	}
	initWindow(w, h);

	auto instance = Vulkan::Initialize(appName, appVersion);
	if (!instance) return Dump();
	uint32_t numGpus = instance->FindGpus();
	if (!numGpus) return Dump();

	auto device = instance->CreateDeviceOnWindow(0, hWnd, hInstance); //Make sure to always declare a device before any children, to make sure they can deconstruct properly
	if (!device) return Dump();

	auto swapChain = device->CreateSwapChain(Vulkan::SwapChainCreateInfo({ w, h }, 2)); //Double-buffered presentation surface
	if (!swapChain) return Dump();

	auto frameBuffer = device->CreateFrameBuffer( {swapChain->GetAttachmentInfo()}, false); //A render target pointing to the swap chain, with no depth buffer
	if (!frameBuffer) return Dump();

	auto cmdDraw = device->CreateCommandBuffer(Vulkan::graphicsIndex);
	if (!cmdDraw) return Dump();

	auto start = std::chrono::steady_clock::now();
	uint32_t frameCount = 0;

	MSG msg = {};
	std::vector<VkClearValue> clearValues(1);
	//Loop until there is a quit message from the window or the user
	while (msg.message != WM_QUIT)
	{
		frameCount++;
		device->Join();
		swapChain->NextBuffer();

		if (!device->PostPresent(swapChain)) return Dump();

		//Fill the draw command buffer
		if (!cmdDraw->Begin(true)) return Dump();
		std::chrono::duration<float> seconds = std::chrono::steady_clock::now() - start;
		float hue = glm::mod(seconds.count(), 20.0f) / 20.0f;
		auto rgb = HueToRGB(hue);


		frameBuffer->SetClearValues({ { rgb.r, rgb.g, rgb.b, 0.0f } });
		cmdDraw->BeginRendering(frameBuffer, false);
		cmdDraw->EndRendering();
		cmdDraw->Blit(frameBuffer, swapChain);
		if (!cmdDraw->End()) return Dump();

		if (!device->ExecuteCommands({ cmdDraw })) return Dump();

		if (!device->PrePresent(swapChain)) return Dump();
		device->Present(swapChain);

		device->Join();

		//Handle the window's messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<float> time = end - start;
	float fps = frameCount / time.count();
	OutputDebugString(("Average FPS: " + std::to_string(fps) + "\n").c_str());

	closeWindow();
	Dump();
	return 0;
}