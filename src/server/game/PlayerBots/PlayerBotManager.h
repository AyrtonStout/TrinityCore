#include "Common.h"
#include "Log.h"
#include "WorldSession.h"
#include "QueryHolder.h"
//#include "WorldSession.h"
//#include "WorldSession.cpp"

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
