#ifndef KEYCOMBO_HPP
#define KEYCOMBO_HPP

#define DISABLE_SAMP_KEYS

#include <vector>
#include <memory>
#include <functional>
#include <string>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc\cpp\imgui_stdlib.h>

#include <nlohmann/json.hpp>
static std::string GetKeyName(uint8_t key) {
	static std::vector <std::pair<uint8_t, std::string>> keys = {
		{ 1 , u8"ЛКМ"}, //Left mouse button
		{ 2 , u8"ПКМ"}, //Right mouse button
		{ 3 , u8"VK_CANCEL"}, //Control-break processing
		{ 4 , u8"МКМ"}, //Middle mouse button (three-button mouse)
		{ 5 , u8"БКМ 1"}, //Windows 2000: X1 mouse button
		{ 6 , u8"БКМ 2"}, //Windows 2000: X2 mouse button
		{ 8 , u8"Backspace"}, //BACKSPACE key
		{ 9 , u8"Tab"}, //TAB key
		{ 12 , u8"Clear"}, //CLEAR key
		{ 13 , u8"Enter"}, //ENTER key
		{ 16 , u8"Shift"}, //SHIFT key
		{ 17 , u8"Ctrl"}, //CTRL key
		{ 18 , u8"Alt"}, //ALT key
		{ 19 , u8"Pause"}, //PAUSE key
		{ 20 , u8"Caps Lock"}, //CAPS LOCK key
		{ 21 , u8"VK_KANA"}, //IME Kana mode
		{ 21 , u8"VK_HANGUEL"}, //IME Hanguel mode (maintained for compatibility; use VK_HANGUL)
		{ 21 , u8"VK_HANGUL"}, //IME Hangul mode
		{ 23 , u8"VK_JUNJA"}, //IME Junja mode
		{ 24 , u8"VK_FINAL"}, //IME final mode
		{ 25 , u8"VK_HANJA"}, //IME Hanja mode
		{ 25 , u8"VK_KANJI"}, //IME Kanji mode
		{ 27 , u8"Esc"}, //ESC key
		{ 28 , u8"VK_CONVERT"}, //IME convert
		{ 29 , u8"VK_NONCONVERT"}, //IME nonconvert
		{ 30 , u8"VK_ACCEPT"}, //IME accept
		{ 31 , u8"VK_MODECHANGE"}, //IME mode change request
		{ 32 , u8"Spacebar"}, //SPACEBAR
		{ 33 , u8"Page Up"}, //PAGE UP key
		{ 34 , u8"Page Down"}, //PAGE DOWN key
		{ 35 , u8"End"}, //END key
		{ 36 , u8"Home"}, //HOME key
		{ 37 , u8"Left arrow"}, //LEFT ARROW key
		{ 38 , u8"Up arrow"}, //UP ARROW key
		{ 39 , u8"Right arrow"}, //RIGHT ARROW key
		{ 40 , u8"Down arrow"}, //DOWN ARROW key
		{ 41 , u8"VK_SELECT"}, //SELECT key
		{ 42 , u8"VK_PRINT"}, //PRINT key
		{ 43 , u8"VK_EXECUTE"}, //EXECUTE key
		{ 44 , u8"Print Screen"}, //PRINT SCREEN key
		{ 45 , u8"Insert"}, //INS key
		{ 46 , u8"Delete"}, //DEL key
		{ 47 , u8"Help"}, //HELP key
		{ 48 , u8"0"}, //0 key
		{ 49 , u8"1"}, //1 key
		{ 50 , u8"2"}, //2 key
		{ 51 , u8"3"}, //3 key
		{ 52 , u8"4"}, //4 key
		{ 53 , u8"5"}, //5 key
		{ 54 , u8"6"}, //6 key
		{ 55 , u8"7"}, //7 key
		{ 56 , u8"8"}, //8 key
		{ 57 , u8"9"}, //9 key
		{ 65 , u8"A(Ф)"}, //A key
		{ 66 , u8"B(И)"}, //B key
		{ 67 , u8"C(C)"}, //C key
		{ 68 , u8"D(В)"}, //D key
		{ 69 , u8"E(У)"}, //E key
		{ 70 , u8"F(А)"}, //F key
		{ 71 , u8"G(П)"}, //G key
		{ 72 , u8"H(Р)"}, //H key
		{ 73 , u8"I(Ш)"}, //I key
		{ 74 , u8"J(О)"}, //J key
		{ 75 , u8"K(Л)"}, //K key
		{ 76 , u8"L(Д)"}, //L key
		{ 77 , u8"M(Ь)"}, //M key
		{ 78 , u8"N(Т)"}, //N key
		{ 79 , u8"O(Щ)"}, //O key
		{ 80 , u8"P(З)"}, //P key
		{ 81 , u8"Q(Й)"}, //Q key
		{ 82 , u8"R(К)"}, //R key
		{ 83 , u8"S(Ы)"}, //S key
		{ 84 , u8"T(Е)"}, //T key
		{ 85 , u8"U(Г)"}, //U key
		{ 86 , u8"V(М)"}, //V key
		{ 87 , u8"W(Ц)"}, //W key
		{ 88 , u8"X(Ч)"}, //X key
		{ 89 , u8"Y(Н)"}, //Y key
		{ 90 , u8"Z(Я)"}, //Z key
		{ 91 , u8"Left Win"}, //Left Windows key (Microsoft® Natural® keyboard)
		{ 92 , u8"Right Win"}, //Right Windows key (Natural keyboard)
		{ 93 , u8"VK_APPS"}, //Applications key (Natural keyboard)
		{ 95 , u8"Sleep"}, //Computer Sleep key
		{ 96 , u8"NumPad 0"}, //Numeric keypad 0 key
		{ 97 , u8"NumPad 1"}, //Numeric keypad 1 key
		{ 98 , u8"NumPad 2"}, //Numeric keypad 2 key
		{ 99 , u8"NumPad 3"}, //Numeric keypad 3 key
		{ 100 , u8"NumPad 4"}, //Numeric keypad 4 key
		{ 101 , u8"NumPad 5"}, //Numeric keypad 5 key
		{ 102 , u8"NumPad 6"}, //Numeric keypad 6 key
		{ 103 , u8"NumPad 7"}, //Numeric keypad 7 key
		{ 104 , u8"NumPad 8"}, //Numeric keypad 8 key
		{ 105 , u8"NumPad 9"}, //Numeric keypad 9 key
		{ 106 , u8"NumPad *"}, //Multiply key
		{ 107 , u8"NumPad +"}, //Add key
		{ 108 , u8"VK_SEPARATOR"}, //Separator key
		{ 109 , u8"NumPad-"}, //Subtract key
		{ 110 , u8"NumPad ."}, //Decimal key
		{ 111 , u8"NumPad /"}, //Divide key
		{ 112 , u8"F1"}, //F1 key
		{ 113 , u8"F2"}, //F2 key
		{ 114 , u8"F3"}, //F3 key
		{ 115 , u8"F4"}, //F4 key
		{ 116 , u8"F5"}, //F5 key
		{ 117 , u8"F6"}, //F6 key
		{ 118 , u8"F7"}, //F7 key
		{ 119 , u8"F8"}, //F8 key
		{ 120 , u8"F9"}, //F9 key
		{ 121 , u8"F10"}, //F10 key
		{ 122 , u8"F11"}, //F11 key
		{ 123 , u8"F12"}, //F12 key
		{ 124 , u8"F13"}, //F13 key
		{ 125 , u8"F14"}, //F14 key
		{ 126 , u8"F15"}, //F15 key
		{ 127 , u8"F16"}, //F16 key
		{ 128 , u8"F17"}, //F17 key
		{ 129 , u8"F18"}, //F18 key
		{ 130 , u8"F19"}, //F19 key
		{ 131 , u8"F20"}, //F20 key
		{ 132 , u8"F21"}, //F21 key
		{ 133 , u8"F22"}, //F22 key
		{ 134 , u8"F23"}, //F23 key
		{ 135 , u8"F24"}, //F24 key
		{ 144 , u8"NumLock"}, //NUM LOCK key
		{ 145 , u8"ScrollLock"}, //SCROLL LOCK key
		{ 160 , u8"Left Shift"}, //Left SHIFT key
		{ 161 , u8"Right Shift"}, //Right SHIFT key
		{ 162 , u8"Left ctrl"}, //Left CONTROL key
		{ 163 , u8"Right ctrl"}, //Right CONTROL key
		{ 164 , u8"Left alt"}, //Left MENU key
		{ 165 , u8"Right alt"}, //Right MENU key
		{ 166 , u8"VK_BROWSER_BACK"}, //Windows 2000: Browser Back key
		{ 167 , u8"VK_BROWSER_FORWARD"}, //Windows 2000: Browser Forward key
		{ 168 , u8"VK_BROWSER_REFRESH"}, //Windows 2000: Browser Refresh key
		{ 169 , u8"VK_BROWSER_STOP"}, //Windows 2000: Browser Stop key
		{ 170 , u8"VK_BROWSER_SEARCH"}, //Windows 2000: Browser Search key
		{ 171 , u8"VK_BROWSER_FAVORITES"}, //Windows 2000: Browser Favorites key
		{ 172 , u8"VK_BROWSER_HOME"}, //Windows 2000: Browser Start and Home key
		{ 173 , u8"VK_VOLUME_MUTE"}, //Windows 2000: Volume Mute key
		{ 174 , u8"VK_VOLUME_DOWN"}, //Windows 2000: Volume Down key
		{ 175 , u8"VK_VOLUME_UP"}, //Windows 2000: Volume Up key
		{ 176 , u8"VK_MEDIA_NEXT_TRACK"}, //Windows 2000: Next Track key
		{ 177 , u8"VK_MEDIA_PREV_TRACK"}, //Windows 2000: Previous Track key
		{ 178 , u8"VK_MEDIA_STOP"}, //Windows 2000: Stop Media key
		{ 179 , u8"VK_MEDIA_PLAY_PAUSE"}, //Windows 2000: Play/Pause Media key
		{ 180 , u8"VK_LAUNCH_MAIL"}, //Windows 2000: Start Mail key
		{ 181 , u8"VK_LAUNCH_MEDIA_SELECT"}, //Windows 2000: Select Media key
		{ 182 , u8"VK_LAUNCH_APP1"}, //Windows 2000: Start Application 1 key
		{ 183 , u8"VK_LAUNCH_APP2"}, //Windows 2000: Start Application 2 key
		{ 186 , u8"VK_OEM_1"}, //Windows 2000: For the US standard keyboard, the ';:' key
		{ 187 , u8"VK_OEM_PLUS"}, //Windows 2000: For any country/region, the '+' key
		{ 188 , u8"VK_OEM_COMMA"}, //Windows 2000: For any country/region, the ',' key
		{ 189 , u8"VK_OEM_MINUS"}, //Windows 2000: For any country/region, the '-' key
		{ 190 , u8"VK_OEM_PERIOD"}, //Windows 2000: For any country/region, the '.' key
		{ 191 , u8"VK_OEM_2"}, //Windows 2000: For the US standard keyboard, the '/?' key
		{ 192 , u8"VK_OEM_3"}, //Windows 2000: For the US standard keyboard, the '`~' key
		{ 219 , u8"VK_OEM_4"}, //Windows 2000: For the US standard keyboard, the '[{' key
		{ 220 , u8"VK_OEM_5"}, //Windows 2000: For the US standard keyboard, the '\|' key
		{ 221 , u8"VK_OEM_6"}, //Windows 2000: For the US standard keyboard, the ']}' key
		{ 222 , u8"VK_OEM_7"}, //Windows 2000: For the US standard keyboard, the 'single-quote/double-quote' key
		{ 223 , u8"VK_OEM_8"}, //
		{ 226 , u8"VK_OEM_102"}, //Windows 2000: Either the angle bracket key or the backslash key on the RT 102-key keyboard
		{ 229 , u8"VK_PROCESSKEY"}, //Windows 95/98, Windows NT 4.0, Windows 2000: IME PROCESS key
		{ 231 , u8"VK_PACKET"}, //Windows 2000: Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP
		{ 246 , u8"VK_ATTN"}, //Attn key
		{ 247 , u8"VK_CRSEL"}, //CrSel key
		{ 248 , u8"VK_EXSEL"}, //ExSel key
		{ 249 , u8"VK_EREOF"}, //Erase EOF key
		{ 250 , u8"VK_PLAY"}, //Play key
		{ 251 , u8"VK_ZOOM"}, //Zoom key
		{ 252 , u8"VK_NONAME"}, //Reserved for future use
		{ 253 , u8"VK_PA1"}, //PA1 key
		{ 254 , u8"VK_OEM_CLEAR"}, //Clear key
	};
	auto it = find_if(keys.begin(), keys.end(), [&](const std::pair <uint8_t, std::string>& elem) { return elem.first == key; });
	return it == keys.end() ? "Unknown" : it->second;
}

