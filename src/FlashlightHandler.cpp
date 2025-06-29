#include "FlashlightHandler.h"
#include "EffectHandler.h"



float FlashlightHandler::GetRandomFloat(float min, float max)
{
	float uni_dist = rng.uniform(min, max);
	return uni_dist;
}

std::uint8_t FlashlightHandler::GetRandomInt8(std::uint8_t min, std::uint8_t max)
{
	float uni_dist = rng.uniform(min, max);
	return uni_dist;
}

// Initialize flicker list hardcoded values
void FlashlightHandler::InitFlickerList()
{
	auto& flickerList = FlickerList;
	// Try 1 cycle for now
	// 1st Cycle
	flickerList[1] = {
		{ 1, {0.75, 0.9, 0.1, 0.2, 0.1, 0.2} },
		{ 2, {0.6, 0.75, 0.1, 0.2, 0.1, 0.2} },
		{ 3, {0.5, 0.6, 0.1, 0.2, 0.1, 0.2} }
	};
}

void FlashlightHandler::Initialize()
{
	logger::info("Initializing FlashlightHandler...");
	const auto UI = RE::UI::GetSingleton();
	if (UI) {
		UI->RegisterSink<RE::MenuOpenCloseEvent>(FlashlightHandler::GetSingleton());
	}
	// Hook GenDynamic to get the TESObjectLIGH form
	_GenerateLight = F4SE::GetTrampoline().write_call<5>(REL::ID(1304102).address() + 0x28C, GenerateLight);
	// Hooked function into toggling the pipboy light with hotkey
	_HandlePipboyLightHotkey = F4SE::GetTrampoline().write_call<5>(REL::ID(181358).address() + 0x14A, HandlePipboyLightHotkey);

	//auto baseAddress = REX::W32::GetModuleHandleA("Fallout4.exe");

	//auto func_start = (char*)baseAddress + 0xD65A10;

	//int64_t addr = (char*)baseAddress + 0xD65A10;

	//_QThread = (std::uint32_t(*))func_start;

	//int64_t addr = (int64_t)func_start;
	/*auto baseAddress = REX::W32::GetModuleHandleA("Fallout4.exe");
	auto func_start = (char*)baseAddress + 0xD65A10;
	uintptr_t addr = (uintptr_t)func_start;*/
	// Test threadID
	//_SetThread = F4SE::GetTrampoline().write_call<6>(addr, SetThread);

	// Vfunc hook OnUpdate
	_Update = REL::Relocation<uintptr_t>(RE::VTABLE::PlayerCharacter[0]).write_vfunc(0xCF, Update);

	//FlashlightHandler::GetSingleton()->InitRNG();
	FlashlightHandler::GetSingleton()->InitFlickerList();

	logger::info("...FlashlightHandler initialized.");
}

FlashlightHandler::EventResult FlashlightHandler::ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
	// Also check in case Pipboy Menu was opened during flickering, then ignore close event
	if (!a_event.opening && a_event.menuName == "PipboyMenu"sv && FlashlightHandler::GetSingleton()->bWasFlashlightOn && !bShouldFlicker) {
		// Flashlight turned off while in Pipboy Menu
		if (RE::PlayerCharacter::GetSingleton()->IsPipboyLightOn()) {
			logger::info("Turning OFF Flashlight..."); // DEBUG
			// Basic for now (without flicker)
			QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
		}
		// Flashlight turned on while in Pipboy Menu
		else if (EffectHandler::GetSingleton()->iActiveFlashlightEffectCount == 0) {
			FlashlightHandler::GetSingleton()->bWasFlashlightOn = false;
			logger::info("Turning ON Flashlight..."); // DEBUG
			// Basic for now (without flicker)
			QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
		}
	}
	return EventResult::kContinue;
}

void FlashlightHandler::Update(RE::PlayerCharacter* a_player, float a_delta)
{
	_Update(a_player, a_delta);


	// Testing
	/*if (bDisabled) {
		timer += *f_SecondsSinceLastFrame_RealTime;
		if (timer >= 2.f) {
			if (!a_player->IsPipboyLightOn()) {
				//static REL::Relocation<uint32_t*> threadID{ REL::ID(1370673) };
				//*threadID = REX::W32::GetCurrentThreadId();
				//a_player->ShowPipboyLight(true, false);
				QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
				logger::info("calclight: {}", a_player->currentProcess->middleHigh->calcLight);
				auto tpLight = a_player->thirdPersonLight.get();
				auto fpLight = a_player->firstPersonLight.get();
				auto pipboyLight = a_player->pipboyLight.get();
				if (pipboyLight) {
					logger::info("setting 1st person light and 3rd person light to pipboy light");
					fpLight = pipboyLight;
					tpLight = pipboyLight;
					//a_player->firstPersonLight = a_player->thirdPersonLight;
				}
				logger::info("Turning on");
			}
			else {
				//a_player->ShowPipboyLight(false, false);
				QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
				logger::info("Turning off");
			}
			timer = 0.f;
		}
	}*/
	
	/*
	//if (bIsEffectOn && ) {
	timer += *f_SecondsSinceLastFrame_RealTime;
	// On Effect Start
	if (bIsEffectOn && bWasFlashlightOn && !bDisabled) {
		// Flicker it
		
		FlickerFlashlight(a_player, timer);
		logger::info("in flickering");
	}

	// On trying to enable flashlight when it is disabled

	if (timer >= 5.f) {
		//TurnOffFlashlight();
		timer = 0.f;
	}
	*/
	//}
	//logger::info("realtime: {}", *f_SecondsSinceLastFrame_RealTime);
	
}

