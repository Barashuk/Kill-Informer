#pragma once
#ifndef FONTS_HPP
#define FONTS_HPP
#define IMGUI_DEFINE_MATH_OPERATORS
#pragma warning(push)
#pragma warning(disable : 4003)
#pragma warning(disable : 4099)

#include <d3drender.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "RenderWare.h"
#include "json_helper.hpp"
#include "Console.hpp"

using namespace std;

D3DCOLOR static toARGB(ImVec4 color) {
	ImU32 _color = ImGui::ColorConvertFloat4ToU32(color);
	ImU32 a = (_color >> 24) & 0xFF;
	ImU32 b = (_color >> 16) & 0xFF;
	ImU32 g = (_color >> 8) & 0xFF;
	ImU32 r = _color & 255;
	return D3DCOLOR_ARGB(a, r, g, b);
}

enum TextAling  {
	LeftAlign = 0,
	MiddleAlign,
	RightAlign,
};

struct stFontInfo;

struct stFontInfo : enable_shared_from_this<stFontInfo>{
private:
	CD3DFont* font = nullptr;
	IDirect3DDevice9* _device = nullptr;
	string _name, _lastName, _inputCombo;
	int _height;
	DWORD _flags, _charSet;
	uint8_t _align;
#ifdef IMGUI_VERSION_NUM
	ImVec2 _pos;
	ImVec4 _color;
#endif
	
	
public:
	stFontInfo(string fontName, int fontHeight = 14, DWORD dwCreateFlags = FCR_BORDER, DWORD dwCharSet = DEFAULT_CHARSET) {
		_lastName = _name = fontName, _height = fontHeight, _flags = dwCreateFlags;
		_charSet = dwCharSet;
		_align = LeftAlign;
		_device = reinterpret_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice());
#ifdef IMGUI_VERSION_NUM 
		_pos = { 10, 400 };
		_color = { 1, 1, 1, 1 };
#endif
		Create();
	}
	void Create() {
		if (font) {
			font->Invalidate();
			delete font, font = nullptr;
		}
		font = new CD3DFont(_name, _height, _flags, _charSet);
		font->Initialize(_device);
	};
	auto& GetName() { return _name; };
	auto& GetHeight() { return _height; };
	auto& GetFlags() { return _flags; };
	auto& GetCharSet() { return _charSet; };
	auto& GetAlign() { return _align; };

	void Initialize() { font->Initialize(_device); }
	void Invalidate() { font->Invalidate(); }
	auto GetDrawHeight() { return font->DrawHeight(); }
	auto GetDrawLength(string text) { return font->DrawLength(text); }

	HRESULT thisPrint(float x, float y, string text, SRColor color = -1, bool skipColorTags = false, bool noColorFormat = false) {
		float width = font->DrawLength(text, false), _x;
		switch (_align) {
		case MiddleAlign: _x = x - width / 2; break;
		case RightAlign: _x = x - width; break;
		case LeftAlign: _x = x; break;
		default: break;
		}
		
		return font->Print(_x, y, text, color, skipColorTags, noColorFormat);
	}
