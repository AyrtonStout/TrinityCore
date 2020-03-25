#ifndef __PLAYERBOT_H
#define __PLAYERBOT_H

#include "Common.h"
#include <G3D/Vector3.h>
#include "GridNotifiers.h"
#include "Log.h"
#include "Map.h"
#include "PathGenerator.h"
#include "Player.h"
#include "PlayerBotSpells.h"
#include "WorldSession.h"

class TC_GAME_API PlayerBot
{
public:
    PlayerBot(uint64 playerGuid, uint32 accountId);
    ~PlayerBot();

    static void SetUpSpells();

    void Login();
    void Update();

    void SendChat(ChatMsg chatType, std::string chatMessage);
    void SendWhisper(std::string target, std::string chatMessage);
    void SendChannelMessage(std::string channel, std::string message);
    void SetAFK(std::string afkMessage = "");
    void SetDND(std::string dndMessage = "");

    void HandleChat(ChatMsg chatType, Language language, uint64 senderGuid, uint64 receiverGuid, std::string message, uint32 achievementId);

    void TargetNearestPlayer();
    void TargetPlayerByName(std::string name);
    void TargetPlayer(Player *player);
    void TargetSelf();

    void FaceUnit(Unit *unit);
    void FaceTarget();
    void FacePosition(Position p);

    void RequestDuel();
    void AcceptDuel();
    void RejectDuel();
    void HandleDuelRequest(uint64 challengerGuid);
    bool IsDueling();
    bool IsDuelInProgress();

    void AttackUnit(Unit* unit);
    void StartAttack();
    void StopAttack();
    void SetWeaponSheath(SheathState state);

    void CastSpell(PlayerBotSpell spell);

    void StartWalkingForward();
    void StartWalkingBackward();
    void StartTurningLeft();
    void StartTurningRight();
    void StopTurning();
    void StopWalkingStraight(); //This means to stop walking forward, OR stop backpedaling
    void StartStrafingLeft();
    void StartStrafingRight();
    void StopStrafing();
    void SendMovementHeartbeat(); //If the bot is moving and hasn't changed direction in the last second, it needs to broadcast a heartbeat packet
    void RPWalk(bool rpWalk);
    void StopAllWalking();

    void TeleportToUnit(const Unit *unit);

    void FollowPlayer(uint64 playerGuid);
    void StopFollowingPlayer();
    void UpdateFollowingPlayer();
    bool WalkToPoint(float x, float y, float z);
    bool WalkToPoint(Position p);

    void AddPatrolPoint(float x, float y, float z);
    void ClearPatrol();
    void StartPatrolling();
    void StopPatrolling();
    void ResetPatrol();

    void InviteToParty();
    void InviteToParty(std::string playerName);
    void AcceptPartyInvite();
    void DeclinePartyInvite();

    uint64 GetGuid() { return m_playerGuid; }

private:
    uint64 m_playerGuid;
    uint32 m_accountId;
    WorldSession* m_session;
    uint32 m_lastUpdateTime;
    uint32 m_lastPositionUpdate;

    std::vector<G3D::Vector3*> *m_patrolPath = NULL;
    uint8 m_patrolIndex = 0;
    bool m_isPatrolling = false;
    std::mutex m_patrolLock;

    G3D::Vector3* m_targetPoint = NULL;
    Player* m_followingPlayer = NULL;

    std::mutex m_pointWalkLock;

    float GetSpeed();
    float GetMinFollowDistance();
    float GetEffectiveOrientation();
    Position* CalculatePosition(float newOrientation = NAN);
    void BuildMovementPacket(WorldPacket* packet, uint32 MovementFlags, float orientation = NAN);

    bool UpdatePointWalk();
    void UpdatePatrol();

    static std::map<PlayerBotSpell, SpellDescriptor> m_spellLookup;

    void GeneratePath(float x, float y, float z);
    Position* GetIntermediatePoint(Position p); //This is used to get a point between the bot and the position

    std::string GetCurrentZoneName();
    Player* GetNearestPlayer();
};

#endif
