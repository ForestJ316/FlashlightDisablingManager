#include <unordered_set>

#include "Papyrus.h"
#include "FlashlightHandler.h"

namespace Papyrus
{

	// Error posting and effect checks courtesy of LighthousePapyrusExtender by GELUXRUM
	// https://github.com/GELUXRUM/LighthousePapyrusExtender
	bool EffectStarted(RE::BSScript::IVirtualMachine& a_vm, uint32_t a_stackID, std::monostate, RE::EffectSetting* a_magicEffect)
	{
		logger::info("Effect Started");
		if (!a_magicEffect) {
			a_vm.PostError("MagicEffect is None", a_stackID, Severity::kError);
			return false;
		}
		auto a_player = RE::PlayerCharacter::GetSingleton();
		const RE::BSTArray<RE::BSTSmartPointer<RE::ActiveEffect>> effectList = a_player->GetActiveEffectList()->data;
		if (effectList.empty()) {
			a_vm.PostError("Player has no active effects", a_stackID, Severity::kError);
			return false;
		}

		//auto flashlightHandler = FlashlightHandler::GetSingleton();		
		for (const auto& currentEffect : effectList) {
			if (auto mgef = currentEffect ? currentEffect->effect->effectSetting : nullptr; mgef) {
				// Ignore inactive flags
				if (currentEffect->flags.all(RE::ActiveEffect::Flags::kInactive) || currentEffect->flags.all(RE::ActiveEffect::Flags::kDispelled)) {
					continue;
				}
				// Otherwise check if the mgef is the same as the one passed to the function
				// Can have multiples of same mgef, so need to check for uniqueID
				else if (mgef == a_magicEffect) {
					logger::info("current effect unique id: {}", currentEffect->uniqueID);
					//flashlightHandler->AddEffectID(currentEffect->uniqueID);
				}
			}
		}
		//flashlightHandler->TurnOffFlashlight();
		return true;
	}

	bool EffectFinished(RE::BSScript::IVirtualMachine& a_vm, uint32_t a_stackID, std::monostate, RE::EffectSetting* a_magicEffect)
	{
		logger::info("Effect Finished");
		if (!a_magicEffect) {
			a_vm.PostError("MagicEffect is None", a_stackID, Severity::kError);
			return false;
		}
		auto a_player = RE::PlayerCharacter::GetSingleton();
		const RE::BSTArray<RE::BSTSmartPointer<RE::ActiveEffect>> effectList = a_player->GetActiveEffectList()->data;
		if (effectList.empty()) {
			a_vm.PostError("Player has no active effects", a_stackID, Severity::kError);
			return false;
		}

		//auto flashlightHandler = FlashlightHandler::GetSingleton();
		std::unordered_set<uint16_t> CurrentActiveEffects;
		for (const auto& currentEffect : effectList) {
			if (auto mgef = currentEffect ? currentEffect->effect->effectSetting : nullptr; mgef) {
				// Ignore inactive flags
				if (currentEffect->flags.all(RE::ActiveEffect::Flags::kInactive) || currentEffect->flags.all(RE::ActiveEffect::Flags::kDispelled)) {
					continue;
				}
				// Otherwise check if the mgef is the same as the one passed to the function
				// Can have multiples of same mgef, so need to check for uniqueID
				else if (mgef == a_magicEffect) {
					logger::info("current effect unique id: {}", currentEffect->uniqueID);
					CurrentActiveEffects.insert(currentEffect->uniqueID);
				}
			}
		}

		//flashlightHandler->RemoveFinishedEffectID
		return true;
	}

	bool RegisterFunctions(RE::BSScript::IVirtualMachine*)
	{
		//a_vm->BindNativeMethod("FlashlightDisability"sv, "EffectStarted"sv, Papyrus::EffectStarted);
		//a_vm->BindNativeMethod("FlashlightDisability"sv, "EffectFinished"sv, Papyrus::EffectFinished);
		logger::info("Registered Papyrus native functions.");
		return true;
	}
}