#ifdef IMGUI_VERSION_NUM 
	HRESULT Print(string text, SRColor color) {
		return this->thisPrint(_pos.x, _pos.y, text, color);
	};

	HRESULT Print(string text, ImVec2 pos) {
		return this->thisPrint(pos.x, pos.y, text, toARGB(_color));
	};

	HRESULT Print(string text, ImVec4 color) {
		return this->thisPrint(_pos.x, _pos.y, text, toARGB(color));
	};
	HRESULT Print(string text) {
		auto res = this->thisPrint(_pos.x, _pos.y, text, toARGB(_color));;
		return res;
	};
	void Setting(bool draw_pos = true, bool draw_aling = true, bool add = false) {
		static vector <pair<string, DWORD> >  flagsRender = {
			{u8"Жирность##font_bold_", FCR_BOLD },
			{u8"Курсив##font_italics_", FCR_ITALICS },
			{u8"Обводка##font_border_", FCR_BORDER },
		};
		static vector <string> values = {
			{u8"По левому краю",},
			{u8"По центру"},
			{u8"По правому краю"}
		};
		auto DragFloat = [](const string& label, float& value, const float& min, const float& max, const float width = 70) {
			ImGui::PushItemWidth(width);
			if (ImGui::DragFloat(label.c_str(), &value, 1, min, max, "%.f")) {
				if (value < min) {
					value = min;
				}
				else if (value > max) {
					value = max;
				}
			}
			ImGui::PopItemWidth();
		};
		string uniqueID = std::to_string(reinterpret_cast<uint32_t>(this));
		string label = "##font_child_one" + uniqueID;
		ImGui::BeginGroup();
		ImGui::PushItemWidth(150);
		label = u8"Название##font_name_" + uniqueID;
		if (ImGui::InputText(label.c_str(), &_name, ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiInputTextCallbackData* data) {
				auto& c = data->EventChar;
				if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ')) 
					return 1; 
				return 0;
			})) {
			if (!_name.empty()) {
				_lastName = _name;
				Create();
			}
		}
		if (_name.empty() && ImGui::GetActiveID() == 0) {
			_name = _lastName;
			Create();
		}
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(150);
		label = u8"Высота##font_height_" + uniqueID;
		if (ImGui::DragInt(label.c_str(), &_height, 1, 8, 70)) {
			Create();
		}
		ImGui::PopItemWidth();
		
		bool flag;
		for (auto& elem : flagsRender) {
			flag = (_flags & elem.second) == elem.second;
			label = elem.first + uniqueID;
			if (ImGui::Checkbox(label.c_str(), &flag)) {
				_flags ^= elem.second;
				Create();
			}
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		label = "##font_child_two" + uniqueID;
		ImGui::BeginGroup();
		label = u8"Цвет текста##color_text_" + uniqueID;
		ImGui::PushItemWidth(200);
		ImGui::ColorEdit4(label.c_str(), (float*)&_color,
			ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoDragDrop |
			ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker
		);
		ImGui::PopItemWidth();
		if (draw_pos) {
			auto size = ImGui::GetMainViewport()->Size;
			label = u8"X##color_text_x_" + uniqueID;
			DragFloat(label, _pos.x, 0, size.x);
			ImGui::SameLine();
			label = u8"Y##color_text_y_" + uniqueID;
			DragFloat(label, _pos.y, 0, size.y);
			ImGui::SameLine();
			ImGui::Text(u8"Позиция");
		}
		if (draw_aling) {
			label = u8"Выравнивание##text_align" + uniqueID;
			if (_inputCombo.empty()) {
				_inputCombo = values.at(_align);
			}
			ImGui::PushItemWidth(200);
			if (ImGui::BeginCombo(label.data(), _inputCombo.data())) {
				for (size_t i = 0; i < values.size(); i++) {
					auto& text = values.at(i);
					if (ImGui::Selectable(text.data(), text.data() == _inputCombo)) {
						_inputCombo = text;
						_align = static_cast<uint8_t>(i);
					}
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
		}
		if(!add)
			ImGui::EndGroup();
	};
	auto& GetPos() { return _pos; };
	auto& GetColor() { return _color; };

#endif
	~stFontInfo() {
		font->Invalidate();
		delete font, font = nullptr;
	}
};

using font_ptr_t = std::shared_ptr<stFontInfo>;

class FontsHandler {
public:
	FontsHandler() = delete;
	FontsHandler(const FontsHandler&) = delete;
	~FontsHandler() = delete;
	static auto& GetFonts(void) {
		static std::list<font_ptr_t> fonts;
		return fonts;
	}
	static auto AddFont(string fontName, int fontHeight = 14, DWORD dwCreateFlags = FCR_BORDER, DWORD dwCharSet = DEFAULT_CHARSET) {
		auto &font = make_shared<stFontInfo>(fontName, fontHeight, dwCreateFlags, dwCharSet);
		GetFonts().push_back(font);
		return GetFonts().back();
	}
	static auto CopyFont(font_ptr_t font) {
		auto new_font = FontsHandler::AddFont(font->GetName(), font->GetHeight(), font->GetFlags(), font->GetCharSet());
		new_font->GetPos() = font->GetPos();
		new_font->GetColor() = font->GetColor();
		new_font->GetAlign() = font->GetAlign();
		return new_font;
	}

	static auto DeleteFont(font_ptr_t font) {
		font->Invalidate();
		auto it = remove_if(GetFonts().begin(), GetFonts().end(), [&](const font_ptr_t& elem) {
			return elem == font;
			});
		GetFonts().erase(it, GetFonts().end());

	}

	static void Clear() {
		auto& fonts = GetFonts();
		if (fonts.empty())
			return;
		for (auto& font : fonts) {
			font->Invalidate();
		}
		fonts.clear();
	}
	static void OnLost() {
		auto& fonts = GetFonts();
		if (fonts.empty())
			return;
		for (auto& font : fonts) {
			font->Invalidate();
		}
	}
	static void OnReset() {
		auto& fonts = GetFonts();
		if (fonts.empty())
			return;
		for (auto& font : fonts) {
			font->Initialize();
		}
	}
};

/*friend font_ptr_t*/

static void to_json(nlohmann::json& j, const font_ptr_t& p) {
	j = nlohmann::json {
		{ "Name", p->GetName() },
		{ "Height", p->GetHeight() },
		{ "Flags", p->GetFlags() },
		{ "Charset", p->GetCharSet() },
		{ "Align", p->GetAlign() },
#ifdef IMGUI_VERSION_NUM 
		{ "Pos", p->GetPos() },
		{ "Color", p->GetColor() }
#endif
	};
}

static void from_json(const nlohmann::json& j, font_ptr_t& p) {
	j.at("Name").get_to(p->GetName());
	j.at("Height").get_to(p->GetHeight());
	j.at("Flags").get_to(p->GetFlags());
	j.at("Charset").get_to(p->GetCharSet());
	j.at("Align").get_to(p->GetAlign());
#ifdef IMGUI_VERSION_NUM 
	j.at("Pos").get_to(p->GetPos());
	j.at("Color").get_to(p->GetColor());
#endif
	p->Create();
}

#pragma warning(pop)
#endif
