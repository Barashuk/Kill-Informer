#ifndef FONTS_HPP
#define FONTS_HPP
#include <d3drender.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <memory>
using namespace std;

static IDirect3DDevice9* _device = nullptr;

struct stFontInfo;

struct stFontInfo : enable_shared_from_this<stFontInfo>{
private:
	CD3DFont* font = nullptr;
	string _name, _lastName;
	int _height;
	DWORD _flags, _charSet;
	void Create() {
		if (font) {
			font->Invalidate();
			delete font, font = nullptr;
		}
		font = new CD3DFont(_name, _height, _flags, _charSet);
		font->Initialize(_device);
	}
public:
	stFontInfo(string fontName, int fontHeight = 14, DWORD dwCreateFlags = FCR_BORDER, DWORD dwCharSet = DEFAULT_CHARSET) {
		_lastName = _name = fontName, _height = fontHeight, _flags = dwCreateFlags;
		_charSet = dwCharSet;
		Create();
	}
	void Initialize(IDirect3DDevice9 *device) { font->Initialize(device); }
	void Invalidate() { font->Invalidate(); }
	HRESULT Print(float x, float y, std::string_view text, SRColor color = -1, bool skipColorTags = false, bool noColorFormat = false) {
		return font->Print(x, y, text, color, skipColorTags, noColorFormat);
	}
	void Setting() {
		string uniqueID = std::to_string(reinterpret_cast<uint32_t>(this));
		string label = u8"Название##font_name_" + uniqueID;
		auto id = ImGui::GetID(label.c_str());
		ImGui::BeginChild("her her", { 200, 100 }, true);
		ImGui::PushItemWidth(100);
		if (ImGui::InputText(label.c_str(), &_name)) {

		}
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(100);
		label = u8"Высота##font_height_" + uniqueID;
		if (ImGui::DragInt(label.c_str(), &_height, 1, 8, 70)) {
			Create();
		}
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("her her hero4", { 200, 200 }, true);
		static vector <pair<string, DWORD> >  flagsRender = {
			{u8"Жирность##font_bold_", FCR_BOLD },
			{u8"Курсив##font_italics_", FCR_ITALICS },
			{u8"Обводка##font_border_", FCR_BORDER },
		};
		bool flag;
		for (auto &elem : flagsRender ) {
			flag = (_flags & elem.second) == elem.second;
			label = elem.first + uniqueID;
			if (ImGui::Checkbox(label.c_str(), &flag)) {
				_flags ^= elem.second;
				Create();
			}
		}
		ImGui::EndChild();
	}
/*
	HRESULT PrintU8(float x, float y, std::string_view text, SRColor color = -1, bool skipColorTags = false, bool noColorFormat = false);
	HRESULT PrintShadow(float x, float y, std::string_view text, DWORD color = -1, DWORD color_shadow = 0xFF000000);
	HRESULT PrintShadowU8(float x, float y, std::string_view text, DWORD color = -1, DWORD color_shadow = 0xFF000000);

	float DrawLength(std::string_view text, bool noColorFormat = false) const;
	float DrawLengthU8(std::string_view text, bool noColorFormat = false) const;
	size_t GetCharPos(std::string_view text, float x, bool noColorFormat = false) const;
	size_t GetCharPosU8(std::string_view text, float x, bool noColorFormat = false) const;

	float DrawHeight() const { return font->DrawHeight(); };

	size_t BindColorGrad(const stColorGrad& grad);*/
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
	static auto Init(IDirect3DDevice9* device) {
		_device = device;
	}

	static auto& GetFonts(void) {
		static std::vector<font_ptr_t> timers;
		return timers;
	}
	static auto AddFont(string fontName, int fontHeight = 14, DWORD dwCreateFlags = FCR_BORDER, DWORD dwCharSet = DEFAULT_CHARSET) {
		auto &font = make_shared<stFontInfo>(fontName, fontHeight, dwCreateFlags, dwCharSet);
		GetFonts().push_back(font);
		return font;
	}
	static void Clear() {
		auto &fonts = GetFonts();
		if (fonts.empty())
			return;
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
			font->Initialize(_device);
		}
	}

};

#endif
