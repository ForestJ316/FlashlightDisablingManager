#pragma once

class DisableEffectHandler
{
public:
	static DisableEffectHandler* GetSingleton()
	{
		static DisableEffectHandler singleton;
		return std::addressof(singleton);
	}
	static void Initialize();

	int iActiveFlashlightEffectCount = 0;

private:
	static void OnEffectStart(RE::ActiveEffect* a_effect);
	static inline REL::Relocation<decltype(OnEffectStart)> _OnEffectStart;

	static void OnEffectFinish(RE::ActiveEffect* a_effect);
	static inline REL::Relocation<decltype(OnEffectFinish)> _OnEffectFinish;

	static void OnEffectFinishLoadGame(RE::ActiveEffect* a_effect);
	static inline REL::Relocation<decltype(OnEffectFinishLoadGame)> _OnEffectFinishLoadGame;

	static inline RE::EffectSetting* FlashlightEffectID;

	std::string sActionToDoAfterMenu = "";

	class FaderMenuEvent : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	private:
		using EventResult = RE::BSEventNotifyControl;

	public:
		static FaderMenuEvent* GetSingleton()
		{
			static FaderMenuEvent singleton;
			return std::addressof(singleton);
		}
		EventResult ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*);
	};
};
