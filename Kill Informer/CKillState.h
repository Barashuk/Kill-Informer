#pragma once
#include <game_sa/CPed.h>
#include <vector>
#include <string>
#include <map>
#include <bass.h>
#include <imgui.h>
#include "FontsHandler.hpp"
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <magic_enum.hpp>
#include <chrono>
using namespace std;

enum UniqueKillStatus {
	NoMessage = 0,
	PrepareToFight = 1 << 0, //При спавне
	Vengeance = 1 << 1, //Возмездие. Убийство игрока, который убил локального игрока
	Team_Killer = 1 << 2, //Убийство игрока с таким же цветов, как у локального игрока
	First_Blood = 1 << 3, //Первое убийство
	Humilition = 1 << 4, //Убийство с ножа
	Penetration = 1 << 5, //Убийство игрока с любюго из четырех дилдаков в игре
	Assshot = 1 << 6, //Убийство с последним выстрелем в жопу
	Headshot = 1 << 7, // Убийство с последним выстрелом в голову
	//Возможно добавлю в будущем
	//шахид - убийство другого игрока взрывом, при этом смерть локального игрока от этого же взрыва
	//PrepareToFight = 1 << 0,
};

enum EventStyle {
	Message,
	Status
};

enum ZipWorkType {
	AddFileToPath,
	DeleteSound,
	RenameFile,

};

struct stStat {
private:
	int _value = 0;
	vector <int> _values;
public:
	stStat() {
		_values.resize(100, 0);
		_value = 0;
	}
	stStat& operator++(int){
		_value++;
		return *this;
	}
	stStat& operator()(int __value) {
		_values.at(__value)++;
		return *this;
	}
	stStat& operator = (int __value) {
		_value = __value;
		return *this;
	}
	auto value() {
		return _value;
	}
};

struct stDKA {
	stStat Kills, Deaths, Assists, KillStreaks;
	stDKA() {
		Kills = 0, Deaths = 0, Assists = 0, KillStreaks = 0;
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
	int				value,
					style;

	string			name,
					sound;
	ImVec4			color;
	bool			active;
public:	
	stElementEvent(int _val = 0, string _name = string(), string _sound = string(), ImVec4 _color = ImVec4(1, 1, 1, 1), bool _active = true,
		int _style = Message) {
		value = _val, name = _name, sound = _sound, color = _color, active = _active;
		style = _style;
	}
	void Clear() {
		value = 0, name.clear(), sound.clear(), color = ImVec4(1, 1, 1, 1);
		active = true;
		style = 0;
	}
	bool IsValid() {
		return value != -1;
	}
	string GetTextToRender() {
		return string_format("{%08X}%s", toARGB(color), name.data());
	}
};

class stString {
private:
	string Orig, Back;
public:
	stString& operator = (const string& text) {
		Orig = Back = text;
		return *this;
	}
	auto& Value() {
		return Orig;
	}
	void Restore() {
		Orig = Back;
	}
};

struct stPackSetting {
	int				timeDrawKillString = 3,
					timeResetKillStreak = 5,
					timeDisplayStatuses = 4,
					typePosStatuses = 0;

	bool			ShowDistance = true,
					AutoDisplayTime = true;
	ImVec4			colorDistance = ImVec4(1, 1, 1, 1);
	stString		KillString,
					DistString,
					StatusString;
	stPackSetting() {
		KillString = "{cKiller}{nKiller} {cText}killed {cVictim}{nVictim}{cText}.";
		StatusString = "{cKiller}{nKiller}{cText} made a";
		DistString = "{cText}Dist: {dColor}{dist}{cText}m";
	}

};

struct stPack {
	font_ptr_t		fontKills, 
					fontDeaths, 
					fontAssists,
					fontMessage,
					fontStatuses;
	vector <stElementEvent>
					KillsEvents,
					DeathsEvents,
					AssistsEvents,
					StreaksEvents,
					UniqueEvents;
	vector <string>	SoundFiles;
	///Other Setting
	stPackSetting	setting;



	void Copy(const stPack &copy) {
		this->KillsEvents = copy.KillsEvents;
		this->DeathsEvents = copy.DeathsEvents;
		this->AssistsEvents = copy.AssistsEvents;
		this->StreaksEvents = copy.StreaksEvents;
		this->UniqueEvents = copy.UniqueEvents;
		this->SoundFiles = copy.SoundFiles;
		this->setting = copy.setting;
		this->fontKills = FontsHandler::CopyFont(copy.fontKills);
		this->fontDeaths = FontsHandler::CopyFont(copy.fontDeaths);
		this->fontAssists = FontsHandler::CopyFont(copy.fontAssists);
		this->fontMessage = FontsHandler::CopyFont(copy.fontMessage);
		this->fontStatuses = FontsHandler::CopyFont(copy.fontStatuses);
	}
	void DeleteFonts() {
		FontsHandler::DeleteFont(this->fontKills);
		FontsHandler::DeleteFont(this->fontDeaths);
		FontsHandler::DeleteFont(this->fontAssists);
		FontsHandler::DeleteFont(this->fontMessage);
		FontsHandler::DeleteFont(this->fontStatuses);
	}
};

struct stSetting {
	string			currentPack,
					activeElementMenu,
					lastPath;
	int				menuID = 0;
};

using system_clock_t = chrono::system_clock;
using time_point_t = system_clock_t::time_point;

using music_p = pair <string, HSTREAM>;
using status_p = pair<string, time_point_t>;

class CKillState {
private:
	key_ptr_t					activeMenu;
	stDKA						KDA;
	vector <stDamagePlayer>		DamagePlayerInfo;
	map <string, stPack>			Packs;
	fs::path					pathDir;
	stSetting					cfg;

						
	string						renderString,
								lastWhoKillLocalPlayerName;
	stElementEvent				new_event_add;
	vector <music_p>			musicHandles;
	vector <status_p>			StatusesStrings;

	ImGuiFileDialog				fileDialog;

	bool						bOpenMenu = false,
								bOpenMusicDialog = false,
								bFirstBlood = false,
								bPrepareToFight = true;

	time_point_t				timeRenderMessage,
								timeKillStreak;

	void						DetectKillsAndDeaths();
	void						PrepareKillString(CPed* killer, CPed* victim, CPed* assist, int flags);

	void						ReplaceStrings(std::string& str, const std::string& from, const std::string& to);

	stElementEvent				GetRandomEventByID(vector <stElementEvent>& events, int id, bool zero_search = false);



	void						InitFonts();

	int							AddToPlayMusic(string name, bool last_free = false);
	void						AutoPlayMusic();
	void						ValidateMusicFiles();
	void						RenameFileInVector(string search, string name);

	vector <string>				AddToUniqueStatuses(int flags);
	void						AddToRenderStatuses(stElementEvent event);


	
	void						OpenMenu();
	void						DrawStats();
	void						DrawKillString();
	void						DrawStatuses();

	void						LoadSetting();
	void						SaveSetting();

	void						DrawPacks();
	void						DrawHeader();
	void						DrawFontsSetting();
	void						DrawTableEvents(string label, vector <stElementEvent> &events, vector<string> &music);
	void						DrawPages();
	bool						DrawMusicDialog();
	bool						DrawFileExplorer();

	void						ParsePacks();
	void						SavePacks();

	void						WorkWithZip(int mode, string entryName, string file = string());
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

