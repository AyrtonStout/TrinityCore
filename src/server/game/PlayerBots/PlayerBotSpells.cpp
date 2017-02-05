#include "PlayerBot.h"
#include "PlayerBotSpells.h"

std::map<PlayerBotSpell, SpellDescriptor> PlayerBot::m_spellLookup;

SpellDescriptor CreateDescriptor(PlayerBotSpell spell, SpellCastTargetFlags flags)
{
    SpellDescriptor tmp = { spell, flags };
    return tmp;
}

void PlayerBot::SetUpSpells()
{
    PlayerBot::m_spellLookup[SPELL_POWER_WORD_SHIELD] = CreateDescriptor(SPELL_POWER_WORD_SHIELD, TARGET_FLAG_UNIT);
    PlayerBot::m_spellLookup[SPELL_LESSER_HEAL] = CreateDescriptor(SPELL_LESSER_HEAL, TARGET_FLAG_UNIT);
    PlayerBot::m_spellLookup[SPELL_SMITE] = CreateDescriptor(SPELL_SMITE, TARGET_FLAG_UNIT);
}
