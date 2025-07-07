#pragma once

static float* f_SecondsSinceLastFrame_RealTime = (float*)REL::ID(1013228).address();

// Queue Toggle Pipboy Light
typedef void(_fastcall* tQueuedTogglePipboyLight)(RE::TaskQueueInterface*);
static REL::Relocation<tQueuedTogglePipboyLight> QueuedTogglePipboyLight{ REL::ID(588241) };

// Add Spell
typedef void(_fastcall* tActorAddSpell)(RE::PlayerCharacter*, RE::SpellItem*);
static REL::Relocation<tActorAddSpell> ActorAddSpell{ REL::ID(1433810) };
// Remove Spell
typedef void(_fastcall* tActorRemoveSpell)(RE::PlayerCharacter*, RE::SpellItem*);
static REL::Relocation<tActorRemoveSpell> ActorRemoveSpell{ REL::ID(1500183) };
// Has Spell
typedef bool(_fastcall* tActorHasSpell)(RE::PlayerCharacter*, RE::SpellItem*);
static REL::Relocation<tActorHasSpell> ActorHasSpell{ REL::ID(850247) };
