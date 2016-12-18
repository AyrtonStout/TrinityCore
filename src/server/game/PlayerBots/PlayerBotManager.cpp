#include "PlayerBotManager.h"
#include "TestBotties.h"
#include "WorldSession.h"
#include "World.h"
//#include "CharacterHandler.cpp"

#include <thread>
#include <chrono>

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

/*
This function is just a proof of concept. This code might belong in the bots themselves as a "Login" function.
*/
void ThingsAreAHappenin()
{

    std::this_thread::sleep_for(std::chrono::seconds(3));
    TestBotties *woot = new TestBotties();
    woot->HelloWorldThingie();


    uint64 whaddup = 1;
    ObjectGuid *playerGuid = new ObjectGuid(whaddup);

    uint32 accountId = 1;
    uint64 playerId = 1;


    std::string&& accountName = "GMGUY"; //Wtf is this double ampersand? It's an 'rvalue reference' http://stackoverflow.com/questions/5481539/what-does-t-double-ampersand-mean-in-c11
    AccountTypes accountType = SEC_PLAYER; //This might be wrong
    uint8 expansion = 2; //Magic constant!! This means WotLK
    time_t muteTime = 0; //This seems to prevent players from talking. These are bots. If I make them spam and cuss it's my own problem and I'll hail it a victory.
    LocaleConstant locale = LOCALE_enUS;
    uint32 recruiter = 0; //I believe this is the ID of the account that recruited this account. Not 100% sure. Probably doesn't matter anyway here.
    bool isARecruiter = false;
    bool isPlayerBot = true; //I sure hope so!

    WorldSession *sesh = new WorldSession(accountId, accountName.c_str(), NULL, accountType, expansion, muteTime, locale, recruiter, isARecruiter, isPlayerBot);
    sWorld->AddSession(sesh);

    sesh->LoginBot(playerId);

}

bool PlayerBotManager::Initialize()
{
    std::thread (ThingsAreAHappenin).detach();
    return true;
}
