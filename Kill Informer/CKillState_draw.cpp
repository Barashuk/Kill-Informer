#include "CKillState.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <sampapi/CGame.h>
#include <sstream>
#include <functional>
#include <limits>
namespace R1 = sampapi::v037r1;

void CKillState::OpenMenu() {
	bMenu = !bMenu;
	R1::RefGame()->SetCursorMode(bMenu? R1::CURSOR_LOCKCAMANDCONTROL : R1::CURSOR_NONE, bMenu);
}

void CKillState::DrawPacks() {
	auto packsList = [this](const string& label) {
		static bool changeName = false;
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
					auto it = find_if(Packs.begin(), Packs.end(), [&](const stPack& elem) { return elem.Name == cfg.currentPack; });
					if (it == Packs.end())
						return;
					auto name = cfg.currentPack + ".zip";
					auto path = pathDir / "Packs";
					fs::remove(path / name);
					auto new_name = utf8_to_cp1251(nameToChange);
					it->Name = new_name ;
					cfg.currentPack = new_name;
					changeName = false;
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
			if (ImGui::BeginCombo(_label.data(), namePack.data())) {
				for (size_t i = 0; i < Packs.size(); i++) {
					auto text = cp1251_to_utf8(Packs.at(i).Name);
					if (ImGui::Selectable(text.data(), text.data() == namePack)) {
						namePack = text;
						//add convert
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
	if (!bMenu)
		return;
	auto& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize({ 800, 600 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
	if (ImGui::Begin("Kill Informer", &bMenu, flags))  {
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
		ImGui::Text(u8"Выберите шрифт для редакрирования");
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
		DrawTable("##kill_table", curPack->KillsEvents);
		
	}
}

void CKillState::DrawTable(string label, vector<stElementEvent>& events) {
	auto columnInputInt = [](string label, string hint, uint8_t& value, int id = -1) -> bool {
		bool result = false;
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		ImGui::Text(hint.c_str());
		if (id > -1) {
			label = "X" + label + "_id_delete_" + to_string(id);
			if (ImGui::Button(label.c_str())) {
				result = true;
			}
			ImGui::SameLine();
		}
		label = label + "_id";
		if (ImGui::InputInt(label.c_str(), (int*)&value)) {
			if (value < 0)
				value = 0;
			if (value > 255)
				value = 255;
		}
		ImGui::PopItemWidth();
		return result;
	};
	auto columnInputText = [](string label, string hint, string& value) {
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		label = label + "_name";
		ImGui::Text(hint.c_str());
		ImGui::InputText(label.c_str(), &value);
		ImGui::PopItemWidth();
	};
	auto columnInputSound = [](string label, string hint, string& value) {
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		label = label + "_sound";
		ImGui::Text(hint.c_str());
		ImGui::InputText(label.c_str(), &value);
		ImGui::PopItemWidth();
	};
	auto columnInputColor = [](string label, string hint, ImVec4& value) {
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		label = label + "_color";
		ImGui::Text(hint.c_str());
		ImGui::ColorEdit4(label.c_str(), (float*)&value,
			ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoDragDrop |
			ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker
		);
		ImGui::PopItemWidth();
	};
	
	ImGuiTableFlags flags = ImGuiTableFlags_Borders;
	if (ImGui::BeginTable(label.c_str(), 4, flags)) {
		ImGui::TableNextRow();
		//---------------------------
		columnInputInt(label, u8"Введите ид", new_event_add.value);
		//---------------------------
		columnInputText(label, u8"Введите имя", new_event_add.name);
		//---------------------------
		columnInputSound(label, u8"Выберите звук", new_event_add.sound);
		//---------------------------
		columnInputColor(label, u8"Выберите цвет", new_event_add.color);
		//---------------------------
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		
		if (ImGui::Button(u8"Добавить", ImVec2(ImGui::GetColumnWidth(), 0))) {
			if (new_event_add.name.empty()) {
				Console::Add("Название не может быть пустым");
			}
			else {
				events.push_back(new_event_add);
				new_event_add.Clear();
			}
		}
		ImGui::EndTable();
	}
	ImGui::Separator();
	if (events.empty()) {
		ImGui::Text(u8"Список событий пуст");
		return;
	}
	else {
		label = label + "_list";
		if (ImGui::BeginTable(label.c_str(), 4, flags)) {
			for (size_t id = 0; id < events.size(); id++) {
				auto& event = events.at(id);
				ImGui::TableNextRow();
				//---------------------------
				if (columnInputInt(label, u8"Введите ид", event.value, id)) {
					events.erase(remove_if(events.begin(), events.end(), [&](const stElementEvent& _event) {
						return _event.value == event.value &&
							_event.name == event.name &&
							_event.sound == event.sound /*&&
							_event.color == event.color*/; }
					), events.end());
				}
				//---------------------------
				columnInputText(label, u8"Введите имя", event.name);
				//---------------------------
				columnInputText(label, u8"Выберите звук", event.sound);
				//---------------------------
				columnInputColor(label, u8"Выберите цвет", event.color);
			}

			ImGui::EndTable();
		}
	}

	
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