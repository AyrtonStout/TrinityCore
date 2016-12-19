#ifndef __PLAYERBOTMANAGER_H
#define __PLAYERBOTMANAGER_H

#include "Common.h"
#include "Log.h"
#include "WorldSession.h"
#include "QueryHolder.h"

class TC_GAME_API PlayerBotManager
{
private:
    PlayerBotManager();
    ~PlayerBotManager();
public:
    static PlayerBotManager* instance();
    bool Initialize();
};

#define sPlayerBotManager PlayerBotManager::instance()

#endif
