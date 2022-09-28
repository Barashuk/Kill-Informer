#pragma once
#ifndef CONSOLE_HPP
#define CONSOLE_HPP
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include "json_helper.hpp"
#include "KeyCombo.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace std;

#define JSON_FILENAME "console.json"

enum OverlayPos {
	Custom = -1,
	TopLeft = 0,
	TopRight,
	BottomLeft,
	BottomRight,
	
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


template<typename ... Args>
static std::string string_format(const std::string& format, Args ... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

struct stLine {
	uint8_t _type;
	string _time, _text;
	stLine(uint8_t type, string time, string text) {
		_type = type, _time = time, _text = text;
	};
};

struct stConsoleSetting {
	uint8_t  stateRender = StateDraw::Overlay;
	uint8_t posOverlay = OverlayPos::BottomRight;
	bool AutoScroll = true, openConsole = true, showTime = true;
};

class Console{
public:
	Console() = delete;
	Console(const Console&) = delete;
	~Console() = delete;

	static auto GetSetting() {
		static stConsoleSetting cfg;
		return &cfg;
	}

	static auto & GetHotKey() {
		static key_ptr_t Activation;
		return Activation;
	}

	static auto &GetLines(void) {
		static vector <stLine> Lines;
		return Lines;
	}
	template<typename ... Args>
	static void Info(const std::string& format, Args ... args) {
		Add(string_format(format, args...), ErrorsType::Info);
	}

	template<typename ... Args>
	static auto Warning(const std::string& format, Args ... args) {
		Add(string_format(format, args ....), ErrorsType::Warning);
	}

	static void AddToFile(string str) {
		auto path = fs::current_path() / "console.log";
		fstream file(path.string().c_str(), ios::app | ios::binary);
		file << str << endl;
		file.close();
	}

	static void Add(string text, ErrorsType type = ErrorsType::Info) {
		text = cp1251_to_utf8(text);
		GetLines().push_back(stLine(type, "[ " + GetTimestamp() + " ]" , text));;
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
		ss << std::put_time(std::localtime(&in_time_t), "%R:%S");
		return ss.str();
	}

	static void Init() {
		GetHotKey() = KeyHandler::AddHotKey(VK_F2, [&]() {
			auto cfg = GetSetting();
			cfg->stateRender++;
			if (cfg->stateRender > StateDraw::Console) cfg->stateRender = None;
			if (cfg->stateRender == StateDraw::Console) {
				cfg->openConsole = !cfg->openConsole;
			}
		});
		
		auto path = fs::current_path() / JSON_FILENAME;
		if (!fs::exists(path))
			return;
		auto cfg = GetSetting();
		fstream file(path.string().c_str(), ios::in | ios::binary);
		json _json = json::parse(file);
		_json["posOverlay"].get_to(cfg->posOverlay);
		_json["stateRender"].get_to(cfg->stateRender);
		_json["AutoScroll"].get_to(cfg->AutoScroll);
		_json["showTime"].get_to(cfg->showTime);
		_json["Activation"].get_to(GetHotKey());
		file.close();
	};

	static void Release() {
		auto cfg = GetSetting();
		json _json;
		_json["posOverlay"] = cfg->posOverlay;
		_json["stateRender"] = cfg->stateRender;
		_json["AutoScroll"] = cfg->AutoScroll;
		_json["showTime"] = cfg->showTime;
		_json["Activation"] = GetHotKey();
		auto path = fs::current_path() / JSON_FILENAME;
		fstream file(path.string().c_str(), ios::out | ios::binary);
		file << _json.dump(4);
		file.close();
	}

	static void DrawLines() {
		auto cfg = GetSetting();
		for (auto line : GetLines()) {
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
			if (cfg->showTime) {
				ImGui::Text(line._time.c_str());
				ImGui::SameLine();
			}
			ImGui::Text(line._text.c_str());
			if (cfg->AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);
		}
	}

	static void DrawOverlay() {
		auto cfg = GetSetting();
		if (cfg->stateRender != StateDraw::Overlay)
			return;
		ImGuiIO& io = ImGui::GetIO();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
		if (cfg->posOverlay != -1) {
			const float PAD = 10.0f;
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 work_pos = viewport->WorkPos; 
			ImVec2 work_size = viewport->WorkSize;
			ImVec2 window_pos, window_pos_pivot;
			window_pos.x = (cfg->posOverlay & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
			window_pos.y = (cfg->posOverlay & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
			window_pos_pivot.x = (cfg->posOverlay & 1) ? 1.0f : 0.0f;
			window_pos_pivot.y = (cfg->posOverlay & 2) ? 1.0f : 0.0f;
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
			window_flags |= ImGuiWindowFlags_NoMove;
		}
		ImGui::SetNextWindowBgAlpha(0.65f); 
		ImGui::SetNextWindowSize({ 700, 150 }, ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Overlay log##overlay_log", NULL, window_flags)) {
			DrawLines();
			if (ImGui::BeginPopupContextWindow()) {
				if (ImGui::MenuItem("Custom", NULL, cfg->posOverlay == -1)) cfg->posOverlay = OverlayPos::Custom;
				if (ImGui::MenuItem("Top-left", NULL, cfg->posOverlay == 0)) cfg->posOverlay = OverlayPos::TopLeft;
				if (ImGui::MenuItem("Top-right", NULL, cfg->posOverlay == 1)) cfg->posOverlay = OverlayPos::TopRight;
				if (ImGui::MenuItem("Bottom-left", NULL, cfg->posOverlay == 2)) cfg->posOverlay = OverlayPos::BottomLeft;
				if (ImGui::MenuItem("Bottom-right", NULL, cfg->posOverlay == 3)) cfg->posOverlay = OverlayPos::BottomRight;
				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}
	
	static void DrawConsole() {
		auto cfg = GetSetting();
		if (cfg->stateRender != StateDraw::Console)
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
			ImGui::Checkbox("Auto-scroll", &cfg->AutoScroll);
			ImGui::Checkbox("Time", &cfg->showTime);
			GetHotKey()->Draw(u8"Активация");
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

#endif