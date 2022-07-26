#ifndef KEYCOMBO_HPP
#define KEYCOMBO_HPP

#include <vector>
#include <memory>
#include <functional>
#include <string>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc\cpp\imgui_stdlib.h>

using namespace std;

struct stHotKey;

using key_ptr_t = std::shared_ptr<stHotKey>;
using func_t = const std::function<void()>;
#define mb(text) MessageBox(NULL, text, "debug", 0);

struct stHotKey : public std::enable_shared_from_this <stHotKey>{
private:
	func_t  _callback;
	bool Ctrl, Alt, Shift;
	uint8_t Key;
	string inputText;
public:
	
public:
	stHotKey(uint8_t key, func_t& func, bool ctrl = false, bool alt = false) : Key(key), _callback(func), Ctrl(ctrl), Alt(alt) {
		inputText = to_string(Key);
	};
	void Exec() {
		_callback();
	}
	void Draw() {
		string text = to_string((uint32_t)this);
		ImGui::PushItemWidth(100);
		ImGui::InputText(text.c_str(), &inputText, ImGuiInputTextFlags_ReadOnly);
		auto g = ImGui::GetCurrentContext();
		text = to_string(g->ActiveId);
		ImGui::Text(text.c_str());
		ImGui::PopItemWidth();
		
		
	}
	bool Check(uint32_t key) {
		return  std::tie(Key) == std::tie(key);
	}
};


class KeyHandler {
public:
	KeyHandler() = delete;
	~KeyHandler() = delete;
	KeyHandler(const KeyHandler&) = delete;
	static auto& get_timers(void) {
		static std::vector<key_ptr_t> timers;
		return timers;
	}
	static auto AddHotKey(uint8_t key, func_t &func) {
		auto& new_hotkey = std::make_shared<stHotKey>(key, func);
		get_timers().push_back(new_hotkey);
		return new_hotkey;
	}
	static void WndHandler(uint32_t msg, WPARAM wParam, LPARAM lParam) {
		if (get_timers().empty())
			return;
		if (msg == WM_KEYDOWN) {
			if ((HIWORD(lParam) & KF_REPEAT) != KF_REPEAT) {
				auto _find = find_if(get_timers().begin(), get_timers().end(), [wParam](const key_ptr_t& combo) { return combo->Check(wParam); });
				if (_find != get_timers().end())
					_find->get()->Exec();
				return;
			}
		}
	}
	static void Clear() {
		get_timers().clear();
	}
	static string GetKeyNameText(uint8_t key) {

	}
};

#endif
