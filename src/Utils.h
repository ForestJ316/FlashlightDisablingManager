#pragma once
// https://www.pcg-random.org/posts/ease-of-use-without-loss-of-power.html
#include "randutils.hpp"
#include <unordered_map>
#include <unordered_set>

namespace Utils
{
	static randutils::default_rng rng;
	inline float GetRandomFloat(float a_min, float a_max)
	{
		float uni_dist = rng.uniform(a_min, a_max);
		return uni_dist;
	}
	inline int GetRandomInt(int a_min, int a_max)
	{
		int uni_dist = rng.uniform(a_min, a_max);
		return uni_dist;
	}
	
	inline std::string GetIniHexFormID(std::string a_iniEntry)
	{
		auto splitPos = a_iniEntry.find("|");
		if (splitPos != std::string::npos) {
			return a_iniEntry.substr(0, splitPos);
		}
		// Return empty string so the game doesn't crash if incorrect format gets read
		return "";
	}
	inline std::string GetIniPluginName(std::string a_iniEntry)
	{
		auto splitPos = a_iniEntry.find("|");
		if (splitPos != std::string::npos) {
			return a_iniEntry.substr(splitPos + 1, a_iniEntry.length() - 1);
		}
		// Return empty string so the game doesn't crash if incorrect format gets read
		return "";
	}
	std::string ConvertIniEntryToDecimal(std::string a_iniEntry);
	std::uint32_t GetEditedFormID(std::uint32_t a_formID);
	std::pair<std::string, float> GetDisabledIniEntryFromForm(std::unordered_map<std::string, float> a_list, RE::TESForm* a_form);
	std::string GetDisabledIniEntryFromForm(std::unordered_set<std::string> a_list, RE::TESForm* a_form);
}
