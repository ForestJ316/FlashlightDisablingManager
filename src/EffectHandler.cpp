#include "EffectHandler.h"
#include "FlashlightHandler.h"

void EffectHandler::Initialize()
{
	logger::info("Initializing EffectHandler...");

	_OnEffectStart = REL::Relocation<uintptr_t>(RE::VTABLE::ScriptEffect[0]).write_vfunc(0x16, OnEffectStart); // Start
	_OnEffectFinish = REL::Relocation<uintptr_t>(RE::VTABLE::ScriptEffect[0]).write_vfunc(0x17, OnEffectFinish); // Finish
	_OnEffectFinishLoadGame = REL::Relocation<uintptr_t>(RE::VTABLE::ScriptEffect[0]).write_vfunc(0xD, OnEffectFinishLoadGame); // Finish Load Game

	auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (dataHandler) {
		FlashlightEffectID = dataHandler->LookupForm<RE::EffectSetting>(0x800, "Flashlight Disabling Manager.esm");
		if (!FlashlightEffectID) {
			logger::critical("Failed to initialize FDM_Flashlight Magic Effect");
		}
	}
	const auto UI = RE::UI::GetSingleton();
	if (UI) {
		UI->RegisterSink<RE::MenuOpenCloseEvent>(FaderMenuEvent::GetSingleton());
	}
	logger::info("...EffectHandler initialized.");
}

void EffectHandler::OnEffectStart(RE::ActiveEffect* a_effect)
{
	_OnEffectStart(a_effect);

	if (a_effect->effect->effectSetting == FlashlightEffectID && a_effect->target->GetTargetStatsObject() == RE::PlayerCharacter::GetSingleton()) {
		// Increment a counter instead of storing the unique ID
		EffectHandler::GetSingleton()->iActiveFlashlightEffectCount += 1;
		// Turn on/off only after Fader Menu
		const auto UI = RE::UI::GetSingleton();
		if (!UI->GetMenuOpen("LoadingMenu"sv) || !UI->GetMenuOpen("FaderMenu"sv)) {
			FlashlightHandler::GetSingleton()->TurnOffFlashlight();
		}
		else {
			EffectHandler::GetSingleton()->sActionToDoAfterMenu = "Off";
		}
	}
}

void EffectHandler::OnEffectFinish(RE::ActiveEffect* a_effect)
{
	_OnEffectFinish(a_effect);

	if (a_effect->effect->effectSetting == FlashlightEffectID && a_effect->target->GetTargetStatsObject() == RE::PlayerCharacter::GetSingleton()) {
		auto effectHandler = EffectHandler::GetSingleton();
		// Check for 0 in case of some wild race condition or any other unknown behavior that might duplicate the vfunc call
		if (effectHandler->iActiveFlashlightEffectCount > 0) {
			effectHandler->iActiveFlashlightEffectCount -= 1;
			if (effectHandler->iActiveFlashlightEffectCount == 0) {
				// Turn on/off only after Fader Menu
				const auto UI = RE::UI::GetSingleton();
				if (!UI->GetMenuOpen("LoadingMenu"sv) || !UI->GetMenuOpen("FaderMenu"sv)) {
					FlashlightHandler::GetSingleton()->TurnOnFlashlight();
				}
				else {
					EffectHandler::GetSingleton()->sActionToDoAfterMenu = "On";
				}
			}	
		}
	}
}

void EffectHandler::OnEffectFinishLoadGame(RE::ActiveEffect* a_effect)
{
	_OnEffectFinishLoadGame(a_effect);

	// If loaded game and effect is active, OnEffectStart won't fire, so have to check in here
	if (a_effect->effect->effectSetting == FlashlightEffectID && a_effect->target->GetTargetStatsObject() == RE::PlayerCharacter::GetSingleton()
		&& a_effect->conditionStatus.any(RE::ActiveEffect::ConditionStatus::kTrue)) {
		// Increment a counter instead of storing the unique ID
		EffectHandler::GetSingleton()->iActiveFlashlightEffectCount += 1;
		// Turn on/off only after Fader Menu
		const auto UI = RE::UI::GetSingleton();
		if (!UI->GetMenuOpen("LoadingMenu"sv) || !UI->GetMenuOpen("FaderMenu"sv)) {
			FlashlightHandler::GetSingleton()->TurnOffFlashlight();
		}
		else {
			EffectHandler::GetSingleton()->sActionToDoAfterMenu = "Off";
		}
	}
}

EffectHandler::FaderMenuEvent::EventResult EffectHandler::FaderMenuEvent::ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
	if (!a_event.opening && a_event.menuName == "FaderMenu"sv && EffectHandler::GetSingleton()->sActionToDoAfterMenu != "") {
		std::string& sAction = EffectHandler::GetSingleton()->sActionToDoAfterMenu;
		if (sAction == "On") {
			FlashlightHandler::GetSingleton()->TurnOnFlashlight();
		}
		else if (sAction == "Off") {
			FlashlightHandler::GetSingleton()->TurnOffFlashlight();
		}
		sAction = "";
	}
	return EventResult::kContinue;
}
