#include "PlayerBot.h"

///TODO This needs to be expanded to include swimming and flying movement types
float PlayerBot::GetSpeed()
{
    Player *self = m_session->GetPlayer();
    uint32 movementFlags = self->GetUnitMovementFlags();

    UnitMoveType moveType;
    //If you're walking you move the same speed forward and backwards
    if (movementFlags & MOVEMENTFLAG_WALKING) {
        moveType = MOVE_WALK;
    } //You move slower backpedaling than running
    else if (movementFlags & MOVEMENTFLAG_BACKWARD) {
        moveType = MOVE_RUN_BACK;
    } 
    else {
        moveType = MOVE_RUN;
    }

    return self->GetSpeed(moveType);
}

float PlayerBot::GetEffectiveOrientation()
{
    Player *self = m_session->GetPlayer();
    float effectiveOrientation = self->GetOrientation();
    uint32 movementFlags = self->GetUnitMovementFlags();
    if (movementFlags & MOVEMENTFLAG_STRAFE_LEFT) {
        if (movementFlags & MOVEMENTFLAG_FORWARD) {
            effectiveOrientation += (float) M_PI_4;
        }
        else if (movementFlags & MOVEMENTFLAG_BACKWARD) {
            effectiveOrientation -= (float) M_PI_4;
        }
        else {
            effectiveOrientation += (float) M_PI_2;
        }
    }
    else if (movementFlags & MOVEMENTFLAG_STRAFE_RIGHT) {
        if (movementFlags & MOVEMENTFLAG_FORWARD) {
            effectiveOrientation -= (float) M_PI_4;
        }
        else if (movementFlags & MOVEMENTFLAG_BACKWARD) {
            effectiveOrientation += (float) M_PI_4;
        }
        else {
            effectiveOrientation -= (float) M_PI_2;
        }
    }
    return effectiveOrientation;
}

Position* PlayerBot::CalculatePosition(float newOrientation /* NAN */)
{
    Player *self = m_session->GetPlayer();

    newOrientation = !std::isnan(newOrientation) ? newOrientation : self->GetOrientation();
    if (!self->isMoving()) {
        return new Position(self->GetPositionX(), self->GetPositionY(), self->GetPositionZ(), newOrientation);
    }

    float moveSpeed = GetSpeed();
    uint32 elapsedTime = getMSTime() - m_lastPositionUpdate;

    float effectiveOrientation = GetEffectiveOrientation();
    float deltaX = cosf(effectiveOrientation) * moveSpeed * (elapsedTime / 1000.0);
    float deltaY = sinf(effectiveOrientation) * moveSpeed * (elapsedTime / 1000.0);

    //If we are moving backwards, make the values negative
    if (self->GetUnitMovementFlags() & MOVEMENTFLAG_BACKWARD) {
        deltaX *= -1;
        deltaY *= -1;
    }

    float newX = self->GetPositionX() + deltaX;
    float newY = self->GetPositionY() + deltaY;
    float newZ = self->GetMap()->GetHeight(newX, newY, self->GetPositionZ());

    return new Position(newX, newY, newZ, newOrientation);
}

void PlayerBot::BuildMovementPacket(WorldPacket *packet, uint32 movementFlags, float orientation /* NAN */)
{
    Player *self = m_session->GetPlayer();
    *packet << self->GetGUID().WriteAsPacked();
    *packet << (uint32) movementFlags; //Flags1
    *packet << (uint16) self->GetExtraUnitMovementFlags(); //Flags2
    *packet << (uint32) getMSTime(); //Time
    *packet << CalculatePosition(orientation)->PositionXYZOStream();
    *packet << (uint32) 0; // Fall Time
}

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

    BuildMovementPacket(packet, self->GetUnitMovementFlags(), angle);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::RPWalk(bool rpWalk)
{
    Player *self = m_session->GetPlayer();

    uint32 movementFlags;
    WorldPacket *packet = new WorldPacket(); 
    if (rpWalk) {
        packet->SetOpcode(MSG_MOVE_SET_WALK_MODE);
        movementFlags = self->GetUnitMovementFlags() | MOVEMENTFLAG_WALKING;
    }
    else {
        packet->SetOpcode(MSG_MOVE_SET_RUN_MODE);
        movementFlags = self->GetUnitMovementFlags() & ~MOVEMENTFLAG_WALKING; //If we were already walking, remove the flag
    }

    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StartWalkingForward()
{
    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_START_FORWARD);

    Player *self = m_session->GetPlayer();
    uint32 movementFlags = self->GetUnitMovementFlags() | MOVEMENTFLAG_FORWARD;
    movementFlags = movementFlags & ~MOVEMENTFLAG_BACKWARD; //If we were walking backward, remove that flag
    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StartWalkingBackward()
{
    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_START_BACKWARD);

    Player *self = m_session->GetPlayer();
    uint32 movementFlags = self->GetUnitMovementFlags() | MOVEMENTFLAG_BACKWARD;
    movementFlags = movementFlags & ~MOVEMENTFLAG_FORWARD; //If we were walking forward, remove that flag
    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::SendMovementHeartbeat()
{
    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_HEARTBEAT);

    Player *self = m_session->GetPlayer();
    BuildMovementPacket(packet, self->GetUnitMovementFlags());

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StopWalkingStraight()
{
    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_STOP);

    Player *self = m_session->GetPlayer();

    uint32 movementFlags = self->GetUnitMovementFlags();
    movementFlags = movementFlags & ~(MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD); //Remove these flags from movementFlags

    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StartStrafingLeft()
{
    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_START_STRAFE_LEFT);

    Player *self = m_session->GetPlayer();
    uint32 movementFlags = self->GetUnitMovementFlags() | MOVEMENTFLAG_STRAFE_LEFT;
    movementFlags = movementFlags & ~MOVEMENTFLAG_STRAFE_RIGHT; //If we were strafing right, remove that flag
    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StartStrafingRight()
{
    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_START_STRAFE_RIGHT);

    Player *self = m_session->GetPlayer();
    uint32 movementFlags = self->GetUnitMovementFlags() | MOVEMENTFLAG_STRAFE_RIGHT;
    movementFlags = movementFlags & ~MOVEMENTFLAG_STRAFE_LEFT; //If we were strafing left, remove that flag
    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StopStrafing()
{
    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_STOP_STRAFE);

    Player *self = m_session->GetPlayer();

    uint32 movementFlags = self->GetUnitMovementFlags();
    movementFlags = movementFlags & ~(MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT); //Remove these flags from movementFlags

    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}
