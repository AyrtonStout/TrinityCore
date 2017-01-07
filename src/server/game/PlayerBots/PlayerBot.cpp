#include "PlayerBot.h"

PlayerBot::PlayerBot(uint64 playerGuid, uint32 accountId) : m_playerGuid(playerGuid), m_accountId(accountId)
{
    std::string&& accountName = "GMGUY"; //Wtf is this double ampersand? It's an 'rvalue reference' http://stackoverflow.com/questions/5481539/what-does-t-double-ampersand-mean-in-c11
    AccountTypes accountType = SEC_PLAYER; //This might be wrong
    uint8 expansion = 2; //Magic constant!! This means WotLK
    time_t muteTime = 0; //This seems to prevent players from talking. These are bots. If I make them spam and cuss it's my own problem and I'll hail it a victory.
    LocaleConstant locale = LOCALE_enUS;
    uint32 recruiter = 0; //I believe this is the ID of the account that recruited this account. Not 100% sure. Probably doesn't matter anyway here.
    bool isARecruiter = false;
    bool isPlayerBot = true; //I sure hope so!

    m_session = new WorldSession(accountId, accountName.c_str(), NULL, accountType, expansion, muteTime, locale, recruiter, isARecruiter, isPlayerBot);
    sWorld->AddSession(m_session);
}

PlayerBot::~PlayerBot()
{
}

void PlayerBot::Login()
{
    m_session->LoginBot(m_playerGuid); //This is asynchronous, and needs to be waited for
    while (!m_session->GetPlayer() || m_session->GetPlayer()->IsLoading()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    m_session->GetPlayer()->UpdateLocalChannels(m_session->GetPlayer()->GetZoneId()); //Join channels like 'General' and 'Trade'
}

void PlayerBot::TargetNearestPlayer()
{
    WorldPacket *packet = new WorldPacket();

    Player* nearestPlayer = GetNearestPlayer();
    if (!nearestPlayer) {
        uint64 noTarget = 0;
        *packet << noTarget;
    }
    else {
        *packet << nearestPlayer->GetGUID();
    }

    m_session->HandleSetSelectionOpcode(*packet);
}

Player* PlayerBot::GetNearestPlayer()
{
    Player *self = m_session->GetPlayer();

    std::list<Player*> nearbyPlayers;
    self->GetPlayerListInGrid(nearbyPlayers, self->GetVisibilityRange());

    //The first value can't be NULL because then a player distance of 0 is treated as NULL. 
    //I tried NAN instead and that also didn't work (not sure why). So instead I just start it as a huge number.
    float minDistance = std::numeric_limits<float>::max();
    Player* closestPlayer = NULL;
    for (Player* p : nearbyPlayers) {
        if (p->GetGUID() != self->GetGUID()) { 
            float distance = self->GetDistance(p);
            if (distance < minDistance) {
                minDistance = distance;
                closestPlayer = p;
            }
        }
    }

    return closestPlayer;
}

void PlayerBot::RequestDuel()
{
    Unit* target = m_session->GetPlayer()->GetSelectedUnit();
    if (!target)
        return;

    WorldPacket *packet = new WorldPacket();
    *packet << (uint8) 2; //Cast Count (No idea what this is used for)
    *packet << (uint32) 7266; //Spell ID
    *packet << (uint8) 0; //Cast Flags
    *packet << (uint32) 2; //Target Mask (This says that this spell has a target)
    *packet << (uint64) target->GetGUID(); //Target GUID
    m_session->HandleCastSpellOpcode(*packet);
}

void PlayerBot::HandleChat(ChatMsg chatType, Language language, uint64 senderGuid, uint64 receiverGuid, std::string message, uint32 achievementId)
{
    if (message == "duel?") {
        TargetNearestPlayer();
        RequestDuel();
    }
}

/* This may be a better way to do it (mostly copied from Creature.cpp), but this has a linker error for some reason
Player* PlayerBot::GetNearestPlayer()
{
    Player* self = m_session->GetPlayer();
    Player* target = nullptr;

    Trinity::NearestPlayerInObjectRangeCheck checker(self, self->GetVisibilityRange());
    Trinity::PlayerLastSearcher<Trinity::NearestPlayerInObjectRangeCheck> searcher(self, target, checker);
    self->VisitNearbyObject(self->GetVisibilityRange(), searcher);

    return target;
}
*/
