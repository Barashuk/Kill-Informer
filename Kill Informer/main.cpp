#ifndef main_cpp
#define main_cpp
#include <string>

#include "plugin.h"
#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include "KeyCombo.hpp"

#include <sampapi/CChat.h>
#include <sampapi/CGame.h>
namespace r1 = SAMPAPI_NAMESPACE::v037r1;
using namespace plugin;

static WNDPROC  hOrigProcImGui = nullptr;
static HWND     hMain = NULL;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	KeyHandler::WndHandler(msg, wParam, lParam);
	return CallWindowProc(hOrigProcImGui, hWnd, msg, wParam, lParam);
}

key_ptr_t combo = nullptr;

class CMain {
public:
	static bool open;
	static void OnRelease() {
		KeyHandler::Clear();
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		SetWindowLongA(hMain, GWL_WNDPROC, (LONG)hOrigProcImGui);	
	}
	static void OnInit() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGuiIO& io = ImGui::GetIO();
		io.LogFilename = nullptr;
		io.IniFilename = nullptr;
		io.Fonts->AddFontDefault();
		hMain = RsGlobal.ps->window;
		ImGui_ImplWin32_Init(hMain);
		ImGui_ImplDX9_Init(reinterpret_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice()));
		hOrigProcImGui = (WNDPROC)SetWindowLongA(hMain, GWL_WNDPROC, (LONG)WndProc);
		auto test = []() { 
			r1::RefGame()->SetCursorMode(r1::CURSOR_LOCKCAMANDCONTROL, (open = !open));
		};
		combo = KeyHandler::AddHotKey(VK_F12, test);
	}
	static void OnDraw() {
		static bool init = false;
		if (!init) {
			OnInit();
			init = true;
		}
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		if (open) {
			ImGui::SetNextWindowPos({ 10, 400 }, ImGuiCond_Once);
			ImGui::SetNextWindowSize({ 400, 200 }, ImGuiCond_Once);
			ImGui::Begin("test", &open);
			combo->Draw();
			ImGui::End();
		}
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	static void OnReset() {
		
	}
	static void OnLost() {
		ImGui_ImplDX9_InvalidateDeviceObjects();
	}
	CMain() {
		Events::d3dLostEvent += OnLost;
		Events::d3dResetEvent += OnReset;
		Events::drawingEvent += OnDraw;
	}
	~CMain() {
		Events::d3dLostEvent -= OnLost;
		Events::d3dResetEvent -= OnReset;
		Events::drawingEvent -= OnDraw;
		OnRelease();
	}
} object;

bool CMain::open = false;

#endif

