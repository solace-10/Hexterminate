// Copyright 2014 Pedro Nunes
//
// This file is part of Hexterminate.
//
// Hexterminate is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hexterminate is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hexterminate. If not, see <http://www.gnu.org/licenses/>.

#include "hexterminate.h"
#include "menus/shiptweaks.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/hyperspacecore.h"
#include "ship/ship.h"
#include "ship/controller/controllerai.h"

#include <math/constants.h>
#include <math/misc.h>
#include <physics/rigidbody.h>
#include <physics/shape.h>
#include <physics/simulation.h>
#include <render/debugrender.h>

// clang-format off
#include <beginexternalheaders.h>
#include <glm/gtc/matrix_access.hpp>
#include <endexternalheaders.h>
// clang-format on

namespace Hexterminate
{

ControllerAI::ControllerAI( Ship* pShip )
    : Controller( pShip )
    , m_pTargetShip( nullptr )
    , m_TargetTimer( 0.0f )
    , m_AccuracyTimer( 0.0f )
    , m_MinimumWeaponRange( FLT_MAX )
    , m_PowerToAddons( false )
    , m_HasWeapons( false )
    , m_PatrolTimer( 0.0f )
    , m_AlternatorTimer( 5.0f )
{
    // We just give a random patrol point at the start, with additional patrol points being
    // generated once we reach the first one.
    GenerateNextPatrolPoint();

    ResetAlternatorTimer();
}

void ControllerAI::Update( float delta )
{
    ManageAddons( delta );
    AcquireTarget( delta );
    FireControl();

    GetShip()->SetThrust( ShipThrust::None );
    GetShip()->SetSteer( ShipSteer::None );

    if ( IsSuspended() == false )
    {
        HandleOrders( delta );
    }

    m_AccuracyTimer += delta;

    AddonQuantumStateAlternator* pAlternator = GetShip()->GetQuantumStateAlternator();
    if ( pAlternator != nullptr && pAlternator->GetQuantumState() != QuantumState::Inactive )
    {
        m_AlternatorTimer -= delta;
        if ( m_AlternatorTimer <= 0.0f )
        {
            GetShip()->FlipQuantumState();
            ResetAlternatorTimer();
        }
    }
}

void ControllerAI::ResetAlternatorTimer()
{
    // Between 5 and 10s.
    m_AlternatorTimer = static_cast<float>( 5000 + rand() % 5000 ) / 1000.0f;
}

void ControllerAI::HandleOrders( float delta )
{
    FleetCommandOrder order = GetShip()->GetFleetCommandOrder();
    if ( order == FleetCommandOrder::StickToFormation )
    {
        const glm::vec2& formationPosition = GetShip()->GetFormationPosition();
        const glm::vec2& formationDirection = GetShip()->GetFormationDirection();

        const float goalRadius = 60.0f;

        bool result = MoveToPosition( formationPosition, goalRadius, formationDirection );
        if ( g_pGame->GetCurrentSector()->GetShipTweaks()->GetDrawNavigation() )
        {
            if ( result )
            {
                Genesis::FrameWork::GetDebugRender()->DrawCircle( glm::vec3( formationPosition.x, formationPosition.y, 0.0f ), goalRadius, glm::vec3( 0.0f, 1.0f, 0.0f ) );
            }
            else
            {
                Genesis::FrameWork::GetDebugRender()->DrawCircle( glm::vec3( formationPosition.x, formationPosition.y, 0.0f ), goalRadius, glm::vec3( 1.0f, 1.0f, 1.0f ) );
            }
        }
    }
    else if ( order == FleetCommandOrder::Patrol )
    {
        if ( MoveToPosition( m_PatrolPosition, 60.0f ) )
        {
            m_PatrolTimer -= delta;
            if ( m_PatrolTimer <= 0.0f )
            {
                GenerateNextPatrolPoint();
            }
        }
    }
}

// Tries to acquire the closest hostile target. It checks if there is a closer ship every couple of seconds.
void ControllerAI::AcquireTarget( float delta )
{
    m_TargetTimer -= delta;

    if ( m_pTargetShip != nullptr && m_pTargetShip->IsTerminating() == false && m_pTargetShip->IsDestroyed() == false && m_TargetTimer > 0.0f )
        return;

    m_pTargetShip = nullptr;

    float minDistance = FLT_MAX;
    for ( auto& pShip : g_pGame->GetCurrentSector()->GetShipList() )
    {
        if ( Faction::sIsEnemyOf( pShip->GetFaction(), GetShip()->GetFaction() ) == false )
            continue;
        else if ( pShip->GetTowerModule() == nullptr || pShip->GetTowerModule()->GetHealth() <= 0.0f )
            continue;
        else if ( pShip->GetDockingState() != DockingState::Undocked )
            continue;
        else if ( pShip->GetHyperspaceCore() != nullptr && pShip->GetHyperspaceCore()->IsJumping() )
            continue;

        float distance = glm::distance( GetShip()->GetTowerPosition(), pShip->GetTowerPosition() );
        if ( distance < minDistance )
        {
            minDistance = distance;
            m_pTargetShip = pShip;
        }
    }

    m_TargetTimer = gRand( 3.5f, 5.0f );
}

void ControllerAI::FireControl()
{
    if ( GetTargetShip() == GetShip() )
    {
        return;
    }

    m_MinimumWeaponRange = FLT_MAX;
    m_HasWeapons = false;

    // HACK: Handle the particle accelerator's case, as it isn't a real weapon.
    for ( auto& pAddonModule : GetShip()->GetAddonModules() )
    {
        AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pAddonModule->GetModuleInfo() );
        if ( pAddonInfo->GetCategory() == AddonCategory::ParticleAccelerator )
        {
            m_HasWeapons = true;
            m_MinimumWeaponRange = 600.0f;
            break;
        }
    }

