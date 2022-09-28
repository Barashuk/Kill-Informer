#include "CKillState.h"
#include <libzippp/libzippp.h>
#include <nlohmann/json.hpp>
#include "json_helper.hpp"
using json = nlohmann::json;
using namespace libzippp;

void to_json(nlohmann::json& j, const stSetting& v) {
	j = json{
		{"currentPack", cp1251_to_utf8(v.currentPack)},
		{"activeElementMenu", v.activeElementMenu},
		{"lastPath", v.lastPath},
		{"menuID", v.menuID}
	};
}

void from_json(const nlohmann::json& j, stSetting& v) {
	string text;
	from_json_with_check(j, "currentPack", text);
	from_json_with_check(j, "activeElementMenu", v.activeElementMenu);
	from_json_with_check(j, "lastPath", v.lastPath);
	from_json_with_check(j, "menuID", v.menuID);
	v.currentPack = cp1251_to_utf8(text);
}

void to_json(nlohmann::json& j, const stElementEvent& v) {
	j = json{
		{"value", v.value},
		{"style", v.style},
		{"name", v.name},
		{"sound", v.sound},
		{"color", v.color},
		{"active", v.active},
	};
}

void from_json(const nlohmann::json& j, stElementEvent& v) {
	from_json_with_check(j, "value", v.value);
	from_json_with_check(j, "style", v.style);
	from_json_with_check(j, "name", v.name);
	from_json_with_check(j, "sound", v.sound);
	from_json_with_check(j, "color", v.color);
	from_json_with_check(j, "active", v.active);
}

void to_json(nlohmann::json& j, const stPackSetting& v) {
	j = json{
		{"timeDrawKillString", v.timeDrawKillString},
		{"timeResetKillStreak", v.timeResetKillStreak},
		{"timeDisplayStatuses", v.timeDisplayStatuses},
		{"typePosStatuses", v.typePosStatuses},
		{"ShowDistance", v.ShowDistance},
		{"AutoDisplayTime", v.AutoDisplayTime},
		{"colorDistance", v.colorDistance},
	};
}

void from_json(const nlohmann::json& j, stPackSetting& v) {
	from_json_with_check(j, "timeDrawKillString", v.timeDrawKillString);
	from_json_with_check(j, "timeResetKillStreak", v.timeResetKillStreak);
	from_json_with_check(j, "timeDisplayStatuses", v.timeDisplayStatuses);
	from_json_with_check(j, "typePosStatuses", v.typePosStatuses);
	from_json_with_check(j, "ShowDistance", v.ShowDistance);
	from_json_with_check(j, "AutoDisplayTime", v.AutoDisplayTime);
	from_json_with_check(j, "colorDistance", v.colorDistance);
}


