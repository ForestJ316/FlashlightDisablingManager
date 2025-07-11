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
	// If hex length is more than 6 digits
	if (hexStr.length() > 6) {
		int firstDigit = -1;
		// If it's 7 digits then check if it's a DLC Form
		if (hexStr.length() == 7) {
			firstDigit = std::stoi(hexStr.substr(0, 1), 0, 16); // Convert to int
		}
		// If it's not a DLC Form then keep the last 6 numbers only
		if (firstDigit > 6 || firstDigit == -1) {
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
