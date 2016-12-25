#include "PlayerBot.h"

PlayerBot::PlayerBot(uint64 playerGuid, uint32 accountId) : m_playerGuid(playerGuid), m_accountId(accountId)
{
    std::string&& accountName = "GMGUY"; //Wtf is this double ampersand? It's an 'rvalue reference' http://stackoverflow.com/questions/5481539/what-does-t-double-ampersand-mean-in-c11
    AccountTypes accountType = SEC_PLAYER; //This might be wrong
    uint8 expansion = 2; //Magic constant!! This means WotLK
    time_t muteTime = 0; //This seems to prevent players from talking. These are bots. If I make them spam and cuss it's my own problem and I'll hail it a victory.
    LocaleConstant locale = LOCALE_enUS;
    uint32 recruiter = 0; //I believe this is the ID of the account that recruited this account. Not 100% sure. Probably doesn't matter anyway here.
    bool isARecruiter = false;
    bool isPlayerBot = true; //I sure hope so!

    m_session = new WorldSession(accountId, accountName.c_str(), NULL, accountType, expansion, muteTime, locale, recruiter, isARecruiter, isPlayerBot);
    sWorld->AddSession(m_session);
}

PlayerBot::~PlayerBot()
{
}

void PlayerBot::Login()
{
    m_session->LoginBot(m_playerGuid); //This is asynchronous, and needs to be waited for
    while (!m_session->GetPlayer() || m_session->GetPlayer()->IsLoading()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    m_session->GetPlayer()->UpdateLocalChannels(m_session->GetPlayer()->GetZoneId()); //Join channels like 'General' and 'Trade'
}

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