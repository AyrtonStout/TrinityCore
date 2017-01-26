#include "PlayerBotManager.h"

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

void PlayerBotManager::MainThread()
{
    std::this_thread::sleep_for(std::chrono::seconds(3));

    while (true)
    {
        PlayerBot *bot = GetOfflineBot();
        if (bot) {
            m_botMap[bot->GetGuid()] = bot;
            bot->Login();
        }

        //harry->TargetNearestPlayer();
        //harry->SendChat(CHAT_MSG_SAY, "hi");
        //harry->SendWhisper("Cassinia", "hi");
        //harry->SendChannelMessage("general", "whatup");

        std::this_thread::sleep_for(std::chrono::seconds(2));
        //harry->RequestDuel();
    }
}

PlayerBot *PlayerBotManager::GetOfflineBot()
{
    std::string query = "SELECT character_id, account_id "
                        "FROM character_bots cb "
                        "LEFT JOIN characters c "
                        "ON cb.character_id = c.guid "
                        "WHERE online = 0 "
                        "LIMIT 1";

    QueryResult result = CharacterDatabase.Query(query.c_str());
    if (result) {
        Field* fields = result->Fetch();
        return new PlayerBot(fields[0].GetUInt64(), fields[1].GetUInt64());
    }
    else {
        return NULL;
    }
}

bool PlayerBotManager::Initialize()
{
    PlayerBot::SetUpSpells();
    std::thread([this] { MainThread(); }).detach();
    return true;
}

void PlayerBotManager::HandleChatPacket(WorldPacket *packet, uint64 botGuid)
{
    uint8 chatType, chatTag; //Idk what chatTag is
    uint32 language, someFlagsThatMightNotEvenBeUsed, messageLength, achievementId;
    uint64 senderGuid, receiverGuid;
    std::string message, channelName;
    *packet >> chatType;
    *packet >> language;
    *packet >> senderGuid;
    *packet >> someFlagsThatMightNotEvenBeUsed;

    if (chatType == CHAT_MSG_CHANNEL) {
        *packet >> channelName;
    }

    *packet >> receiverGuid;
    *packet >> messageLength; //Message is 1 bigger than the actual string. e.g. "hi" has a length of 3
    *packet >> message;
    *packet >> chatTag;

    if (chatType == CHAT_MSG_ACHIEVEMENT || chatType == CHAT_MSG_GUILD_ACHIEVEMENT) {
        *packet >> achievementId;
    }

    PlayerBot *bot = m_botMap[botGuid];
    if (!bot)
        return;

    bot->HandleChat((ChatMsg) chatType, (Language) language, senderGuid, receiverGuid, message, achievementId);
}

