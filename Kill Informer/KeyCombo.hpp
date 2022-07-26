#ifndef KEYCOMBO_HPP
#define KEYCOMBO_HPP

#include <vector>
#include <memory>
#include <functional>
#include <string>

#include <sampapi/CChat.h>
namespace r1 = SAMPAPI_NAMESPACE::v037r1;


using namespace std;

struct stKeyCombo {
	uint8_t Key;
	bool Ctrl, Alt, Shift;
	std::function <void(void)> _func;
	stKeyCombo(uint8_t key, std::function<void()> func, bool ctrl = false, bool alt = false, bool shift = false) {
		Key = key, _func = func, Ctrl = ctrl, Alt = alt, Shift = shift;
	}
	void Exec() {
		_func();
	}
};

class KeyHandler {
public:
	KeyHandler() = delete;
	KeyHandler(const KeyHandler& key) = delete;
	~KeyHandler() = delete;
	static auto GetCombos() {
		static vector <unique_ptr<stKeyCombo>> combos;
		return &combos;
	}
	static auto AddCombo(uint8_t key, std::function<void()> func) {
		auto &new_combo = make_unique<stKeyCombo>(stKeyCombo(key, func));
		GetCombos()->push_back(std::move(new_combo));
		return std::move(new_combo);
	}
	static void WndHandler(uint32_t msg, WPARAM wParam, LPARAM lParam) {
		if (GetCombos()->empty())
			return;
		if (msg == WM_KEYDOWN) {
			if (lParam >> 30 == 0) {
				auto _find = find_if(GetCombos()->begin(), GetCombos()->end(), [wParam](const unique_ptr < stKeyCombo>& combo) { return combo->Key == wParam; });
				_find->get()->Exec();
				std::string text = to_string(lParam >> 30);
				r1::RefChat()->AddMessage(-1, text.c_str());;
				return;
			}
		}
	}
	static void Clear() {
		GetCombos()->clear();
	}
};





#endif
