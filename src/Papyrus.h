#pragma once

namespace Papyrus
{
	using Severity = RE::BSScript::ErrorLogger::Severity;

	//bool EffectStarted(RE::BSScript::IVirtualMachine& a_vm, uint32_t a_stackID, std::monostate, RE::EffectSetting* a_magicEffect);
	//bool EffectFinished(RE::BSScript::IVirtualMachine& a_vm, uint32_t a_stackID, std::monostate, RE::EffectSetting* a_magicEffect);
	bool RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm);
}
