#pragma once
// https://www.pcg-random.org/posts/ease-of-use-without-loss-of-power.html
#include "randutils.hpp"
#include <unordered_map>

// Offsets
//static float* f_SecondsSinceLastFrame_RealTime = (float*)RELOCATION_ID(1013228, 1013228).address();
static float* f_SecondsSinceLastFrame_RealTime = (float*)REL::ID(1013228).address();
// 1388854
// TaskQueueInterface thread
//static std::uint32_t* TaskthreadID = (std::uint32_t*)REL::ID(1370673).address();

// Queue Toggle Pipboy Light
typedef void(_fastcall* tQueuedTogglePipboyLight)(RE::TaskQueueInterface*);
static REL::Relocation<tQueuedTogglePipboyLight> QueuedTogglePipboyLight{ REL::ID(588241) };

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

	static void TurnOnFlashlight();
	static void TurnOffFlashlight();

private:
	randutils::default_rng rng;
	float GetRandomFloat(float min, float max);
	std::uint8_t GetRandomInt8(std::uint8_t min, std::uint8_t max);

	// Hooked funcs
	static RE::BSLight* GenerateLight(RE::TESObjectLIGH* a1, __int64 a2, RE::NiNode* a3, bool a4, bool a5, bool a6, RE::BSLight** a7, float a8, bool a9);
	static inline REL::Relocation<decltype(GenerateLight)> _GenerateLight;

	static void HandlePipboyLightHotkey(RE::TaskQueueInterface* a1);
	static inline REL::Relocation<decltype(HandlePipboyLightHotkey)> _HandlePipboyLightHotkey;
	
	// Vfunc
	static void Update(RE::PlayerCharacter* a_player, float a_delta);
	static inline REL::Relocation<decltype(Update)> _Update;

	static void FlickerFlashlight(RE::PlayerCharacter* a_player, std::string a_flickerType);

	//static std::vector<uint16_t>::iterator GetUniqueIDPosition(std::vector<uint16_t>& a_vector, uint16_t a_uniqueID);
	
	static inline RE::TESObjectLIGH* flashlightObject;

	

	bool bWasFlashlightOn = false;

	void InitFlickerList();
	struct FlickerCycle
	{
		// Light radius min/max percentage of default radius (for random)
		float minRadius;
		float maxRadius;
		// On time min/max (for random)
		float minOn;
		float maxOn;
		// Off time min/max (for random)
		float minOff;
		float maxOff;
	};
	std::unordered_map<std::uint8_t, std::unordered_map<std::uint8_t, FlickerCycle>> FlickerList;
	
	static inline bool bShouldFlicker = false;
	static inline float fFlickerTimer = 0.f;
	static inline std::uint8_t iChosenCycle = 0;
	static inline std::uint8_t iCycleCounter = 0;
};
