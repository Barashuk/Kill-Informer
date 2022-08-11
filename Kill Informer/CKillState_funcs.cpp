#include "CKillState.h"
#include <sampapi/CNetGame.h>
#include <game_sa/ePedState.h>
#include "Console.hpp"
#include "plugin.h"
namespace R1 = sampapi::v037r1;
#pragma warning (disable: 26812)
#pragma warning (disable: 6011)

void CKillState::Init() {
	KillString = "{cKiller}{nKiller} {cText}killed {cVictim}{nVictim}.";
	DistString = "{cText}Dist:{dist} m";
	activeMenu = KeyHandler::AddHotKey(VK_F12, [this]() { OpenMenu(); });
	pathDir = fs::current_path() / "Kill Informer";
	bMenu = false;
	LoadSetting();
	ParsePacks();
}

void CKillState::Release() {
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
	if (bMenu && msg == WM_KEYDOWN) {
		if (wParam == 'T')
			return false;
		return true;
	}
	return true;
}