std::map<int, void*> testArr;

RE::BSLight* FlashlightHandler::GenerateLight(RE::TESObjectLIGH* a1, __int64 a2, RE::NiNode* a3, bool a4, bool a5, bool a6, RE::BSLight** a7, float a8, bool a9)
{
	logger::info("In GenLight");
	//logger::info("a1 fade: {}, a2: {}, a3: {}, a4: {}, a5: {}, a6: {}, a8: {}, a9: {}"sv, a1->fade, a2, a3->GetName(), a4, a5, a6, a8, a9);
	auto currentThreadID = REX::W32::GetCurrentThreadId();
	logger::info("Current Thread ID is: {}", currentThreadID);
	//a1->fade = 0.5;
	
	//flashlightObject = a1;

	//bDisabled = true; // For testing
	//auto taskQueue = RE::TaskQueueInterface::GetSingleton();
	//logger::info("thread before set is: {}", taskQueue->QProcessingThread());
	//taskQueue->SetProcessingThread(currentThreadID);
	


	//logger::info("thread after set is: {}", taskQueue->QProcessingThread());
	//SetThread(currentThreadID);

	//logger::info("Task Thread ID is: {}", test);


	return _GenerateLight(a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

void FlashlightHandler::HandlePipboyLightHotkey(RE::TaskQueueInterface* a1)
{
	logger::info("Attempting to toggle Pipboy Light on or off."); // DEBUG
	if (EffectHandler::GetSingleton()->iActiveFlashlightEffectCount > 0) {
		// Do flicker
		// DEBUG
		/*for (int i = 0; i < 20; ++i) {
			auto test = FlashlightHandler::GetSingleton()->GetRandomFloat(1.f, 20.f);
			logger::info("i is: {}, random float between 1-20 is: {}", i, test);
		}*/
		FlickerFlashlight(RE::PlayerCharacter::GetSingleton(), "Regular");
	}
	else {
		QueuedTogglePipboyLight(a1);
	}
}

void FlashlightHandler::TurnOnFlashlight()
{
	auto a_player = RE::PlayerCharacter::GetSingleton();
	// Turn on only if it was on before
	if (!a_player->IsPipboyLightOn() && FlashlightHandler::GetSingleton()->bWasFlashlightOn) {
		// If Pipboy Menu is open then it will get enabled on menu close
		if (!RE::UI::GetSingleton()->GetMenuOpen("PipboyMenu"sv)) {
			FlashlightHandler::GetSingleton()->bWasFlashlightOn = false;
			logger::info("Turning ON Flashlight..."); // DEBUG
			// Basic for now (without flicker)
			QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
		}
	}
}

void FlashlightHandler::TurnOffFlashlight()
{
	auto a_player = RE::PlayerCharacter::GetSingleton();
	// IsPipboyLightOn() can return true while flickering, so have an extra flag for multiple effect toggles
	// The flag will also serve as a check whether the flashlight was on before
	if ((a_player->IsPipboyLightOn() || RE::PipboyManager::GetSingleton()->wasPipboyLightActive) && !FlashlightHandler::GetSingleton()->bWasFlashlightOn) {
		FlashlightHandler::GetSingleton()->bWasFlashlightOn = true;
		// Flashlight is always off in Pipboy Menu, so have to do it on menu close instead
		if (a_player->IsPipboyLightOn()) {
			logger::info("Turning OFF Flashlight..."); // DEBUG
			// Basic for now (without flicker)
			QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
		}
	}
}

void FlashlightHandler::FlickerFlashlight(RE::PlayerCharacter* a_player, std::string a_flickerType)
{
	// Add regular flicker for now
	// Have to add a check in case the time randomized to before TaskQueueInterface toggled the flashlight
	// In which case keep checking it every x amount of seconds (not every frame) until it's valid and can do the next cycle
	if (a_flickerType == "Regular") {
		iChosenCycle = FlashlightHandler::GetSingleton()->GetRandomInt8(1, FlashlightHandler::GetSingleton()->FlickerList.size());
		if (a_player->IsPipboyLightOn()) {

		}
		else {

		}
		bShouldFlicker = true;
	}
}
