#pragma once


#include <game_sa/CPed.h>
#include <vector>
#include <string>
#include <map>
#include <bass.h>
#include "FontsHandler.hpp"
#include <ImGuiFileDialog/ImGuiFileDialog.h>

using namespace std;

struct stStat {
private:
	uint32_t value = 0;
	vector <uint32_t> values;
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
		Kills =0, Deaths = 0, Assists = 0;
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
	int				value;
	string			name,
					sound;
	ImVec4			color;
public:
	stElementEvent(int _val = 0, string _name = string(), string _sound = string(), ImVec4 _color = ImVec4(1, 1, 1, 1)) {
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

using music_p = pair <string, HSTREAM>;

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

	ImGuiFileDialog				fileDialog;
	string						lastPath;
	int							elemMusic = -1;
	vector <music_p>			musicHandles;




	bool						bOpenMenu = false,
								bOpenMusicDialog = false;

	void						DetectKillsAndDeaths();
	void						PrepareKillString(CPed* killer, CPed* victim, CPed* assist);



	void						InitFonts();

	void						AddToPlayMusic(string name, string pack_name, bool last_free = false);
	void						AutoPlayMusic();


	
	void						OpenMenu();
	void						DrawStats();

	void						LoadSetting();
	void						SaveSetting();

	void						DrawPacks();
	void						DrawHeader();
	void						DrawFontsSetting();
	void						DrawTableEvents(string label, vector <stElementEvent> &events, vector<string> &music);

	bool						DrawMusicDialog();
	bool						DrawFileExplorer();

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

