#include "main.h"
#include <RakHook\rakhook.hpp>
#include <RakNet\BitStream.h>
#include <bass.h>
#include <CustomFont.cpp>
#include "game_sa\eWeaponType.h"
#include "game_sa\ePedPieceTypes.h"
#include "game_sa\eEntityType.h"
#include "game_sa\CWeapon.h"
#include <sampapi\CInput.h>
#include <sampapi\CChat.h>



namespace fs = std::filesystem;
using namespace plugin;

static WNDPROC  hOrigProc = nullptr;
static HWND     hMain = NULL;
static IDirect3DDevice9* device = nullptr;
std::unique_ptr <CKillState> killstate;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return CallWindowProc(hOrigProc, hWnd, 0, 0, 0);
	}
	if (!killstate->WndProc(hWnd, msg, wParam, lParam)) {
		return false;
	}
	if(!KeyHandler::WndHandler(msg, wParam, lParam))
		return CallWindowProc(hOrigProc, 0, 0, 0, 0);
	
	return CallWindowProc(hOrigProc, hWnd, msg, wParam, lParam);
}

CdeclEvent < AddressList<0x5659D1, H_CALL, 0x567B5B, H_CALL, 0x60508B, H_CALL, 0x607FBE, H_CALL, 0x608217, H_CALL, 
	0x60A943, H_CALL, 0x61CD40, H_CALL, 0x736306, H_CALL, 0x73A0C5, H_CALL, 0x73B074, H_CALL,
	0x73BA50, H_CALL>, PRIORITY_AFTER, ArgPick6N <CPed*, 0, CEntity*, 1, eWeaponType, 2, int, 3, ePedPieceTypes, 4, int, 5 >,
	void(CPed*, CEntity*, eWeaponType, int, ePedPieceTypes, int )>  generateDamageEvent;

class CMain {
public:
	static void DamageEvent(CPed* victim, CEntity* creator, eWeaponType weapon, int damageFactor, ePedPieceTypes pedPiece, int direction) {
		killstate->DamageEvent(victim, creator, weapon, damageFactor, pedPiece, direction);
	}
	static void OnRelease() {
		killstate->Release();
		rakhook::destroy();
		KeyHandler::Clear();
		Console::Release();
		//FontsHandler::Clear();
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		SetWindowLongA(hMain, GWL_WNDPROC, (LONG)hOrigProc);
	}
	static void OnInit() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGuiIO& io = ImGui::GetIO();
		io.LogFilename = nullptr;
		io.IniFilename = nullptr;
		io.Fonts->Clear();
		io.MouseDrawCursor = false;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		char buffer[MAX_PATH];	
		GetWindowsDirectory(buffer, MAX_PATH);
		io.Fonts->AddFontFromFileTTF(fs::path(fs::path(buffer) / "Fonts" / "trebucbd.ttf").string().c_str(), 16, NULL, io.Fonts->GetGlyphRangesCyrillic());
		static const ImWchar icons_ranges[] = { ICON_MIN_IGFD, ICON_MAX_IGFD, 0 };
		ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
		io.Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, 15.0f, &icons_config, icons_ranges);
		Console::Init();		
		rakhook::initialize();
		killstate = std::make_unique<CKillState>();
		killstate->Init();
		hMain = RsGlobal.ps->window;
		device = reinterpret_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice());
		ImGui_ImplWin32_Init(hMain);
		ImGui_ImplDX9_Init(device);
		hOrigProc = (WNDPROC)SetWindowLongA(hMain, GWL_WNDPROC, (LONG)WndProc);
		BASS_Init(-1, 44100, 0, hMain, NULL);
	}
	static void OnDraw() {
		static bool init = false;
		if (!init) {
			OnInit();
			init = true;
			
		}
		IDirect3DStateBlock9* d3d9_state_block = NULL;
		if (device->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
			return;
		d3d9_state_block->Capture();
		killstate->Draw();
		d3d9_state_block->Apply();
		d3d9_state_block->Release();
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		Console::Draw();
		killstate->DrawMenu();
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		killstate->Process();
	}
	static void OnReset() {
		FontsHandler::OnReset();
	}
	static void OnLost() {
		FontsHandler::OnLost();
		ImGui_ImplDX9_InvalidateDeviceObjects();
				
	}
	CMain() {
		Events::d3dLostEvent += OnLost;
		Events::d3dResetEvent += OnReset;
		Events::drawingEvent += OnDraw;
		generateDamageEvent += DamageEvent;
	}
	~CMain() {
		Events::d3dLostEvent -= OnLost;
		Events::d3dResetEvent -= OnReset;
		Events::drawingEvent -= OnDraw;
		generateDamageEvent -= DamageEvent;
		OnRelease();
	}
} object;

