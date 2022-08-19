#define IMGUI_DEFINE_MATH_OPERATORS
#include "CKillState.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <sampapi/CGame.h>
#include <sstream>
#include <functional>
#include <libzippp/libzippp.h>
#include "IconsFontAwesome6.h"
using namespace libzippp;

namespace R1 = sampapi::v037r1;

void CKillState::OpenMenu() {
	bOpenMenu = !bOpenMenu;
	if (!bOpenMenu) {
		bOpenMusicDialog = false;
	}
	R1::RefGame()->SetCursorMode(bOpenMenu? R1::CURSOR_LOCKCAMANDCONTROL : R1::CURSOR_NONE, bOpenMenu);
}

void CKillState::DrawPacks() {
	static bool changeName = false;
	auto packsList = [this](const string& label) {
		static string nameToChange, backName;
		auto size = Packs.size();
		string _label;
		if (changeName) {
			_label = u8"Введите новое название" + label + "_change_name";
			ImGui::PushItemWidth(200);
			if (ImGui::InputText(_label.c_str(), &nameToChange,
				ImGuiInputTextFlags_NoUndoRedo | ImGuiInputTextFlags_EnterReturnsTrue)) {
				if (nameToChange.empty()) {
					nameToChange = backName;
				}
				else {
					try {
						auto it = find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) { return elem.Name == cfg.currentPack; });
						if (it == Packs.end())
							return;
						auto new_name = utf8_to_cp1251(nameToChange);
						auto name = cfg.currentPack + ".zip";
						auto copy_name = new_name + ".zip";
						auto path = pathDir / "Packs";
						fs::copy(path / name, path / copy_name);
						fs::remove(path / name);
						it->Name = new_name;
						cfg.currentPack = new_name;
						changeName = false;
					}
					catch (const std::exception& e) {
						Console::Info("%s %s", e.what(), __FUNCTION__);
					}
				}
			}
			ImGui::PopItemWidth();
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
				&& ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly)) {
				changeName = false;
			}
		}
		else {
			_label = label + "_arrow_plus";
			if (ImGui::ArrowButton(_label.c_str(), ImGuiDir_::ImGuiDir_Left)) {
				int d = distance(Packs.begin(), find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) {return elem.Name == cfg.currentPack; }));
				if (size >= 2) {
					d--;
					if (d < 0)
						d = size - 1;
					cfg.currentPack = Packs.at(d).Name;
				}
			}
			ImGui::SameLine();
			auto namePack = cp1251_to_utf8(cfg.currentPack);
			ImGui::PushItemWidth(200);
			_label = label + "_combo";
			if (ImGui::BeginCombo(_label.data(), namePack.data())) {
				for (size_t i = 0; i < Packs.size(); i++) {
					auto text = cp1251_to_utf8(Packs.at(i).Name);
					if (ImGui::Selectable(text.data(), text.data() == namePack)) {
						namePack = text;
						cfg.currentPack = utf8_to_cp1251(namePack);
					}
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
				&& ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly)) {
				changeName = true;
				nameToChange = cp1251_to_utf8(cfg.currentPack);
				backName = cfg.currentPack;
			}
			_label = _label + "_arrow_left";
			if (ImGui::ArrowButton(_label.c_str(), ImGuiDir_Right)) {
				size_t d = distance(Packs.begin(), find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) {return elem.Name == cfg.currentPack; }));
				if (size >= 2) {
					d++;
					if (d > size - 1)
						d = 0;
					cfg.currentPack = Packs.at(d).Name;
				}
			}
		}
	};
	packsList(u8"##current_pack_draw");
	if (changeName)
		return;
	ImGui::SameLine();
	if (ImGui::Button(u8"Копировать")) {
		try {
			auto name = cfg.currentPack + ".zip";
			auto new_name = cfg.currentPack + " - copy.zip";
			auto path = pathDir / "Packs";
			if (fs::exists(path / new_name)) {
				Console::Info("Файл существует");
				return;
			}
			fs::copy_file(path / name, path / new_name);
			auto it = find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) {return elem.Name == cfg.currentPack; });
			if (it == Packs.end())
				return;
			stPack new_pack = *it;
			new_name = new_name.substr(0, new_name.find_last_of("."));
			new_pack.Name = new_name;
			Packs.push_back(new_pack);
			cfg.currentPack = new_name;
		}
		catch (const std::exception&e) {
			Console::Info(" %s %s", e.what(), __FUNCTION__);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(u8"Удалить")) {
		if (Packs.size() < 2) {
			Console::Add("Должен существовать хотя бы один пак");
			return;
		}
		auto name = cfg.currentPack + ".zip";
		auto path = pathDir / "Packs";
		fs::remove(path / name);
		Packs.erase(remove_if(Packs.begin(), Packs.end(), [&](const stPack &elem) {return elem.Name == cfg.currentPack; }), Packs.end());
		cfg.currentPack = Packs.back().Name;
	}	
}