void PlayerBotManager::HandleDuelRequest(WorldPacket *packet, uint64 botGuid)
{
    uint64 objectGuid, casterGuid;
    *packet >> objectGuid;
    *packet >> casterGuid;

    if (casterGuid == botGuid) {
        return; //This happens if the bot initiated the duel
    }

    PlayerBot *bot = m_botMap[botGuid];
    if (!bot)
        return;

    //The duel packet is sent before the bots are flagged as being in a duel. Wait for the flag to be set.
    for (uint8 i = 0; i < 100; i++) {
        if (bot->IsDueling()) {
            bot->AcceptDuel();
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    TC_LOG_INFO("server", "Bot was challenged to a duel but never was flagged as being in a duel");
}

void PlayerBotManager::HandlePacket(WorldPacket *packet, uint64 botGuid)
{
    if (!packet)
        return;

    switch (packet->GetOpcode())
    {
        case 66:
            TC_LOG_INFO("server", "Set login time speed");
            break;
        case 103:
            TC_LOG_INFO("server", "Contact list");
            break;
        case 150: //Can be whisper or channel or say or yell or anything really
            HandleChatPacket(packet, botGuid);
            break;
        case 153:
            TC_LOG_INFO("server", "Channel notified");
            break;
        case 169:
        case 170:
        case 215: //Animation set
        case 502:
        case 533: //Object despawn animation
            //TC_LOG_INFO("server", "Update object");
            break;
        case 181: //Start forward movement
        case 182: //Start backpedal
        case 183: //Stop movement
        case 184: //Start strafe left
        case 185: //Start strafe right
        case 186: //Stop strafe 
        case 187: //Start jump
        case 188: //Start turn left
        case 189: //Start turn right
        case 190: //Stop turn
        case 194: //Movement set run mode
        case 195: //Movement set walk mode
        case 201: //Fall land
        case 218: //Set facing
            TC_LOG_INFO("server", "Movement animation");
            break;
        case 221:
            //TC_LOG_INFO("server", "Monster moved");
            break;
        case 232:
            TC_LOG_INFO("server", "Force movement root");
            break;
        case 234:
            TC_LOG_INFO("server", "Force movement unroot");
            break;
        case 238:
            TC_LOG_INFO("server", "Movement heartbeat");
            break;
        case 253:
            TC_LOG_INFO("server", "Tutorial Flags");
            break;
        case 290:
            TC_LOG_INFO("server", "Initialize factions");
            break;
        case 297:
            TC_LOG_INFO("server", "Action buttons");
            break;
        case 298:
            TC_LOG_INFO("server", "Initial spells");
            break;
        case 295:
            TC_LOG_INFO("server", "Set proficiency");
            break;
        case 304:
            TC_LOG_INFO("server", "Spell cast failed");
            break;
        case 305: //Spell start
            TC_LOG_INFO("server", "Spell 'Start'");
            break;
        case 306: //Spell go
            TC_LOG_INFO("server", "Spell 'Go'");
            break;
        case 307: //Spell failed
        case 678: //Spell failed other
            TC_LOG_INFO("server", "Spell 'Failure'");
            break;
        case 323:
            TC_LOG_INFO("server", "Auto attack start");
            break;
        case 324:
            TC_LOG_INFO("server", "Auto attack stop");
            break;
        case 330:
            TC_LOG_INFO("server", "Attack state update");
            break;
        case 334:
            TC_LOG_INFO("server", "Cancel combat");
            break;
        case 336:
            TC_LOG_INFO("server", "Spell heal log");
            break;
        case 341:
            TC_LOG_INFO("server", "Bind point update");
            break;
        case 359: //Seems to fire for the person who requested it (and maybe also the receiver)
            HandleDuelRequest(packet, botGuid);
            break;
        case 362:
            TC_LOG_INFO("server", "Duel complete");
            break;
        case 363:
            TC_LOG_INFO("server", "Duel winner");
            break;
        case 494:
            TC_LOG_INFO("server", "Auth response");
            break;
        case 521:
            TC_LOG_INFO("server", "Account data times");
            break;
        case 566:
            TC_LOG_INFO("server", "Login verify world");
            break;
        case 588:
            TC_LOG_INFO("server", "Spell log execute");
            break;
        case 592:
            TC_LOG_INFO("server", "Spell non damage log");
            break;
        case 652:
            TC_LOG_INFO("server", "PvP Credit");
            break;
        case 677:
            TC_LOG_INFO("server", "Set forced reactions");
            break;
        case 695:
            TC_LOG_INFO("server", "Duel countdown");
            break;
        case 706: //Init
        case 707: //Update
            TC_LOG_INFO("server", "Change world state");
            break;
        case 751:
            TC_LOG_INFO("server", "Addon info");
            break;
        case 756:
            TC_LOG_INFO("server", "Update weather");
            break;
        case 781:
        case 782:
            //TC_LOG_INFO("server", "Set run/walk mode");
            break;
        case 809:
        case 827:
            TC_LOG_INFO("server", "Set dungeon difficulty");
            break;
        case 829:
            TC_LOG_INFO("server", "Message of the day");
            break;
        case 912:
            //TC_LOG_INFO("server", "Time sync");
            break;
        case 969:
            TC_LOG_INFO("server", "Feature system status");
            break;
        case 1008:
            TC_LOG_INFO("server", "Userlist add");
            break;
        case 1009:
            TC_LOG_INFO("server", "Userlist remove");
            break;
        case 1054:
            TC_LOG_INFO("server", "Send unlearn spells");
            break;
        case 1109:
            TC_LOG_INFO("server", "Learned dance moves");
            break;
        case 1130:
            TC_LOG_INFO("server", "Criteria update");
            break;
        case 1149:
            TC_LOG_INFO("server", "All achievement data");
            break;
        case 1152:
            TC_LOG_INFO("server", "Power update");
            break;
        case 1173:
        case 1174:
            TC_LOG_INFO("server", "Aura update");
            break;
        case 1195:
            TC_LOG_INFO("server", "Client cache version");
            break;
        case 1212:
            TC_LOG_INFO("server", "Equipment set list");
            break;
        case 1216:
            TC_LOG_INFO("server", "Talents info");
            break;
        default:
            TC_LOG_INFO("server", std::to_string(packet->GetOpcode()));
    }
}
