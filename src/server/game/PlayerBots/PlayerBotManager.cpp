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

    PlayerBot *iambotlol = new PlayerBot(56, 15);
    iambotlol->Login();

    while (true)
    {
        harry->TargetNearestPlayer();
        harry->SendChat(CHAT_MSG_SAY, "hi");
        harry->SendWhisper("Cassinia", "hi");
        harry->SendChannelMessage("general", "whatup");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool PlayerBotManager::Initialize()
{
    std::thread (ThingsAreAHappenin).detach();
    return true;
}
