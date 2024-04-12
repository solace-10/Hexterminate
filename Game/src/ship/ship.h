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

#pragma once

#include <list>
#include <map>
#include <vector>

#include <scene/sceneobject.h>
#include <sound/soundmanager.h>

#include "fleet/fleet.fwd.h"
#include "fleet/fleetcommand.h"
#include "perks.h"
#include "ship/addon/addon.h"
#include "ship/module.h"
#include "ship/moduleinfo.h"
#include "ship/navigationstats.h"
#include "ship/ship.fwd.h"
#include "ship/shipcollisioninfo.h"
#include "ship/weapon.h"

namespace Genesis
{
class ResourceModel;
class ResourceSound;
class ShaderUniform;

namespace Physics
{
    class CollisionFilterInfo;
    class RigidBody;
    class CompoundShape;
    class Shape;
    using ShapeContainer = std::vector<ShapeSharedPtr>;
    using CollisionCallbackHandle = unsigned long;
} // namespace Physics
} // namespace Genesis

namespace Hexterminate
{

class AddonInfo;
class Controller;
class DamageTracker;
class DestructionSequence;
class Inventory;
class Faction;
class HyperspaceCore;
class ShipInfo;
class ShipShaderUniforms;
class Shipyard;
class Shield;

using ControllerUniquePtr = std::unique_ptr<Controller>;

static const int sNumShipAddons = 6;

static const unsigned int SHIP_FLAG_NONE = 0;
static const unsigned int SHIP_FLAG_FLAGSHIP = 1 << 0;

class ShipCustomisationData
{
public:
    ShipCustomisationData()
        : m_ShipName( "" )
        , m_CaptainName( "" )
        , m_pModuleInfoHexGrid( nullptr )
        , m_StartingSong( "" )
    {
    }

    ShipCustomisationData( ModuleInfoHexGrid* pModuleInfoHexGrid )
        : m_ShipName( "" )
        , m_CaptainName( "" )
        , m_pModuleInfoHexGrid( pModuleInfoHexGrid )
        , m_StartingSong( "" )
    {
    }

    std::string m_ShipName;
    std::string m_CaptainName;
    ModuleInfoHexGrid* m_pModuleInfoHexGrid;
    std::string m_StartingSong;
};

class ShipSpawnData
{
public:
    ShipSpawnData()
        : m_PositionX( 0.0f )
        , m_PositionY( 0.0f )
    {
    }
    ShipSpawnData( float x, float y )
        : m_PositionX( x )
        , m_PositionY( y )
    {
    }

    float m_PositionX;
    float m_PositionY;
};

enum class ShipThrust
{
    None,
    Forward,
    Backward
};

enum class ShipSteer
{
    None,
    Left,
    Right
};

enum class ShipDodge
{
    None,
    Left,
    Right
};

enum class ShipStrafe
{
    None,
    Left,
    Right
};

enum class TransformSpace
{
    Local,
    World
};

class Ship : public Genesis::SceneObject
{
public:
    Ship();
    virtual ~Ship();

    virtual void Update( float fDelta );
    virtual void Render();

    void SetInitialisationParameters( Faction* pFaction, FleetWeakPtr pFleetWeakPtr, const ShipCustomisationData& ShipCustomisationData, const ShipSpawnData& shipSpawnData, const ShipInfo* pShipInfo );
    void Initialize();

    Module* AddModule( ModuleInfo* pModuleInfo, int x, int y );
    ModuleInfo* RemoveModule( int x, int y );
    void ClearModules();
    void SetModuleBulkEdit( bool isEnabled );

    Genesis::Physics::RigidBody* GetRigidBody() const;
    glm::vec3 GetCentre( TransformSpace transformSpace ) const;
    const ModuleHexGrid& GetModuleHexGrid() const;

    Faction* GetFaction() const;

    template<typename T>
    const std::vector<T*>& GetModules() const
    {
        static_assert( T::GetType() != ModuleType::Invalid );
        static_assert( static_cast<size_t>( T::GetType() ) < static_cast<size_t>( ModuleType::Count ) );
        return *reinterpret_cast<std::vector<T*> const*>( &m_Modules[ static_cast<size_t>( T::GetType() ) ] );
    }