void CKillState::DrawMenu() {
	if (!bOpenMenu)
		return;
	auto& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize({ 800, 600 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
	if (ImGui::Begin("Kill Informer", &bOpenMenu, flags)) {
		
		activeMenu->Draw(u8"Активация меню");
		if (Packs.empty()) {
			ImGui::Text(u8"Не найден не один пак\nОтсканирууйте папку с паками");
			if (ImGui::Button(u8"Сканировать")) {
				ParsePacks();
			}
			ImGui::End();
			return;
		}
		DrawPacks();
		DrawHeader();
		ImGui::End();
	}
}

void CKillState::DrawFontsSetting() {
	auto it = find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) {return elem.Name == cfg.currentPack; });
	if (it == Packs.end())
		return;
	using font_draw = pair <string, function<void(void)>>;
	static font_draw current;
	vector <font_draw> buttons = {
		{u8"Убийства##font_draw_kills", [&]() { it->fontKills->Setting(); }},
		{u8"Смерти##font_draw_deaths", [&]() { it->fontDeaths->Setting(); }},
		{u8"Ассисты##font_draw_assist", [&]() { it->fontAssists->Setting(); }},
		{u8"Сообщения##font_draw_messages", [&]() { it->fontMessage->Setting(); }},
	};
	ImGui::BeginGroup();
	for (auto &button : buttons) {
		if (ImGui::Button(button.first.c_str(), ImVec2(150, 40))) {
			current = button;
		}
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	if (current.second) {
		current.second();
	}
	else {
		ImGui::Text(u8"Выберите шрифт для редактирования");
	}
	ImGui::EndGroup();

}

void CKillState::DrawHeader() {
	static vector <string> buttons {
		u8"Шрифты##tab_bar_fonts",
		u8"Убийства##tab_bar_kills",
		u8"Серия убийств##tab_bar_killsstreaks",
		u8"Смерти##tab_bar_deaths",
		u8"Ассисты##tab_bar_assists",
		u8"События##tab_bar_events",
		u8"Прочее##tab_bar_other",
	};
	auto curPack = find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) {return elem.Name == cfg.currentPack; });
	auto& io = ImGui::GetStyle();	
	ImVec2 Size({ (800 - io.WindowPadding.x - io.ItemSpacing.x * buttons.size()) / (float)buttons.size() , 25});
	for (size_t i = 0; i < buttons.size(); i++) {
		if (ImGui::Button(buttons.at(i).c_str(), Size)) {
			cfg.activeElementMenu = buttons.at(i);
		}
		if(i < buttons.size() - 1)
			ImGui::SameLine();
	}
	ImGui::Separator();
	if (cfg.activeElementMenu == u8"Шрифты##tab_bar_fonts") {
		DrawFontsSetting();
	}
	else if (cfg.activeElementMenu == u8"Убийства##tab_bar_kills") {
		DrawTableEvents("##kill_table", curPack->KillsEvents, curPack->SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"Серия убийств##tab_bar_killsstreaks") {
		DrawTableEvents("##killstreaks_table", curPack->StreaksEvents, curPack->SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"Смерти##tab_bar_deaths") {
		DrawTableEvents("##deaths_table", curPack->DeathsEvents, curPack->SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"Ассисты##tab_bar_assists") {
		DrawTableEvents("##assist_table", curPack->AssistsEvents, curPack->SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"События##tab_bar_events") {
		DrawTableEvents("##uniqui_table", curPack->UniqueEvents, curPack->SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"Прочее##tab_bar_other") {
		
	}
}

void CKillState::DrawTableEvents(string label, vector<stElementEvent>& events, vector <string> &music) {
	auto columnInputInt = [](string label, int& value, int id = -1) -> bool {
		bool result = false;
		string text;
		ImVec2 size(25, 0);
		ImGui::TableNextColumn();
		if (id > -1) {
			text = "X" + label + "_del_" + to_string(id);
			ImGui::PushItemWidth(size.x);
			if (ImGui::Button(text.c_str(), size)) {
				result = true;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}
		text = label + "_id_" + to_string(id);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		if (ImGui::InputInt(text.c_str(), &value, 1, 10, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue)) {
			if (value < 0)
				value = 0;
			if (value > 255)
				value = 255;
		}
		ImGui::PopItemWidth();
		return result;
	};
	auto columnInputBool = [](string label, bool& value) {
		ImGui::TableNextColumn();
		label = label + "_check";
		ImGui::Checkbox(label.c_str(), &value);

	};
	auto columnInputText = [](string label, string& value) {
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		label = label + "_name";
		ImGui::InputText(label.c_str(), &value);		
		ImGui::PopItemWidth();
	};
	auto columnInputSound = [&](string label, string& value, vector <string> &music, int elem = -1) {
		//column size 187
		float size = 187;
		ImVec2 label_size, _size;
		auto& style = ImGui::GetStyle();
		ImGui::TableNextColumn();
		/*if (elem != -1)
			return;*/
		ImGui::BeginGroup();
		string text = label + "_one";
		label_size = ImGui::CalcTextSize(text.c_str(), NULL, true);
		 _size = ImGui::CalcItemSize(ImVec2(0,0), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
		ImGui::BeginChild(text.c_str(), ImVec2(size * 0.80f, _size.y), false);
		label = label + "_music_combo";
		ImGui::PushItemWidth(size * 0.8f - 2.0f);
		if (music.empty()) {
			ImGui::Text(u8"Добавьте звуки");
		}
		else {
			string _value;
			if (value.empty()) {
				_value = u8"Выберите звук";
			}
			else {
				_value = value;
			}
			if (ImGui::BeginCombo(label.c_str(), _value.c_str())) {
				for (size_t i = 0; i < music.size(); i++) {
					auto text = music.at(i);
					if (ImGui::Selectable(text.data(), text.data() == _value)) {
						value = text;
					}
				}
				ImGui::EndCombo();
			}
		}
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::EndGroup();
		ImGui::SameLine(0, 2);
		ImGui::BeginGroup();
		 text = label + "_two";
		label_size = ImGui::CalcTextSize(text.c_str(), NULL, true);
		_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
		ImGui::BeginChild(text.c_str(), ImVec2(size * 0.20f, _size.y), false);
		string b = string(ICON_FA_MUSIC) + u8"##menu_open";
		if (ImGui::Button(b.c_str(), ImVec2(0, _size.y))) {
			ImGui::OpenPopup(u8"Cписок звуков");
			bOpenMusicDialog = true;
			
		}
		if (DrawMusicDialog()) {
			value = music.back();
		}
		ImGui::EndChild();
		ImGui::EndGroup();
	};
	auto columnInputColor = [](string label, ImVec4& value) {
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		label = label + "_color";
		ImGui::ColorEdit4(label.c_str(), (float*)&value,
			ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoDragDrop |
			ImGuiColorEditFlags_NoTooltip
		);
		ImGui::PopItemWidth();
		
	};
	bool uTable = label == "##uniqui_table";
	static auto flagsTable = ImGuiTableFlags_None | ImGuiTableFlags_Borders;
	ImGui::Separator();
	if (ImGui::BeginTable(label.c_str(), 5, flagsTable)) {
		ImGui::TableSetupColumn(u8"Ид", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn(u8"ВКЛ", ImGuiTableColumnFlags_WidthFixed, 50 );
		ImGui::TableSetupColumn(u8"Имя", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn(u8"Звук", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn(u8"Цвет", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();
		ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeightWithSpacing() * 1.5f);
		//---------------------------
		columnInputInt(label, new_event_add.value);
		//---------------------------
		columnInputBool(label, new_event_add.active);
		//---------------------------
		columnInputText(label, new_event_add.name);
		//---------------------------
		columnInputSound(label, new_event_add.sound, music);
		//---------------------------
		columnInputColor(label, new_event_add.color);
		ImGui::EndTable();
		if (ImGui::Button(u8"Добавить", ImVec2(ImGui::GetWindowWidth(), 0))) {
			if (new_event_add.name.empty()) {
				Console::Add("Название не может быть пустым");
			}
			else {
				events.push_back(new_event_add);
				new_event_add.Clear();
			}
		}
		
	}
	ImGui::Separator();
	if (events.empty()) {
		ImGui::Text(u8"Список событий пуст");
		return;
	}
	else {
		label = label + "_list";
		if (ImGui::BeginTable(label.c_str(), 5, flagsTable)) {
			static auto flagColumn = ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoReorder;
			ImGui::TableSetupColumn(u8"Ид", flagColumn);
			ImGui::TableSetupColumn(u8"ВКЛ", ImGuiTableColumnFlags_WidthFixed, 50);
			ImGui::TableSetupColumn(u8"Имя", flagColumn);
			ImGui::TableSetupColumn(u8"Звук", flagColumn);
			ImGui::TableSetupColumn(u8"Цвет", flagColumn);
			ImGui::TableHeadersRow();
			for (size_t id = 0; id < events.size(); id++) {
				auto& event = events.at(id);
				ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeightWithSpacing() * 1.5f);
				//---------------------------
				if (columnInputInt(label, event.value, id)) {
					events.erase(remove_if(events.begin(), events.end(), [&](const stElementEvent& _event) {
						return _event.value == event.value &&
							_event.name == event.name &&
							_event.sound == event.sound &&
							_event.active == event.active
							; }
					), events.end());
				}
				//---------------------------
				columnInputBool(label + to_string(id), event.active);
				//---------------------------
				columnInputText(label + to_string(id), event.name);
				//---------------------------
				columnInputSound(label + to_string(id), event.sound, music, id);
				//---------------------------
				columnInputColor(label + to_string(id), event.color);
			}

			ImGui::EndTable();
		}
	}	
}

bool CKillState::DrawMusicDialog() {
	ImVec2 center = ImGui::GetMainViewport()->GetCenter(), size = { 400, 300 };
	ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	string delete_name;
	if (ImGui::BeginPopupModal(u8"Cписок звуков", &bOpenMusicDialog, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
		string b = string(ICON_FA_MUSIC) + u8" добавить звуки##dialog_open_menu";
		if (ImGui::Button(b.c_str())) {
			ImGuiFileDialogFlags flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_DontShowHiddenFiles | ImGuiFileDialogFlags_DisableCreateDirectoryButton;
			const char* filters = u8"Звуковые файлы{.mp3,.wav,.ogg}";
			fileDialog.OpenDialog("SelectMusicFile", ICON_IGFD_FOLDER_OPEN u8" Выберите папку", filters, lastPath, 1, nullptr, flags);
			
		}
		if (DrawFileExplorer()) {
			bOpenMusicDialog = false;
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return true;
		}
		auto curPack = find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) {return elem.Name == cfg.currentPack; });
		auto& music_list = curPack->SoundFiles;
		for (size_t id = 0; id < music_list.size(); id++) {
			auto play = string(ICON_FA_PLAY) + "##button_play_music_id_" + to_string(id);
			auto button = string(ICON_FA_TRASH) + "##button_delete_music_id_" + to_string(id);
			auto& name = music_list.at(id);
			if (ImGui::Button(button.c_str(), ImVec2(30, 0))) {
				delete_name = name;
			}
			ImGui::SameLine();
			if (ImGui::Button(play.c_str(), ImVec2(30, 0))) {
				AddToPlayMusic(name, cfg.currentPack, true);
			}
			ImGui::SameLine();
			ImGui::Text(music_list.at(id).c_str());
		}
		if (!delete_name.empty()) {
			music_list.erase(remove_if(music_list.begin(), music_list.end(), [&](const string& elem) {
				return elem == delete_name;
				}), music_list.end());
			auto path = pathDir / "Packs" / (cfg.currentPack + ".zip");
			ZipArchive zf(path.u8string());
			zf.open(ZipArchive::Write);
			zf.deleteEntry("Sounds/" + delete_name);
			zf.close();
			delete_name.clear();
		}
		ImGui::EndPopup();
	}
	return false;
}

bool CKillState::DrawFileExplorer() {
	bool result = false;
	auto curPack = find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) {return elem.Name == cfg.currentPack; });
	auto loc = std::setlocale(LC_ALL, ".UTF8");
	if (fileDialog.Display("SelectMusicFile", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize, ImVec2(700, 400))) {
		if (fileDialog.IsOk()) {
			lastPath = fileDialog.GetFilePathName();
			fs::directory_entry file(lastPath);
			auto filename = file.path().filename().string();
			auto& music = curPack->SoundFiles;
			if (find(music.begin(), music.end(), filename) == music.end()) {
				auto p = pathDir / "Packs" / (cp1251_to_utf8(cfg.currentPack) + ".zip");
				ZipArchive zf(p.u8string());
				zf.open(ZipArchive::Write);
				fs::path u8file = string("Sounds/") + filename;
				zf.addFile(u8file.u8string(), file.path().u8string());
				zf.close();
				music.push_back(filename);
				sort(music.begin(), music.end());
				result = true;
			}
			else {
				Console::Add("Текущий файл уже существует");
			}
		}
		fileDialog.Close();
	}
	loc = std::setlocale(LC_ALL, "");
	return result;
}

void CKillState::DrawStats() {
	auto it = find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) {return elem.Name == cfg.currentPack; });
	if (it == Packs.end())
		return;
	it->fontKills->Print(string_format("Kills: %d", KDA.Kills));
	it->fontDeaths->Print(string_format("Deaths: %d", KDA.Deaths));
	it->fontAssists->Print(string_format("Assists: %d", KDA.Assists));
}

void CKillState::Draw() {
	DrawStats();
}