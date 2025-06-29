#pragma once
#include "Utils.h"
#include <unordered_map>

class CellHandler : public RE::BSTEventSink<RE::BGSActorCellEvent>
{
private:
	using EventResult = RE::BSEventNotifyControl;

public:
	static CellHandler* GetSingleton()
	{
		static CellHandler singleton;
		return std::addressof(singleton);
	}
	EventResult ProcessEvent(const RE::BGSActorCellEvent& a_event, RE::BSTEventSource<RE::BGSActorCellEvent>*);

	static void Initialize();

private:
	static inline RE::SpellItem* CellDisableLightSpell;
	static inline std::unordered_set<std::string> CellList;

	bool bEnteredDisabledCell = false;

	class LoadingMenuEvent : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	private:
		using EventResult = RE::BSEventNotifyControl;

	public:
		static LoadingMenuEvent* GetSingleton()
		{
			static LoadingMenuEvent singleton;
			return std::addressof(singleton);
		}
		EventResult ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*);
	};
};
