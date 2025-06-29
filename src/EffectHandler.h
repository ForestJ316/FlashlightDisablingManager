#pragma once

class EffectHandler
{
public:
	static EffectHandler* GetSingleton()
	{
		static EffectHandler singleton;
		return std::addressof(singleton);
	}
	static void Initialize();

	int iActiveFlashlightEffectCount = 0;

private:
	static void OnEffectStart(RE::ActiveEffect* a_effect);
	static inline REL::Relocation<decltype(OnEffectStart)> _OnEffectStart;

	static void OnEffectFinish(RE::ActiveEffect* a_effect);
	static inline REL::Relocation<decltype(OnEffectFinish)> _OnEffectFinish;

	static inline RE::EffectSetting* FlashlightEffectID;
};
