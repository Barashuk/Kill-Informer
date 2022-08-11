#pragma once
#include <game_sa/CPed.h>
#include <vector>
#include <string>
#include <map>
#include "FontsHandler.hpp"
using namespace std;

struct stStat {
private:
	uint16_t value = 0;
	vector <uint16_t> values;
public:
	stStat() {
		values.resize(100, 0);
		value = 0;
	}
	stStat& operator++(int){
		value++;
		return *this;
	}
	stStat& operator()(uint32_t _value) {
		values.at(_value)++;
		return *this;
	}
	stStat& operator = (uint32_t _value) {
		value = _value;
		return *this;
	}
};

struct stDKA {
	stStat Kills, Deaths, Assists;
	stDKA() {
		Kills = Deaths = Assists = 0;
	}
};

struct stDamagePlayer {
	uint8_t		weapon = 0,
				part = 0;

	float		damage = 0;
	CPed*		killer = nullptr,
				*victim = nullptr;
	stDamagePlayer() = delete;
	stDamagePlayer(uint8_t _weapon, float _damage, CPed* _killer, CPed* _victim, uint8_t _part) {
		weapon = _weapon, damage = _damage, killer = _killer, victim = _victim, part = _part;
	};
};

struct stElementEvent{
public:
	uint8_t			value;
	string			name,
					sound;
	ImVec4			color;
public:
	stElementEvent(uint8_t _val = 0, string _name = string(), string _sound = string(), ImVec4 _color = ImVec4(1, 1, 1, 1)) {
		value = _val, name = _name, sound = _sound, color = _color;
	}
/*
	auto &GetValue() {
		return value;
	}
	auto& GetName() {
		return name;
	}
	auto& GetSound() {
		return sound;
	}
	auto& GetColor() {
		return color;
	}*/
	void Clear() {
		value = 0, name.clear(), sound.clear(), color = ImVec4(1, 1, 1, 1);
	}
};

struct stPack {
	string			Name;
	font_ptr_t		fontKills, 
					fontDeaths, 
					fontAssists,
					fontMessage;
	vector <stElementEvent>
					KillsEvents,
					DeathsEvents,
					AssistsEvents;
					


	vector <string>	SoundFiles;
};

struct stSetting {
	string			currentPack,
					activeElementMenu;
};

class CKillState {
private:
	key_ptr_t					activeMenu;
	stDKA						KDA;
	vector <stDamagePlayer>		DamagePlayerInfo;
	vector <stPack>				Packs;
	fs::path					pathDir;
	stSetting					cfg;

						
	string						KillString,
								DistString,
								StatusString,
								renderString;
	stElementEvent				new_event_add;

	bool						bMenu;

	void						DetectKillsAndDeaths();


	void						PrepareKillString(CPed* killer, CPed *victim, CPed* assist);
	void						OpenMenu();
	void						DrawStats();

	void						LoadSetting();
	void						SaveSetting();

	void						DrawPacks();
	void						DrawHeader();
	void						DrawFontsSetting();
	void						DrawTable(string label, vector <stElementEvent> &events);
	void						ParsePacks();
	void						SavePacks();
public:
	void Init();
	void DamageEvent(CPed* victim, CEntity* creator, eWeaponType weapon, int damageFactor, ePedPieceTypes pedPiece, int direction);
	void DrawMenu();
	void Draw();
	bool IsOpenMenu();
	void Process();
	void Release();
	bool WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


};

