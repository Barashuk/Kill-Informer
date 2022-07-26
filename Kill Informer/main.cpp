#include "plugin.h"
#include "imgui.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"
using namespace plugin;

static bool open = true;
void test() {
	ImGui::SetNextWindowSize({ 100, 100 });
	ImGui::SetNextWindowPos({ 100, 100 });
	ImGui::Begin("test", &open);
	ImGui::Text("fasfd");
	ImGui::End();
}

static WNDPROC  hOrigProcImGui = nullptr;
static HWND     hMain = NULL;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	return CallWindowProc(hOrigProcImGui, hWnd, msg, wParam, lParam);
}


class CMain {
public:
	static void OnRelease() {
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		SetWindowLongA((HWND)hMain, GWL_WNDPROC, (LONG)hOrigProcImGui);
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
		hOrigProcImGui = (WNDPROC)SetWindowLongA((HWND)hMain, GWL_WNDPROC, (LONG)WndProc);
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
		test();
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	static void OnReset() {
		ImGui_ImplDX9_InvalidateDeviceObjects();
	}
	static void OnLost() {

	}
	CMain() {
		Events::d3dLostEvent += OnLost;
		Events::d3dResetEvent += OnReset;
		Events::drawingEvent += OnDraw;
	}
	~CMain() {
		OnRelease();
	}
} killInformer;