void SetText(std::string &str, const std::string &text, const bool &value) {
	if (!value)
		return;
	if (str.empty()) {
		str = text + " + ";
	}
	else {
		str += text + " + ";
	}
}

template <typename mod>
struct stModState {
public:
	mod _active, _old, _restore;
	stModState(mod active) {
		_active = active;
	};
	stModState() { };
	void SaveState() {
		_restore = _active;
	}
	void RestoreState() {
		_active = _restore;
	};
};

struct stHotKey;
using key_ptr_t = std::shared_ptr<stHotKey>;
using func_t = const std::function<void()>;

struct stHotKey : public std::enable_shared_from_this <stHotKey>{
private:
	func_t  _callback;
	std::string inputText;
	ImGuiID ItemID;
	ImGuiWindow* window;
	bool Edit = false;
	void SaveState() {
		_key.SaveState();
		_ctrl.SaveState();
		_alt.SaveState();
		_shift.SaveState();
	}
public:
	stModState<bool> _ctrl, _alt, _shift;
	stModState<uint8_t> _key;
	stHotKey(uint8_t key, func_t& func, bool ctrl = false, bool alt = false, bool shift = false) : _callback(func) {
		_key = key;
		_ctrl = ctrl;
		_alt = alt;
		_shift = shift;
		Prepare();
	};
	void RestoreState() {
		_key.RestoreState();
		_ctrl.RestoreState();
		_alt.RestoreState();
		_shift.RestoreState();
	}
	void Exec() {
		_callback();
	}
	void Draw(std::string label = u8"Нажмите кнопку для смены бинда", float width = 150) {
		std::string text = label + "##" + std::to_string(reinterpret_cast<uint32_t>(this));
		ImGui::PushItemWidth(width);
		ImGui::InputText(text.c_str(), &inputText, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_CallbackAlways);
		ItemID = ImGui::GetID(text.c_str());
		window = ImGui::GetCurrentWindowRead();
		ImGui::PopItemWidth();
		if (!Edit && ImGui::TempInputIsActive(ItemID)) {
			SaveState();
			inputText = u8"Нажмите любую клавишу";
			Edit = true;
		}
	}
	auto Check(uint32_t key, bool ctrl, bool shift, bool alt) {
		return std::tie(_key._active, _ctrl._active, _shift._active, _alt._active) ==
			std::tie(key, ctrl, shift, alt);
	}
	auto SetNewState(uint32_t key, bool ctrl, bool shift, bool alt) {
		_key = key;
		_ctrl = ctrl;
		_shift = shift;
		_alt = alt;
		Edit = false;
		Prepare();
	}
	auto ActiveHotKey() {
		return ItemID;
	}
	auto WindowRender() {
		return window;
	}
	auto GetInput() {
		return &inputText;
	}
	void Prepare() {
		inputText.clear();
		SetText(inputText, "shift", _shift._active);
		SetText(inputText, "ctrl", _ctrl._active);
		SetText(inputText, "alt", _alt._active);
		inputText += GetKeyName(_key._active);
	}
};

