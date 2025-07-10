#pragma once
#include "Utils.h"
#include <unordered_map>

class WeatherEffectHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
private:
	using EventResult = RE::BSEventNotifyControl;

public:
	static WeatherEffectHandler* GetSingleton()
	{
		static WeatherEffectHandler singleton;
		return std::addressof(singleton);
	}
	static void Initialize();

	EventResult ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*);

	void ApplyWeatherCheckToPlayer();

private:
	static inline RE::SpellItem* WeatherCheckSpell;
	static inline RE::EffectSetting* WeatherCheckEffectID;
	static inline RE::SpellItem* WeatherDisableLightSpell;
	
	static inline std::unordered_map<std::string, float> WeatherList;

	std::string sActionToDoAfterMenu = "";

	static void OnEffectStart(RE::ActiveEffect* a_effect);
	static inline REL::Relocation<decltype(OnEffectStart)> _OnEffectStart;

	static void OnEffectFinish(RE::ActiveEffect* a_effect);
	static inline REL::Relocation<decltype(OnEffectFinish)> _OnEffectFinish;

	static void OnEffectFinishLoadGame(RE::ActiveEffect* a_effect);
	static inline REL::Relocation<decltype(OnEffectFinishLoadGame)> _OnEffectFinishLoadGame;
};
