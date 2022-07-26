#ifndef KEYCOMBO_HPP
#define KEYCOMBO_HPP

#include <vector>
#include <memory>
#include <functional>
#include <string>

#include <imgui.h>
#include <misc\cpp\imgui_stdlib.h>

using namespace std;

struct stKeyCombo;

using key_ptr_t = std::shared_ptr<stKeyCombo>;
using func_t = const std::function<void()>;

struct stKeyCombo : public std::enable_shared_from_this <stKeyCombo>{
private:
	func_t  _callback;
	bool Ctrl, Alt, Shift;
	uint8_t Key;
	string inputText;
public:
	
public:
	stKeyCombo(uint8_t key, func_t& func, bool ctrl = false, bool alt = false) :
		Key(key), _callback(func), Ctrl(ctrl), Alt(alt) 
	{
		inputText = to_string(Key);
	};
	void Exec() {
		_callback();
	}
	void Draw() {
		ImGui::InputText("press key to change", &inputText,
			ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_None | ImGuiInputTextFlags_NoHorizontalScroll);
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
	static auto AddCombo(uint8_t key, func_t &func) {
		auto& new_timer = std::make_shared<stKeyCombo>(key, func);
		get_timers().push_back(new_timer);
		return new_timer;
	}
	static void WndHandler(uint32_t msg, WPARAM wParam, LPARAM lParam) {
		if (get_timers().empty())
			return;
		if (msg == WM_KEYDOWN) {
			if ((HIWORD(lParam) & KF_REPEAT) == KF_REPEAT) {
				auto _find = find_if(get_timers().begin(), get_timers().end(), [wParam](const key_ptr_t& combo) { return combo->Check(wParam); });
				if (_find == get_timers().end())
					return;
				_find->get()->Exec();
				return;
			}
		}
	}
	static void Clear() {
		get_timers().clear();
	}
};

#endif
