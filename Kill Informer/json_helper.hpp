#pragma once
#ifndef JSON_HELPER_
#define JSON_HELPER_
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <string>
#include <Windows.h>

template <typename Type>
static void from_json_with_check(const nlohmann::json& j, const std::string &section, Type& value) {
	if (j.contains(section)) {
		j.at(section).get_to<Type>(value);
	}
}

static void to_json(nlohmann::json& j, const ImVec2& v) {
	j = nlohmann::json{
		{"x", v.x},
		{"y", v.y},
	};
}

static void from_json(const nlohmann::json& j, ImVec2& v) {
	j.at("x").get_to(v.x);
	j.at("y").get_to(v.y);
}

static void to_json(nlohmann::json& j, const ImVec4& v) {
	j = nlohmann::json{
		{"w", v.w},
		{"x", v.x},
		{"y", v.y},
		{"z", v.z},
	};
}

static void from_json(const nlohmann::json& j, ImVec4& v) {
	j.at("w").get_to(v.w);
	j.at("x").get_to(v.x);
	j.at("y").get_to(v.y);
	j.at("z").get_to(v.z);
}

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

static std::string utf8_to_cp1251(std::string const& utf8) {
	if (!utf8.empty()) {
		int wchlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), NULL, 0);
		if (wchlen > 0 && wchlen != 0xFFFD) {
			std::vector<wchar_t> wbuf(wchlen);
			MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), &wbuf[0], wchlen);
			std::vector<char> buf(wchlen);
			WideCharToMultiByte(1251, 0, &wbuf[0], wchlen, &buf[0], wchlen, 0, 0);

			return std::string(&buf[0], wchlen);
		}
	}
	return std::string();
}

#endif // JSON_HELPER_