    for ( auto& pWeaponModule : GetShip()->GetWeaponModules() )
    {
        Weapon* pWeapon = pWeaponModule->GetWeapon();
        if ( pWeapon == nullptr )
        {
            continue;
        }

        if ( pWeaponModule->IsDestroyed() )
        {
            continue;
        }
        else
        {
            m_HasWeapons = true;
        }

        if ( GetTargetShip() == nullptr )
        {
            if ( pWeapon->GetInfo()->GetBehaviour() == WeaponBehaviour::Turret )
            {
                // TODO : Fix this case
                // const glm::vec3& forward = pWeapon->GetWorldTransform().getColumn( 1 );
                // const glm::vec3& translation = pWeapon->GetWorldTransform().getTranslation();
                // pWeapon->TurnTowards( glm::vec3( translation( 0 ) + forward( 0 ), translation( 1 ) + forward( 1 ), 0.0f ) );
                pWeapon->TurnTowards( glm::vec3( 5000.0f, 0.0f, 0.0f ) );
            }
        }
        else
        {
            float weaponRange = pWeapon->GetInfo()->GetRange( GetShip() );
            if ( weaponRange < m_MinimumWeaponRange )
                m_MinimumWeaponRange = weaponRange;

            // g_pGame->GetDebugRender()->DrawLine( pWeapon->GetWorldTransform().getTranslation(), GetTargetShip()->GetTowerPosition(), glm::vec3( 1.0f, 0.0f, 0.0f ) );

            float projectileSpeed = pWeapon->GetInfo()->GetSpeed();

            glm::vec2 targetPosition( GetTargetShip()->GetTowerPosition() );
            glm::vec2 predictedPosition;
            glm::vec2 weaponPosition( glm::column( pWeapon->GetWorldTransform(), 3 ) );
            if ( projectileSpeed > 0.0f && PredictTarget( weaponPosition, predictedPosition, projectileSpeed ) == false )
                continue;
            else if ( projectileSpeed <= 0.0f )
                predictedPosition = targetPosition;

            // Slowly move the target back and forth to avoid always having the ships firing with deadly accuracy at the same point
            glm::vec2 finalTargetPosition;
            glm::vec2 dir = predictedPosition - targetPosition;
            float length = glm::length( dir );
            if ( length > 0.0f )
            {
                dir = glm::normalize( dir );
                float accuracy = 0.6f + fabsf( cosf( m_AccuracyTimer / 3.0f ) ) * 0.3f;
                finalTargetPosition = targetPosition + dir * length * accuracy;
            }
            else
            {
                finalTargetPosition = predictedPosition;
            }

            // g_pGame->GetDebugRender()->DrawLine( GetTargetShip()->GetTowerPosition(), finalTargetPosition, glm::vec3( 0.0f, 1.0f, 0.0f ) );
            // g_pGame->GetDebugRender()->DrawCircle( finalTargetPosition, 10.0f, glm::vec3( 0.0f, 1.0f, 0.0f ) );

            // We always want to turn the turrets towards their target even if they're out of firing range,
            // it just looks more natural.
            if ( pWeapon->GetInfo()->GetBehaviour() == WeaponBehaviour::Turret )
            {
                pWeapon->TurnTowards( glm::vec3( finalTargetPosition.x, finalTargetPosition.y, 0.0f ) );
            }

            if ( pWeapon->CanFire() && !m_PowerToAddons )
            {
                const float weaponRangeFalloff = weaponRange + 50.0f; // a small threshold is added so weapons keep firing even if slightly out of range

                glm::vec2 diff = targetPosition - weaponPosition;
                if ( glm::length( diff ) < weaponRangeFalloff && pWeapon->IsTurnedTowards( glm::vec3( finalTargetPosition.x, finalTargetPosition.y, 0.0f ) ) )
                {
                    pWeapon->Fire();
                }
            }
        }
    }
}

