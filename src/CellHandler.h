#pragma once
#include <unordered_set>

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
	static inline std::unordered_set<std::uint32_t> CellList;

	bool bEnteredDisabledCell = false;

	class FaderMenuEvent : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	private:
		using EventResult = RE::BSEventNotifyControl;

	public:
		static FaderMenuEvent* GetSingleton()
		{
			static FaderMenuEvent singleton;
			return std::addressof(singleton);
		}
		EventResult ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*);
	};
};
