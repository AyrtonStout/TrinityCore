#include "PlayerBot.h"
#include "PlayerBotManager.h"
#include "WorldSession.h"
#include "World.h"

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
    uint64 playerGuid = 1;
    uint64 playerAccount = 1;
    PlayerBot *harry = new PlayerBot(playerGuid, playerAccount);
    harry->Login();

    while (true)
    {
        harry->SendChannelMessage("General", "hey");
        std::this_thread::sleep_for(std::chrono::seconds(7));
        harry->SendChannelMessage("LocalDefense", "hey yeah");
        std::this_thread::sleep_for(std::chrono::seconds(7));
        harry->SendChannelMessage("Trade", "hey nice");
        std::this_thread::sleep_for(std::chrono::seconds(7));
    }
}

bool PlayerBotManager::Initialize()
{
    std::thread (ThingsAreAHappenin).detach();
    return true;
}
