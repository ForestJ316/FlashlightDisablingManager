#include "FlashlightHandler.h"
#include "EffectHandler.h"

float FlashlightHandler::GetRandomFloat(float a_min, float a_max)
{
	float uni_dist = rng.uniform(a_min, a_max);
	return uni_dist;
}

int FlashlightHandler::GetRandomInt(int a_min, int a_max)
{
	int uni_dist = rng.uniform(a_min, a_max);
	return uni_dist;
}

// Initialize flicker list hardcoded values
void FlashlightHandler::InitFlickerList()
{
	auto& flickerList = FlickerList;
	// Populate with some random cycles
	// minRadius, maxRadius, minOn, maxOn, minOff, maxOff
	flickerList[1] = {
		{ 1, { 0.6f, 0.75f, 0.1f, 0.2f, 0.1f, 0.2f } },
		{ 2, { 0.6f, 0.7f, 0.07f, 0.12f, 0.07f, 0.12f } },
		{ 3, { 0.3f, 0.5f, 0.07f, 0.12f, 0.07f, 0.12f } },
		{ 4, { 0.3f, 0.4f, 0.05f, 0.1f, 0.05f, 0.1f } }
	};
	flickerList[2] = {
		{ 1, { 0.9f, 1.0f, 0.15f, 0.2f, 0.15f, 0.2f } },
		{ 2, { 0.35f, 0.6f, 0.1f, 0.15f, 0.1f, 0.15f } },
		{ 3, { 0.2f, 0.3f, 0.1f, 0.15f, 0.1f, 0.15f } }
	};
	flickerList[3] = {
		{ 1, { 0.75f, 0.9f, 0.05f, 0.125f, 0.05f, 0.125f } },
		{ 2, { 0.6f, 0.75f, 0.1f, 0.15f, 0.1f, 0.15f } },
		{ 3, { 0.2f, 0.4f, 0.08f, 0.12f, 0.08f, 0.12f } },
		{ 4, { 0.2f, 0.4f, 0.1f, 0.15f, 0.1f, 0.15f } }
	};
	flickerList[4] = {
		{ 1, { 0.8f, 0.9f, 0.1f, 0.1f, 0.1f, 0.1f } },
		{ 2, { 0.4f, 0.6f, 0.075f, 0.1f, 0.075f, 0.1f } },
		{ 3, { 0.4f, 0.7f, 0.075f, 0.1f, 0.075f, 0.1f } },
		{ 4, { 0.2f, 0.4f, 0.1f, 0.1f, 0.1f, 0.1f } },
		{ 5, { 0.15f, 0.35f, 0.075f, 0.125f, 0.075f, 0.125f } }
	};
}

void FlashlightHandler::Initialize()
{
	logger::info("Initializing FlashlightHandler...");
	const auto UI = RE::UI::GetSingleton();
	if (UI) {
		UI->RegisterSink<RE::MenuOpenCloseEvent>(FlashlightHandler::GetSingleton());
	}
	// Hooked function into toggling the pipboy light with hotkey
	_HandlePipboyLightHotkey = F4SE::GetTrampoline().write_call<5>(REL::ID(181358).address() + 0x14A, HandlePipboyLightHotkey);
	// Play Pipboy Audio
	_PlayPipboyAudio = F4SE::GetTrampoline().write_call<5>(REL::ID(520007).address() + 0x23, PlayPipboyAudio);
	// Hook GenDynamic to get the TESObjectLIGH form
	_GenerateLight = F4SE::GetTrampoline().write_call<5>(REL::ID(1304102).address() + 0x28C, GenerateLight);
	// Hook Notify Light Event to stop regular functionality while flickering
	_NotifyPipboyLightEvent = F4SE::GetTrampoline().write_call<5>(REL::ID(1304102).address() + 0x472, NotifyPipboyLightEvent);
	
	// Vfunc hook OnUpdate
	_Update = REL::Relocation<uintptr_t>(RE::VTABLE::PlayerCharacter[0]).write_vfunc(0xCF, Update);

	FlashlightHandler::GetSingleton()->InitFlickerList();

	logger::info("...FlashlightHandler initialized.");
}

FlashlightHandler::EventResult FlashlightHandler::ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
	// Check in case Pipboy Menu was opened during flickering, then ignore close event
	if (!a_event.opening && a_event.menuName == "PipboyMenu"sv && FlashlightHandler::GetSingleton()->bWasFlashlightOn && sFlickerType == "") {
		// Flashlight turned off while in Pipboy Menu
		if (RE::PlayerCharacter::GetSingleton()->IsPipboyLightOn()) {
			InitFlashlightFlicker("Off");
		}
		// Flashlight turned on while in Pipboy Menu
		else if (EffectHandler::GetSingleton()->iActiveFlashlightEffectCount == 0) {
			FlashlightHandler::GetSingleton()->bWasFlashlightOn = false;
			InitFlashlightFlicker("On");
		}
	}
	return EventResult::kContinue;
}

