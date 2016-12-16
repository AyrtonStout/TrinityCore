#include "PlayerBotManager.h"
#include "TestBotties.h"
#include "WorldSession.h"
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

    std::string&& accountName = "GMGUY"; //Wtf is this double ampersand? It's an 'rvalue reference' http://stackoverflow.com/questions/5481539/what-does-t-double-ampersand-mean-in-c11
    AccountTypes accountType = SEC_PLAYER; //This might be wrong
    uint8 expansion = 2; //Magic constant!! This means WotLK
    time_t muteTime = 0; //This seems to prevent players from talking. These are bots. If I make them spam and cuss it's my own problem and I'll hail it a victory.
    LocaleConstant locale = LOCALE_enUS;
    uint32 recruiter = 0; //I believe this is the ID of the account that recruited this account. Not 100% sure. Probably doesn't matter anyway here.
    bool isARecruiter = false;
    bool isPlayerBot = true; //I sure hope so!

    WorldSession *sesh = new WorldSession(accountId, accountName.c_str(), NULL, accountType, expansion, muteTime, locale, recruiter, isARecruiter, isPlayerBot);

    return true;
}
