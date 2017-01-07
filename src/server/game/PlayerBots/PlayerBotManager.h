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

    void MainThread();

    void HandleChatPacket(WorldPacket *packet, uint64 botGuid);

    std::map<uint64, PlayerBot*> m_botMap;
public:
    static PlayerBotManager* instance();
    bool Initialize();
    void HandlePacket(WorldPacket const* packet, uint64 botGuid);
};

#define sPlayerBotManager PlayerBotManager::instance()

#endif
