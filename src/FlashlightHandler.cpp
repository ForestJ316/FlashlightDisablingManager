#include "FlashlightHandler.h"
#include "DisableEffectHandler.h"
#include "Utils.h"
#include "Offsets.h"

FlashlightHandler::FlickerData::FlickerData(const FlickerDataDefaults& cycleDefaults)
{
	timeOn = Utils::GetRandomFloat(cycleDefaults.minOn, cycleDefaults.maxOn);
	timeOff = Utils::GetRandomFloat(cycleDefaults.minOff, cycleDefaults.maxOff);
}

// Initialize flicker list hardcoded values
void FlashlightHandler::InitFlickerList()
{
	auto& flickerList = FlickerList;
	// Populate with some random cycles
	// minOn, maxOn, minOff, maxOff
	flickerList[1] = {
		{ 1, { 0.1f, 0.2f, 0.1f, 0.2f } },
		{ 2, { 0.07f, 0.12f, 0.07f, 0.12f } },
		{ 3, { 0.07f, 0.12f, 0.07f, 0.12f } },
		{ 4, { 0.05f, 0.1f, 0.05f, 0.1f } }
	};
	flickerList[2] = {
		{ 1, { 0.15f, 0.2f, 0.15f, 0.2f } },
		{ 2, { 0.1f, 0.15f, 0.1f, 0.15f } },
		{ 3, { 0.1f, 0.15f, 0.1f, 0.15f } }
	};
	flickerList[3] = {
		{ 1, { 0.05f, 0.125f, 0.05f, 0.125f } },
		{ 2, { 0.1f, 0.15f, 0.1f, 0.15f } },
		{ 3, { 0.08f, 0.12f, 0.08f, 0.12f } },
		{ 4, { 0.1f, 0.15f, 0.1f, 0.15f } }
	};
	flickerList[4] = {
		{ 1, { 0.1f, 0.1f, 0.1f, 0.1f } },
		{ 2, { 0.075f, 0.1f, 0.075f, 0.1f } },
		{ 3, { 0.075f, 0.1f, 0.075f, 0.1f } },
		{ 4, { 0.1f, 0.1f, 0.1f, 0.1f } }
	};
	flickerList[5] = {
		{ 1, { 0.075f, 0.125f, 0.075f, 0.125f } },
		{ 2, { 0.1f, 0.1f, 0.1f, 0.1f } },
		{ 3, { 0.08f, 0.1f, 0.8f, 0.1f } }
	};
}

void FlashlightHandler::Initialize()
{
	logger::info("Initializing FlashlightHandler...");
	const auto UI = RE::UI::GetSingleton();
	if (UI) {
		UI->RegisterSink<RE::MenuOpenCloseEvent>(FlashlightHandler::GetSingleton());
	}
	// Hooked TogglePipboyLight() function in task unpack
	_PipboyLightTaskUnpack = F4SE::GetTrampoline().write_call<5>(REL::ID(1546751).address() + 0x2717, PipboyLightTaskUnpack);
	// Play Pipboy Audio
	_PlayPipboyAudio = F4SE::GetTrampoline().write_call<5>(REL::ID(520007).address() + 0x23, PlayPipboyAudio);
	// Hook Notify Light Event for flickering
	_NotifyPipboyLightEventLock = F4SE::GetTrampoline().write_call<5>(REL::ID(261449).address() + 0x28, NotifyPipboyLightEventLock);

	// Vfunc hook OnUpdate
	_Update = REL::Relocation<uintptr_t>(RE::VTABLE::PlayerCharacter[0]).write_vfunc(0xCF, Update);

	FlashlightHandler::GetSingleton()->InitFlickerList();

	logger::info("...FlashlightHandler initialized.");
}

FlashlightHandler::EventResult FlashlightHandler::ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
	// Check in case Pipboy Menu was opened during flickering, then ignore close event
	if (!a_event.opening && a_event.menuName == "PipboyMenu"sv && bWasFlashlightOn && sFlickerType == "") {
		// Flashlight turned off while in Pipboy Menu
		if (RE::PlayerCharacter::GetSingleton()->IsPipboyLightOn()) {
			InitFlashlightFlicker("Off");
		}
		// Flashlight turned on while in Pipboy Menu
		else if (DisableEffectHandler::GetSingleton()->iActiveFlashlightEffectCount == 0) {
			bWasFlashlightOn = false;
			InitFlashlightFlicker("On");
		}
	}
	return EventResult::kContinue;
}

