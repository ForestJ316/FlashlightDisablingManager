#include "Utils.h"

std::string Utils::ConvertIniEntryToDecimal(std::string a_iniEntry)
{
	std::string sHexFormID = GetIniHexFormID(a_iniEntry);
	std::string sPluginName = GetIniPluginName(a_iniEntry);
	if (sHexFormID != "" && sPluginName != "") {
		auto iDecFormID = std::stoi(sHexFormID, 0, 16);
		return std::to_string(iDecFormID) + "|" + sPluginName;
	}
	// Return empty string so the game doesn't crash if incorrect format gets read
	return "";
}

std::uint32_t Utils::GetEditedFormID(std::uint32_t a_formID)
{
	// Convert to hex
	std::stringstream hexForm;
	hexForm << std::hex << a_formID;
	auto hexStr = hexForm.str();
	hexForm.str(std::string()); // Clear stringstream for next iteration
	// Only check hex with more than 6 digits
	if (hexStr.length() > 6) {
		// Max length is 8 digits, we care to check only the first 2
		int firstDigits = std::stoi(hexStr.substr(0, hexStr.length() - 6), 0, 16);
		// If it's not a DLC Form then keep the last 6 numbers only
		// DLC Forms go from 1 to 6
		if (firstDigits > 6) {
			hexStr = hexStr.substr(hexStr.length() - 6, hexStr.length());
		}
	}
	// Convert back to decimal
	return std::stoi(hexStr, 0, 16);
}

std::pair<std::string, float> Utils::GetDisabledIniEntryFromForm(std::unordered_map<std::string, float> a_list, RE::TESForm* a_form)
{
	auto editedFormID = GetEditedFormID(a_form->formID);
	std::string editedFormIDStr = std::to_string(editedFormID);
	// Check if the Form ID is in the file
	auto sourceFiles = a_form->sourceFiles.array->data();
	for (std::uint32_t i = 0; i < a_form->sourceFiles.array->size(); ++i)
	{
		// Format the form + file in the ini format "formID|plugin"
		std::string formattedForm = editedFormIDStr + "|" + std::string(sourceFiles[i]->filename);
		if (auto disabledEntry = a_list.find(formattedForm); disabledEntry != a_list.end()) {
			return *disabledEntry;
		}
	}
	return std::make_pair("", -1.0f);
}

std::string Utils::GetDisabledIniEntryFromForm(std::unordered_set<std::string> a_list, RE::TESForm* a_form)
{
	auto editedFormID = GetEditedFormID(a_form->formID);
	std::string editedFormIDStr = std::to_string(editedFormID);
	// Check if the Form ID is in the file
	auto sourceFiles = a_form->sourceFiles.array->data();
	for (std::uint32_t i = 0; i < a_form->sourceFiles.array->size(); ++i)
	{
		// Format the form + file in the ini format "formID|plugin"
		std::string formattedForm = editedFormIDStr + "|" + std::string(sourceFiles[i]->filename);
		if (a_list.find(formattedForm) != a_list.end()) {
			return formattedForm;
		}
	}
	return "";
}
