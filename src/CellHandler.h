#pragma once
#include <unordered_set>


// Add Spell
typedef void(_fastcall* tActorAddSpell)(RE::PlayerCharacter*, RE::SpellItem*);
static REL::Relocation<tActorAddSpell> ActorAddSpell{ REL::ID(1433810) };
// Remove Spell
typedef void(_fastcall* tActorRemoveSpell)(RE::PlayerCharacter*, RE::SpellItem*);
static REL::Relocation<tActorRemoveSpell> ActorRemoveSpell{ REL::ID(1500183) };
// Has Spell
typedef bool(_fastcall* tActorHasSpell)(RE::PlayerCharacter*, RE::SpellItem*);
static REL::Relocation<tActorHasSpell> ActorHasSpell{ REL::ID(850247) };


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

	static inline RE::SpellItem* CellSpell;
	bool bEnteredDisabledCell = false;

private:
	static inline std::unordered_set<std::uint32_t> DisabledCells;

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
