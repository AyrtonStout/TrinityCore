#include "PlayerBot.h"

constexpr float MIN_FOLLOW_DISTANCE = 1.3f;
constexpr float MIN_ATTACK_DISTANCE = 0.6f;

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
    TC_LOG_INFO("server", "Calculate Position");
    Player *self = m_session->GetPlayer();

    newOrientation = !std::isnan(newOrientation) ? newOrientation : self->GetOrientation();
    if (!self->isMoving() && !self->isTurning()) {
        return new Position(self->GetPositionX(), self->GetPositionY(), self->GetPositionZ(), newOrientation);
    }

    float moveSpeed = GetSpeed();
    float turnSpeed = (self->isMoving()) ? 2650 : 2000; //Number of milliseconds it takes to rotate 360 degrees

    uint32 elapsedTime = getMSTime() - m_lastPositionUpdate;

    float effectiveOrientation = GetEffectiveOrientation();

    float orientationChange = 0;
    if (self->isTurning()) {
        orientationChange = (M_PI * 2) * (elapsedTime / turnSpeed);
        if (self->GetUnitMovementFlags() & MOVEMENTFLAG_LEFT) {
            newOrientation += orientationChange;
            //effectiveOrientation += orientationChange;
        }
        else {
            newOrientation -= orientationChange;
            //effectiveOrientation -= orientationChange;
        }
    }


    float deltaX, deltaY;
    if (self->isMoving()) {
        if (self->isTurning()) {
            //effectiveOrientation += M_PI_2;
            float offset = fmod(effectiveOrientation + M_PI_2, 2 * M_PI);
            float radius = (moveSpeed * (turnSpeed / 1000)) / (2 * M_PI);
            float distance = 2 * sinf(orientationChange / 2) * radius * 1.1;
            deltaX = cosf(orientationChange / 2.0 + offset) * distance; ///TODO maybe make this negative
        	deltaY = sinf(orientationChange / 2.0 + offset) * distance; ///TODO maybe make this negative
        }
        else {
            TC_LOG_INFO("server", "Effective Orientation: %f, Elapsed Time: %d", effectiveOrientation, elapsedTime);
            deltaX = cosf(effectiveOrientation) * moveSpeed * (elapsedTime / 1000.0);
            deltaY = sinf(effectiveOrientation) * moveSpeed * (elapsedTime / 1000.0);
        }
    }
    else {
        deltaX = 0;
        deltaY = 0;
    }

    //If we are moving backwards, make the values negative
    if (self->GetUnitMovementFlags() & MOVEMENTFLAG_BACKWARD) {
        deltaX *= -1;
        deltaY *= -1;
    }

    float newX = self->GetPositionX() + deltaX;
    float newY = self->GetPositionY() + deltaY;

    auto map = self->GetMap();

    // GetHeight() doesn't like it if you have a Z height that is below the current map's minimum height. So use our existing Z
    // if we have it, but use the minimum grid height if it's higher. This works for most cases, but doesn't seem to work properly
    // if you go indoors, or need to use the height of objects. For example, the blacksmith in goldshire has stairs that go up
    // off of the ground, and the bot ignores these stairs and just walks through the blacksmith
    float currentZ = std::max(self->GetPositionZ(), map->GetGridHeight(newX, newY));
    float newZ = map->GetHeight(newX, newY, currentZ);

    TC_LOG_INFO("server", "speed: %f, turnSpeed: %f, e-Orientation: %f, o-change: %f, dX: %f, dY: %f, newX: %f, newY: %f, newZ: %f",
        moveSpeed, turnSpeed, effectiveOrientation, orientationChange, deltaX, deltaY, newX, newY, newZ);
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

void PlayerBot::FaceUnit(Unit *unit)
{
    if (!unit)
        return;

    Player* self = m_session->GetPlayer();

    if (unit->GetGUID() == self->GetGUID())
        return;

    FacePosition(unit->GetPosition());
}

void PlayerBot::FaceTarget()
{
    Unit *target = m_session->GetPlayer()->GetSelectedUnit();
    FaceUnit(target);
}

