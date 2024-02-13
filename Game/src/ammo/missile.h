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

#include "ammo/ammo.h"

namespace Genesis
{
class ResourceModel;
}

namespace Hexterminate
{

class Weapon;
class Trail;
class Ship;

enum class MissileType
{
    Missile,
    Torpedo,
    Rocket
};

///////////////////////////////////////////////////////////////////////////////
// Missile
///////////////////////////////////////////////////////////////////////////////

class Missile : public Ammo
{
public:
    Missile();
    virtual ~Missile() override;

    virtual void Create( Weapon* pWeapon, float additionalRotation = 0.0f ) override;
    virtual void Update( float delta ) override;
    virtual void Render() override;
    virtual void Kill() override;
    virtual bool CanBeIntercepted() const override;
    virtual bool CanBypassShields() const override;

protected:
    virtual MissileType GetType() const;
    virtual const std::string GetResourceName() const;
    virtual Trail* CreateTrail() const;

    void SetGlowSize( float size );
    float GetGlowSize() const;
    void SetGlowColour( const Genesis::Color& colour );
    const Genesis::Color& GetGlowColour() const;

private:
    Ship* FindClosestShip( const glm::vec3& position );
    void UpdateTarget( float delta );
    void TrackTarget( float delta );
    void UpdateGlow();

    Genesis::ResourceModel* m_pModel;
    Trail* m_pTrail;
    Ship* m_pTargetShip;
    float m_LaunchTimer;
    float m_SwarmTimer;
    float m_GlowSize;
    Genesis::Color m_GlowColour;
};

inline void Missile::SetGlowSize( float size )
{
    m_GlowSize = size;
}

inline float Missile::GetGlowSize() const
{
    return m_GlowSize;
}

inline void Missile::SetGlowColour( const Genesis::Color& glowColour )
{
    m_GlowColour = glowColour;
}

inline const Genesis::Color& Missile::GetGlowColour() const
{
    return m_GlowColour;
}

inline bool Missile::CanBeIntercepted() const
{
    return m_LaunchTimer > 0.5f;
}

inline bool Missile::CanBypassShields() const
{
    return true;
}

} // namespace Hexterminate
