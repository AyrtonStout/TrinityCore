#include "PlayerBot.h"
#include <Server\Packets\CombatPackets.h>

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
    m_lastUpdateTime = getMSTime();
    m_lastPositionUpdate = getMSTime();
}

void PlayerBot::Update()
{
    Player *self = m_session->GetPlayer();

    if (self->isMoving()) {
        SendMovementHeartbeat();
    }

    if (m_followingPlayer) {
        UpdateFollowingPlayer();
    }

    bool pointWalkFinished = false;
    if (m_targetPoint) {
        pointWalkFinished = UpdatePointWalk();
    }
    else if (m_isPatrolling) {
        UpdatePatrol(); //Player was told to patrol, but hasn't yet started to walk anywhere
    }

    if (pointWalkFinished) {
        if (m_isPatrolling) {
            m_patrolIndex++;
            UpdatePatrol(); //Player finished walking to a point, and should walk to the next one
        }
        else {
            StopWalkingStraight(); //Player has nowhere else to walk to, and should stop
        }
    }

    m_lastUpdateTime = getMSTime();
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

void PlayerBot::TargetPlayerByName(std::string name)
{
    WorldPacket *packet = new WorldPacket();

    Player* player = ObjectAccessor::FindPlayerByName(name);
    if (!player) {
        uint64 noTarget = 0;
        *packet << noTarget;
    }
    else {
        *packet << player->GetGUID();
    }

    m_session->HandleSetSelectionOpcode(*packet);
}

void PlayerBot::TargetSelf()
{
    WorldPacket *packet = new WorldPacket();
    *packet << m_playerGuid;
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
            float distance = self->GetDistance(p->GetPosition());
            if (distance < minDistance) {
                minDistance = distance;
                closestPlayer = p;
            }
        }
    }

    return closestPlayer;
}

void PlayerBot::HandleChat(ChatMsg chatType, Language language, uint64 senderGuid, uint64 receiverGuid, std::string message, uint32 achievementId)
{
    if (message == "duel?") {
        TargetNearestPlayer();
        RequestDuel();
    }
    else if (message == "party?") {
        InviteToParty();
    }
    else if (message.substr(0, 6) == "party ") {
        InviteToParty(message.substr(6));
    }
    else if (message == "attack") {
        StartAttack();
    }
    else if (message == "stop") {
        StopAttack();
    }
    else if (message == "draw") {
        SetWeaponSheath(SHEATH_STATE_MELEE);
    }
    else if (message == "sheath") {
        SetWeaponSheath(SHEATH_STATE_UNARMED);
    }
    else if (message == "shield") {
        TargetSelf();
        CastSpell(SPELL_POWER_WORD_SHIELD);
    }
    else if (message == "lesser heal") {
        TargetSelf();
        CastSpell(SPELL_LESSER_HEAL);
    }
    else if (message == "smite") {
        TargetNearestPlayer();
        CastSpell(SPELL_SMITE);
    }
    else if (message == "target") {
        TargetNearestPlayer();
    }
    else if (message.substr(0, 7) == "target ") {
        TargetPlayerByName(message.substr(7));
    }
    else if (message == "face") {
        FaceTarget();
    }
    else if (message == "forward") {
        StartWalkingForward();
    }
    else if (message == "backward") {
        StartWalkingBackward();
    }
    else if (message == "no") {
        StopWalkingStraight();
    }
    else if (message == "rp walk on") {
        RPWalk(true);
    }
    else if (message == "rp walk off") {
        RPWalk(false);
    }
    else if (message == "strafe left") {
        StartStrafingLeft();
    }
    else if (message == "strafe right") {
        StartStrafingRight();
    }
    else if (message == "stop strafing") {
        StopStrafing();
    }
    else if (message == "turn left") {
        StartTurningLeft();
    }
    else if (message == "turn right") {
        StartTurningRight();
    }
    else if (message == "stop turning") {
        StopTurning();
    }
    else if (message == "follow") {
        FollowPlayer(senderGuid);
    }
    else if (message == "stop following") {
        StopFollowingPlayer();
    }
    else if (message.substr(0, 4) == "say ") {
        SendChat(CHAT_MSG_SAY, message.substr(4));
    }
    else if (message.substr(0, 5) == "yell ") {
        SendChat(CHAT_MSG_YELL, message.substr(5));
    }
    else if (message.substr(0, 8) == "whisper ") {
        std::string remainder = message.substr(8);
        std::size_t firstIndex = remainder.find_first_of(" ");
        std::string target = remainder.substr(0, firstIndex);

        if (remainder.length() < firstIndex + 1) {
            return;
        }

        std::string messageContent = remainder.substr(firstIndex + 1);
        SendWhisper(target, messageContent);
    }
    else if (message == "add patrol point") {
        Player *sender = ObjectAccessor::FindPlayer(ObjectGuid(senderGuid));
        Position position = sender->GetPosition();
        AddPatrolPoint(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ());
    }
    else if (message == "clear patrol") {
        ClearPatrol();
    }
    else if (message == "stop patrolling") {
        StopPatrolling();
    }
    else if (message == "start patrolling") {
        StartPatrolling();
    }
    else if (message == "reset patrol") {
        ResetPatrol();
    }
    else if (message == "tp") {
        Player *sender = ObjectAccessor::FindPlayer(ObjectGuid(senderGuid));
        TeleportToUnit(sender);
    }
}

void PlayerBot::CastSpell(PlayerBotSpell spell)
{
    Unit* target = m_session->GetPlayer()->GetSelectedUnit();
    if (!target)
        return;

    SpellDescriptor spellDesc = PlayerBot::m_spellLookup[spell];
    if (spellDesc.spellId == 0) {
        return;
    }

    WorldPacket *packet = new WorldPacket();
    *packet << (uint8) 2; //Some random counter
    *packet << (uint32) spellDesc.spellId;
    *packet << (uint8) 0; //Cast Flags
    *packet << (uint32) spellDesc.targetFlag; //Target Mask (This says that this spell has a target)
    *packet << (uint64) target->GetGUID(); //Target GUID
    m_session->HandleCastSpellOpcode(*packet);
}

void PlayerBot::StartAttack()
{
    Unit *target = m_session->GetPlayer()->GetSelectedUnit();
    if (target) {
        SetWeaponSheath(SHEATH_STATE_MELEE);

        WorldPacket packet = WorldPacket();
        packet << target->GetGUID();

        auto swingPacket = WorldPackets::Combat::AttackSwing(std::move(packet));
        m_session->HandleAttackSwingOpcode(swingPacket);
    }
}

void PlayerBot::StopAttack()
{
    WorldPacket packet = WorldPacket();

    auto swingPacket = WorldPackets::Combat::AttackStop(std::move(packet));
    m_session->HandleAttackStopOpcode(swingPacket);
}

void PlayerBot::SetWeaponSheath(SheathState state)
{
    SheathState currentState = m_session->GetPlayer()->GetSheath();
    if (currentState == state)
        return;

    WorldPacket *packet = new WorldPacket();
    *packet << (uint32) state;
    m_session->HandleSetSheathedOpcode(*packet);
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
