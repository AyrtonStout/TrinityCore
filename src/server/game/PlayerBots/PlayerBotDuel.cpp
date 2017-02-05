#include "PlayerBot.h"

void PlayerBot::RequestDuel()
{
    Unit* target = m_session->GetPlayer()->GetSelectedUnit();
    if (!target)
        return;

    WorldPacket *packet = new WorldPacket();
    *packet << (uint8) 2; //Random mysterious spell counter
    *packet << (uint32) 7266; //Spell ID
    *packet << (uint8) 0; //Cast Flags
    *packet << (uint32) TARGET_FLAG_UNIT; //Target Mask (This says that this spell has a target)
    *packet << (uint64) target->GetGUID(); //Target GUID
    m_session->HandleCastSpellOpcode(*packet);
}

void PlayerBot::HandleDuelRequest(uint64 challengerGuid)
{

}

void PlayerBot::AcceptDuel()
{
    TC_LOG_INFO("server", "Bot is attempting to accept duel");
    WorldPacket *packet = new WorldPacket();
    *packet << uint64(0); //The duel handler expects to find a GUID in it but doesn't actually use it
    m_session->HandleDuelAcceptedOpcode(*packet);
}

void PlayerBot::RejectDuel()
{
    WorldPacket *packet = new WorldPacket();
    *packet << uint64(0); //The duel handler expects to find a GUID in it but doesn't actually use it
    m_session->HandleDuelCancelledOpcode(*packet);
}

bool PlayerBot::IsDueling()
{
    return m_session->GetPlayer()->duel != NULL;
}

