#include "CKillState.h"
#include <random>
#include <sampapi/CNetGame.h>

#include <game_sa/ePedState.h>
#include <libzippp/libzippp.h>
#include "Console.hpp"
#include "plugin.h"
#include "IconsFontAwesome6.h"
#include "IconsFontAwesome6Brands.h"

enum ePedPieceTypess {
	PED_PIECE_UNKNOWN = 0,
	PED_PIECE_TORSO = 3,
	PED_PIECE_ASS,
	PED_PIECE_LEFT_ARM,
	PED_PIECE_RIGHT_ARM,
	PED_PIECE_LEFT_LEG,
	PED_PIECE_RIGHT_LEG,
	PED_PIECE_HEAD
};

namespace R1 = sampapi::v037r1;
using namespace libzippp;
#pragma warning (disable: 26812)
#pragma warning (disable: 6011)

void CKillState::Init() {
	//KillString = "{cKiller}{nKiller} {cText}killed {cVictim}{nVictim}{cText}.";
	//StatusString = "{cKiller}{nKiller}{cText} made a";
	//DistString = "{cText}Dist: {dColor}{dist}{cText}m";
	activeMenu = KeyHandler::AddHotKey(VK_F12, [this]() { OpenMenu(); });
	pathDir = fs::current_path() / "Kill Informer";
	fileDialog.SetFileStyle(IGFD_FileStyleByExtention, ".mp3", ImVec4(0.0f, 1.0f, 1.0f, 1.0f), ICON_FA_MUSIC);
	fileDialog.SetFileStyle(IGFD_FileStyleByExtention, ".wav", ImVec4(0.0f, 1.0f, 1.0f, 1.0f), ICON_FA_MUSIC);
	fileDialog.SetFileStyle(IGFD_FileStyleByExtention, ".ogg", ImVec4(0.0f, 1.0f, 1.0f, 1.1f), ICON_FA_MUSIC);
	fileDialog.SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FOLDER); // for all link dirs
	fileDialog.SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FILE); // for all link files
	InitFonts();
	LoadSetting();
	ParsePacks();
}

void CKillState::Release() {
	if (fileDialog.IsOpened()) {
		fileDialog.Close();
	}
	if (!musicHandles.empty()) {
		for (auto &it : musicHandles) {
			BASS_StreamFree(it.second);
		}
	}
	SavePacks();
	SaveSetting();
}

