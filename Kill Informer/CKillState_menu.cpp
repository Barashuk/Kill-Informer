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
		auto search_iter = [this](const string& text) -> map <string, stPack>::iterator {
			map <string, stPack>::iterator iter;
			for (auto it = Packs.begin(); it != Packs.end(); it++) {
				if (it->first == text) {
					iter = it;
				}
			}
			return iter;
		};
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
						auto new_name = utf8_to_cp1251(nameToChange);
						if (new_name != cfg.currentPack) {
							auto &it = Packs.at(cfg.currentPack);
							auto name = cfg.currentPack + ".zip";
							auto copy_name = new_name + ".zip";
							auto path = pathDir / "Packs";
							fs::copy(path / name, path / copy_name);
							fs::remove(path / name);
							stPack new_pack;
							new_pack.Copy(it);
							it.DeleteFonts();
							Packs.insert(pair<string, stPack>(new_name, new_pack));
							Packs.erase(cfg.currentPack);
							cfg.currentPack = new_name;
							changeName = false;
						}
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
			auto spasing = ImGui::GetStyle().ItemInnerSpacing;
			if (ImGui::ArrowButton(_label.c_str(), ImGuiDir_::ImGuiDir_Left)) {
				auto it  = search_iter(cfg.currentPack);
				cfg.currentPack = std::prev(it == Packs.begin() ? Packs.end() : it)->first;
			}
			ImGui::SameLine(0, spasing.x);
			auto namePack = cp1251_to_utf8(cfg.currentPack);
			ImGui::PushItemWidth(200);
			_label = label + "_combo";
			if (ImGui::BeginCombo(_label.data(), namePack.data())) {
				for (auto &elem : Packs) {
					auto text = cp1251_to_utf8(elem.first);
					if (ImGui::Selectable(text.data(), text.data() == namePack)) {
						namePack = text;
						cfg.currentPack = utf8_to_cp1251(namePack);
					}
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
				&& ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly)) {
				changeName = true;
				nameToChange = cp1251_to_utf8(cfg.currentPack);
				backName = cfg.currentPack;
			}
			_label = _label + "_arrow_left";
			ImGui::SameLine(0, spasing.x);
			if (ImGui::ArrowButton(_label.c_str(), ImGuiDir_Right)) {
				auto it = std::next(search_iter(cfg.currentPack));
				cfg.currentPack = it == Packs.end() ? Packs.begin()->first : it->first;
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
			new_name = new_name.substr(0, new_name.find_last_of("."));
			stPack new_pack;
			new_pack.Copy(Packs.at(cfg.currentPack));
			Packs.insert(pair<string, stPack>(new_name, new_pack));
			cfg.currentPack = new_name;
		}
		catch (const std::exception&e) {
			Console::Info("%s %s", e.what(), __FUNCTION__);
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
		Packs.at(cfg.currentPack).DeleteFonts();
		Packs.erase(cfg.currentPack);
		cfg.currentPack = std::prev(Packs.end())->first;	
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

void CKillState::DrawHeader() {
	static vector <string> buttons{
		u8"Шрифты##tab_bar_fonts",
		u8"Убийства##tab_bar_kills",
		u8"Серия убийств##tab_bar_killsstreaks",
		u8"Смерти##tab_bar_deaths",
		u8"Ассисты##tab_bar_assists",
		u8"Уникальные##tab_bar_events",
		u8"Прочее##tab_bar_other",
	};
	auto &curPack = Packs.at(cfg.currentPack);
	auto& io = ImGui::GetStyle();
	auto color = io.Colors[ImGuiCol_ButtonActive];
	ImVec2 Size({ (800 - io.WindowPadding.x - io.ItemSpacing.x * buttons.size()) / (float)buttons.size() , 25 });
	for (size_t i = 0; i < buttons.size(); i++) {
		string text = buttons.at(i);
		auto res = cfg.activeElementMenu == text;
		if (res) { ImGui::PushStyleColor(ImGuiCol_Button, color); }
		if (ImGui::Button(text.c_str(), Size)) { cfg.activeElementMenu = text; }
		if (res) { ImGui::PopStyleColor(); }

		if (i < buttons.size() - 1)
			ImGui::SameLine();
	}
	ImGui::Separator();
	DrawPages();
}

void CKillState::DrawPages() {
	auto &curPack = Packs.at(cfg.currentPack);
	auto renderHint = [](const string& hint) {
		if (hint.empty())
			return;
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(hint.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	};
	auto inputInt = [&](string label, int& value, int min = 0, int max = 255, string hint = string()) {
		ImGui::PushItemWidth(150);
		if (ImGui::InputInt(label.c_str(), &value, 1, 10, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue)) {
			if (value < min)
				value = min;
			if (value > max)
				value = max;
		}
		ImGui::PopItemWidth();
		renderHint(hint);
	};
	if (cfg.activeElementMenu == u8"Шрифты##tab_bar_fonts") {
		DrawFontsSetting();
	}
	else if (cfg.activeElementMenu == u8"Убийства##tab_bar_kills") {
		DrawTableEvents("##kill_table", curPack.KillsEvents, curPack.SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"Серия убийств##tab_bar_killsstreaks") {
		DrawTableEvents("##killstreaks_table", curPack.StreaksEvents, curPack.SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"Смерти##tab_bar_deaths") {
		DrawTableEvents("##deaths_table", curPack.DeathsEvents, curPack.SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"Ассисты##tab_bar_assists") {
		DrawTableEvents("##assist_table", curPack.AssistsEvents, curPack.SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"Уникальные##tab_bar_events") {
		DrawTableEvents("##uniqui_table", curPack.UniqueEvents, curPack.SoundFiles);
	}
	else if (cfg.activeElementMenu == u8"Прочее##tab_bar_other") {
		auto& setting = curPack.setting;
		inputInt(u8"Время отобржаения строки убийства##time_draw_kill_string", setting.timeDrawKillString, 1, 10, u8"в секундах");
		inputInt(u8"Время сброса счетчика убийств##time_reset_kill_streak", setting.timeResetKillStreak, 1, 10, u8"в секундах");
		ImGui::Checkbox(u8"Отображать дистанцию при убийстве##show_distance", &setting.ShowDistance);
		if (setting.ShowDistance) {
			ImGui::PushItemWidth(200);
			ImGui::ColorEdit4(u8"Цвет дистанции##color_distance", (float*)&setting.colorDistance, ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoTooltip );
			ImGui::PopItemWidth();
		}
		ImGui::Checkbox(u8"Автоматическое время отображения статусов##auto_display_time", &setting.AutoDisplayTime);
		if (!setting.AutoDisplayTime) {
			inputInt(u8"Время отображения статусов##timeDisplayStatuses", setting.timeDisplayStatuses , 1, 10, u8"в секундах");
		}
		auto Draw = [&](string label, stString &value, string hint = string(), float width = 400) {
			ImGui::PushItemWidth(width);
			ImGui::InputText(label.data(), &value.Value());
			ImGui::PopItemWidth();
			renderHint(hint);
			ImGui::SameLine();
			if (ImGui::Button("Восстановить")) {
				value.Restore();
			}
		};
		Draw("##killstring_edit", setting.KillString, 
			u8"{cKiller} - цвет убийцы\n"
			u8"{nKiller} - имя убийцы\n"
			u8"{cVictim} - цвет жертвы\n"
			u8"{nVictim} - имя жертвы\n"
			u8"{cText} - общий цвет текста");
		Draw("##diststring_edit", setting.DistString,
			u8"{cText} - общий цвет текста"
			u8"{dist} - дистанция"
			u8"{dColor} - цвет дистанции"
		);
		Draw("##statusstring_edit", setting.StatusString,
			u8"{cKiller} - цвет убийцы\n"
			u8"{nKiller} - имя убийцы\n"
			u8"{cText} - общий цвет текста");
		

	}
}

void CKillState::DrawFontsSetting() {
	static vector <string> posStatuses = {
		u8"Левый верхний угол",
		u8"Сверху по центру",
		u8"Правый верхний угол",

		u8"Слева по центру",
		u8"По центру экрана",
		u8"Справа по центру",

		u8"Левый нижний угол",
		u8"Снизу по центру",
		u8"Правый нижний угол",
	};
	static string activeElement;
	auto color = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
	auto &it = Packs.at(cfg.currentPack);
	using font_draw = pair <string, function<void(void)>>;
	static font_draw current;
	vector <font_draw> buttons = {
		{u8"Убийства##font_draw_kills", [&]() { it.fontKills->Setting(); }},
		{u8"Смерти##font_draw_deaths", [&]() { it.fontDeaths->Setting(); }},
		{u8"Ассисты##font_draw_assist", [&]() { it.fontAssists->Setting(); }},
		{u8"Сообщения##font_draw_messages", [&]() { it.fontMessage->Setting(); }},
		{u8"Статусы##font_draw_statuses", [&]() {
			it.fontStatuses->Setting(false, false, true);
			string text;
			text = posStatuses.at(it.setting.typePosStatuses);
			ImGui::PushItemWidth(250);
			if (ImGui::BeginCombo(u8"Позиция##status_position", text.c_str())) {
				for (size_t i = 0; i < posStatuses.size(); i++) {
					auto& select = posStatuses.at(i);
					if (ImGui::Selectable(select.c_str(), select == text)) {
						it.setting.typePosStatuses = i;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::EndGroup();
		}}
	};
	ImGui::BeginGroup();
	for (auto &button : buttons) {
		auto res = button.first == activeElement;
		if (res) { ImGui::PushStyleColor(ImGuiCol_Button, color); }
		if (ImGui::Button(button.first.c_str(), ImVec2(150, 40))) {
			current = button;
			activeElement = current.first;
		}
		if (res) { ImGui::PopStyleColor(); }
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

void CKillState::DrawTableEvents(string label, vector<stElementEvent>& events, vector <string> &music) {
	auto columnInputUnique = [](string label, int& value, int id = -1) -> int {
		int result = 0;
		string text, current_value;
		ImVec2 size(25, 0);
		ImGui::TableNextColumn();
		if (id > -1) {
			text = ICON_FA_TRASH + label + "_del_" + to_string(id);
			ImGui::PushItemWidth(size.x);
			if (ImGui::Button(text.c_str(), size)) {
				result = 1;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			text = ICON_FA_CLONE + label + "_copy_" + to_string(id);
			ImGui::PushItemWidth(size.x);
			if (ImGui::Button(text.c_str(), size)) {
				result = 2;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}
		text = label + "_id_" + to_string(id);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		constexpr auto entries = magic_enum::enum_entries<UniqueKillStatus>();
		current_value = magic_enum::enum_name(static_cast<UniqueKillStatus>(value));
		if (ImGui::BeginCombo(text.c_str(), current_value.c_str())) {
			for (const auto it : entries) {
				if (ImGui::Selectable(it.second.data(), it.second == current_value)) {
					value = static_cast<int>(it.first);
					result = 3;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		return result;
	};
	auto columnInputInt = [](string label, int& value, int id = -1) -> int {
		int result = 0;
		string text;
		ImVec2 size(25, 0);
		ImGui::TableNextColumn();
		if (id > -1) {
			text = ICON_FA_TRASH + label + "_del_" + to_string(id);
			ImGui::PushItemWidth(size.x);
			if (ImGui::Button(text.c_str(), size)) {
				result = 1;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			text = ICON_FA_CLONE + label + "_copy_" + to_string(id);
			ImGui::PushItemWidth(size.x);
			if (ImGui::Button(text.c_str(), size)) {
				result = 2;
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
		ImVec2 label_size, _size;
		auto& style = ImGui::GetStyle();
		ImGui::TableNextColumn();
		float size = ImGui::GetColumnWidth();
		ImGui::BeginGroup();
		string text = label + "_one";
		label_size = ImGui::CalcTextSize(text.c_str(), NULL, true);
		_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.ItemInnerSpacing.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
		ImGui::BeginChild(text.c_str(), ImVec2(size * 0.80f + style.ItemInnerSpacing.x, _size.y), false);
		label = label + "_music_combo";
		ImGui::PushItemWidth(size * 0.8f);
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
		ImGui::SameLine(0, style.ItemInnerSpacing.x);
		ImGui::BeginGroup();
		text = label + "_two";
		label_size = ImGui::CalcTextSize(text.c_str(), NULL, true);
		_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.ItemInnerSpacing.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
		ImGui::BeginChild(text.c_str(), ImVec2(size * 0.20f, _size.y), false);
		string b = string(ICON_FA_MUSIC) + u8"##menu_open";
		if (ImGui::Button(b.c_str(), ImVec2(0, _size.y))) {
			ImGui::OpenPopup(u8"Cписок звуков");
			bOpenMusicDialog = true;

		}
		if (DrawMusicDialog()) {
			value = music.back();
			sort(music.begin(), music.end());
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
			ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs |
			ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoSidePreview 
		);
	};
	auto columnInputStyle = [](string label, int &value, bool prepare = false) -> bool {
		bool res = false;
		ImGui::TableNextColumn();
		label = label + "_style";
		static vector <string> values{
			u8"Сообщения",
			u8"Статусы",
		};
		if (prepare) {
			ImGui::BeginDisabled();
		}
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		string text = values.at(value);
		if (ImGui::BeginCombo(label.c_str(), text.c_str())) {
			for (size_t i = 0; i < values.size(); i++) {
				auto& elem = values.at(i);
				if (ImGui::Selectable(elem.c_str(), elem == text)) {
					value = i;
					res = true;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		if (prepare) {
			ImGui::EndDisabled();
		}
		return res;
	};
	bool uTable = label == "##uniqui_table";
	static auto flagsTable = ImGuiTableFlags_None | ImGuiTableFlags_Borders;
	ImGui::Separator();
	if (ImGui::BeginTable(label.c_str(), 6, flagsTable)) {
		ImGui::TableSetupColumn(u8"Ид", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn(u8"ВКЛ", ImGuiTableColumnFlags_WidthFixed, 40 );
		ImGui::TableSetupColumn(u8"Имя", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn(u8"Звук", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn(u8"Стиль отображения", ImGuiTableColumnFlags_WidthFixed, 120);
		ImGui::TableSetupColumn(u8"Цвет", ImGuiTableColumnFlags_WidthFixed, 40);
		ImGui::TableHeadersRow();
		ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeightWithSpacing() * 1.5f);
		//---------------------------
		if (uTable) {
			static int lastStyle = -1;
			if (columnInputUnique(label, new_event_add.value) == 3) {
				if (new_event_add.value == PrepareToFight) {
					lastStyle = new_event_add.style;
					new_event_add.style = Status;
				}
				else {
					new_event_add.style = lastStyle;
				}
			}

		}
		else {
			columnInputInt(label, new_event_add.value);
		}
		//---------------------------
		columnInputBool(label, new_event_add.active);
		//---------------------------
		columnInputText(label, new_event_add.name);
		//---------------------------
		columnInputSound(label, new_event_add.sound, music);
		//---------------------------
		columnInputStyle(label, new_event_add.style, (uTable && new_event_add.value == PrepareToFight));
		//---------------------------
		columnInputColor(label, new_event_add.color);
		ImGui::EndTable();
		if (ImGui::Button(u8"Добавить", ImVec2(ImGui::GetWindowWidth(), 0))) {
			if (new_event_add.name.empty()) {
				Console::Add("Название не может быть пустым");
			}
			else {
				if (new_event_add.value == PrepareToFight) {
					new_event_add.style = Status;
				}
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
		ImGui::BeginChild("##table_child");
		if (ImGui::BeginTable(label.c_str(), 6, flagsTable)) {
			static auto flagColumn = ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoReorder;
			ImGui::TableSetupColumn(u8"Ид", flagColumn);
			ImGui::TableSetupColumn(u8"ВКЛ", ImGuiTableColumnFlags_WidthFixed, 40);
			ImGui::TableSetupColumn(u8"Имя", flagColumn);
			ImGui::TableSetupColumn(u8"Звук", flagColumn);
			ImGui::TableSetupColumn(u8"Стиль отображения", ImGuiTableColumnFlags_WidthFixed, 120);
			ImGui::TableSetupColumn(u8"Цвет", ImGuiTableColumnFlags_WidthFixed, 40);
			ImGui::TableHeadersRow();
			for (size_t id = 0; id < events.size(); id++) {
				auto& event = events.at(id);
				ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeightWithSpacing() * 1.5f);
				//---------------------------
				int res = 0;
				if (uTable) {
					res = columnInputUnique(label, event.value, id);
				}
				else {
					res = columnInputInt(label, event.value, id);
				}
				if (res == 1) {
					events.erase(events.begin() + id);
					continue;
				}
				if (res == 2) {
					events.push_back(event);
				}
				//---------------------------
				columnInputBool(label + to_string(id), event.active);
				//---------------------------
				columnInputText(label + to_string(id), event.name);
				//---------------------------
				columnInputSound(label + to_string(id), event.sound, music, id);
				//---------------------------
				columnInputStyle(label + to_string(id), event.style, (uTable && event.value == PrepareToFight));
				//---------------------------
				columnInputColor(label + to_string(id), event.color);
			}

			ImGui::EndTable();
		}
		ImGui::EndChild();
	}	
}

bool CKillState::DrawMusicDialog() {
	static int EditID = -1;
	static string lastName, buffer, ext;
	ImVec2 center = ImGui::GetMainViewport()->GetCenter(), size = { 400, 300 };
	ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	string delete_name;
	if (ImGui::BeginPopupModal(u8"Cписок звуков", &bOpenMusicDialog, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
		string b = string(ICON_FA_MUSIC) + u8" добавить звуки##dialog_open_menu";
		if (ImGui::Button(b.c_str())) {
			ImGuiFileDialogFlags flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_DontShowHiddenFiles | ImGuiFileDialogFlags_DisableCreateDirectoryButton;
			const char* filters = u8"Звуковые файлы{.mp3,.wav,.ogg}";
			if (!fs::exists(cfg.lastPath))
				cfg.lastPath = fs::current_path().string();
			fileDialog.OpenDialog("SelectMusicFile", ICON_IGFD_FOLDER_OPEN u8" Выберите папку", filters, cfg.lastPath, 1, nullptr, flags);
		}
		if (DrawFileExplorer()) {
			bOpenMusicDialog = false;
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return true;
		}
		auto& curPack = Packs.at(cfg.currentPack);
		auto& music_list = curPack.SoundFiles;
		ImGui::BeginChild("##music_list_child");
		for (size_t id = 0; id < music_list.size(); id++) {
			auto play = string(ICON_FA_PLAY) + "##button_play_music_id_" + to_string(id);
			auto button = string(ICON_FA_TRASH) + "##button_delete_music_id_" + to_string(id);
			auto& name = music_list.at(id);
			if (ImGui::Button(button.c_str(), ImVec2(30, 0))) {
				delete_name = name;
			}
			ImGui::SameLine();
			if (ImGui::Button(play.c_str(), ImVec2(30, 0))) {
				AddToPlayMusic(name, true);
			}
			ImGui::SameLine();
			ImGui::PushID(id);
			if (EditID != -1 && EditID == id) {
				if (ImGui::InputText("##rename_file_in_zip", &buffer, ImGuiInputTextFlags_EnterReturnsTrue )) {			
					EditID = -1;
					music_list.at(id) = buffer + "." + ext;


					WorkWithZip(RenameFile, lastName, music_list.at(id));
					RenameFileInVector(lastName, music_list.at(id));


				}
				if (ImGui::IsItemDeactivated() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
					EditID = -1;
					music_list.at(id) = lastName ;
				}
			}
			else {
				ImGui::Text(music_list.at(id).c_str());
			}
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemClicked()) {
				EditID = id;
				lastName = music_list.at(id);
				buffer = lastName.substr(0, lastName.find_last_of("."));
				ext = lastName.substr(lastName.find_last_of(".") + 1);
			}
			ImGui::PopID();			
		}
		if (!delete_name.empty()) {
			music_list.erase(remove_if(music_list.begin(), music_list.end(), [&](const string& elem) {
				return elem == delete_name;
				}), music_list.end());
			WorkWithZip(DeleteSound, delete_name);
			delete_name.clear();
			ValidateMusicFiles();
		}
		ImGui::EndChild();
		ImGui::EndPopup();
	}
	return false;
}

bool CKillState::DrawFileExplorer() {
	bool result = false;
	auto &curPack = Packs.at(cfg.currentPack);
	auto loc = std::setlocale(LC_ALL, ".UTF8");
	if (fileDialog.Display("SelectMusicFile", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize, ImVec2(700, 400))) {
		if (fileDialog.IsOk()) {
			cfg.lastPath = fileDialog.GetFilePathName();
			fs::directory_entry file(cfg.lastPath);
			auto filename = file.path().filename().string();
			auto& music = curPack.SoundFiles;
			if (find(music.begin(), music.end(), filename) == music.end()) {
				fs::path u8file = string("Sounds/") + filename;
				WorkWithZip(AddFileToPath, u8file.u8string(), file.path().u8string());
				music.push_back(filename);
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

void CKillState::WorkWithZip(int mode, string entryName, string file ) {
	auto p = pathDir / "Packs" / (cp1251_to_utf8(cfg.currentPack) + ".zip");
	ZipArchive::OpenMode _mode;
	switch (mode) {
	case AddFileToPath:
	case DeleteSound:
	case RenameFile:
		_mode = ZipArchive::Write;
		break;
	default:
		break;
	}
	int res = -1;
	ZipArchive zf(p.u8string());
	zf.open(_mode);
	switch (mode) {
	case AddFileToPath:
		zf.addFile(entryName, file);
		break;
	case DeleteSound:
		zf.deleteEntry("Sounds/" + entryName);
		break;
	case RenameFile:
		res = zf.renameEntry("Sounds/" + entryName, "Sounds/" + file);
		//Console::Info("%d %s %s", res, entryName.data(), file.data());
		break;
	default:
		break;
	}
	zf.close();
}