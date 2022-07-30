#ifndef LOG_HPP
#define LOG_HPP
//#define TEST_CONSOLE
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>

#include <imgui.h>
#include <nlohmann/json.hpp>

#include "KeyCombo.hpp"
#include <sampapi/CGame.h>
namespace r1 = SAMPAPI_NAMESPACE::v037r1;

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace std;

#define JSON_FILENAME "console.json"

enum OverlayPos {
	TopLeft = 0,
	TopRight,
	BottomLeft,
	BottomRight,
	Custom
};

enum StateDraw {
	None = 0,
	Overlay,
	Console,
};
enum ErrorsType {
	Info = 0,
	Warning,
	Success,
	Error,
};

static std::string cp1251_to_utf8(std::string str) {
	int result_w = MultiByteToWideChar(1251, 0, str.data(),
		static_cast<int>(str.size()), NULL, 0);
	if (result_w == 0)
		return "";

	std::wstring wres(result_w, '\0');
	if (!MultiByteToWideChar(1251, 0, str.data(), static_cast<int>(str.size()),
		wres.data(), result_w))
		return "";

	int result_c =
		WideCharToMultiByte(CP_UTF8, 0, wres.data(),
			static_cast<int>(wres.size()), NULL, 0, NULL, NULL);
	if (result_c == 0)
		return "";

	std::string res(result_c, '\0');
	if (!WideCharToMultiByte(CP_UTF8, 0, wres.data(),
		static_cast<int>(wres.size()), res.data(), result_c,
		0, 0))
		return "";

	return res;
}

struct stLine {
	uint8_t _type;
	string _time, _text;
	stLine(uint8_t type, string time, string text) {
		_type = type, _time = time, _text = text;
	};
};

using line_t = stLine;

class Console{
public:
	static uint8_t posOverlay, stateRender;
	static bool AutoScroll, openConsole, showTime;
	static key_ptr_t Activation;
public:
	Console() = delete;
	Console(const Console&) = delete;
	~Console() = delete;

	static auto &GetLines(void) {
		static vector <line_t> Lines;
		return Lines;
	}

	static void AddToFile(string str) {
		auto path = fs::current_path() / "console.log";
		fstream file(path.string().c_str(), ios::app | ios::binary);
		file << str << endl;
		file.close();
	}

	static void Add(string text, ErrorsType type = ErrorsType::Info) {
		text = cp1251_to_utf8(text);
		GetLines().push_back(line_t(type, "[ " + GetTimestamp() + " ]" , text));;
		string _type;
		switch (type) {
		case ErrorsType::Info: _type = "[ Info ]"; break;
		case ErrorsType::Warning: _type = "[ Warning ]"; break;
		case ErrorsType::Success: _type = "[ Success ]"; break;
		case ErrorsType::Error:  _type = "[ Error ]"; break;
		default: _type = "[ default ]"; break;
		}
		AddToFile(_type + "[ " + GetTimestamp() + " ] " + text);
	}

	static string GetTimestamp() {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);


