#include "FlashlightHandler.h"
#include "CellHandler.h"
#include "DisableEffectHandler.h"
#include "WeatherEffectHandler.h"

namespace
{
	void InitializeLog()
	{
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = logger::log_directory();
		if (!path) {
			util::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("{}.log"sv, Version::NAME.data());
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
		const auto level = spdlog::level::trace;
#else
		const auto level = spdlog::level::info;
#endif

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%H:%M:%S:%e] %v"s);
	}

	void MessageHandler(F4SE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type)
		{
			case F4SE::MessagingInterface::kGameDataReady:
				FlashlightHandler::Initialize();
				CellHandler::Initialize();
				DisableEffectHandler::Initialize();
				WeatherEffectHandler::Initialize();
				break;
			case F4SE::MessagingInterface::kPreLoadGame:
				logger::info("In PreLoadGame");
				FlashlightHandler::GetSingleton()->ResetVars(true);
				DisableEffectHandler::GetSingleton()->ResetVars();
				if (RE::PlayerCharacter::GetSingleton()->parentCell) {
					logger::info("parent cell exists in preload game, with formid: {}", RE::PlayerCharacter::GetSingleton()->parentCell->formID);
				}
			case F4SE::MessagingInterface::kPostLoadGame:
				logger::info("In PostLoadGame");
				WeatherEffectHandler::GetSingleton()->ApplyWeatherCheckToPlayer();
			default:
				break;
		}
	}

	extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
	{
		a_info->infoVersion = F4SE::PluginInfo::kVersion;
		a_info->name = Version::NAME.data();
		a_info->version = Version::MAJOR;

		if (a_f4se->IsEditor()) {
			logger::critical("loaded in editor");
			return false;
		}

		const auto ver = a_f4se->RuntimeVersion();
		if (ver < F4SE::RUNTIME_1_10_163) {
			logger::critical("unsupported runtime v{}", ver.string());
			return false;
		}

		return true;
	}

	extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
	{
		F4SE::Init(a_f4se);
		F4SE::AllocTrampoline(256);

		InitializeLog();
		logger::info(FMT_STRING("{}: {}.{}.{}"), Version::NAME.data(), Version::MAJOR, Version::MINOR, Version::PATCH);
		logger::info("Game version: {}", a_f4se->RuntimeVersion().string());

		if (!F4SE::GetMessagingInterface()->RegisterListener(MessageHandler))
		{
			logger::critical("Cannot register listener!");
			return false;
		}		
		return true;
	}

	extern "C" DLLEXPORT constinit auto F4SEPlugin_Version = []() noexcept {
		F4SE::PluginVersionData data{};

		data.PluginName(Version::NAME.data());
		data.PluginVersion(Version::MAJOR);
		data.AuthorName("ForestJ316");
		data.UsesAddressLibrary(true);
		data.UsesSigScanning(false);
		data.IsLayoutDependent(true);
		data.HasNoStructUse(false);
		data.CompatibleVersions({ F4SE::RUNTIME_1_10_163 });

		return data;
	}();
}