void PlayerBot::FacePosition(Position p)
{
    Player *self = m_session->GetPlayer();

    float angle = self->GetAbsoluteAngle(p);

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
    Player *self = m_session->GetPlayer();

    uint32 movementFlags = self->GetUnitMovementFlags();

    if ((movementFlags & (MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD)) == 0) { //We aren't moving. Do nothing
        return;
    }

    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_STOP);

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
    Player *self = m_session->GetPlayer();

    uint32 movementFlags = self->GetUnitMovementFlags();
    if ((movementFlags & (MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT)) == 0) { //We aren't moving. Do nothing
        return;
    }

    WorldPacket *packet = new WorldPacket(); 
    packet->SetOpcode(MSG_MOVE_STOP_STRAFE);

    movementFlags = movementFlags & ~(MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT); //Remove these flags from movementFlags

    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StartTurningLeft()
{
    WorldPacket *packet = new WorldPacket();
    packet->SetOpcode(MSG_MOVE_START_TURN_LEFT);

    Player *self = m_session->GetPlayer();

    uint32 movementFlags = self->GetUnitMovementFlags() | MOVEMENTFLAG_LEFT;
    movementFlags = movementFlags & ~MOVEMENTFLAG_RIGHT; //If we were turning right, remove that flag

    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StartTurningRight()
{
    WorldPacket *packet = new WorldPacket();
    packet->SetOpcode(MSG_MOVE_START_TURN_RIGHT);

    Player *self = m_session->GetPlayer();

    uint32 movementFlags = self->GetUnitMovementFlags() | MOVEMENTFLAG_RIGHT;
    movementFlags = movementFlags & ~MOVEMENTFLAG_LEFT; //If we were turning right, remove that flag

    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StopTurning()
{
    Player *self = m_session->GetPlayer();

    uint32 movementFlags = self->GetUnitMovementFlags();
    if ((movementFlags & (MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT)) == 0) { //We aren't moving. Do nothing
        return;
    }

    WorldPacket *packet = new WorldPacket();
    packet->SetOpcode(MSG_MOVE_STOP_TURN);

    movementFlags = movementFlags & ~(MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT); //Remove these flags from movementFlags

    BuildMovementPacket(packet, movementFlags);

    m_lastPositionUpdate = getMSTime();
    m_session->HandleMovementOpcodes(*packet);
}

void PlayerBot::StopAllWalking()
{
    StopFollowingPlayer();
    StopWalkingStraight();
    StopStrafing();
    StopTurning();
    if (m_targetPoint) {
        delete m_targetPoint;
        m_targetPoint = NULL;
    }
}

void PlayerBot::FollowPlayer(uint64 playerGuid)
{
    Player *self = m_session->GetPlayer();
    ObjectGuid *guid = new ObjectGuid(playerGuid);
    Unit *target = ObjectAccessor::GetUnit(*self, *guid);
    if (!target || target->GetTypeId() != TYPEID_PLAYER) {
        return;
    }

    m_followingPlayer = (Player *) target;
}

void PlayerBot::UpdateFollowingPlayer()
{
    Player *self = m_session->GetPlayer();
    m_pointWalkLock.lock();

    float distance = self->GetDistance(m_followingPlayer->GetPosition());
    if (distance > self->GetVisibilityRange()) {
        delete m_targetPoint;
        m_targetPoint = NULL;
        m_followingPlayer = NULL;
        StopWalkingStraight();
        m_pointWalkLock.unlock();
        return;
    }
    m_pointWalkLock.unlock();
    if (distance > GetMinFollowDistance()) {
        Position *offsetPosition = GetIntermediatePoint(m_followingPlayer->GetPosition());
        WalkToPoint(*offsetPosition);
        delete offsetPosition;
    }
}

void PlayerBot::StopFollowingPlayer()
{
    m_pointWalkLock.lock();

    if (!m_followingPlayer) {
        m_pointWalkLock.unlock();
        return;
    }

    m_followingPlayer = NULL;
    if (m_targetPoint) {
        delete m_targetPoint;
        m_targetPoint = NULL;
    }
    m_targetPoint = NULL;

    m_pointWalkLock.unlock();

    StopWalkingStraight();
}

bool PlayerBot::UpdatePointWalk()
{
    TC_LOG_INFO("server", "Update Point Walk");
    Player *self = m_session->GetPlayer();
    m_pointWalkLock.lock();
    if (!m_targetPoint) {
        m_pointWalkLock.unlock();
        return false;
    }

    float distance = self->GetDistance(m_targetPoint->x, m_targetPoint->y, m_targetPoint->z);

    if (distance < GetMinFollowDistance() || distance > self->GetVisibilityRange()) {
        delete m_targetPoint;
        m_targetPoint = NULL;
        m_pointWalkLock.unlock();
        return true;
    }
    m_pointWalkLock.unlock();
    return false;
}

float PlayerBot::GetMinFollowDistance()
{
    return IsDuelInProgress() ? MIN_ATTACK_DISTANCE : MIN_FOLLOW_DISTANCE;
}

bool PlayerBot::WalkToPoint(float x, float y, float z)
{
    return WalkToPoint(Position(x, y, z));
}

bool PlayerBot::WalkToPoint(Position p)
{
    TC_LOG_INFO("server", "Walk to point");
    Player *self = m_session->GetPlayer();

    GetIntermediatePoint(p);
    m_pointWalkLock.lock();
    float distance = self->GetDistance(p);
    TC_LOG_INFO("server", "Walk Distance: %f, X: %f, Y: %f", distance, self->GetPositionX(), self->GetPositionY());
    if (distance < GetMinFollowDistance() || distance > self->GetVisibilityRange()) {
        m_pointWalkLock.unlock();
        TC_LOG_INFO("server", "Exit walk to point 1");
        return false;
    }

    FacePosition(p);
    if ((self->GetUnitMovementFlags() & MOVEMENTFLAG_FORWARD) != MOVEMENTFLAG_FORWARD) {
        StartWalkingForward();
    }

    if (m_targetPoint) {
        delete m_targetPoint;
    }

    m_targetPoint = new G3D::Vector3(p.GetPositionX(), p.GetPositionY(), p.GetPositionZ());
    TC_LOG_INFO("server", "Walking to X: %f, Y: %f", p.GetPositionX(), p.GetPositionY());
    m_pointWalkLock.unlock();
    TC_LOG_INFO("server", "Exit walk to point 2");
    return true;
}

void PlayerBot::AddPatrolPoint(float x, float y, float z) 
{
    TC_LOG_INFO("server", "Add Patrol");
    m_patrolLock.lock();
    if (m_patrolPath == NULL) {
        m_patrolPath = new std::vector<G3D::Vector3*>();
    }
    G3D::Vector3 *point = new G3D::Vector3(x, y, z);
    m_patrolPath->push_back(point);
    m_patrolLock.unlock();
}

void PlayerBot::ClearPatrol() 
{
    TC_LOG_INFO("server", "Clear Patrol");
    m_patrolLock.lock();
    if (m_patrolPath == NULL) {
        m_patrolLock.unlock();
        return;
    }

    for (auto point : *m_patrolPath) {
        delete point;
    }
    delete m_patrolPath;
    m_patrolPath = NULL;

    m_patrolLock.unlock();
    StopPatrolling();
}

void PlayerBot::ResetPatrol() 
{
    TC_LOG_INFO("server", "Reset Patrol");
    StopPatrolling();

    m_patrolLock.lock();
    m_patrolIndex = 0;
    m_patrolLock.unlock();
}

void PlayerBot::StartPatrolling() 
{
    TC_LOG_INFO("server", "Start Patrol");
    m_patrolLock.lock();
    if (!m_patrolPath || m_patrolPath->size() == 0) {
        m_patrolLock.unlock();
        TC_LOG_INFO("server", "There is no good patrol path. Aborting");
        TC_LOG_INFO("server", "Exit Start patrol 1");
        return;
    }
    if (m_patrolIndex >= m_patrolPath->size()) {
        m_patrolIndex = 0;
    }

    m_isPatrolling = true;
    m_patrolLock.unlock();
    TC_LOG_INFO("server", "Exit Start patrol");
}

void PlayerBot::StopPatrolling() 
{
    TC_LOG_INFO("server", "Stop Patrol");
    StopAllWalking();

    m_patrolLock.lock();
    m_isPatrolling = false;
    m_patrolLock.unlock();
}

void PlayerBot::UpdatePatrol()
{
    TC_LOG_INFO("server", "Update Patrol");
    m_patrolLock.lock();
    if (!m_patrolPath || m_patrolPath->size() == 0) {
        TC_LOG_INFO("server", "No valid patrol path. Aborting");
        m_patrolLock.unlock();
        return;
    }

    if (m_patrolIndex >= m_patrolPath->size()) {
        m_patrolIndex = 0;
    }

    G3D::Vector3 *point = m_patrolPath->at(m_patrolIndex);
    if (!WalkToPoint(point->x, point->y, point->z)) {
        m_patrolIndex++; //This happens if the bot is already standing on the point he needs to walk to. If this is the case, skip this point.
    }
    m_patrolLock.unlock();
}

Position* PlayerBot::GetIntermediatePoint(Position p)
{
    Player *self = m_session->GetPlayer();

    float distanceRatio = 0.4f;
    float newX = ((1 - distanceRatio) * p.GetPositionX() + (distanceRatio * self->GetPositionX()));
    float newY = ((1 - distanceRatio) * p.GetPositionY() + (distanceRatio * self->GetPositionY()));

    return new Position(newX, newY, p.GetPositionZ());
}

///TODO Use this for smart pathfinding
void PlayerBot::GeneratePath(float x, float y, float z)
{
    Player *self = m_session->GetPlayer();
    PathGenerator *generator = new PathGenerator(self);
    bool success = generator->CalculatePath(x, y, z);
    if (success) {
        TC_LOG_INFO("server", "Great Success");
    }
    else {
        TC_LOG_INFO("server", "Great Failure");
    }

    Movement::PointsArray pathPoints = generator->GetPath();

    for (auto &point : pathPoints) {
        TC_LOG_INFO("server", "X: %f", point.x);
        TC_LOG_INFO("server", "Y: %f", point.y);
        TC_LOG_INFO("server", "Z: %f", point.z);
    }
}

// FIXME This function does teleport the unit, and thus is still useful. But this breaks the bot after it teleports and the server must be restarted
void PlayerBot::TeleportToUnit(const Unit *unit)
{
    TC_LOG_INFO("server", "Initiating teleport");
    Player *self = m_session->GetPlayer();
    self->TeleportTo(unit->GetMapId(), unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetOrientation(), 0);
    TC_LOG_INFO("server", "Finished teleport");

    /// This stuff may possibly help in making this not break. Unsure
    //WorldPacket *packet = new WorldPacket();
    //packet->SetOpcode(MSG_MOVE_WORLDPORT_ACK);
    //TC_LOG_INFO("server", "Acking world port");
    //m_session->HandleMoveWorldportAckOpcode(*packet);
    //TC_LOG_INFO("server", "Acking world port done");
}