void FlashlightHandler::TurnOnFlashlight()
{
	// Turn on only if it was on before
	if (FlashlightHandler::GetSingleton()->bWasFlashlightOn) {
		auto a_player = RE::PlayerCharacter::GetSingleton();
		// If it's currently flickering then turn the flashlight on after the flicker
		if (sFlickerType != "") {
			// Treat it as off flicker for the purpose of turning it on after
			if (sFlickerType == "Hotkey") {
				sFlickerType = "Off";
			}
			sForceOnOff = "ForceOn";
			FlashlightHandler::GetSingleton()->bWasFlashlightOn = false;
		}
		else if (!a_player->IsPipboyLightOn()) {
			// If Pipboy Menu is open then it will get enabled on menu close
			if (!RE::UI::GetSingleton()->GetMenuOpen("PipboyMenu"sv)) {
				FlashlightHandler::GetSingleton()->bWasFlashlightOn = false;
				InitFlashlightFlicker("On");
			}
		}
		// If it somehow got turned on just reset vars
		else if (a_player->IsPipboyLightOn()) {
			ResetVars(true);
		}
	}	
}

void FlashlightHandler::TurnOffFlashlight()
{
	// IsPipboyLightOn() can return true while flickering, so have an extra flag for multiple effect toggles
	// The flag will also serve as a check whether the flashlight was on before
	if (!FlashlightHandler::GetSingleton()->bWasFlashlightOn) {
		auto a_player = RE::PlayerCharacter::GetSingleton();
		// If it's currently flickering then turn the flashlight off after the flicker
		if (sFlickerType != "") {
			sForceOnOff = "ForceOff";
			FlashlightHandler::GetSingleton()->bWasFlashlightOn = true;
		}
		else if (a_player->IsPipboyLightOn() || RE::PipboyManager::GetSingleton()->wasPipboyLightActive) {
			FlashlightHandler::GetSingleton()->bWasFlashlightOn = true;
			// Flashlight is always off in Pipboy Menu, so have to do it on menu close instead
			if (a_player->IsPipboyLightOn()) {
				InitFlashlightFlicker("Off");
			}
		}
	}
}

void FlashlightHandler::PipboyLightTaskUnpack(RE::PlayerCharacter* a_player, bool a_unk)
{
	// If a toggle got queued during flickering, only execute the queued toggles (in case of racing with other functions or plugins)
	if (!bQueuedToggle.empty()) {
		bQueuedToggle.pop();
		_PipboyLightTaskUnpack(a_player, a_unk);
	}
	// Don't mess with the flashlight during flickering
	else if (sFlickerType == "") {
		// Assume hotkey got pressed
		if (DisableEffectHandler::GetSingleton()->iActiveFlashlightEffectCount > 0) {
			// If flashlight somehow got turned on
			if (RE::PlayerCharacter::GetSingleton()->IsPipboyLightOn()) {
				_PipboyLightTaskUnpack(a_player, a_unk);
			}
			std::chrono::duration<float> fHotkeyDelay = std::chrono::system_clock::now() - FlashlightHandler::GetSingleton()->hotkeySystemTimeSnapshot;
			// 10 second delay between hotkey flickers to prevent spamming
			if (fHotkeyDelay.count() >= 10.f) {
				FlashlightHandler::GetSingleton()->hotkeySystemTimeSnapshot = std::chrono::system_clock::now();
				FlashlightHandler::GetSingleton()->InitFlashlightFlicker("Hotkey");
			}
		}
		else if (DisableEffectHandler::GetSingleton()->iActiveFlashlightEffectCount == 0) {
			_PipboyLightTaskUnpack(a_player, a_unk);
		}
	}
}

void FlashlightHandler::InitFlashlightFlicker(std::string a_flickerType)
{
	auto iRandomFlicker = Utils::GetRandomInt(1, (int)FlickerList.size());
	if (auto chosenFlicker = FlickerList.find(iRandomFlicker); chosenFlicker != FlickerList.end()) {
		for (const auto& cycleData : chosenFlicker->second) {
			Flicker.insert({ cycleData.first, cycleData.second });
		}
		sFlickerType = a_flickerType;
		bQueuedToggle.push(true);
		QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
	}
}

void FlashlightHandler::PlayPipboyAudio(const char* a1)
{
	auto flashlightHandler = FlashlightHandler::GetSingleton();
	// If there is no disabling effect then play audio like regular
	if (flashlightHandler->Flicker.empty()
		// Or if turning on, then play audio on last On action, skip last Off action
		// Except if turning off while flashlight is flickering on (can race for double audio, but rare enough compromise)
		|| (sFlickerType == "On" && flashlightHandler->Flicker.size() == 1 && sForceOnOff != "ForceOff")
		// Or if turning off, then play audio on last Off action
		// Except if turning on while flashlight is flickering off (can race for double audio, but rare enough compromise)
		|| (sFlickerType == "Off" && flashlightHandler->Flicker.size() == 1 && RE::PlayerCharacter::GetSingleton()->IsPipboyLightOn() && sForceOnOff != "ForceOn")) {
		_PlayPipboyAudio(a1);
	}
}

