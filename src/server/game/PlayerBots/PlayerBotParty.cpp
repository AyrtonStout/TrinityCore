#include "PlayerBot.h"

void PlayerBot::InviteToParty()
{
    Player *self = m_session->GetPlayer();
    Unit *target = self->GetSelectedUnit();
    if (!target || target->GetTypeId() != TYPEID_PLAYER) {
        return;
    }

    InviteToParty(target->GetName());
}

void PlayerBot::InviteToParty(std::string playerName) 
{
    WorldPacket *packet = new WorldPacket();
    *packet << playerName;
    *packet << (uint32) 0; //Useless but needed for the packet parser in the handler
    m_session->HandleGroupInviteOpcode(*packet);
}

void PlayerBot::AcceptPartyInvite()
{
    WorldPacket *packet = new WorldPacket();
    *packet << (uint32) 0; //Useless but needed for the packet parser in the handler
    m_session->HandleGroupAcceptOpcode(*packet);
}

void PlayerBot::DeclinePartyInvite()
{
    m_session->HandleGroupDeclineOpcode(*(new WorldPacket()));
}

