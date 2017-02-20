#include "PlayerBot.h"

void PlayerBot::FaceTarget()
{
    Unit *target = m_session->GetPlayer()->GetSelectedUnit();
    if (!target)
        return;

    Player *self = m_session->GetPlayer();

    if (target->GetGUID() == self->GetGUID())
        return;

    Position p = target->GetPosition();

    float angle = m_session->GetPlayer()->GetAngle(p);

    WorldPacket *packet = new WorldPacket();
    packet->SetOpcode(MSG_MOVE_SET_FACING);
    *packet << self->GetGUID().WriteAsPacked();
    *packet << (uint32) 0; //Flags1 
                           ///Search for MOVEMENTFLAG if these need to be set in the future
    *packet << (uint16) 0; //Flags2 
                           ///Search for MOVEMENTFLAG2 if these need to be set in the future
                           ///(TODO If the bot is swimming these flags will need to be set, and we will need to insert the pitch of the bot)
    *packet << (uint32) getMSTime(); //Time
    Position *newPosition = new Position(self->GetPositionX(), self->GetPositionY(), self->GetPositionZ(), angle);
    *packet << newPosition->PositionXYZOStream();
    *packet << (uint32) 0; // Fall Time
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::WalkForward()
{
    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_START_FORWARD);

    Player *self = m_session->GetPlayer();
    *packet << self->GetGUID().WriteAsPacked();
    *packet << (uint32) MOVEMENTFLAG_FORWARD; //Flags1
    *packet << (uint16) 0; //Flags2
    *packet << (uint32) getMSTime(); //Time
    Position *newPosition = new Position(self->GetPositionX(), self->GetPositionY(), self->GetPositionZ(), self->GetOrientation());
    *packet << newPosition->PositionXYZOStream();
    *packet << (uint32) 0; // Fall Time

    uint32 startTime = getMSTime();
    m_session->HandleMovementOpcodes(*packet);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    uint32 endTime = getMSTime();

    uint32 elapsedTime = endTime - startTime;

    float moveSpeed = playerBaseMoveSpeed[MOVE_RUN];

    WorldPacket *endPacket = new WorldPacket(); 
    endPacket->SetOpcode(MSG_MOVE_STOP);
    *endPacket << self->GetGUID().WriteAsPacked();
    *endPacket << (uint32) MOVEMENTFLAG_NONE; //Flags1
    *endPacket << (uint16) 0; //Flags2
    *endPacket << (uint32) getMSTime(); //Time

    float deltaX = cosf(self->GetOrientation()) * moveSpeed * (elapsedTime / 1000.0);
    float deltaY = sinf(self->GetOrientation()) * moveSpeed * (elapsedTime / 1000.0);

    float newX = self->GetPositionX() + deltaX;
    float newY = self->GetPositionY() + deltaY;
    float newZ = self->GetMap()->GetHeight(newX, newY, self->GetPositionZ());

    Position *endPosition = new Position(newX, newY, newZ, self->GetOrientation());
    *endPacket << endPosition->PositionXYZOStream();
    *endPacket << (uint32) 0; // Fall Time
    m_session->HandleMovementOpcodes(*endPacket);
}
