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

#include "ship/ship.fwd.h"

// clang-format off
#include <beginexternalheaders.h>
#include <glm/gtx/transform.hpp>
#include <endexternalheaders.h>
// clang-format on

namespace Genesis
{
class ResourceSound;
}

namespace Hexterminate
{

class AddonInfo;
class AddonModule;
class Ship;

bool AddonModuleSortPredicate( AddonModule* pModule1, AddonModule* pModule2 );

///////////////////////////////////////////////////////////////////////////////
// Addon
///////////////////////////////////////////////////////////////////////////////

class Addon
{
protected:
    Addon( AddonModule* pInfo, Ship* pOwner );

public:
    virtual ~Addon() {}

    virtual void Initialise() {}
    virtual void Render( const glm::mat4& modelTransform ) {}

    virtual bool CanUse() const;
    virtual void Activate();
    virtual void Deactivate();
    virtual void Update( float delta );

    AddonModule* GetModule() const;
    AddonInfo* GetInfo() const;
    float GetCooldown() const;
    bool IsActive() const;

protected:
    Ship* GetOwner() const;

    AddonModule* m_pModule;
    AddonInfo* m_pInfo;
    Ship* m_pOwner;
    float m_Cooldown;
    bool m_IsActive;
};

inline AddonModule* Addon::GetModule() const
{
    return m_pModule;
}

inline AddonInfo* Addon::GetInfo() const
{
    return m_pInfo;
}

inline float Addon::GetCooldown() const
{
    return m_Cooldown;
}

inline bool Addon::IsActive() const
{
    return m_IsActive;
}

inline Ship* Addon::GetOwner() const
{
    return m_pOwner;
}

///////////////////////////////////////////////////////////////////////////////
// AddonModuleRepairer
///////////////////////////////////////////////////////////////////////////////

class AddonModuleRepairer : public Addon
{
public:
    AddonModuleRepairer( AddonModule* pModule, Ship* pOwner );
    virtual ~AddonModuleRepairer(){};
    virtual void Activate() override;

private:
    float m_RepairAmount;
};

///////////////////////////////////////////////////////////////////////////////
// AddonHangarBay
///////////////////////////////////////////////////////////////////////////////

class AddonHangarBay : public Addon
{
public:
    AddonHangarBay( AddonModule* pModule, Ship* pOwner );
    virtual ~AddonHangarBay(){};
    virtual bool CanUse() const { return false; }
};

///////////////////////////////////////////////////////////////////////////////
// AddonDroneBay
///////////////////////////////////////////////////////////////////////////////

class AddonDroneBay : public Addon
{
public:
    AddonDroneBay( AddonModule* pModule, Ship* pOwner );
    virtual ~AddonDroneBay(){};
    virtual bool CanUse() const { return false; }
};

///////////////////////////////////////////////////////////////////////////////
// AddonFuelInjector
///////////////////////////////////////////////////////////////////////////////

class AddonFuelInjector : public Addon
{
public:
    AddonFuelInjector( AddonModule* pModule, Ship* pOwner );
    virtual ~AddonFuelInjector(){};
    float GetEnginePowerMultiplier() const;

private:
    float m_EnginePowerMultiplier;
};

inline float AddonFuelInjector::GetEnginePowerMultiplier() const
{
    return m_EnginePowerMultiplier;
}

///////////////////////////////////////////////////////////////////////////////
// AddonQuantumStateAlternator
///////////////////////////////////////////////////////////////////////////////

class AddonQuantumStateAlternator : public Addon
{
public:
    AddonQuantumStateAlternator( AddonModule* pModule, Ship* pOwner );
    virtual ~AddonQuantumStateAlternator(){};
    float GetShieldResistance() const;

    void SetState( QuantumState state );
    QuantumState GetQuantumState() const;

private:
    void LoadSFX();
    void PlaySFX();

    float m_ShieldResistance;
    Genesis::ResourceSound* m_pSFX;
    mutable QuantumState m_State;
};

inline float AddonQuantumStateAlternator::GetShieldResistance() const
{
    return m_ShieldResistance;
}

} // namespace Hexterminate