void CKillState::PrepareKillString(CPed* killer, CPed* victim, CPed* assist, int flags) {
	auto AddEvent = [this](vector<stElementEvent>& events, int value, vector <string>& statuses, bool search_zero = false) {
		auto event = GetRandomEventByID(events, value, search_zero);
		if (event.IsValid()) {
			if (event.style == Message) {
				statuses.push_back(event.GetTextToRender());
				AddToPlayMusic(event.sound);
			}
			else {
				AddToRenderStatuses(event);
			}
		}
	};

	auto it = Packs.at(cfg.currentPack);
	auto* pPlayers = R1::RefNetGame()->GetPlayerPool();
	auto pLocalSA = FindPlayerPed();
	string KillerName, VictimName, AssistName;
	string KillerColor, VictimColor, AssistColor;
	string AssistString;
	renderString = it.setting.KillString.Value();
	vector <string> statuses;
	if (victim == pLocalSA) {
		auto kID = pPlayers->Find(killer);
		KillerName = pPlayers->GetName(kID);
		KillerColor = string_format("{%08X}", pPlayers->GetPlayer(kID)->GetColorAsARGB() | 0xFF000000);
		VictimName = pPlayers->m_localInfo.m_szName;
		VictimColor = string_format("{%08X}", pPlayers->GetLocalPlayer()->GetColorAsARGB() | 0xF0000000);
		lastWhoKillLocalPlayerName = KillerName;
		AddEvent(it.DeathsEvents, KDA.Deaths.value(), statuses, true);
		auto vec = AddToUniqueStatuses(flags);
		if (!vec.empty())
			statuses.insert(statuses.end(), vec.begin(), vec.end());
		bPrepareToFight = true;
	}
	else if (killer == pLocalSA) {
		auto vID = pPlayers->Find(victim);
		VictimName = pPlayers->GetName(vID);
		VictimColor = string_format("{%08X}", pPlayers->GetPlayer(vID)->GetColorAsARGB() | 0xFF000000);
		KillerName = pPlayers->m_localInfo.m_szName;
		KillerColor = string_format("{%08X}", pPlayers->GetLocalPlayer()->GetColorAsARGB() | 0xF0000000);		
		if (VictimName == lastWhoKillLocalPlayerName) {
			flags |= Vengeance;
			lastWhoKillLocalPlayerName.clear();
		}
		AddEvent(it.KillsEvents, KDA.Kills.value(), statuses);
		AddEvent(it.StreaksEvents, KDA.KillStreaks.value(), statuses);
		auto vec = AddToUniqueStatuses(flags);
		if (!vec.empty())
			statuses.insert(statuses.end(), vec.begin(), vec.end());
		
	}
	else if (killer != pLocalSA && victim != pLocalSA && assist == pLocalSA) {
		auto vID = pPlayers->Find(victim);
		auto kID = pPlayers->Find(killer);
		VictimName = pPlayers->GetName(vID);
		VictimColor = string_format("{%08X}", pPlayers->GetPlayer(vID)->GetColorAsARGB() | 0xFF000000);
		KillerName = pPlayers->GetName(kID);
		KillerColor = string_format("{%08X}", pPlayers->GetPlayer(kID)->GetColorAsARGB() | 0xFF000000);
		AssistString = string_format("{%08X}", pPlayers->GetLocalPlayer()->GetColorAsARGB() | 0xF0000000) + pPlayers->m_localInfo.m_szName;
	}
	if (it.setting.ShowDistance) {
		renderString += " " + it.setting.DistString.Value();
		auto pos = DistanceBetweenPoints(killer->GetPosition(), victim->GetPosition());
		ReplaceStrings(renderString, "{dColor}", string_format("{%08X}", toARGB(it.setting.colorDistance)));
		ReplaceStrings(renderString, "{dist}", string_format("%.02f", pos));
	}
	if (!AssistString.empty()) {
		renderString += "\n{cText}Assist: " + AssistString;
	}
	string text;
	auto size = statuses.size();
	if (size > 0) {
		for (size_t i = 0; i < size; i++) {
			if (i == size - 1) {
				text = text.substr(0, text.size() - 2);
				text += " and " + statuses.at(i);
			}
			else {
				text += statuses.at(i) + "{cText}, ";
			}
		}
		renderString += "\n" + it.setting.StatusString.Value() + " " + text;
	}
	ReplaceStrings(renderString, "{nKiller}", KillerName);
	ReplaceStrings(renderString, "{cKiller}", KillerColor);
	ReplaceStrings(renderString, "{nVictim}", VictimName);
	ReplaceStrings(renderString, "{cVictim}", VictimColor);
	ReplaceStrings(renderString, "{cText}", string_format("{%08X}", toARGB(it.fontMessage->GetColor())));
	renderString = utf8_to_cp1251(renderString);
	timeRenderMessage = system_clock_t::now() + chrono::seconds(it.setting.timeDrawKillString);
}

void CKillState::DetectKillsAndDeaths() {
	auto it = Packs.at(cfg.currentPack);
	if (DamagePlayerInfo.empty())
		return;
	auto& last = DamagePlayerInfo.back();
	auto victim = last.victim;
	auto killer = last.killer;
	auto pLocalSA = FindPlayerPed();
	if (!killer || !victim && !pLocalSA)
		return;
	if (victim->m_nPedState != ePedState::PEDSTATE_DIE &&
		victim->m_nPedState != ePedState::PEDSTATE_DEAD)
		return;
	if (victim == pLocalSA) {
		KDA.Deaths(last.weapon)++;
		PrepareKillString(killer, pLocalSA, nullptr, 0);
		DamagePlayerInfo.erase(remove_if(DamagePlayerInfo.begin(), DamagePlayerInfo.end(), [&](const stDamagePlayer& elem) { return elem.victim == pLocalSA; }), DamagePlayerInfo.end());
	}
	else if (killer == pLocalSA) {
		KDA.Kills(last.weapon)++;
		KDA.KillStreaks(last.weapon)++;
		int flags = 0;
		flags |= last.part == PED_PIECE_ASS ? Assshot : 0;
		flags |= last.part == PED_PIECE_HEAD ? Headshot : 0;
		flags |= last.weapon == WEAPON_KNIFE ? Humilition : 0;
		flags |= (last.weapon == WEAPON_DILDO1 || last.weapon == WEAPON_DILDO2 || last.weapon == WEAPON_VIBE1 || last.weapon == WEAPON_VIBE2) ? Penetration : 0;
		if (!bFirstBlood) {
			bFirstBlood = true;
			flags |= First_Blood;
		}		
		PrepareKillString(killer, victim, nullptr, flags);
		DamagePlayerInfo.erase(remove_if(DamagePlayerInfo.begin(), DamagePlayerInfo.end(), [&](const stDamagePlayer& elem) {  return elem.killer == pLocalSA; }), DamagePlayerInfo.end());
		timeKillStreak = system_clock_t::now() + chrono::seconds(it.setting.timeResetKillStreak);
	}
	/*else if (killer != pLocalSA && victim != pLocalSA) {
		//Ассисты не работают. Почини
		auto it = find_if(DamagePlayerInfo.begin(), DamagePlayerInfo.end(), [&](const stDamagePlayer& elem) { return elem.killer == pLocalSA; });
		if (it != DamagePlayerInfo.end()) {
			KDA.Assist(last.weapon)++;
			PrepareKillString(killer, victim, pLocalSA);
			Console::Add("Ya pomog");
		}
		DamagePlayerInfo.erase(remove_if(DamagePlayerInfo.begin(), DamagePlayerInfo.end(), [&](const stDamagePlayer& elem) { return elem.victim == victim; }), DamagePlayerInfo.end());
	}*/		
}

