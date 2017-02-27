#ifndef __PLAYERBOTMANAGER_H
#define __PLAYERBOTMANAGER_H

#include "Common.h"
#include "Log.h"
#include "PlayerBot.h"
#include "WorldSession.h"
#include "World.h"
#include "QueryHolder.h"

#include <thread>
#include <chrono>

class TC_GAME_API PlayerBotManager
{
private:
    PlayerBotManager();
    ~PlayerBotManager();

    void BotLoginThread();
    void BotUpdateThread();

    void HandleChatPacket(WorldPacket *packet, uint64 botGuid);
    void HandleDuelRequest(WorldPacket *packet, uint64 botGuid);

    PlayerBot *GetOfflineBot();

    std::map<uint64, PlayerBot*> m_botMap;
public:
    static PlayerBotManager* instance();
    bool Initialize();
    void HandlePacket(WorldPacket *packet, uint64 botGuid);
};

#define sPlayerBotManager PlayerBotManager::instance()

#endif