    const ModuleVector& GetModules( ModuleType type ) const;
    const ModuleVector& GetModules() const;

    Shield* GetShield() const;

    float GetEnergy() const;
    float GetEnergyCapacity() const;
    bool ConsumeEnergy( float quantity );
    void DamageModule( Weapon* pWeapon, Module* pModule, float delta ); // Damages a particular Module.
    void DamageModule( WeaponSystem weaponSystem, DamageType damageType, float damageAmount, int burst, Ship* pDealtBy, Module* pModule, float delta );
    void DamageShield( Weapon* pWeapon, float delta, const glm::vec3& hitPosition ); // Damages the Shield.
    void DamageShield( WeaponSystem weaponSystem, DamageType damageType, float damageAmount, int burst, Ship* pDealtBy, float delta, const glm::vec3& hitPosition );
    void OnCollision( Genesis::Physics::RigidBody* pRigidBodyA, Genesis::Physics::RigidBody* pRigidBodyB, Genesis::Physics::ShapeWeakPtr pShapeA, Genesis::Physics::ShapeWeakPtr pShapeB, const glm::vec3& hitPosition );

    inline TowerModule* GetTowerModule() const; // Should always be valid unless we are editing the ship. Also, by design, a Ship can only have one TowerModule.
    inline const glm::vec3& GetTowerPosition() const; // The Tower's world position. Use this for targetting, as this is the most important part of a ship.
    glm::mat4x4 GetTransform() const;

    inline void SetThrust( ShipThrust thrust ); // Used by Controllers to move the Ship
    inline void SetSteer( ShipSteer direction ); // User by Controllers to steer the Ship
    inline void SetStrafe( ShipStrafe direction );
    inline void SetDodge( ShipDodge dodge );

    inline ShipThrust GetThrust() const;
    inline ShipSteer GetSteer() const;
    inline ShipStrafe GetStrafe() const;
    inline ShipDodge GetDodge() const;

    const ShipSpawnData& GetShipSpawnData() const;

    Shipyard* GetShipyard() const;
    DockingState GetDockingState() const;
    void Dock( Shipyard* pShipyard );
    void Undock();
    void NotifyDockingFinished();

    HyperspaceCore* GetHyperspaceCore() const;
    Controller* GetController() const;

    void OnModuleDestroyed( Module* pModule );
    void OnShipDestroyed();
    void SpawnLoot();

    void Repair( float repairAmount );

    bool IsDestroyed() const;
    bool IsFlagship() const;

    void Terminate();
    bool IsTerminating() const;

    ShipShaderUniforms* GetShipShaderUniforms() const;
    void GetBoundingBox( glm::vec3& topLeft, glm::vec3& bottomRight ) const;
    const ShipInfo* GetShipInfo() const;
    bool HasPerk( Perk perk ) const;
    void DisruptEngines();
    bool AreEnginesDisrupted() const;
    void RammingSpeed();
    bool IsRammingSpeedEnabled() const;
    float GetRammingSpeedCooldown() const;
    void FlipQuantumState();
    int GetIntegrity() const;
    const NavigationStats& GetNavigationStats() const;

    FleetCommandOrder GetFleetCommandOrder() const;
    void SetFleetCommandOrder( FleetCommandOrder order );
    const glm::vec2& GetFormationPosition() const;
    void SetFormationPosition( const glm::vec2& position );
    const glm::vec2& GetFormationDirection() const;
    void SetFormationDirection( const glm::vec2& direction );

    const glm::vec3& GetCentreOfMass() const;

    AddonQuantumStateAlternator* GetQuantumStateAlternator() const;

protected:

    template<typename T>
    std::vector<T*>& GetModulesInternal()
    {
        return const_cast<std::vector<T*>&>( GetModules<T>() );
    }

    ModuleVector& GetModulesInternal( ModuleType type );