// Old Notify in ShowPipboyLight
/*
RE::BSTEventSource<RE::PipboyLightEvent>* FlashlightHandler::NotifyPipboyLightEvent(RE::BSTEventSource<RE::PipboyLightEvent>* a1, const RE::PipboyLightEvent& a2)
{
	if (sFlickerType != "") {
		auto flashlightHandler = FlashlightHandler::GetSingleton();
		auto currentCycle = flashlightHandler->Flicker.begin();
		if (currentCycle != flashlightHandler->Flicker.end()) {
			auto a_player = RE::PlayerCharacter::GetSingleton();
			// IsPipboyLightOn() is set before the Notify, so can catch on turn on or turn off
			if (a_player->IsPipboyLightOn()) {
				fNextFlicker = currentCycle->second.timeOn;
				// Skip the last Off call
				// If turned off while flickering on then do last Off call
				if (sFlickerType == "On" && flashlightHandler->Flicker.size() == 1) {
					flashlightHandler->Flicker.erase(currentCycle);
					std::string wasForceType = sForceOnOff;
					flashlightHandler->ResetVars(false);
					// If got turned off while flickering to on, do another regular toggle
					// (can be double audio with last On call, but it's rare enough case and compromise for potential racing)
					if (wasForceType == "ForceOff") {
						bQueuedToggle.push(true);
						QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
						return nullptr;
					}
					return _NotifyPipboyLightEvent(a1, a2);
				}
			}
			else {
				fNextFlicker = currentCycle->second.timeOff;
				// Cycle ends when flashlight turns off
				flashlightHandler->Flicker.erase(currentCycle);
				// If the whole cycle ended then stop checking OnUpdate
				if (flashlightHandler->Flicker.empty()) {
					std::string wasFlicker = sFlickerType;
					std::string wasForceType = sForceOnOff;
					flashlightHandler->ResetVars(false);
					// If got turned on while flickering to off, do another regular toggle
					// (can be double audio on last Off call, but it's rare enough case and compromise for potential racing)
					if (wasForceType == "ForceOn") {
						bQueuedToggle.push(true);
						QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
						return nullptr;
					}
					// When turning flashlight off, send Notify at end of flicker
					if (wasFlicker == "Off") {
						return _NotifyPipboyLightEvent(a1, a2);
					}
				}
			}
			return nullptr;
		}
	}
	return _NotifyPipboyLightEvent(a1, a2);
}
*/

void FlashlightHandler::NotifyPipboyLightEventLock(RE::BSSpinLock* a1, const char* a2)
{
	// BSSpinLock::Lock
	_NotifyPipboyLightEventLock(a1, a2);

	if (sFlickerType != "") {
		auto flashlightHandler = FlashlightHandler::GetSingleton();
		auto currentCycle = flashlightHandler->Flicker.begin();
		if (currentCycle != flashlightHandler->Flicker.end()) {
			auto a_player = RE::PlayerCharacter::GetSingleton();
			// IsPipboyLightOn() is set before the Notify, so can catch on turn on or turn off
			if (a_player->IsPipboyLightOn()) {
				fNextFlicker = currentCycle->second.timeOn;
				// Skip the last Off call
				// If turned off while flickering on then do last Off call
				if (sFlickerType == "On" && flashlightHandler->Flicker.size() == 1) {
					flashlightHandler->Flicker.erase(currentCycle);
					std::string wasForceType = sForceOnOff;
					flashlightHandler->ResetVars(false);
					// If got turned off while flickering to on, do another regular toggle
					// (can be double audio with last On call, but it's rare enough case and compromise for potential racing)
					if (wasForceType == "ForceOff") {
						bQueuedToggle.push(true);
						QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
					}
				}
			}
			else {
				fNextFlicker = currentCycle->second.timeOff;
				// Cycle ends when flashlight turns off
				flashlightHandler->Flicker.erase(currentCycle);
				// If the whole cycle ended then stop checking OnUpdate
				if (flashlightHandler->Flicker.empty()) {
					std::string wasForceType = sForceOnOff;
					flashlightHandler->ResetVars(false);
					// If got turned on while flickering to off, do another regular toggle
					// (can be double audio on last Off call, but it's rare enough case and compromise for potential racing)
					if (wasForceType == "ForceOn") {
						bQueuedToggle.push(true);
						QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
					}
				}
			}
		}
	}
}

void FlashlightHandler::Update(RE::PlayerCharacter* a_player, float a_delta)
{
	_Update(a_player, a_delta);

	// Start checking from when flashlight actually turns on/off (after Notify event)
	if (sFlickerType != "" && fNextFlicker > 0.f) {
		fTimer += *f_SecondsSinceLastFrame_RealTime;
		if (fTimer >= fNextFlicker) {
			fNextFlicker = 0.f;
			fTimer = 0.f;
			bQueuedToggle.push(true);
			QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
		}
	}
}

void FlashlightHandler::ResetVars(bool a_fullDefault)
{
	if (a_fullDefault) {
		bWasFlashlightOn = false;
	}
	bQueuedToggle = std::queue<bool>();
	sFlickerType = "";
	fTimer = 0.f;
	fNextFlicker = 0.f;
	sForceOnOff = "";
}
