#include "CKillState.h"
#include <libzippp/libzippp.h>
#include <nlohmann/json.hpp>
#include "json_helper.hpp"
using json = nlohmann::json;
using namespace libzippp;

void to_json(nlohmann::json& j, const stSetting& v) {
	j = json{
		


		{"currentPack", cp1251_to_utf8(v.currentPack)},
		{"activeElementMenu", v.activeElementMenu}
	};
}

void from_json(const nlohmann::json& j, stSetting& v) {
	string text;
	from_json_with_check(j, "currentPack", text);
	from_json_with_check(j, "activeElementMenu", v.activeElementMenu);
	v.currentPack = cp1251_to_utf8(text);
}

void to_json(nlohmann::json& j, const stElementEvent& v) {
	j = json{
		{"value", v.value},
		{"name", v.name},
		{"sound", v.sound},
		{"color", v.color},
	};
}

void from_json(const nlohmann::json& j, stElementEvent& v) {
	j.at("value").get_to(v.value);
	j.at("name").get_to(v.name);
	j.at("sound").get_to(v.sound);
	j.at("color").get_to(v.color);	
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
			ZipArchive zf(entry_dir.path().string());
			zf.open(ZipArchive::ReadOnly);
			auto& entries = zf.getEntries();
			auto it = find_if(entries.begin(), entries.end(), [](const ZipEntry& elem) {
				return elem.getName() == "fonts.json";
				});
			if (it != entries.end()) {
				string data = it->readAsText();
				json fonts = json::parse(data);
				fonts["Kills"].get_to(pack.fontKills);
				fonts["Deaths"].get_to(pack.fontDeaths);
				fonts["Assists"].get_to(pack.fontAssists);
				fonts["Message"].get_to(pack.fontMessage);
			}
			it = find_if(entries.begin(), entries.end(), [](const ZipEntry& elem) {
				return elem.getName() == "events.json";
				});
			if (it != entries.end()) {
				string data = it->readAsText();
				json events = json::parse(data);
				events["Kills"].get_to(pack.KillsEvents);
				events["Deaths"].get_to(pack.DeathsEvents);
				events["Assists"].get_to(pack.AssistsEvents);
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
			pack.Name = filename;
			Packs.push_back(pack);

		}
		if (Packs.empty())
			return;

		cfg.currentPack = Packs.back().Name;
		std::sort(Packs.begin(), Packs.end(), [](const stPack& first, const stPack& second) {
			return first.Name < second.Name;
			});
	}
	catch (const std::exception& e) {
		Console::Add(e.what());
	}
}

void CKillState::SavePacks() {
	try {
		for (auto& pack : Packs) {

			auto p = pathDir / "Packs" / (pack.Name + ".zip");
			ZipArchive zf(p.string());
			zf.open(ZipArchive::Write);
			json fonts;
			fonts["Kills"] = pack.fontKills;
			fonts["Deaths"] = pack.fontDeaths;
			fonts["Assists"] = pack.fontAssists;
			fonts["Message"] = pack.fontMessage;
			string dump_fonts = fonts.dump(4);
			zf.addData("fonts.json", dump_fonts.data(), dump_fonts.length());

			json events;
			events.clear();
			events["Kills"] = pack.KillsEvents;
			events["Deaths"] = pack.DeathsEvents;
			events["Assists"] = pack.AssistsEvents;
			string dump_events = events.dump(4);
			zf.addData("events.json", dump_events.data(), dump_events.length());
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