void CKillState::ParsePacks() {
	try {
		if (!fs::exists(pathDir)) {
			fs::create_directory(pathDir);
		}
		vector<string> Dirs = { "Fonts", "Packs" };
		for (auto& dir : Dirs) {
			auto path = pathDir / dir;
			if (!fs::exists(path))
				fs::create_directory(path);
		}
		auto dirPacks = pathDir / Dirs.back();
		if (fs::is_empty(dirPacks)) {

		}
		for (const auto& entry_dir : fs::directory_iterator(dirPacks)) {
			if (fs::is_directory(entry_dir))
				continue;
			if (!entry_dir.path().has_filename())
				continue;
			auto filename = entry_dir.path().filename().string();
			auto ext = entry_dir.path().filename().extension().string();
			ext = ext.substr(1, ext.length() - 1);
			filename = filename.substr(0, filename.find_last_of('.'));
			if (ext != "zip")
				continue;
			stPack pack;
			pack.fontKills = FontsHandler::AddFont("Comic Sans MS", 14, FCR_BOLD | FCR_BORDER);
			pack.fontDeaths = FontsHandler::AddFont("Comic Sans MS", 14, FCR_BOLD | FCR_BORDER);
			pack.fontAssists = FontsHandler::AddFont("Comic Sans MS", 14, FCR_BOLD | FCR_BORDER);
			pack.fontMessage = FontsHandler::AddFont("Comic Sans MS", 14, FCR_BOLD | FCR_BORDER);
			pack.fontStatuses = FontsHandler::AddFont("Comic Sans MS", 14, FCR_BOLD | FCR_BORDER);
			ZipArchive zf(entry_dir.path().u8string());
			zf.open(ZipArchive::ReadOnly);
			auto& entries = zf.getEntries();
			
			auto it = find_if(entries.begin(), entries.end(), [](const ZipEntry& elem) {
				return elem.getName() == "fonts.json";
				});
			if (it != entries.end()) {
				json fonts = json::parse(it->readAsText());
				from_json_with_check(fonts, "Kills", pack.fontKills);
				from_json_with_check(fonts, "Deaths", pack.fontDeaths);
				from_json_with_check(fonts, "Assists", pack.fontAssists);
				from_json_with_check(fonts, "Message", pack.fontMessage);
				from_json_with_check(fonts, "Statuses", pack.fontStatuses);
			}
			it = find_if(entries.begin(), entries.end(), [](const ZipEntry& elem) {				
				return elem.getName() == "events.json";
				});
			if (it != entries.end()) {
				json events = json::parse(it->readAsText());
				from_json_with_check(events, "Kills", pack.KillsEvents);
				from_json_with_check(events, "Deaths", pack.DeathsEvents);
				from_json_with_check(events, "Assists", pack.AssistsEvents);
				from_json_with_check(events, "Streaks", pack.StreaksEvents);
				from_json_with_check(events, "Unique", pack.UniqueEvents);
			}
			it = find_if(entries.begin(), entries.end(), [](const ZipEntry& elem) {
				return elem.getName() == "setting.json";
				});
			if (it != entries.end()) {
				json events = json::parse(it->readAsText());
				pack.setting = events;
			}
			for (auto file = entries.begin(); file != entries.end(); ++file) {
				string s = "Sounds/";
				string filename = file->getName();
				size_t pos = filename.find(s);
				if (pos == string::npos)
					continue;
				filename = filename.substr(s.length());
				if (filename.empty())
					continue;
				pos = filename.find_last_of(".");
				if (pos == string::npos)
					continue;
				string ext = filename.substr(pos + 1);
				if (ext != "wav" && ext != "ogg" && ext != "mp3")
					continue;
				pack.SoundFiles.push_back(filename);
			}
			zf.close();
			Packs.insert(pair<string, stPack>(filename, pack));
		}
		if (Packs.empty())
			return;
		ValidateMusicFiles();
		cfg.currentPack = std::prev(Packs.end())->first;
	}
	catch (const std::exception& e) {
		Console::Add(e.what());
	}
}

void CKillState::SavePacks() {
	try {
		for (auto& _pack : Packs) {
			auto p = pathDir / "Packs" / (_pack.first + ".zip");
			auto& pack = _pack.second;
			ZipArchive zf(p.u8string());
			zf.open(ZipArchive::Write);
			json fonts;
			fonts.clear();
			fonts["Kills"] = pack.fontKills;
			fonts["Deaths"] = pack.fontDeaths;
			fonts["Assists"] = pack.fontAssists;
			fonts["Message"] = pack.fontMessage;
			fonts["Statuses"] = pack.fontStatuses;
			string dump_fonts = fonts.dump(4);
			zf.addData("fonts.json", dump_fonts.data(), dump_fonts.length());

			json events;
			events.clear();
			events["Kills"] = pack.KillsEvents;
			events["Deaths"] = pack.DeathsEvents;
			events["Assists"] = pack.AssistsEvents;
			events["Streaks"] = pack.StreaksEvents;
			events["Unique"] = pack.UniqueEvents;
			string dump_events = events.dump(4);
			zf.addData("events.json", dump_events.data(), dump_events.length());

			json setting;
			setting.clear();
			setting = pack.setting;
			string dump_setting = setting.dump(4);
			zf.addData("setting.json", dump_setting.data(), dump_setting.length());

			zf.close();
		}
	}
	catch (const std::exception& e) {
		Console::Add(e.what());
	}
}

void CKillState::LoadSetting() {
	try {
		json setting;
		fstream  f((pathDir / "setting.json").string(), ios::in | ios::binary);
		setting = json::parse(f);
		cfg = setting["Setting"];
		f.close();
	}
	catch (const std::exception& e) {
		Console::Info("%s %s", e.what(), __FUNCTION__);
	}
}

void CKillState::SaveSetting() {
	try {
		json setting;
		setting["Setting"] = cfg;
		fstream  f((pathDir / "setting.json").string(), ios::out | ios::binary);
		auto d = setting.dump(4);
		f << d;
		f.close();
	}
	catch (const std::exception& e) {
		Console::Info("%s %s", e.what(), __FUNCTION__);
	}
}