void CKillState::Process() {
	DetectKillsAndDeaths();
	AutoPlayMusic();
	auto NetGame = R1::RefNetGame();
	if (!NetGame)
		return;
	auto pPlayers = NetGame->GetPlayerPool();
	if (!pPlayers)
		return;
	auto pSampLocalPlayer = pPlayers->GetLocalPlayer();
	if (bPrepareToFight && pSampLocalPlayer && pSampLocalPlayer->m_bIsActive) {
		AddToUniqueStatuses(PrepareToFight);
		bPrepareToFight = false;
	}
}

void CKillState::DamageEvent(CPed* victim, CEntity* killer, eWeaponType weapon, int damage, ePedPieceTypes part, int direction) {
	if (!victim || !killer)
		return;
	if (killer->m_nType == ENTITY_TYPE_PED) {
		if (victim->m_nPedState == ePedState::PEDSTATE_DIE ||
			victim->m_nPedState == ePedState::PEDSTATE_DEAD)
			return;
		float _damage = (float)damage * 0.3333f;
		auto _killer = reinterpret_cast<CPed*> (killer);
		DamagePlayerInfo.push_back(stDamagePlayer{ static_cast<uint8_t>(weapon), _damage, _killer , victim, static_cast<uint8_t>(part) });
	}
}

bool CKillState::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (bOpenMenu && msg == WM_KEYDOWN) {
		if (wParam == 'T' || wParam == VK_ESCAPE)
			return false;
		return true;
	}
	return true;
}

void CKillState::InitFonts() {
	vector <string> fontNames = {
		"fa-brands-400.ttf",
		"fa-regular-400.ttf",
		"fa-solid-900.ttf",
	};
	auto pathFonts = pathDir / "Fonts";
	auto& io = ImGui::GetIO();
	for (auto name : fontNames) {
		auto path = pathFonts / name;
		if(!fs::exists(path))
			continue;
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
		io.Fonts->AddFontFromFileTTF(path.string().c_str(), 16.0f, &icons_config, icons_ranges);
	}
}

void CKillState::AutoPlayMusic() {
	if (musicHandles.empty())
		return;
	QWORD len, pos;
	auto& stream = musicHandles.front().second;
	auto status = BASS_ChannelIsActive(stream);
	if (status == BASS_ACTIVE_STOPPED) {
		len = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
		pos = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
		if (pos == 0) {
			BASS_ChannelPlay(stream, false);
		}
		else if (pos == len) {
			BASS_StreamFree(stream);
			musicHandles.erase(musicHandles.begin());
		}
	}
}

