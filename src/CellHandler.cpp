#include "CellHandler.h"

void CellHandler::Initialize()
{
	logger::info("Initializing CellHandler...");
	constexpr auto ini_path = L"Data/F4SE/Plugins/CellsToDisableFlashlight.ini";

	const auto ReadSettingsIni = [&](std::filesystem::path path) {
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.SetAllowKeyOnly(true);
		ini.LoadFile(path.string().c_str());

		// Test
		CSimpleIniA::TNamesDepend keys;
		ini.GetAllKeys("Cell Forms", keys);
		for (CSimpleIniA::TNamesDepend::const_iterator it = keys.begin(); it != keys.end(); ++it) {
			std::uint32_t intValue = std::stoi(it->pItem, 0, 16);
			DisabledCells.insert(intValue);
		}
	};
	ReadSettingsIni(ini_path);
	
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (dataHandler) {
		CellSpell = dataHandler->LookupForm<RE::SpellItem>(0x00801, "Flashlight Disability Framework.esp");
		if (!CellSpell) {
			logger::critical("Failed to initialize FDF_CellSpell");
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
		auto cellHandler = CellHandler::GetSingleton();
		auto a_player = RE::PlayerCharacter::GetSingleton();
		const auto UI = RE::UI::GetSingleton();
		if (DisabledCells.find(a_event.cellID) != DisabledCells.end()) {
			cellHandler->bEnteredDisabledCell = true;
			// Catch-all if cell changed without a Loading Menu
			if (!UI->GetMenuOpen("LoadingMenu"sv) && !ActorHasSpell(a_player, CellHandler::CellSpell)) {
				ActorAddSpell(a_player, CellHandler::CellSpell);
			}
		}
		else {
			// If entered a non-disable cell and previous cell was a disable cell
			if (cellHandler->bEnteredDisabledCell) {
				cellHandler->bEnteredDisabledCell = false;
				// Remove spell if no Loading Menu open, otherwise it will be removed after Fader Menu closes
				if (!UI->GetMenuOpen("LoadingMenu"sv) && ActorHasSpell(a_player, CellHandler::CellSpell)) {
					ActorRemoveSpell(RE::PlayerCharacter::GetSingleton(), CellSpell);
				}
			}
		}
	}
	return EventResult::kContinue;
}

FaderMenuEvent::EventResult FaderMenuEvent::ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
	if (!a_event.opening && a_event.menuName == "FaderMenu"sv) {
		logger::info("Fader Menu off"); // DEBUG
		auto cellHandler = CellHandler::GetSingleton();
		auto a_player = RE::PlayerCharacter::GetSingleton();
		if (cellHandler->bEnteredDisabledCell && !ActorHasSpell(a_player, CellHandler::CellSpell)) {
			ActorAddSpell(a_player, CellHandler::CellSpell);
		}
		else if (!cellHandler->bEnteredDisabledCell && ActorHasSpell(a_player, CellHandler::CellSpell)) {
			ActorRemoveSpell(a_player, CellHandler::CellSpell);
		}
	}
	return EventResult::kContinue;
}
