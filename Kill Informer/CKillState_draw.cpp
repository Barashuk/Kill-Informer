#include "CKillState.h"

void CKillState::DrawStats() {
	auto &it = Packs.at(cfg.currentPack);
	it.fontKills->Print(string_format("Kills: %d", KDA.Kills));
	it.fontDeaths->Print(string_format("Deaths: %d", KDA.Deaths));
	it.fontAssists->Print(string_format("Assists: %d", KDA.Assists));
}

void CKillState::Draw() {
	DrawStats();
	DrawKillString();
	DrawStatuses();
}

void CKillState::DrawKillString() {
	auto &it = Packs.at(cfg.currentPack);
	if (bOpenMenu) {
		renderString = it.setting.KillString.Value();
		if (it.setting.ShowDistance) {
			renderString += " " + it.setting.DistString.Value();
		}
		renderString += "\n" + it.setting.StatusString.Value() + " {FFFF00}test";
		ReplaceStrings(renderString, "{nKiller}", "Killer");
		ReplaceStrings(renderString, "{cKiller}", "{FFFF00FF}");
		ReplaceStrings(renderString, "{nVictim}", "Victim");
		ReplaceStrings(renderString, "{cVictim}", "{FF00FFFF}");
		ReplaceStrings(renderString, "{dColor}", string_format("{%08X}", toARGB(it.setting.colorDistance)));
		ReplaceStrings(renderString, "{dist}", string_format("%.02f", 14.88f));
		ReplaceStrings(renderString, "{cText}", string_format("{%08X}", toARGB(it.fontMessage->GetColor())));
		it.fontMessage->Print(renderString);
		return;
	}

	auto now = system_clock_t::now();
	auto dur_now = chrono::duration_cast<chrono::seconds>(timeRenderMessage - now).count();
	if (dur_now < 0)
		return;
	it.fontMessage->Print(renderString);	
}

void CKillState::DrawStatuses() {
	auto &it = Packs.at(cfg.currentPack);
	auto now = chrono::system_clock::now();
	if (bOpenMenu) {
		if (StatusesStrings.empty()) {
			for (int i = 0; i < 6; i++) {
				StatusesStrings.push_back({ "line " + to_string(i) , now + chrono::seconds(999) });
			}
		}
	} else {
		if (!StatusesStrings.empty()) {
			auto it = remove_if(StatusesStrings.begin(), StatusesStrings.end(), [&](const status_p& elem) {
				return elem.first.find("line") != string::npos;
				});
			StatusesStrings.erase(it, StatusesStrings.end());
		}
	}
	
	if (StatusesStrings.empty())
		return;
	auto font = it.fontStatuses;
	float height = font->GetDrawHeight(), width, maxHeight = height * (float)StatusesStrings.size();
	const auto& size = ImGui::GetMainViewport()->Size;
	auto& style = ImGui::GetStyle();
	
	ImVec2 pos;
	for (size_t i = 0; i < StatusesStrings.size(); i++) {
		auto text = StatusesStrings.at(i).first;
		width = font->GetDrawLength(text);
		switch (it.setting.typePosStatuses) {
		case 0: pos.x = style.WindowPadding.x, pos.y = style.WindowPadding.y + height * (float)i; break;
		case 1: pos.x = size.x / 2 - width / 2, pos.y = style.WindowPadding.y + height * (float)i; break;
		case 2: pos.x = size.x - style.WindowPadding.x - width, pos.y = style.WindowPadding.y + height * (float)i; break;
		case 3: pos.x = style.WindowPadding.x, pos.y = size.y / 2 - maxHeight / 2 + height * (float)i; break;
		case 4: pos.x = size.x / 2 - width / 2, pos.y = size.y / 2 - maxHeight / 2 + height * (float)i; break;
		case 5: pos.x = size.x - style.WindowPadding.x - width, pos.y = size.y / 2 - maxHeight / 2 + height * (float)i; break;
		case 6: pos.x = style.WindowPadding.x, pos.y = size.y - style.WindowPadding.x - maxHeight + height * (float)i; break;
		case 7: pos.x = size.x / 2 - width / 2, pos.y = size.y - style.WindowPadding.x - maxHeight + height * (float)i; break;
		case 8: pos.x = size.x - style.WindowPadding.x - width, pos.y = size.y - style.WindowPadding.x - maxHeight + height * (float)i; break;
		default: break;
		}
		font->Print(text, pos);
		auto dur_now = chrono::duration_cast<chrono::milliseconds>(StatusesStrings.at(i).second - now).count();
		if (dur_now < 0) {
			auto it = remove_if(StatusesStrings.begin(), StatusesStrings.end(), [&](const status_p& elem) {
				return chrono::duration_cast<chrono::milliseconds>(elem.second - now).count() < 0;
				});
			StatusesStrings.erase(it, StatusesStrings.end());
		}
	}
	
}