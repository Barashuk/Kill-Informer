#ifndef main_cpp
#define main_cpp
#include <string>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "plugin.h"
#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include "KeyCombo.hpp"
#include "Console.hpp"
#include "FontsHandler.hpp"

#include <sampapi/CChat.h>
#include <sampapi/CGame.h>
namespace r1 = SAMPAPI_NAMESPACE::v037r1;
namespace fs = std::filesystem;
using namespace plugin;
using json = nlohmann::json;

static WNDPROC  hOrigProcImGui = nullptr;
static HWND     hMain = NULL;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	if(!KeyHandler::WndHandler(msg, wParam, lParam))
		return CallWindowProc(hOrigProcImGui, hWnd, 0, 0, 0);
	return CallWindowProc(hOrigProcImGui, hWnd, msg, wParam, lParam);
}

key_ptr_t combo = nullptr;
font_ptr_t font = nullptr;

class CMain {
public:
	static bool open;
	static void OnRelease() {
		KeyHandler::Clear();
		Console::Release();
		FontsHandler::Clear();
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
		io.Fonts->Clear();
		char buffer[MAX_PATH];	
		GetWindowsDirectory(buffer, MAX_PATH);
		io.Fonts->AddFontFromFileTTF(fs::path(fs::path(buffer) / "Fonts" / "trebucbd.ttf").string().c_str(), 16, NULL, io.Fonts->GetGlyphRangesCyrillic());
		hMain = RsGlobal.ps->window;
		auto device = reinterpret_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice());
		ImGui_ImplWin32_Init(hMain);
		ImGui_ImplDX9_Init(device);
		hOrigProcImGui = (WNDPROC)SetWindowLongA(hMain, GWL_WNDPROC, (LONG)WndProc);
		auto test = []() { 
			open = !open;
			r1::RefGame()->SetCursorMode(open ? r1::CURSOR_LOCKCAMANDCONTROL : r1::CURSOR_NONE, open);
		};
		combo = KeyHandler::AddHotKey(VK_F12, test);
		Console::Init();
		FontsHandler::Init(device);
		font = FontsHandler::AddFont("Trebuc", 16, FCR_BOLD | FCR_BORDER);
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
			font->Setting();
			ImGui::End();
		}
		Console::Draw();
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		font->Print(10, 400, "testing");
	}
	static void OnReset() {
		FontsHandler::OnReset();
	}
	static void OnLost() {
		ImGui_ImplDX9_InvalidateDeviceObjects();
		FontsHandler::OnLost();
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