void FlashlightHandler::TurnOnFlashlight()
{
	auto a_player = RE::PlayerCharacter::GetSingleton();
	// Turn on only if it was on before
	if (FlashlightHandler::GetSingleton()->bWasFlashlightOn) {
		// If it's currently flickering then turn the flashlight on after the flicker
		if (sFlickerType == "Off" || sFlickerType == "Hotkey") {
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
	auto a_player = RE::PlayerCharacter::GetSingleton();
	// IsPipboyLightOn() can return true while flickering, so have an extra flag for multiple effect toggles
	// The flag will also serve as a check whether the flashlight was on before
	if (!FlashlightHandler::GetSingleton()->bWasFlashlightOn) {
		// If it's currently flickering then turn the flashlight off after the flicker
		if (sFlickerType == "On") {
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

void FlashlightHandler::HandlePipboyLightHotkey(RE::TaskQueueInterface* a1)
{
	// Don't mess with the flashlight during flickering
	if (sFlickerType == "") {
		if (EffectHandler::GetSingleton()->iActiveFlashlightEffectCount > 0) {
			// If flashlight somehow got turned on
			if (RE::PlayerCharacter::GetSingleton()->IsPipboyLightOn()) {
				QueuedTogglePipboyLight(a1);
			}
			FlashlightHandler::GetSingleton()->InitFlashlightFlicker("Hotkey");
		}
		else if (EffectHandler::GetSingleton()->iActiveFlashlightEffectCount == 0) {
			QueuedTogglePipboyLight(a1);
		}
	}
}

void FlashlightHandler::InitFlashlightFlicker(std::string a_flickerType)
{
	auto flashlightHandler = FlashlightHandler::GetSingleton();
	auto& flickerList = flashlightHandler->FlickerList;
	auto iRandomFlicker = flashlightHandler->GetRandomInt(1, (int)flickerList.size());
	if (flickerList.find(iRandomFlicker) != flickerList.end()) {
		auto& chosenFlickerCycle = flickerList.find(iRandomFlicker)->second;
		for (const auto& cycleData : chosenFlickerCycle) {
			flashlightHandler->Flicker.insert({ cycleData.first, cycleData.second });
		}
		sFlickerType = a_flickerType;
		QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
	}
}

void FlashlightHandler::PlayPipboyAudio(const char* a1)
{
	auto flashlightHandler = FlashlightHandler::GetSingleton();
	// If there is no disabling effect then play audio like regular
	if (flashlightHandler->Flicker.empty()
		// Or if turning on, then play audio on first On action, skip last Off action
		// Except if turning off while flashlight is flickering on (can race for double audio, but rare enough compromise)
		|| (sFlickerType == "On" && flashlightHandler->Flicker.size() == 1 && sForceOnOff != "ForceOff")
		// Or if turning off, then play audio on last Off action
		// Except if turning on while flashlight is flickering off (can race for double audio, but rare enough compromise)
		|| (sFlickerType == "Off" && flashlightHandler->Flicker.size() == 1 && RE::PlayerCharacter::GetSingleton()->IsPipboyLightOn() && sForceOnOff != "ForceOn")) {
		_PlayPipboyAudio(a1);
	}
}

// This function doesn't execute if the IsPipboyLightOn() is false
// WARNING: Possible bug with default radius getting overwritten somewhere with a lower one (race condition? flashlight object change?)
RE::BSLight* FlashlightHandler::GenerateLight(RE::TESObjectLIGH* a1, __int64 a2, RE::NiNode* a3, bool a4, bool a5, bool a6, RE::BSLight** a7, float a8, bool a9)
{	
	// Only when flickering
	if (sFlickerType != "") {
		auto flashlightHandler = FlashlightHandler::GetSingleton();
		// Store the default radius
		if (flashlightHandler->iDefaultRadius == 0) {
			flashlightHandler->iDefaultRadius = a1->data.radius;
			flashlightHandler->FlashlightLight = a1;
		}
		auto currentCycle = flashlightHandler->Flicker.begin();
		// If flashlight currently flickering then change radius appropriately
		if (currentCycle != flashlightHandler->Flicker.end()) {
			// If turning on then skip the last off call
			a1->data.radius = (uint32_t)round(currentCycle->second.newRadius * flashlightHandler->iDefaultRadius);
		}
		// Otherwise change back to default
		else if (a1->data.radius != flashlightHandler->iDefaultRadius) {
			a1->data.radius = flashlightHandler->iDefaultRadius;
			flashlightHandler->iDefaultRadius = 0;
		}
		// If flashlight turning on then skip last off call
		if (sFlickerType == "On" && flashlightHandler->Flicker.size() == 1) {
			a1->data.radius = flashlightHandler->iDefaultRadius;
			flashlightHandler->iDefaultRadius = 0;
		}
	}
	return _GenerateLight(a1, a2, a3, a4, a5, a6, a7, a8, a9);
}


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

void FlashlightHandler::Update(RE::PlayerCharacter* a_player, float a_delta)
{
	_Update(a_player, a_delta);

	// Start checking from when flashlight actually turns on/off (after Notify event)
	if (sFlickerType != "" && fNextFlicker > 0.f) {
		fTimer += *f_SecondsSinceLastFrame_RealTime;
		if (fTimer >= fNextFlicker) {
			fNextFlicker = 0.f;
			fTimer = 0.f;
			QueuedTogglePipboyLight(RE::TaskQueueInterface::GetSingleton());
		}
	}
}

void FlashlightHandler::ResetVars(bool a_fullDefault)
{
	if (a_fullDefault) {
		auto flashlightHandler = FlashlightHandler::GetSingleton();
		flashlightHandler->bWasFlashlightOn = false;
		if (flashlightHandler->iDefaultRadius > 0 && flashlightHandler->FlashlightLight != nullptr) {
			flashlightHandler->FlashlightLight->data.radius = flashlightHandler->iDefaultRadius;
			flashlightHandler->iDefaultRadius = 0;
		}
	}
	sFlickerType = "";
	fTimer = 0.f;
	fNextFlicker = 0.f;
	sForceOnOff = "";
}
