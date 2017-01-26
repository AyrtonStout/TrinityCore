#ifndef __PLAYERBOTSPELLS_H
#define __PLAYERBOTSPELLS_H

enum PlayerBotSpell {
    SPELL_POWER_WORD_SHIELD = 48065,
    SPELL_FLASH_HEAL = 48071,
    SPELL_HOLY_NOVA = 48077,
    SPELL_LESSER_HEAL = 2050,
    SPELL_SMITE = 585,
};

typedef struct SpellDescriptor {
    PlayerBotSpell spellId;
    SpellCastTargetFlags targetFlag;
};

#endif
