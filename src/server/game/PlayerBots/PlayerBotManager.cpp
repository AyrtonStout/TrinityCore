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
        std::this_thread::sleep_for(std::chrono::seconds(7));
        harry->SendChat(CHAT_MSG_SAY, "Hey girl. -Sent from my iBot");
        std::this_thread::sleep_for(std::chrono::seconds(7));
        harry->SetAFK();
        std::this_thread::sleep_for(std::chrono::seconds(7));
        harry->SendChatWithTarget(CHAT_MSG_WHISPER, "Whoa oa oa oaoooo", "Cassinia");
        std::this_thread::sleep_for(std::chrono::seconds(7));
        harry->SetDND();
        std::this_thread::sleep_for(std::chrono::seconds(7));
        harry->SetAFK("I'm afk nub");
    }
}

bool PlayerBotManager::Initialize()
{
    std::thread (ThingsAreAHappenin).detach();
    return true;
}
