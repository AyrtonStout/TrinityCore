#include "PlayerBotManager.h"
#include "TestBotties.h"
//#include "CharacterHandler.cpp"

PlayerBotManager::PlayerBotManager()
{
    //whatup
}


PlayerBotManager::~PlayerBotManager()
{
}

PlayerBotManager* PlayerBotManager::instance()
{
    static PlayerBotManager instance;
    return &instance;
}

bool PlayerBotManager::Initialize()
{
    TC_LOG_INFO("server", "DOING IMPORTANT INITIALIZATION STUFF");

    TestBotties *woot = new TestBotties();
    woot->HelloWorldThingie();


    uint64 whaddup = 1;
    ObjectGuid *playerGuid = new ObjectGuid(whaddup);

    TC_LOG_INFO("server", "DOING IMPORTANT INITIALIZATION STUFF 2");
    //Create byte buffer and put guid in it.
    //ByteBuffer *sup = new ByteBuffer();
    TC_LOG_INFO("server", "DOING IMPORTANT INITIALIZATION STUFF 3");
    //*sup << 1;
    TC_LOG_INFO("server", "DOING IMPORTANT INITIALIZATION STUFF 4");
    //sup->read();
    //*sup >> *playerGuid;
    TC_LOG_INFO("server", "DOING IMPORTANT INITIALIZATION STUFF 5");

    //WorldPacket *woot = new WorldPacket();
    //&woot << 1;
    //WorldPacket &whales = *woot;

    uint32 accountId = 1;
    //LoginQueryHolder *holder = new LoginQueryHolder(accountId, *playerGuid);
    //HandlePlayerLogin(holder);
    /*
    if (!holder->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        return false;
    }
    */

    return true;
}
