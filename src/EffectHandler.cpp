#include "EffectHandler.h"
#include "FlashlightHandler.h"

void EffectHandler::Initialize()
{
	logger::info("Initializing EffectHandler...");

	_OnEffectStart = REL::Relocation<uintptr_t>(RE::VTABLE::ScriptEffect[0]).write_vfunc(0x16, OnEffectStart); // Start
	_OnEffectFinish = REL::Relocation<uintptr_t>(RE::VTABLE::ScriptEffect[0]).write_vfunc(0x17, OnEffectFinish); // Finish

	auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (dataHandler) {
		FlashlightEffectID = dataHandler->LookupForm<RE::EffectSetting>(0x00800, "Flashlight Disability Framework.esp");
		if (!FlashlightEffectID) {
			logger::critical("Failed to initialize FDF_Flashlight Magic Effect");
		}
	}
	logger::info("...EffectHandler initialized.");
}

void EffectHandler::OnEffectStart(RE::ActiveEffect* a_effect)
{
	_OnEffectStart(a_effect);

	if (a_effect->effect->effectSetting == FlashlightEffectID && a_effect->target->GetTargetStatsObject() == RE::PlayerCharacter::GetSingleton()) {
		logger::info("In On Effect START with effect uniqueID: {}", a_effect->uniqueID); // DEBUG		
		// Increment a counter instead of storing the unique ID
		EffectHandler::GetSingleton()->iActiveFlashlightEffectCount += 1;
		FlashlightHandler::GetSingleton()->TurnOffFlashlight();
	}
}

void EffectHandler::OnEffectFinish(RE::ActiveEffect* a_effect)
{
	_OnEffectFinish(a_effect);

	if (a_effect->effect->effectSetting == FlashlightEffectID && a_effect->target->GetTargetStatsObject() == RE::PlayerCharacter::GetSingleton()) {
		logger::info("In On Effect FINISH with effect uniqueID: {}", a_effect->uniqueID); // DEBUG
		auto effectHandler = EffectHandler::GetSingleton();
		// Check for 0 in case of some wild race condition or any other unknown behavior that might duplicate the vfunc call
		if (effectHandler->iActiveFlashlightEffectCount > 0) {
			effectHandler->iActiveFlashlightEffectCount -= 1;
			if (effectHandler->iActiveFlashlightEffectCount == 0) {
				FlashlightHandler::GetSingleton()->TurnOnFlashlight();
			}	
		}
	}
}