    void InitializeReactors();
    bool DamageShared( WeaponSystem weaponSystem, float baseDamage, int burst, Faction* pDealtByFaction, float delta, float* pFrameDamage, float* pDisplayDamage ) const;
    void ApplyDodge( float& dodgeTimer, float enginePower );
    void ApplyStrafe( float enginePower );
    void CreateDefaultController();
    void RenderModuleHexGrid( const glm::mat4& modelTransform );
    void RenderModuleHexGridOutline( const glm::mat4& modelTransform );
    void UpdateReactors( float delta );
    void UpdateRepair( float delta );
    void UpdateShield( float delta );
    void UpdateEngines( float delta );
    void UpdateSounds( float delta );
    void CalculateNavigationStats();
    float CalculateMaximumLinearSpeed( float linearThrust, float mass ) const;
    float CalculateMaximumAngularSpeed( float torque, float mass ) const;
    float CalculateMass() const;
    void OnModulesChanged();

    // Switching a controller isn't instantaneous. The new controller only becomes active the next time the ship is updated.
    // Without this it was possible for a controller to unintentionally cause itself to be deleted.
    void SwitchController( ControllerUniquePtr&& pController );

    void CreateRigidBody();
    void DestroyRigidBody();
    void RebuildShield();

    void SetSharedShaderParameters( Module* pModule, Genesis::Material* pMaterial );
    void CalculateBoundingBox();
    void CalculateRammingDamage( const Ship* pRammingShip, const Ship* pRammedShip, const ModuleInfo* pRammingModuleInfo, float& damageToRammingShip, float& damageToRammedShip );

    bool IsVisible() const;

    void PlayDestructionSequence();
    void OnFlagshipDestroyed();

    void UpdateModuleLinkState();
    void RecursiveModuleLinkState( Module* pModule );

    ControllerUniquePtr m_pController;
    ControllerUniquePtr m_pNextController;

    ShipThrust m_Thrust;
    ShipSteer m_Steer;
    ShipStrafe m_Strafe;
    ShipDodge m_Dodge;
    float m_DodgeTimer;

    Genesis::Physics::RigidBody* m_pRigidBody;
    Genesis::Physics::CompoundShapeSharedPtr m_pCompoundShape;
    glm::vec3 m_StartPosition;

    float m_EnergyCapacity;
    float m_Energy;

    ShipCustomisationData m_ShipCustomisationData;
    ShipSpawnData m_ShipSpawnData;
    Faction* m_pFaction;

    ModuleHexGrid m_ModuleHexGrid;

    using ModuleContainer = std::array<ModuleVector, static_cast<size_t>( ModuleType::Count )>;
    ModuleContainer m_Modules;
    mutable ModuleVector m_AllModules;
    mutable bool m_AllModulesDirty;

    glm::vec3 m_TowerPosition;
    glm::vec3 m_ShipyardFocusPoint;

    DockingState m_DockingState;
    Shipyard* m_pShipyard;

    bool m_ModuleBulkEdit;

    HyperspaceCore* m_pHyperspaceCore;
    Shield* m_pShield;

    DestructionSequence* m_pDestructionSequence;

    float m_AmountToRepair;
    float m_RepairTimer;
    float m_RepairStep;

    FleetSharedPtr m_pFleet;
    unsigned int m_Flags;

    bool m_IsTerminating;
    bool m_IsDestroyed;
    bool m_UpdatingLinks;

    glm::vec3 m_BoundingBoxTopLeft;
    glm::vec3 m_BoundingBoxBottomRight;

    const ShipInfo* m_pShipInfo;

    Genesis::Sound::SoundInstanceSharedPtr m_pEngineSound;

    ShipShaderUniforms* m_pUniforms;
    float m_EngineDisruptionTimer;

    FleetCommandOrder m_Order;
    glm::vec2 m_FormationPosition;
    glm::vec2 m_FormationDirection;

    float m_RammingSpeedTimer;
    float m_RammingSpeedCooldown;
    Genesis::Sound::SoundInstanceSharedPtr m_pRammingSpeedSound;

    DamageTracker* m_pDamageTracker;
    glm::vec3 m_CentreOfMass;
    Genesis::Physics::CollisionCallbackHandle m_CollisionCallbackHandle;

