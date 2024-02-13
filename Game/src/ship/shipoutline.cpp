// Copyright 2021 Pedro Nunes
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

#include <genesis.h>
#include <rendersystem.h>
#include <resources/resourcemodel.h>
#include <shadercache.h>

#include <imgui/imgui.h>

#include "faction/faction.h"
#include "hexterminate.h"
#include "player.h"
#include "ship/hyperspacecore.h"
#include "ship/ship.h"
#include "ship/shipoutline.h"

namespace Hexterminate
{

ShipOutline::ShipOutline()
{
    m_Thickness = 1.15f;
    Genesis::ShaderCache* pShaderCache = Genesis::FrameWork::GetRenderSystem()->GetShaderCache();

    m_pMaterialPlayer = new Genesis::Material();
    m_pMaterialPlayer->name = "ship_outline_player";
    m_pMaterialPlayer->shader = pShaderCache->Load( "ship_outline_player" );

    m_pMaterialFriendly = new Genesis::Material();
    m_pMaterialFriendly->name = "ship_outline_friendly";
    m_pMaterialFriendly->shader = pShaderCache->Load( "ship_outline_friendly" );

    m_pMaterialHostile = new Genesis::Material();
    m_pMaterialHostile->name = "ship_outline_hostile";
    m_pMaterialHostile->shader = pShaderCache->Load( "ship_outline_hostile" );
}

ShipOutline::~ShipOutline()
{
    delete m_pMaterialPlayer;
    delete m_pMaterialFriendly;
    delete m_pMaterialHostile;
}

void ShipOutline::DrawShipOutlineTweaks()
{
    if ( ImGui::CollapsingHeader( "Friend/foe outline" ) )
    {
        ImGui::SliderFloat( "Thickness", &m_Thickness, 1.0f, 2.0f );
    }
}

bool ShipOutline::AffectsShip( Ship* pShip ) const
{
    return IsEnabled() && GetOutlineMaterial( pShip ) != nullptr && ( ( pShip->GetHyperspaceCore() == nullptr ) || ( pShip->GetHyperspaceCore() != nullptr && pShip->GetHyperspaceCore()->IsJumping() == false ) );
}

Genesis::Material* ShipOutline::GetOutlineMaterial( Ship* pShip ) const
{
    if ( pShip == g_pGame->GetPlayer()->GetShip() )
    {
        return m_pMaterialPlayer;
    }
    else if ( Faction::sIsEnemyOf( pShip->GetFaction(), g_pGame->GetPlayerFaction() ) )
    {
        return m_pMaterialHostile;
    }
    else
    {
        return m_pMaterialFriendly;
    }
}

} // namespace Hexterminate
