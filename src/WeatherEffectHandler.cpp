#include "WeatherEffectHandler.h"
#include "DisableEffectHandler.h"
#include "Utils.h"
#include "Offsets.h"

void WeatherEffectHandler::Initialize()
{
	logger::info("Initializing WeatherEffectHandler...");

	constexpr auto ini_path = L"Data/F4SE/Plugins/FlashlightDisablingList.ini";
	const auto ReadSettingsIni = [&](std::filesystem::path path) {
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.SetAllowKeyOnly(false);
		ini.SetMultiKey(false); // Get the last key entry only
		ini.LoadFile(path.string().c_str());

		CSimpleIniA::TNamesDepend keys;
		auto weatherSection = ini.GetSection("Weather List");
		for (auto it = weatherSection->begin(); it != weatherSection->end(); ++it) {
			std::uint32_t intKey = std::stoi(it->first.pItem, 0, 16); // Convert hex form to int decimal value
			float floatValue = std::stof(it->second);
			WeatherList.insert({ intKey, floatValue });
		}
		ini.Reset(); // Deallocate memory
	};
	ReadSettingsIni(ini_path);
	
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (dataHandler) {
		WeatherCheckSpell = dataHandler->LookupForm<RE::SpellItem>(0x802, "Flashlight Disabling Manager.esm");
		if (!WeatherCheckSpell) {
			logger::critical("Failed to initialize FDM_Weather Check Spell (EditorID: FDM_WeatherCheckSpell)");
		}
		WeatherCheckEffectID = dataHandler->LookupForm<RE::EffectSetting>(0x803, "Flashlight Disabling Manager.esm");
		if (!WeatherCheckEffectID) {
			logger::critical("Failed to initialize FDM_Weather Check Effect (EditorID: FDM_WeatherCheckEffect)");
		}
		WeatherDisableLightSpell = dataHandler->LookupForm<RE::SpellItem>(0x804, "Flashlight Disabling Manager.esm");
		if (!WeatherDisableLightSpell) {
			logger::critical("Failed to initialize FDM_Weather Disable Light Spell (EditorID: FDM_WeatherDisableLight)");
		}
	}
	const auto UI = RE::UI::GetSingleton();
	if (UI) {
		UI->RegisterSink<RE::MenuOpenCloseEvent>(WeatherEffectHandler::GetSingleton());
	}

	_OnEffectStart = REL::Relocation<uintptr_t>(RE::VTABLE::ScriptEffect[0]).write_vfunc(0x16, OnEffectStart); // Start
	_OnEffectFinish = REL::Relocation<uintptr_t>(RE::VTABLE::ScriptEffect[0]).write_vfunc(0x17, OnEffectFinish); // Finish
	_OnEffectFinishLoadGame = REL::Relocation<uintptr_t>(RE::VTABLE::ScriptEffect[0]).write_vfunc(0xD, OnEffectFinishLoadGame); // Finish Load Game

	logger::info("...WeatherEffectHandler initialized.");
}

void WeatherEffectHandler::ApplyWeatherCheckToPlayer()
{
	auto a_player = RE::PlayerCharacter::GetSingleton();
	if (WeatherCheckSpell && !ActorHasSpell(a_player, WeatherCheckSpell)) {
		ActorAddSpell(a_player, WeatherCheckSpell);
	}
}

std::uint32_t WeatherEffectHandler::GetWeatherFormID(RE::TESWeather* a_weather)
{
	auto weatherFormID = a_weather->formID;
	// Convert to hex
	std::stringstream hexForm;
	hexForm << std::hex << weatherFormID;
	auto hexStr = hexForm.str();
	hexForm.str(std::string()); // Clear stringstream for next iteration
	// Keep last 6 digits
	hexStr = hexStr.substr(hexStr.length() - 6, hexStr.length());
	// Convert back to decimal
	return std::stoi(hexStr, 0, 16);
}