    NavigationStats m_NavigationStats;
};

inline Genesis::Physics::RigidBody* Ship::GetRigidBody() const
{
    return m_pRigidBody;
}

inline const ModuleHexGrid& Ship::GetModuleHexGrid() const
{
    return m_ModuleHexGrid;
}

inline Shield* Ship::GetShield() const
{
    return m_pShield;
}

inline float Ship::GetEnergy() const
{
    return m_Energy;
}

inline float Ship::GetEnergyCapacity() const
{
    return m_EnergyCapacity;
}

inline Faction* Ship::GetFaction() const
{
    return m_pFaction;
}

inline void Ship::SetSteer( ShipSteer direction )
{
    m_Steer = IsRammingSpeedEnabled() ? ShipSteer::None : direction;
}

inline void Ship::SetStrafe( ShipStrafe direction )
{
    m_Strafe = IsRammingSpeedEnabled() ? ShipStrafe::None : direction;
}

inline void Ship::SetThrust( ShipThrust thrust )
{
    m_Thrust = IsRammingSpeedEnabled() ? ShipThrust::Forward : thrust;
}

inline void Ship::SetDodge( ShipDodge dodge )
{
    m_Dodge = IsRammingSpeedEnabled() ? ShipDodge::None : dodge;
}

inline ShipSteer Ship::GetSteer() const
{
    return m_Steer;
}

inline ShipStrafe Ship::GetStrafe() const
{
    return m_Strafe;
}

inline ShipThrust Ship::GetThrust() const
{
    return m_Thrust;
}

inline ShipDodge Ship::GetDodge() const
{
    return m_Dodge;
}

inline TowerModule* Ship::GetTowerModule() const
{
    auto& towerModules = GetModules<TowerModule>();
    return towerModules.empty() ? nullptr : towerModules.front();
}

inline const glm::vec3& Ship::GetTowerPosition() const
{
    return m_TowerPosition;
}

inline const ShipSpawnData& Ship::GetShipSpawnData() const
{
    return m_ShipSpawnData;
}

inline DockingState Ship::GetDockingState() const
{
    return m_DockingState;
}

inline Shipyard* Ship::GetShipyard() const
{
    return m_pShipyard;
}

inline HyperspaceCore* Ship::GetHyperspaceCore() const
{
    return m_pHyperspaceCore;
}

inline Controller* Ship::GetController() const
{
    return m_pController.get();
}

inline const ShipInfo* Ship::GetShipInfo() const
{
    return m_pShipInfo;
}

inline bool Ship::IsDestroyed() const
{
    return m_IsDestroyed;
}

inline ShipShaderUniforms* Ship::GetShipShaderUniforms() const
{
    return m_pUniforms;
}

inline void Ship::DisruptEngines()
{
    m_EngineDisruptionTimer = 15.0f;
}

inline bool Ship::AreEnginesDisrupted() const
{
    return ( m_EngineDisruptionTimer > 0.0f );
}

inline FleetCommandOrder Ship::GetFleetCommandOrder() const
{
    return m_Order;
}

inline void Ship::SetFleetCommandOrder( FleetCommandOrder order )
{
    m_Order = order;
}

inline const glm::vec2& Ship::GetFormationPosition() const
{
    return m_FormationPosition;
}

inline void Ship::SetFormationPosition( const glm::vec2& position )
{
    m_FormationPosition = position;
}

inline const glm::vec2& Ship::GetFormationDirection() const
{
    return m_FormationDirection;
}

inline void Ship::SetFormationDirection( const glm::vec2& direction )
{
    m_FormationDirection = direction;
}

inline bool Ship::IsRammingSpeedEnabled() const
{
    return ( m_RammingSpeedTimer > 0.0f );
}

inline float Ship::GetRammingSpeedCooldown() const
{
    return m_RammingSpeedCooldown;
}

inline const glm::vec3& Ship::GetCentreOfMass() const
{
    return m_CentreOfMass;
}

inline const NavigationStats& Ship::GetNavigationStats() const
{
    return m_NavigationStats;
}

inline ModuleVector& Ship::GetModulesInternal( ModuleType type )
{
    return const_cast<ModuleVector&>( GetModules( type ) );
}

inline const ModuleVector& Ship::GetModules( ModuleType type ) const
{
    SDL_assert( type != ModuleType::Invalid );
    SDL_assert( static_cast<size_t>( type ) < static_cast<size_t>( ModuleType::Count ) );
    return m_Modules[ static_cast<size_t>( type ) ];
}

} // namespace Hexterminate