bool ControllerAI::PredictTarget( const glm::vec2& src, glm::vec2& result, float projectileSpeed )
{
    if ( GetTargetShip() == nullptr )
        return false;

    glm::vec2 dst( GetTargetShip()->GetTowerPosition() );
    glm::vec2 dstv( GetTargetShip()->GetRigidBody()->GetLinearVelocity() );

    float tx = dst.x - src.x;
    float ty = dst.y - src.y;
    float tvx = dstv.x;
    float tvy = dstv.y;

    // Get quadratic equation components
    float a = tvx * tvx + tvy * tvy - projectileSpeed * projectileSpeed;
    float b = 2 * ( tvx * tx + tvy * ty );
    float c = tx * tx + ty * ty;

    float solA, solB;
    int numSolutions = SolveQuadratic( a, b, c, solA, solB );
    if ( numSolutions > 0 )
    {
        float t = solA;
        if ( numSolutions == 2 )
        {
            t = gMin( solA, solB );
            if ( t < 0.0f )
            {
                t = gMax( solA, solB );
            }
        }
        if ( t > 0.0f )
        {
            result = glm::vec2( dst.x + dstv.x * t, dst.y + dstv.y * t );
            return true;
        }
    }

    return false;
}

// Solves a quadratic equation in the form of (ax)^2 + bx + c
// Returns the number of solutions
int ControllerAI::SolveQuadratic( float a, float b, float c, float& solA, float& solB ) const
{
    if ( fabsf( a ) < FLT_MIN )
    {
        if ( fabsf( b ) < FLT_MIN )
        {
            if ( fabsf( c ) < FLT_MIN )
            {
                solA = 0.0f;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            solA = c / b;
            return 1;
        }
    }
    else
    {
        float disc = b * b - 4 * a * c;
        if ( disc >= 0.0f )
        {
            disc = sqrtf( disc );
            a = 2 * a;
            solA = ( -b - disc ) / a;
            solB = ( -b + disc ) / a;
            return 2;
        }
    }
    return 0;
}

void ControllerAI::ManageAddons( float delta )
{
    // Does this ship need to be repaired? It will mark itself for repairing if:
    // a) It's in combat and a module goes below 50% health.
    // b) Is not in combat and a module is at all damage.
    // This prevents the issue of ships sitting around, half burning after a battle, when they could
    // repair themselves.
    bool needsRepair = false;
    const ModuleVector& modules = GetShip()->GetModules();
    for ( auto& module : modules )
    {
        if ( module->IsDestroyed() == false )
        {
            const float threshold = ( m_pTargetShip == nullptr ) ? 1.0f : 0.5f;
            if ( module->GetHealth() / module->GetModuleInfo()->GetHealth( GetShip() ) < threshold )
            {
                needsRepair = true;
                break;
            }
        }
    }

    const AddonModuleList& addonModules = GetShip()->GetAddonModules();
    for ( auto& addonModule : addonModules )
    {
        if ( addonModule->IsDestroyed() )
            continue;

        AddonInfo* pInfo = static_cast<AddonInfo*>( addonModule->GetModuleInfo() );
        if ( pInfo->GetCategory() == AddonCategory::ModuleRepairer && needsRepair && addonModule->GetAddon()->CanUse() )
        {
            if ( GetShip()->GetEnergy() < pInfo->GetActivationCost() )
            {
                m_PowerToAddons = true;
            }
            else
            {
                m_PowerToAddons = false;
                addonModule->GetAddon()->Activate();
            }
        }
        else if ( pInfo->GetCategory() != AddonCategory::ModuleRepairer && addonModule->GetAddon()->CanUse() )
        {
            if ( GetShip()->GetEnergy() >= pInfo->GetActivationCost() )
            {
                addonModule->GetAddon()->Activate();
            }
        }
    }
}

bool ControllerAI::MoveToPosition( const glm::vec2& position, float goalRadius )
{
    return MoveToPosition( position, goalRadius, glm::vec2( 0.0f, 0.0f ) );
}

// Returns true if the goal has been reached.
bool ControllerAI::MoveToPosition( const glm::vec2& position, float goalRadius, const glm::vec2& orientationAtGoal )
{
    // Make sure the player ship does not get rammed while we are docked.
    Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
    if ( pPlayerShip != nullptr )
    {
        DockingState dockingState = pPlayerShip->GetDockingState();
        if ( dockingState == DockingState::Docked || dockingState == DockingState::Docking )
        {
            GetShip()->SetThrust( ShipThrust::None );
            return false;
        }
    }

    glm::vec3 forward( glm::column( GetShip()->GetRigidBody()->GetWorldTransform(), 1 ) );
    const glm::vec3& shipPosition = GetShip()->GetTowerPosition();

    // Create two feelers in front of the ship to see if the position we are navigating to is blocked
    // To prevent having the rays intersect the owning ship, we start the rays from the front of the ship

    // Figure out how many slots there are between the logical center of the ship (the tower) and the
    // front (y1 of the bounding box). Some additional space is added to make sure the ray clears the
    // geometry.
    int towerSlotX, towerSlotY;
    GetShip()->GetTowerModule()->GetHexGridSlot( towerSlotX, towerSlotY );
    int x1, x2, y1, y2;
    GetShip()->GetModuleHexGrid().GetBoundingBox( x1, y1, x2, y2 );
    float forwardOffset = ( y2 - towerSlotY ) * sModuleHalfHeight + sModuleHeight;

    glm::vec3 feelerStartPosition = forward * forwardOffset + shipPosition;
    float feelerAngle = Genesis::kDegToRad * 30.0f; // two lines, X degrees from the forward vector
    float cs = cosf( feelerAngle );
    float sn = sinf( feelerAngle );

    bool feelerCollision = false;

    Genesis::Physics::Simulation* pSimulation = g_pGame->GetPhysicsSimulation();
    const bool drawNavigation = g_pGame->GetCurrentSector()->GetShipTweaks()->GetDrawNavigation();
    for ( int i = 0; i < 2; ++i )
    {
        glm::vec3 feeler( forward.x * cs - forward.y * sn, forward.x * sn + forward.y * cs, 0.0f ); // rotates the feeler from the forward vector
        feeler = feeler * 70.0f + feelerStartPosition;

        Genesis::Physics::RayTestResultVector rayTestResults;
        pSimulation->RayTest( feelerStartPosition, feeler, rayTestResults );

        bool hasHit = false;
        for ( auto& result : rayTestResults )
        {
            // If it's a compound shape, then the ShipCollisionInfo is in the child shape.
            Genesis::Physics::ShapeSharedPtr pShape = !result.GetChildShape().expired() ? result.GetChildShape().lock() : result.GetShape().lock();
            ShipCollisionInfo* pCollisionInfo = reinterpret_cast<ShipCollisionInfo*>( pShape->GetUserData() );
            if ( pCollisionInfo->GetType() == ShipCollisionType::Module && pCollisionInfo->GetShip() != GetShip() )
            {
                hasHit = true;
                feelerCollision = true;
                break;
            }
        }

        if ( drawNavigation )
        {
            Genesis::FrameWork::GetDebugRender()->DrawLine( feelerStartPosition, feeler, hasHit ? glm::vec3( 1.0f, 0.0f, 0.0f ) : glm::vec3( 0.0f, 1.0f, 0.0f ) );
        }

        sn = -sn; // causes the other feeler to the right of the forward vector
    }

    const float distanceToGoal = glm::distance( shipPosition, glm::vec3( position, 0.0f ) );
    const bool goalReached = ( distanceToGoal <= goalRadius );
    const bool courseCorrection = IsCourseCorrectionRequired(
        glm::vec2( forward ),
        glm::vec2( shipPosition ),
        position,
        distanceToGoal );

    if ( feelerCollision || courseCorrection )
    {
        GetShip()->SetThrust( ShipThrust::None );
    }
    else
    {
        GetShip()->SetThrust( goalReached ? ShipThrust::None : ShipThrust::Forward );
    }

    // Adjust turning
    glm::vec3 targetDirection( position.x, position.y, 0.0f );
    if ( goalReached && glm::length( orientationAtGoal ) > glm::epsilon<float>() )
    {
        targetDirection = glm::vec3( orientationAtGoal, 0.0f );
    }
    else
    {
        targetDirection = glm::normalize( targetDirection - shipPosition );
    }

    float theta = atan2( targetDirection.y, targetDirection.x ) - atan2( forward.y, forward.x );
    if ( theta > Genesis::kPi )
    {
        theta -= Genesis::kPi * 2.0f;
    }
    else if ( theta < -Genesis::kPi )
    {
        theta += Genesis::kPi * 2.0f;
    }

    // Don't bother adjusting angle if we're within a few degrees of our target.
    // Also, if our angular velocity exceeds theta, then we let the dampers kick in for angular velocity (turn == 0).
    // Doing this allows us not to overshoot the target.
    const float threshold = 2.0f * Genesis::kDegToRad;
    ShipSteer steering = ShipSteer::None;
    if ( fabsf( glm::length( GetShip()->GetRigidBody()->GetAngularVelocity() ) ) < fabsf( theta ) )
    {
        if ( theta > threshold )
        {
            steering = ShipSteer::Left;
        }
        else if ( theta < -threshold )
        {
            steering = ShipSteer::Right;
        }
    }

    GetShip()->SetSteer( steering );

    return goalReached;
}

bool ControllerAI::IsCourseCorrectionRequired( const glm::vec2& shipForward, const glm::vec2& shipPosition, const glm::vec2& goalPosition, float distanceToGoal ) const
{
    if ( distanceToGoal < 300.0f )
    {
        const glm::vec2 forwardToTarget = glm::normalize( goalPosition - shipPosition );
        const float dot = glm::dot( shipForward, forwardToTarget );
        const float angle = glm::acos( dot ) * Genesis::kRadToDeg;
        return ( angle >= 30.0f );
    }
    else
    {
        return false;
    }
}

void ControllerAI::GenerateNextPatrolPoint()
{
    m_PatrolPosition = glm::vec2( gRand( -2000.0f, 2000.0f ), gRand( -2000.0f, 2000.0f ) );
    m_PatrolTimer = gRand( 3.0f, 6.0f );
}

} // namespace Hexterminate