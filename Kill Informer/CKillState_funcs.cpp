#include "CKillState.h"
#include <sampapi/CNetGame.h>
#include <game_sa/ePedState.h>

#include <libzippp/libzippp.h>
#include "Console.hpp"
#include "plugin.h"
#include "CustomFont.cpp"
#include "IconsFontAwesome6.h"
#include "IconsFontAwesome6Brands.h"

namespace R1 = sampapi::v037r1;
using namespace libzippp;
#pragma warning (disable: 26812)
#pragma warning (disable: 6011)

void CKillState::Init() {
	KillString = "{cKiller}{nKiller} {cText}killed {cVictim}{nVictim}.";
	DistString = "{cText}Dist:{dist} m";
	activeMenu = KeyHandler::AddHotKey(VK_F12, [this]() { OpenMenu(); });
	pathDir = fs::current_path() / "Kill Informer";
	InitFonts();
	LoadSetting();
	ParsePacks();
	fileDialog.SetFileStyle(IGFD_FileStyleByExtention, ".mp3", ImVec4(0.0f, 0.849f, 0.057f, 1.0f), ICON_FA_MUSIC);
	fileDialog.SetFileStyle(IGFD_FileStyleByExtention, ".wav", ImVec4(0.0f, 0.849f, 0.057f, 1.0f), ICON_FA_MUSIC);
	fileDialog.SetFileStyle(IGFD_FileStyleByExtention, ".ogg", ImVec4(0.0f, 0.849f, 0.057f, 1.0f), ICON_FA_MUSIC);
	fileDialog.SetFileStyle(IGFD_FileStyleByTypeFile, nullptr, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ICON_IGFD_FILE);
	fileDialog.SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 1.0f), ICON_IGFD_FOLDER);

}

void CKillState::Release() {
	if (fileDialog.IsOpened()) {
		fileDialog.Close();
	}
	SavePacks();
	SaveSetting();
}

void CKillState::PrepareKillString(CPed* killer, CPed* victim, CPed* assist) {
	auto replace = [](std::string &str, const std::string& from, const std::string& to) {
		if (from.empty())
			return;
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	};
	auto* pPlayers = R1::RefNetGame()->GetPlayerPool();
	auto pLocalSA = FindPlayerPed();
	string KillerName, VictimName, AssistName;
	string KillerColor, VictimColor, AssistColor;
	string AssistString;
	if (victim == pLocalSA) {
		auto kID = pPlayers->Find(killer);
		KillerName = pPlayers->GetName(kID);
		KillerColor = string_format("{%08X}", pPlayers->GetPlayer(kID)->GetColorAsARGB() | 0xFF000000);
		VictimName = pPlayers->m_localInfo.m_szName;
		VictimColor = string_format("{%08X}", pPlayers->GetLocalPlayer()->GetColorAsARGB() | 0xF0000000);
	}
	else if (killer == pLocalSA) {
		auto vID = pPlayers->Find(victim);
		VictimName = pPlayers->GetName(vID);
		VictimColor = string_format("{%08X}", pPlayers->GetPlayer(vID)->GetColorAsARGB() | 0xFF000000);
		KillerName = pPlayers->m_localInfo.m_szName;
		KillerColor = string_format("{%08X}", pPlayers->GetLocalPlayer()->GetColorAsARGB() | 0xF0000000);
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
	renderString = KillString;
	replace(renderString, "{nKiller}", KillerName);
	replace(renderString, "{cKiller}", KillerColor);
	replace(renderString, "{nVictim}", VictimName);
	replace(renderString, "{cVictim}", VictimColor);
	if (!AssistString.empty()) {
		renderString += "{cText}Assist: " + AssistString;
	}



	Console::Add(renderString);
}

void CKillState::DetectKillsAndDeaths() {
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
		PrepareKillString(killer, pLocalSA, nullptr);
		DamagePlayerInfo.erase(remove_if(DamagePlayerInfo.begin(), DamagePlayerInfo.end(), [&](const stDamagePlayer& elem) { 
			return elem.victim == pLocalSA; 
			}), DamagePlayerInfo.end());
		Console::Add("ya zdoh");
	}
	else if (killer == pLocalSA) {
		KDA.Kills(last.weapon)++;
		PrepareKillString(killer, victim, nullptr);
		DamagePlayerInfo.erase(remove_if(DamagePlayerInfo.begin(), DamagePlayerInfo.end(), [&](const stDamagePlayer& elem) { 
			return elem.killer == pLocalSA;
			}), DamagePlayerInfo.end());
		Console::Add("ya ybil");
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
		if (wParam == 'T')
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

void CKillState::AddToPlayMusic(string name, string pack_name, bool last_free) {
	auto isPlayIter = find_if(musicHandles.begin(), musicHandles.end(), [&](const music_p& elemMusic) {
		return elemMusic.first == name;
		});
	if (isPlayIter != musicHandles.end())
		return;
	if (!musicHandles.empty() && last_free) {
		auto& stream = musicHandles.front().second;
		BASS_StreamFree(stream);
		musicHandles.clear();
	}
	auto path = pathDir / "Packs" / (pack_name + ".zip");
	ZipArchive zf(path.string());
	zf.open(ZipArchive::ReadOnly);
	auto& entries = zf.getEntries();
	auto iter = find_if(entries.begin(), entries.end(), [&](const ZipEntry& elem) {
		return elem.getName() == "Sounds/" + name;
		});	
	HSTREAM stream = BASS_StreamCreateFile(true, iter->readAsBinary(), 0, iter->getSize(), 0);
	auto error = BASS_ErrorGetCode();
	if (error == BASS_OK) {
		musicHandles.push_back(music_p(name, stream));
	}
	
	zf.close();
}