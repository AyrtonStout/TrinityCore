#ifndef __PLAYERBOT_H
#define __PLAYERBOT_H

#include "Common.h"
#include "GridNotifiers.h"
#include "Log.h"
#include "Player.h"
#include "PlayerBotSpells.h"
#include "WorldSession.h"

class TC_GAME_API PlayerBot
{
public:
    PlayerBot(uint64 playerGuid, uint32 accountId);
    ~PlayerBot();

    void Login();

    void SendChat(ChatMsg chatType, std::string chatMessage);
    void SendWhisper(std::string target, std::string chatMessage);
    void SendChannelMessage(std::string channel, std::string message);
    void SetAFK(std::string afkMessage = "");
    void SetDND(std::string dndMessage = "");

    void HandleChat(ChatMsg chatType, Language language, uint64 senderGuid, uint64 receiverGuid, std::string message, uint32 achievementId);

    void TargetNearestPlayer();
    void TargetSelf();

    void RequestDuel();
    void AcceptDuel();
    void RejectDuel();
    void HandleDuelRequest(uint64 challengerGuid);
    bool IsDueling();

    void StartAttack();
    void StopAttack();
    void SetWeaponSheath(SheathState state);

    void CastSpell(PlayerBotSpell spell);

    uint64 GetGuid() { return m_playerGuid; }

private:
    uint64 m_playerGuid;
    uint32 m_accountId;
    WorldSession* m_session;

    std::string GetCurrentZoneName();
    Player* GetNearestPlayer();
};

#endif