int CKillState::AddToPlayMusic(string name, bool last_free /*= false*/) {
	int milliseconds = 1000;
	if (name.empty()) {
		return milliseconds;
	}
	auto isPlayIter = find_if(musicHandles.begin(), musicHandles.end(), [&](const music_p& elemMusic) {
		return elemMusic.first == name;
		});
	if (isPlayIter != musicHandles.end())
		return milliseconds;
	if (!musicHandles.empty() && last_free) {
		auto& stream = musicHandles.front().second;
		BASS_StreamFree(stream);
		musicHandles.clear();
	}
	auto path = pathDir / "Packs" / (cfg.currentPack + ".zip");
	ZipArchive zf(path.u8string());
	zf.open(ZipArchive::ReadOnly);
	auto& entries = zf.getEntries();
	auto iter = find_if(entries.begin(), entries.end(), [&](const ZipEntry& elem) {
		return elem.getName() == "Sounds/" + name;
		});	
	if (iter == entries.end()) {
		Console::Info("not found %s", name.c_str());
		return milliseconds;
	}
	HSTREAM stream = BASS_StreamCreateFile(true, iter->readAsBinary(), 0, iter->getSize(), 0);
	auto error = BASS_ErrorGetCode();
	if (error == BASS_OK) {	
		musicHandles.push_back(music_p(name, stream));
		auto length = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
		milliseconds = (int)(BASS_ChannelBytes2Seconds(stream, length) * 1000);
	}
	zf.close();
	return milliseconds;
}

stElementEvent CKillState::GetRandomEventByID(vector<stElementEvent>& events, int id, bool zero_search /* = false */){
	stElementEvent event(-1);
	auto count = count_if(events.begin(), events.end(), [&](const stElementEvent& elem) { return elem.active == true && elem.value == id; });
	if (count == 0) {
		if (zero_search) {
			return GetRandomEventByID(events, 0);
		}
		return event;
	}
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist(1, count);
	auto search = dist(rng);
	count = 0;
	auto it = find_if(events.begin(), events.end(), [&](const stElementEvent& elem) {
		if (elem.active == true && elem.value == id) {
			count++;
		}
		return elem.active == true && elem.value == id && count == search;
		});
	if (it == events.end()) {
		return event;
	}
	event = *it;
	return event;
}

void CKillState::AddToRenderStatuses(stElementEvent event) {
	auto pack = Packs.at(cfg.currentPack);
	auto time = AddToPlayMusic(event.sound);
	auto& setting = pack.setting;
	if (!setting.AutoDisplayTime) {
		time = setting.timeDisplayStatuses * 1000;
	}
	StatusesStrings.push_back({ utf8_to_cp1251(event.GetTextToRender()) , system_clock_t::now() + chrono::milliseconds(time) });
}

vector <string> CKillState::AddToUniqueStatuses(int flags) {
	vector <string> result;
	auto pack = Packs.at(cfg.currentPack);
	constexpr auto entries = magic_enum::enum_entries<UniqueKillStatus>();
	auto& setting = pack.setting;
	for (const auto &it : entries) {
		auto flag = (flags & it.first);
		if (flag == NoMessage)
			continue;
		if(flag != it.first)
			continue;
		auto event = GetRandomEventByID(pack.UniqueEvents, it.first);
		if (!event.IsValid()) {
			continue;
		}
		if (event.style == Message) {
			result.push_back(event.GetTextToRender());
			AddToPlayMusic(event.sound);
		}
		else{
			AddToRenderStatuses(event);
		}
	}
	return result;
}

void CKillState::ReplaceStrings(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
};

void CKillState::ValidateMusicFiles() {
	auto IsFileValid = [](vector<stElementEvent> &events, vector <string>& music) {
		for (auto& event : events) {
			auto iter = find_if(music.begin(), music.end(), [&](const string& name) { return name == event.sound; });
			if (iter == music.end()) {
				event.sound = music.front();
			}
		}
	};
	for (auto &it : Packs ) {
		auto& music = it.second.SoundFiles;
		IsFileValid(it.second.KillsEvents, music);
		IsFileValid(it.second.DeathsEvents, music);
		IsFileValid(it.second.AssistsEvents, music);
		IsFileValid(it.second.StreaksEvents, music);
		IsFileValid(it.second.UniqueEvents, music);
	}
}

void CKillState::RenameFileInVector(string search, string name) {
	auto Rename = [](vector<stElementEvent>& events, string search, string name) {
		auto iter = find_if(events.begin(), events.end(), [&](const stElementEvent& elem) {
			return elem.sound == search;
			});
		if (iter != events.end()) {
			iter->sound = name;
		}
	};
	auto& it = Packs.at(cfg.currentPack);
	Rename(it.KillsEvents, search, name);
	Rename(it.DeathsEvents, search, name);
	Rename(it.AssistsEvents, search, name);
	Rename(it.StreaksEvents, search, name);
	Rename(it.UniqueEvents, search, name);



}