static void to_json(nlohmann::json& j, const key_ptr_t& p) {
	j = nlohmann::json{
		{ "Key", p->_key._active }, 
		{ "Ctrl", p->_ctrl._active },
		{ "Shift", p->_shift._active },
		{ "Alt", p->_alt._active },
	};
}

static void from_json(const nlohmann::json& j, key_ptr_t &p) {
	j.at("Key").get_to(p->_key._active);
	j.at("Ctrl").get_to(p->_ctrl._active);
	j.at("Shift").get_to(p->_shift._active);
	j.at("Alt").get_to(p->_alt._active);
	p->Prepare();
}

class KeyHandler {
public:
	KeyHandler() = delete;
	~KeyHandler() = delete;
	KeyHandler(const KeyHandler&) = delete;
	static auto& GetHotkeys(void) {
		static std::vector<key_ptr_t> timers;
		return timers;
	}
	static auto AddHotKey(uint8_t key, func_t &func, bool ctrl = false, bool alt = false, bool shift = false) {
		auto& new_hotkey = std::make_shared<stHotKey>(key, func, ctrl, alt, shift);
		GetHotkeys().push_back(new_hotkey);
		return new_hotkey;
	}
	static bool WndHandler(uint32_t msg, WPARAM wParam, LPARAM lParam) {
		if (GetHotkeys().empty())
			return true;
		bool shift = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT),
			ctrl = GetAsyncKeyState(VK_LCONTROL) || GetAsyncKeyState(VK_RCONTROL),
			alt = GetAsyncKeyState(VK_LMENU) || GetAsyncKeyState(VK_RMENU);
		if (msg == WM_KEYDOWN ) {
			if (ImGui::GetActiveID() != 0) {
				auto _find = find_if(GetHotkeys().begin(), GetHotkeys().end(), [](const key_ptr_t& combo) { return combo->ActiveHotKey() == ImGui::GetActiveID(); });
				if (_find == GetHotkeys().end())
					return true;
				if (wParam == VK_ESCAPE) {
					ImGui::SetActiveID(0, _find->get()->WindowRender());
					_find->get()->RestoreState();
					return false;
				}
				else {
					if (wParam != VK_CONTROL && wParam != VK_MENU && wParam != VK_SHIFT) {
						_find->get()->SetNewState(wParam, ctrl, shift, alt);
						ImGui::SetActiveID(0, _find->get()->WindowRender());
					}
					else {
						auto &str = *_find->get()->GetInput();
						str.clear();
						SetText(str, "ctrl", ctrl);
						SetText(str, "shift", shift);
						SetText(str, "alt", alt);
						str += u8"Нажмите любую клавишу";
					}
				}
			}
			else if ((HIWORD(lParam) & KF_REPEAT) != KF_REPEAT) {
				auto _find = find_if(GetHotkeys().begin(), GetHotkeys().end(), [&](const key_ptr_t& combo) { return combo->Check(wParam, ctrl, shift, alt); });
				if (_find == GetHotkeys().end())
					return true;
				_find->get()->Exec();
			}
		}
		else if(msg == WM_KEYUP){
			if (ImGui::GetActiveID() != 0) {
				auto _find = find_if(GetHotkeys().begin(), GetHotkeys().end(), [](const key_ptr_t& combo) { return combo->ActiveHotKey() == ImGui::GetActiveID(); });
				if (_find == GetHotkeys().end())
					return true;
				auto &str = *_find->get()->GetInput();
				str.clear();
				SetText(str, "ctrl", ctrl);
				SetText(str, "shift", shift);
				SetText(str, "alt", alt);
				str += u8"Нажмите любую клавишу";
			}
		}
		return true;
	}

	static void Clear() {
		GetHotkeys().clear();
	}
	
};

#endif
