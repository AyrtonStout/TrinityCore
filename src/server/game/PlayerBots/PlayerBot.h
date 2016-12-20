#ifndef __PLAYERBOT_H
#define __PLAYERBOT_H

#include "Common.h"
#include "Log.h"
#include "WorldSession.h"

class TC_GAME_API PlayerBot
{
public:
    PlayerBot(uint64 playerGuid, uint32 accountId);
    ~PlayerBot();

    void Login();
    void SendChat(ChatMsg chatType, std::string chatMessage);
    void SendChatWithTarget(ChatMsg chatType, std::string chatMessage, std::string target);
    void SendChannelMessage(std::string channel, std::string zone, std::string message);
    void SetAFK(std::string afkMessage = "");
    void SetDND(std::string dndMessage = "");

private:
    uint64 m_playerGuid;
    uint32 m_accountId;
    WorldSession* m_session;
};

#endif
