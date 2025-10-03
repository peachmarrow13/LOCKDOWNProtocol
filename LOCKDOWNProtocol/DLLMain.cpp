#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_dx11.h"
#include "ImGui/backends/imgui_impl_win32.h"

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include <cstdio>

#include "Cheats.h"
#include "SDK/PC_classes.hpp"
#include "Utils.h"
#include "SDK/Mec_classes.hpp"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

static void HookSwapChain();
static void Cleanup(HMODULE hModule);
static void InitImGui(HWND hwnd);

static bool ShowMenu = true;
bool Debug = false;
static bool Cleaning = false;
bool init = false;

typedef HRESULT(__stdcall* tPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
tPresent oPresent = nullptr;
IDXGISwapChain* pSwapChain = nullptr;
ID3D11Device* pDevice = nullptr;
ID3D11DeviceContext* pContext = nullptr;
ID3D11RenderTargetView* pRenderTargetView = nullptr;
DXGI_SWAP_CHAIN_DESC sd = {};

static void AddDefaultTooltip(const char* Text)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(Text);
		ImGui::EndTooltip();
	}
}

void InitImGui(HWND hwnd)
{
	if (!pDevice || !pContext) {
		std::cout << "[ERROR] Device or Context is null...\n";
		Cleaning = true;
		Sleep(30);
		return;
	}

	if (!hwnd) {
		std::cout << "[ERROR] HWND is null...\n";
		Cleaning = true;
		Sleep(30);
		return;
	}

	if (Cleaning) return;

	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Platform/Renderer backends
	if (!ImGui_ImplWin32_Init(hwnd)) {
		std::cout << "[ERROR] ImGui_ImplWin32_Init failed\n";
		return;
	}
	if (!ImGui_ImplDX11_Init(pDevice, pContext)) {
		std::cout << "[ERROR] ImGui_ImplDX11_Init failed\n";
		ImGui_ImplWin32_Shutdown();
		return;
	}

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Controller Controls
	io.Fonts->AddFontDefault();
	io.MouseDrawCursor = true;  // Let ImGui draw the cursor

	ImGui::StyleColorsDark();

	if (pSwapChain) { // Create render target if we have a valid swapchain
		ID3D11Texture2D* pBackBuffer = nullptr;
		HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		if (SUCCEEDED(hr)) {
			hr = pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
			if (SUCCEEDED(hr)) {
				std::cout << "[InitImGui] Render target view created successfully\n";
			}
			else {
				std::cout << "[ERROR] Failed to create render target view: " << std::hex << hr << std::endl;
			}
			pBackBuffer->Release();
		}
		else {
			std::cout << "[ERROR] Failed to get back buffer: " << std::hex << hr << std::endl;
		}
	}

	std::cout << "[InitImGui] ImGui initialized successfully\n";
}

// Add WndProc hook for input handling
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC oWndProc = nullptr;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	if (ImGui::GetCurrentContext()) {
		// Let ImGui handle input when menu is shown
		if (ShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
			return true;
		}
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags)
{
	if (Cleaning) {
		return oPresent(SwapChain, SyncInterval, Flags);
	}

	GVars.AutoSetVariables();

	if (!init)
	{
		if (SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			HWND hwnd = FindWindow(L"UnrealWindow", nullptr);
			pDevice->GetImmediateContext(&pContext);

			pSwapChain = SwapChain;
			SwapChain->GetDesc(&sd);

			if (!hwnd) hwnd = GetForegroundWindow();

			InitImGui(hwnd);

			// Hook WndProc for input handling
			if (hwnd) {
				oWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			}

			init = true;
		}
	}

	if (!oPresent)
		return 0;

	if (!ImGui::GetCurrentContext())
		return oPresent(SwapChain, SyncInterval, Flags);

	// Start the ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ShowMenu) {
		ImGui::Begin("Free LOCKDOWN Protocol Cheat by PeachMarrow12", nullptr, ImGuiWindowFlags_NoCollapse);

		ImGui::SeparatorText("Hello, Have Fun Cheating!");

		ImGui::Checkbox("Debug", &Debug);
		AddDefaultTooltip("Enables debug mode with extra buttons.");

		if (Debug) {
			if (ImGui::Button("Print All Actors"))
			{
				Utils::PrintActors(nullptr);
			}

			if (ImGui::Button("Jump"))
			{
				if (GVars.MecChar)
					GVars.MecChar->Jump();
			}
			AddDefaultTooltip("This is just to make sure the MecChar variable isn't null. :)");
			if (ImGui::Button("Print Stamina"))
			{
				std::cout << "Stamina: " << GVars.MecChar->Stamina << "\n";
			}
		}

		ImGui::Checkbox("GodMode", &CVars.GodMode);

		ImGui::Checkbox("Infinite Stamina", &CVars.Stamina);

		ImGui::End();
	}

	if (pRenderTargetView) {
		pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

		D3D11_VIEWPORT vp = {};
		vp.Width = (float)sd.BufferDesc.Width;
		vp.Height = (float)sd.BufferDesc.Height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		pContext->RSSetViewports(1, &vp);
	}

	Cheats::UpdateGodMode();
	Cheats::UpdateInfiniteStamina();

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent ? oPresent(SwapChain, SyncInterval, Flags) : S_OK;
}