		std::stringstream ss;
		ss << std::put_time(std::localtime(&in_time_t), "%R:%S.%M");
		return ss.str();
	}

	static void Init() {
		Activation = KeyHandler::AddHotKey(VK_F2, [&]() {
			stateRender++;
			if (stateRender > StateDraw::Console) stateRender = None;
			if (stateRender == StateDraw::Console) {
				openConsole = !openConsole;
				r1::RefGame()->SetCursorMode(r1::CURSOR_LOCKCAMANDCONTROL, openConsole);
			}
		});
#ifdef TEST_CONSOLE
		KeyHandler::AddHotKey(VK_F3, [&]() {
			Add("test", ErrorsType::Info);
		});
		Add("Info", ErrorsType::Info);
		Add("Warning", ErrorsType::Warning);
		Add("Success", ErrorsType::Success);
		Add("Error", ErrorsType::Error);
#endif
		auto path = fs::current_path() / JSON_FILENAME;
		if (!fs::exists(path))
			return;
		fstream file(path.string().c_str(), ios::in | ios::binary);
		json _json = json::parse(file);
		_json["posOverlay"].get_to(posOverlay);
		_json["stateRender"].get_to(stateRender);
		_json["AutoScroll"].get_to(AutoScroll);
		_json["showTime"].get_to(showTime);
		_json["Activation"].get_to(Activation);
		file.close();
	};

	static void Release() {
		json _json;
		_json["posOverlay"] = posOverlay;
		_json["stateRender"] = stateRender;
		_json["AutoScroll"] = AutoScroll;
		_json["showTime"] = showTime;
		_json["Activation"] = Activation;
		auto path = fs::current_path() / JSON_FILENAME;
		fstream file(path.string().c_str(), ios::out | ios::binary);
		file << _json.dump(4);
		file.close();
	}

	static void DrawLines() {
		for (auto &line : GetLines()) {
			ImVec4 color;
			string type;
			switch (line._type) {
			case ErrorsType::Info: color = { 0.072f, 0.615f, 0.94f, 1 }, type = "[ Info ]"; break;
			case ErrorsType::Warning: color = { 0.976f, 0.567f, 0.03f, 1 }, type = "[ Warning ]"; break;
			case ErrorsType::Success: color = { 0.085f, 0.764f, 0.046f, 1 }, type = "[ Success ]"; break;
			case ErrorsType::Error: color = { 0.764f, 0.046f, 0.046f, 1 }, type = "[ Error ]"; break;
			default: color = { 1, 1, 1, 1 }, type = "[ default ]"; break;
			}
			ImGui::TextColored(color, type.c_str());
			ImGui::SameLine();
			if (showTime) {
				ImGui::Text(line._time.c_str());
				ImGui::SameLine();
			}
			ImGui::Text(line._text.c_str());
			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);
		}
	}

	static void DrawOverlay() {
		if (stateRender != StateDraw::Overlay)
			return;
		ImGuiIO& io = ImGui::GetIO();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
		if (posOverlay != -1) {
			const float PAD = 10.0f;
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 work_pos = viewport->WorkPos; 
			ImVec2 work_size = viewport->WorkSize;
			ImVec2 window_pos, window_pos_pivot;
			window_pos.x = (posOverlay & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
			window_pos.y = (posOverlay & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
			window_pos_pivot.x = (posOverlay & 1) ? 1.0f : 0.0f;
			window_pos_pivot.y = (posOverlay & 2) ? 1.0f : 0.0f;
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
			window_flags |= ImGuiWindowFlags_NoMove;
		}
		ImGui::SetNextWindowBgAlpha(0.65f); 
		ImGui::SetNextWindowSize({ 300, 150 }, ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Overlay log##overlay_log", NULL, window_flags)) {
			DrawLines();
			if (ImGui::BeginPopupContextWindow()) {
				if (ImGui::MenuItem("Custom", NULL, posOverlay == -1)) posOverlay = OverlayPos::Custom;
				if (ImGui::MenuItem("Top-left", NULL, posOverlay == 0)) posOverlay = OverlayPos::TopLeft;
				if (ImGui::MenuItem("Top-right", NULL, posOverlay == 1)) posOverlay = OverlayPos::TopRight;
				if (ImGui::MenuItem("Bottom-left", NULL, posOverlay == 2)) posOverlay = OverlayPos::BottomLeft;
				if (ImGui::MenuItem("Bottom-right", NULL, posOverlay == 3)) posOverlay = OverlayPos::TopRight;
				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}
	
	static void DrawConsole() {
		if (stateRender != StateDraw::Console)
			return;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkSize, window_pos, window_size(520, 600);
		window_pos.x = work_pos.x / 2 - window_size.x / 2;
		window_pos.y = work_pos.y / 2 - window_size.y / 2;
		ImGui::SetNextWindowSize(window_size, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_FirstUseEver);
		ImGui::Begin("Console log##console_log", NULL, window_flags);
		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

		if (ImGui::BeginPopup("Settings")) {
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::Checkbox("Time", &showTime);
			Activation->Draw(u8"Активация");
			ImGui::EndPopup();
		}
		if (ImGui::Button("Settings")) {
			ImGui::OpenPopup("Settings");
		}
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
		DrawLines();
		ImGui::EndChild();
		ImGui::End();
	}

	static void Draw() {
		DrawOverlay();
		DrawConsole();
	};
};

uint8_t Console::posOverlay = OverlayPos::BottomRight;
uint8_t Console::stateRender = StateDraw::Console;
bool Console::AutoScroll = true;
bool Console::openConsole = true;
bool Console::showTime = true;
key_ptr_t Console::Activation = nullptr;
#endif