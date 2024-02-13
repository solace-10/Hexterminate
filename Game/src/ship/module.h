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

#include "ship/hexgrid.h"
#include "ship/moduleinfo.h"
#include "ship/shipcollisioninfo.h"

#include <physics/shape.fwd.h>
#include <scene/sceneobject.h>

#include <list>

namespace Genesis
{
class ResourceModel;
class ResourceSound;

namespace Physics
{
    class Shape;
}
} // namespace Genesis

namespace Hexterminate
{

class Ship;
class ShipInfo;
class ModuleInfo;
class Module;
class WeaponModule;
class AddonModule;
class TowerModule;
class EngineModule;
class ReactorModule;
class ShieldModule;
class Weapon;
class Trail;
class Addon;
class ParticleEmitter;
class ShipCollisionInfo;
class AddonQuantumStateAlternator;

enum class WeaponSystem;

typedef std::vector<Module*> ModuleVector;
typedef std::list<Module*> ModuleList;
typedef std::list<WeaponModule*> WeaponModuleList;
typedef std::list<AddonModule*> AddonModuleList;
typedef std::list<EngineModule*> EngineModuleList;
typedef std::list<ReactorModule*> ReactorModuleList;
typedef std::list<ShieldModule*> ShieldModuleList;
typedef std::list<TowerModule*> TowerModuleList;
typedef std::list<AddonQuantumStateAlternator*> AddonQuantumStateAlternatorList;

typedef HexGrid<Module*> ModuleHexGrid;

static const float sModuleWidth = 20.0f;
static const float sModuleHalfWidth = sModuleWidth / 2.0f;
static const float sModuleHeight = 17.0f;
static const float sModuleHorizontalSpacing = 30.0f;
static const float sModuleHalfHeight = sModuleHeight / 2.0f;

///////////////////////////////////////////////////////////////////////////////
// Module
///////////////////////////////////////////////////////////////////////////////

class Module : public Genesis::SceneObject
{
public:
    Module( ModuleInfo* pInfo );
    virtual ~Module();

    virtual void Initialise( Ship* pShip ) {}
    virtual void OnAllModulesCreated() {}

    virtual void Update( float delta ) override;
    virtual void UpdateShipyard( float delta );
    virtual void Render() override;
    virtual void Render( const glm::mat4& modelTransform, bool drawOutline );
    bool ShouldRender() const;

    inline ModuleInfo* GetModuleInfo() const { return m_pInfo; }
    inline Genesis::ResourceModel* GetModel() const { return m_pModel; }
    inline float GetHealth() const { return m_Health; }
    inline bool IsDestroyed() const { return m_Health <= 0.0f; }
    void Destroy();
    inline void GetHexGridSlot( int& slotX, int& slotY ) const;
    inline void SetHexGridSlot( int slotX, int slotY );

    virtual void ApplyDamage( float amount, DamageType damageType, Ship* pDealtBy );
    virtual void Repair( float amount );

    static glm::vec3 GetLocalPosition( Ship* pShip, int x, int y );
    glm::vec3 GetLocalPosition() const;
    glm::vec3 GetWorldPosition() const;

    virtual void SetOwner( Ship* pShip );
    Ship* GetOwner() const;

    void SetLinked( bool state );
    bool IsLinked() const;

    virtual void TriggerEMP();
    bool IsEMPed() const;

    void TriggerAssemblyEffect();
    float GetAssemblyPercentage() const;

    ShipCollisionInfo* GetCollisionInfo() const;
    Genesis::Physics::ShapeSharedPtr GetPhysicsShape() const;
    void SetPhysicsShape( Genesis::Physics::ShapeSharedPtr pShape );

protected:
    virtual void OnDeathEffect();
    virtual void OnDamageEffect();
    void LoadDeathSFX();
    void PlayDeathSFX();

    ModuleInfo* m_pInfo;
    Ship* m_pOwner;
    Genesis::ResourceModel* m_pModel;
    float m_Health;
    int m_HexGridSlotX;
    int m_HexGridSlotY;
    ParticleEmitter* m_pDamageParticleEmitter;
    Genesis::ResourceSound* m_pDeathSFX;
    bool m_IsLinked;
    float m_DestructionTimer;
    float m_PlasmaWarheadsTimer;
    float m_EMPTimer;
    float m_AssemblyPercentage;
    ShipCollisionInfoUniquePtr m_pCollisionInfo;
    Genesis::Physics::ShapeSharedPtr m_pPhysicsShape;
};

inline void Module::GetHexGridSlot( int& slotX, int& slotY ) const
{
    slotX = m_HexGridSlotX;
    slotY = m_HexGridSlotY;
}

inline void Module::SetHexGridSlot( int slotX, int slotY )
{
    m_HexGridSlotX = slotX;
    m_HexGridSlotY = slotY;
}

inline Ship* Module::GetOwner() const
{
    return m_pOwner;
}

inline void Module::SetLinked( bool state )
{
    m_IsLinked = state;
}

inline bool Module::IsLinked() const
{
    return m_IsLinked;
}

inline bool Module::ShouldRender() const
{
    return m_DestructionTimer <= 0.5f;
}

inline bool Module::IsEMPed() const
{
    return m_EMPTimer > 0.0f;
}

inline void Module::TriggerAssemblyEffect()
{
    m_AssemblyPercentage = 0.0f;
}

inline float Module::GetAssemblyPercentage() const
{
    return m_AssemblyPercentage;
}

inline ShipCollisionInfo* Module::GetCollisionInfo() const
{
    return m_pCollisionInfo.get();
}

///////////////////////////////////////////////////////////////////////////////
// WeaponModule
///////////////////////////////////////////////////////////////////////////////

class WeaponModule : public Module
{
public:
    WeaponModule( ModuleInfo* pInfo );
    virtual ~WeaponModule();

