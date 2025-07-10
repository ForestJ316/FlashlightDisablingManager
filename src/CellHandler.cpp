#include "CellHandler.h"
#include "Offsets.h"

void CellHandler::Initialize()
{
	logger::info("Initializing CellHandler...");

	constexpr auto ini_path = L"Data/F4SE/Plugins/FlashlightDisablingList.ini";
	const auto ReadSettingsIni = [&](std::filesystem::path path) {
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.SetAllowKeyOnly(true);
		ini.SetMultiKey(false); // Get the last key entry only
		ini.LoadFile(path.string().c_str());

		CSimpleIniA::TNamesDepend keys;
		ini.GetAllKeys("Cell List", keys);
		for (CSimpleIniA::TNamesDepend::const_iterator it = keys.begin(); it != keys.end(); ++it) {
			// Check if the key is in the correct format
			// Format is "hex|plugin"
			std::string strValue = it->pItem;
			if (strValue.find("|") != std::string::npos) {
				// Convert the hex to decimal instead
				std::string decimalKey = Utils::ConvertIniEntryToDecimal(strValue);
				CellList.insert(decimalKey);
			}
		}
		ini.Reset(); // Deallocate memory
	};
	ReadSettingsIni(ini_path);
	
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (dataHandler) {
		CellDisableLightSpell = dataHandler->LookupForm<RE::SpellItem>(0x801, "Flashlight Disabling Manager.esm");
		if (!CellDisableLightSpell) {
			logger::critical("Failed to initialize FDM_CellDisableLight (EditorID: FDM_Cell Disable Light Spell)");
		}
	}
	RE::PlayerCharacter::GetSingleton()->RE::BSTEventSource<RE::BGSActorCellEvent>::RegisterSink(CellHandler::GetSingleton());
	const auto UI = RE::UI::GetSingleton();
	if (UI) {
		UI->RegisterSink<RE::MenuOpenCloseEvent>(FaderMenuEvent::GetSingleton());
	}

	logger::info("...CellHandler initialized.");
}

CellHandler::EventResult CellHandler::ProcessEvent(const RE::BGSActorCellEvent& a_event, RE::BSTEventSource<RE::BGSActorCellEvent>*)
{
	if (a_event.flags.get() == RE::BGSActorCellEvent::CellFlag::kEnter) {
		auto a_player = RE::PlayerCharacter::GetSingleton();
		const auto UI = RE::UI::GetSingleton();

		auto cellForm = RE::TESForm::GetFormByID<RE::TESObjectCELL>(a_event.cellID);
		auto disabledEntry = Utils::GetDisabledIniEntryFromForm(CellList, cellForm);
		logger::info("entered cell: {}, is disabled entry: {}", cellForm->formID, disabledEntry.c_str());
		if (disabledEntry != "") {
			bEnteredDisabledCell = true;
			// Catch-all if cell changed without a Loading Menu
			if (!UI->GetMenuOpen("LoadingMenu"sv) && !UI->GetMenuOpen("FaderMenu"sv) && !ActorHasSpell(a_player, CellDisableLightSpell)) {
				ActorAddSpell(a_player, CellDisableLightSpell);
			}
		}
		else {
			// If entered a non-disable cell and previous cell was a disable cell
			if (bEnteredDisabledCell) {
				bEnteredDisabledCell = false;
				// Remove spell if no Loading Menu open, otherwise it will be removed after Fader Menu closes
				if (!UI->GetMenuOpen("LoadingMenu"sv) && !UI->GetMenuOpen("FaderMenu"sv) && ActorHasSpell(a_player, CellDisableLightSpell)) {
					ActorRemoveSpell(RE::PlayerCharacter::GetSingleton(), CellDisableLightSpell);
				}
			}
		}
	}
	return EventResult::kContinue;
}

CellHandler::FaderMenuEvent::EventResult CellHandler::FaderMenuEvent::ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
	if (!a_event.opening && a_event.menuName == "FaderMenu"sv) {
		auto cellHandler = CellHandler::GetSingleton();
		auto a_player = RE::PlayerCharacter::GetSingleton();
		if (cellHandler->bEnteredDisabledCell && !ActorHasSpell(a_player, CellDisableLightSpell)) {
			ActorAddSpell(a_player, CellDisableLightSpell);
		}
		else if (!cellHandler->bEnteredDisabledCell && ActorHasSpell(a_player, CellDisableLightSpell)) {
			ActorRemoveSpell(a_player, CellDisableLightSpell);
		}
	}
	return EventResult::kContinue;
}
