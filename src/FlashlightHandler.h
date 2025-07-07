#pragma once
#include <unordered_map>

class FlashlightHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
private:
	using EventResult = RE::BSEventNotifyControl;

public:
	static FlashlightHandler* GetSingleton()
	{
		static FlashlightHandler singleton;
		return std::addressof(singleton);
	}

	static void Initialize();

	EventResult ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*);

	void TurnOnFlashlight();
	void TurnOffFlashlight();
	void ResetVars(bool a_fullDefault);

private:
	// Order of funcs on pipboy light: PipboyLightTaskUnpack -> PlayPipboyAudio -> GenerateLight -> NotifyPipboyLightEvent
	// Hooked funcs
	static void PipboyLightTaskUnpack(RE::PlayerCharacter* a_player, bool a_unk);
	static inline REL::Relocation<decltype(PipboyLightTaskUnpack)> _PipboyLightTaskUnpack;

	static void PlayPipboyAudio(const char* a1);
	static inline REL::Relocation<decltype(PlayPipboyAudio)> _PlayPipboyAudio;

	static RE::BSTEventSource<RE::PipboyLightEvent>* NotifyPipboyLightEvent(RE::BSTEventSource<RE::PipboyLightEvent>* a1, const RE::PipboyLightEvent& a2);
	static inline REL::Relocation<decltype(NotifyPipboyLightEvent)> _NotifyPipboyLightEvent;
	/*
	static RE::BSLight* GenerateLight(RE::TESObjectLIGH* a1, __int64 a2, RE::NiNode* a3, bool a4, bool a5, bool a6, RE::BSLight** a7, float a8, bool a9);
	static inline REL::Relocation<decltype(GenerateLight)> _GenerateLight;
	*/
	// Vfunc
	static void Update(RE::PlayerCharacter* a_player, float a_delta);
	static inline REL::Relocation<decltype(Update)> _Update;

	struct FlickerDataDefaults
	{
		// Light radius min/max percentage of default radius (for random)
		//float minRadius;
		//float maxRadius;
		// On time min/max (for random)
		float minOn;
		float maxOn;
		// Off time min/max (for random)
		float minOff;
		float maxOff;
	};
	struct FlickerData
	{
		FlickerData(const FlickerDataDefaults& cycleDefaults);
		// Randomized values
		//float newRadius;
		float timeOn;
		float timeOff;
	};
	std::unordered_map<int, std::map<int, FlickerDataDefaults>> FlickerList;
	std::map<int, FlickerData> Flicker;

	void InitFlickerList();

	void InitFlashlightFlicker(std::string a_flickerType);
	
	bool bWasFlashlightOn = false;

	//RE::TESObjectLIGH* FlashlightLight;
	//std::uint32_t iDefaultRadius = 0;

	static inline std::string sFlickerType = "";
	static inline float fTimer = 0.f;
	static inline float fNextFlicker = 0.f;
	static inline std::queue<bool> bQueuedToggle;

	static inline std::string sForceOnOff = "";
};