    virtual void Initialise( Ship* pShip ) override;
    virtual void Update( float delta ) override;
    virtual void Render( const glm::mat4& modelTransform, bool drawOutline ) override;

    float GetDamage() const { return m_fDamage; }
    Weapon* GetWeapon() const;

protected:
    float m_fDamage;

    Weapon* m_pWeapon;
};

inline Weapon* WeaponModule::GetWeapon() const
{
    return m_pWeapon;
}

///////////////////////////////////////////////////////////////////////////////
// EngineModule
///////////////////////////////////////////////////////////////////////////////

class EngineModule : public Module
{
public:
    EngineModule( ModuleInfo* pInfo );
    virtual ~EngineModule() override;

    virtual void Update( float delta ) override;

    virtual void TriggerEMP() override;

    Trail* GetTrail() const;
    const glm::vec3& GetTrailOffset() const;
    void RemoveTrail();
    void Enable();
    void Disable();

protected:
    void UpdateTrail();
    void UpdateGlow( float delta );

    Trail* m_pTrail;
    glm::vec3 m_TrailOffset;
    float m_Output;
    float m_FlickerDuration;
    float m_TimeToNextFlicker;
    float m_RammingSpeedScaling;
    bool m_Enabled;
};

inline Trail* EngineModule::GetTrail() const
{
    return m_pTrail;
}

inline const glm::vec3& EngineModule::GetTrailOffset() const
{
    return m_TrailOffset;
}

///////////////////////////////////////////////////////////////////////////////
// ArmourModule
///////////////////////////////////////////////////////////////////////////////

class ArmourModule : public Module
{
public:
    ArmourModule( ModuleInfo* pInfo );
    virtual ~ArmourModule(){};

    virtual void Update( float delta ) override;

    virtual void ApplyDamage( float amount, DamageType damageType, Ship* pDealtBy ) override;
    virtual void Repair( float amount ) override;
    const glm::vec4 GetOverlayColour() const;

    virtual void SetOwner( Ship* pShip ) override;

protected:
    virtual void OnDamageEffect() override;
    void CalculateOverlayIntensity();
    bool IsUnbrokenStateActive() const;

    bool m_IsRegenerative;
    float m_RegenerationRate;
    float m_DamageTimer;
    float m_OverlayIntensity;
    float m_UnbrokenCooldown;
    float m_UnbrokenTimer;
};

///////////////////////////////////////////////////////////////////////////////
// ShieldModule
///////////////////////////////////////////////////////////////////////////////

class ShieldModule : public Module
{
public:
    ShieldModule( ModuleInfo* pInfo );
    virtual ~ShieldModule(){};

    float GetCapacity() const { return m_fCapacity; }
    float GetPeakRechargeRate() const { return m_fPeakRecharge; }

protected:
    float m_fCapacity;
    float m_fPeakRecharge;
};

///////////////////////////////////////////////////////////////////////////////
// ReactorModule
///////////////////////////////////////////////////////////////////////////////

class ReactorModule : public Module
{
public:
    ReactorModule( ModuleInfo* pInfo );
    virtual ~ReactorModule(){};

    float GetCapacitorStorage() const;
    float GetCapacitorRechargeRate() const;

protected:
    virtual void OnDeathEffect() override;

    float m_CapacitorStorage;
    float m_CapacitorRechargeRate;
};

///////////////////////////////////////////////////////////////////////////////
// AddonModule
///////////////////////////////////////////////////////////////////////////////

class AddonModule : public Module
{
public:
    AddonModule( ModuleInfo* pInfo );
    virtual ~AddonModule();

    virtual void Update( float delta ) override;
    virtual void Render( const glm::mat4& modelTransform, bool drawOutline ) override;

    virtual void SetOwner( Ship* pShip ) override;
    Addon* GetAddon() const;

protected:
    void CreateAddon();
    Addon* m_pAddon;
};

inline Addon* AddonModule::GetAddon() const
{
    return m_pAddon;
}

///////////////////////////////////////////////////////////////////////////////
// TowerModule
///////////////////////////////////////////////////////////////////////////////

class TowerModule : public Module
{
public:
    TowerModule( ModuleInfo* pInfo );
    virtual ~TowerModule() override{};
    virtual void Initialise( Ship* pShip ) override;
    virtual void ApplyDamage( float amount, DamageType damageType, Ship* pDealtBy ) override;

    const glm::vec4 GetOverlayColour( Ship* pShip ) const;

protected:
    virtual void OnDamageEffect() override;

    void AddBonus();
    void RemoveBonus();
};

} // namespace Hexterminate