WeatherEffectHandler::EventResult WeatherEffectHandler::ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
	if (!a_event.opening && a_event.menuName == "LoadingMenu"sv && sActionToDoAfterMenu != "") {
		auto a_player = RE::PlayerCharacter::GetSingleton();
		if (sActionToDoAfterMenu == "Add" && !ActorHasSpell(a_player, WeatherDisableLightSpell)) {
			ActorAddSpell(a_player, WeatherDisableLightSpell);
		}
		else if (sActionToDoAfterMenu == "Remove" && ActorHasSpell(a_player, WeatherDisableLightSpell)) {
			ActorRemoveSpell(a_player, WeatherDisableLightSpell);
		}
		sActionToDoAfterMenu = "";
	}
	return EventResult::kContinue;
}

void WeatherEffectHandler::OnEffectStart(RE::ActiveEffect* a_effect)
{
	_OnEffectStart(a_effect);
	
	// Effect starts once
	if (a_effect->effect->effectSetting == WeatherCheckEffectID) {
		// Do an extra player check in case
		if (auto a_player = RE::PlayerCharacter::GetSingleton(); a_effect->target->GetTargetStatsObject() == a_player) {
			auto currentWeatherID = WeatherEffectHandler::GetSingleton()->GetWeatherFormID(RE::Sky::GetSingleton()->currentWeather);
			if (WeatherList.find(currentWeatherID) != WeatherList.end()) {
				// If weather is in the listed disable weathers then roll the chance to disable flashlight
				auto disableChance = WeatherList.find(currentWeatherID)->second;
				auto randomChance = Utils::GetRandomFloat(0.0, 1.0);
				if (randomChance <= disableChance) {
					// Don't add in Loading Menu
					if (RE::UI::GetSingleton()->GetMenuOpen("LoadingMenu"sv)) {
						WeatherEffectHandler::GetSingleton()->sActionToDoAfterMenu = "Add";
					}
					else if (!ActorHasSpell(a_player, WeatherDisableLightSpell)) {
						ActorAddSpell(a_player, WeatherDisableLightSpell);
					}
				}
			}
		}
	}
}

void WeatherEffectHandler::OnEffectFinish(RE::ActiveEffect* a_effect)
{
	_OnEffectFinish(a_effect);

	// Effect finishes once
	if (a_effect->effect->effectSetting == WeatherCheckEffectID) {
		// Do an extra player check in case
		if (auto a_player = RE::PlayerCharacter::GetSingleton(); a_effect->target->GetTargetStatsObject() == a_player) {
			// Don't remove in Loading Menu
			if (RE::UI::GetSingleton()->GetMenuOpen("LoadingMenu"sv)) {
				WeatherEffectHandler::GetSingleton()->sActionToDoAfterMenu = "Remove";
			}
			else if (ActorHasSpell(a_player, WeatherDisableLightSpell)) {
				ActorRemoveSpell(a_player, WeatherDisableLightSpell);
			}
		}
	}
}

void WeatherEffectHandler::OnEffectFinishLoadGame(RE::ActiveEffect* a_effect)
{
	_OnEffectFinishLoadGame(a_effect);

	// If loaded game and effect is active, OnEffectStart won't fire, so have to check in here
	if (a_effect->effect->effectSetting == WeatherCheckEffectID && a_effect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kTrue)) {
		// Do an extra player check in case
		if (auto a_player = RE::PlayerCharacter::GetSingleton(); a_effect->target->GetTargetStatsObject() == a_player) {
			auto currentWeatherID = WeatherEffectHandler::GetSingleton()->GetWeatherFormID(RE::Sky::GetSingleton()->currentWeather);
			if (WeatherList.find(currentWeatherID) != WeatherList.end()) {
				// If weather is in the listed disable weathers then roll the chance to disable flashlight
				auto disableChance = WeatherList.find(currentWeatherID)->second;
				auto randomChance = Utils::GetRandomFloat(0.0, 1.0);
				if (randomChance <= disableChance) {
					// Don't add in Loading Menu
					if (RE::UI::GetSingleton()->GetMenuOpen("LoadingMenu"sv)) {
						WeatherEffectHandler::GetSingleton()->sActionToDoAfterMenu = "Add";
					}
					else if (!ActorHasSpell(a_player, WeatherDisableLightSpell)) {
						ActorAddSpell(a_player, WeatherDisableLightSpell);
					}
				}
			}
		}
	}
}
