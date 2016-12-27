#include "PlayerBot.h"

/*
 * Sending an emote in this way like "/dance" will not register as a dance. This is for custom emotes.
 */
void PlayerBot::SendChat(ChatMsg chatType, std::string chatMessage)
{
    TC_LOG_INFO("server", "Chat was sent");
    WorldPacket *packet = new WorldPacket();
    *packet << (uint32) chatType;
    *packet << (uint32) LANG_COMMON;
    *packet << chatMessage;
    m_session->HandleMessagechatOpcode(*packet);
}

void PlayerBot::SendChatWithTarget(ChatMsg chatType, std::string chatMessage, std::string target)
{
    TC_LOG_INFO("server", "Target chat was sent");
    WorldPacket *packet = new WorldPacket();
    *packet << (uint32) chatType;
    *packet << (uint32) LANG_COMMON;
    *packet << target;
    *packet << chatMessage;
    m_session->HandleMessagechatOpcode(*packet);
}

/*
 * Channel is a string like "General", "LocalDefense", 
 */
void PlayerBot::SendChannelMessage(std::string channel, std::string message)
{
    WorldPacket *packet = new WorldPacket();
    *packet << (uint32) CHAT_MSG_CHANNEL;
    *packet << (uint32) LANG_COMMON;
    *packet << channel + " - " + GetCurrentZoneName();
    *packet << message;
    m_session->HandleMessagechatOpcode(*packet);
}

void PlayerBot::SetAFK(std::string afkMessage)
{
    TC_LOG_INFO("server", "AFK was sent");
    WorldPacket *packet = new WorldPacket();
    *packet << (uint32) CHAT_MSG_AFK;
    *packet << (uint32) LANG_UNIVERSAL;
    *packet << afkMessage;
    m_session->HandleMessagechatOpcode(*packet);
}

void PlayerBot::SetDND(std::string afkMessage)
{
    TC_LOG_INFO("server", "DND was sent");
    WorldPacket *packet = new WorldPacket();
    *packet << (uint32) CHAT_MSG_DND;
    *packet << (uint32) LANG_UNIVERSAL;
    *packet << afkMessage;
    m_session->HandleMessagechatOpcode(*packet);
}

std::string PlayerBot::GetCurrentZoneName()
{
    Player *player = m_session->GetPlayer();
    uint32 areaId = player->GetAreaId();
    std::string zoneName = "Unknown";
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId))
    {
        int locale = m_session->GetSessionDbcLocale();
        std::string areaName = area->area_name[locale];
        if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(area->zone))
            zoneName = zone->area_name[locale];
    }
    
    return zoneName;
}