// Attach hook
void HookPresent()
{
	if (!pSwapChain) {
		std::cout << "[ERROR] SwapChain is null...\n";
		Cleaning = true;
		Sleep(30);
		return;
	}

	void** vTable = *reinterpret_cast<void***>(pSwapChain);
	oPresent = (tPresent)vTable[8];

	DWORD oldProtect;
	VirtualProtect(&vTable[8], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
	vTable[8] = (void*)&hkPresent;
	VirtualProtect(&vTable[8], sizeof(void*), oldProtect, &oldProtect);
}

DWORD MainThread(HMODULE hModule)
{
	AllocConsole();
	FILE* Dummy;
	freopen_s(&Dummy, "CONOUT$", "w", stdout);
	freopen_s(&Dummy, "CONIN$", "r", stdin);

	std::cout << "Cheat Injecting...\n";

	HookSwapChain(); // Create a dummy device and swapchain to get the vtable
	HookPresent(); // Hook the Present function

	std::cout << "Cheat Injected\n";

	while (!Cleaning)
	{
		if (GetAsyncKeyState(VK_END) & 1) // Exit with END key
		{
			std::cout << "Exiting...\n";
			Cleaning = true;
			break;
		}

		if (GetAsyncKeyState(VK_INSERT) & 1) // Toggle Cheat Menu with INSERT key
		{
			ShowMenu = !ShowMenu;
			std::cout << "Menu: " << (ShowMenu ? "ON" : "OFF") << "\n";
			ImGui::GetIO().MouseDrawCursor = ShowMenu;
			ShowCursor(false);
		}

		Sleep(100);
	}

	Cleanup(hModule);
	return 0;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		Cleaning = true;
		break;
	}

	return TRUE;
}

void HookSwapChain()
{
	ZeroMemory(&sd, sizeof(DXGI_SWAP_CHAIN_DESC));
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = FindWindow(L"UnrealWindow", nullptr); //! Fix: Add Window Name to prevent having two windows with same class causing issues
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL FeatureLevel;
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;

	while (!pSwapChain) {
		if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
			&FeatureLevelsRequested, 1, D3D11_SDK_VERSION,
			&sd, &pSwapChain, &pDevice, &FeatureLevel, &pContext)))
		{
			void** vTable = *reinterpret_cast<void***>(pSwapChain);
			// Present = vTable[8]
			// ResizeBuffers = vTable[13]
		}
		else
		{
			break;
		}
	}
}

void Cleanup(HMODULE hModule)
{
	Cleaning = true;
	std::cout << "Cleaning up...\n";

	Sleep(300); // Wait a bit to ensure no threads are using resources

	if (ImGui::GetCurrentContext())
	{
		ImGui::GetIO().MouseDrawCursor = false;
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	HWND hwnd = FindWindow(L"UnrealWindow", nullptr);
	if (hwnd && oWndProc)
	{
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
		oWndProc = nullptr;
	}

	if (pSwapChain)
	{
		void** vTable = *reinterpret_cast<void***>(pSwapChain);
		if (vTable && oPresent)
		{
			DWORD oldProtect;
			if (VirtualProtect(&vTable[8], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect))
			{
				vTable[8] = (void*)oPresent;
				VirtualProtect(&vTable[8], sizeof(void*), oldProtect, &oldProtect);
			}
		}
	}

	// Clean up DirectX resources
	if (pRenderTargetView) {
		pRenderTargetView->Release();
		pRenderTargetView = nullptr;
	}
	if (pContext) {
		pContext->Release();
		pContext = nullptr;
	}
	if (pDevice) {
		pDevice->Release();
		pDevice = nullptr;
	}
	if (pSwapChain) {
		pSwapChain->Release();
		pSwapChain = nullptr;
	}
	if (oPresent) {
		oPresent = nullptr;
	}

	std::cout << "Cleanup complete. Unloading DLL...\n";

	// Clean up console
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
}