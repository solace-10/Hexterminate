// Copyright 2016 Pedro Nunes
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

#include "menus/hotbar.h"
#include "globals.h"
#include "hexterminate.h"
#include "menus/eva.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/addon/addon.h"
#include "ship/addon/addonphasebarrier.h"
#include "ship/inventory.h"
#include "ship/shield.h"
#include <configuration.h>
#include <genesis.h>
#include <gui/gui.h>

namespace Hexterminate
{

static const std::string sKeyNames[ sNumShipAddons ] = {
    "1", "2", "3", "4", "5", "6"
};

Hotbar::Hotbar()
{
    using namespace Genesis;

    Gui::GuiManager* pGuiManager = FrameWork::GetGuiManager();

    m_pRoot = new Gui::GuiElement();
    m_pRoot->SetPosition( 0, 128 );
    m_pRoot->SetSize( 512, 512 );
    m_pRoot->SetHiddenForCapture( true );
    pGuiManager->AddElement( m_pRoot );

    ResourceImage* pHotbarBgImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/hotbar/background1.png" );
    m_pBackground = new Gui::Image();
    m_pBackground->SetPosition( 0.0f, 0.0f );
    m_pBackground->SetSize( (float)pHotbarBgImage->GetWidth(), (float)pHotbarBgImage->GetHeight() );
    m_pBackground->SetTexture( pHotbarBgImage );
    m_pBackground->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pRoot->AddElement( m_pBackground );

    ResourceImage* pHotbarBg2Image = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/hotbar/background2.png" );
    m_pBackground2 = new Gui::Image();
    m_pBackground2->SetPosition( 40.0f, 0.0f );
    m_pBackground2->SetSize( (float)pHotbarBg2Image->GetWidth(), (float)pHotbarBg2Image->GetHeight() );
    m_pBackground2->SetTexture( pHotbarBg2Image );
    m_pBackground2->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pRoot->AddElement( m_pBackground2 );

    // Both the shield bar and the bridge's health bar are inside a panel, so they can be clipped when
    // we need to change their visible size to match the shield / health values.
    m_pShieldBarPanel = new Gui::Panel();
    m_pShieldBarPanel->SetPosition( 0.0f, 30.0f );
    m_pShieldBarPanel->SetSize( 48.0f, 200.0f );
    m_pShieldBarPanel->Show( false );
    m_pBackground->AddElement( m_pShieldBarPanel );

    m_pPhaseBarPanel = new Gui::Panel();
    m_pPhaseBarPanel->SetPosition( 0.0f, 286.0f );
    m_pPhaseBarPanel->SetSize( 48.0f, 200.0f );
    m_pPhaseBarPanel->Show( false );
    m_pBackground->AddElement( m_pPhaseBarPanel );

    m_pShieldBar = new Gui::Image();
    m_pShieldBar->SetPosition( 0.0f, 0.0f );
    m_pShieldBar->SetSize( 48.0f, 200.0f );
    m_pShieldBar->SetTexture( (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/hotbar/shieldbar.png" ) );
    m_pShieldBarPanel->AddElement( m_pShieldBar );

    m_pPhaseBar = new Gui::Image();
    m_pPhaseBar->SetPosition( 0.0f, 0.0f );
    m_pPhaseBar->SetSize( 48.0f, 200.0f );
    m_pPhaseBar->SetTexture( (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/hotbar/phasebar.png" ) );
    m_pPhaseBarPanel->AddElement( m_pPhaseBar );

    ResourceImage* pPowerImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/hotbar/powerbar.png" );
    m_pPower = new Gui::Image();
    m_pPower->SetPosition( 40.0f, 0.0f );
    m_pPower->SetSize( 16.0f, 512.0f );
    m_pPower->SetTexture( pPowerImage );
    m_pPower->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pPower->SetBlendMode( BlendMode::Screen );
    m_pRoot->AddElement( m_pPower );

    const float iconSeparation = 8.0f;
    const float iconSize = 64.0f;
    const float offsetX = 60.0f;
    const float offsetY = 32.0f;

    for ( int i = 0; i < sMaxAbilityCount; ++i )
    {
        const float iterOffsetY = offsetY + ( iconSize + iconSeparation ) * i;

        m_pAddonGui[ i ] = new Gui::Image();
        m_pAddonGui[ i ]->SetPosition( offsetX, iterOffsetY );
        m_pAddonGui[ i ]->SetSize( iconSize, iconSize );
        m_pAddonGui[ i ]->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
        m_pAddonGui[ i ]->Show( false );

        m_pTextGui[ i ] = new Gui::Text();
        m_pTextGui[ i ]->SetFont( EVA_FONT );
        m_pTextGui[ i ]->SetPosition( offsetX + 58, iterOffsetY );
        m_pTextGui[ i ]->SetSize( 128.0f, 128.0f );
        m_pTextGui[ i ]->SetColour( 0.0f, 1.0f, 1.0f, 0.75f );
        m_pTextGui[ i ]->SetMultiLine( false );
        m_pTextGui[ i ]->SetText( sKeyNames[ i ] );

        m_pCooldownGui[ i ] = new Gui::Panel();
        m_pCooldownGui[ i ]->SetPosition( offsetX + 12.0f, iterOffsetY + iconSize - 8.0f );
        m_pCooldownGui[ i ]->SetSize( iconSize - 24.0f, 4.0f );
        m_pCooldownGui[ i ]->SetColour( 0.0f, 1.0f, 1.0f, 0.75f );
        m_pCooldownGui[ i ]->Show( false );

        m_pRoot->AddElement( m_pAddonGui[ i ] );
        m_pRoot->AddElement( m_pTextGui[ i ] );
        m_pRoot->AddElement( m_pCooldownGui[ i ] );
    }
}

Hotbar::~Hotbar()
{
    Genesis::FrameWork::GetGuiManager()->RemoveElement( m_pRoot );
}

void Hotbar::UpdatePhaseBar( Ship* pPlayerShip )
{
    AddonPhaseBarrier* pPhaseBarrierAddon = nullptr;
    const AddonModuleList& addonModules = pPlayerShip->GetAddonModules();
    for ( const auto& pAddonModule : addonModules )
    {
        if ( pAddonModule->GetAddon()->GetInfo()->GetCategory() == AddonCategory::PhaseBarrier )
        {
            pPhaseBarrierAddon = static_cast<AddonPhaseBarrier*>( pAddonModule->GetAddon() );
            break;
        }
    }

    if ( pPhaseBarrierAddon == nullptr )
    {
        m_pPhaseBarPanel->Show( false );
    }
    else
    {
        const float barHeight = floor( pPhaseBarrierAddon->GetCharge() * 200.0f );

        m_pPhaseBar->SetPosition( 0.0f, -200.0f + barHeight );
        m_pPhaseBarPanel->SetPosition( 0.0f, 486.0f - barHeight );
        m_pPhaseBarPanel->SetHeight( barHeight );

        m_pPhaseBarPanel->Show( true );
    }
}

void Hotbar::UpdateShieldBar( Ship* pPlayerShip )
{
    Shield* pShield = pPlayerShip->GetShield();
    if ( pShield == nullptr )
    {
        m_pShieldBarPanel->Show( false );
    }
    else
    {
        const float health = pShield->GetCurrentHealthPoints();
        const float maxHealth = pShield->GetMaximumHealthPoints();
        float ratio = ( maxHealth > 0.0f ) ? ( health / maxHealth ) : 0.0f;
        if ( ratio > 1.0f )
            ratio = 1.0f;
        const float barHeight = floor( ratio * 200.0f );

        m_pShieldBar->SetPosition( 0.0f, -200.0f + barHeight );
        m_pShieldBarPanel->SetPosition( 0.0f, 230.0f - barHeight );
        m_pShieldBarPanel->SetHeight( barHeight );

        m_pShieldBarPanel->Show( true );
    }
}

void Hotbar::ResizeBar( Genesis::Gui::Panel* pBar, float ratio, float originalY )
{
    const float maxHeight = 200.0f;
    float height = maxHeight * ratio;
    pBar->SetHeight( height );
}

void Hotbar::Update( float fDelta )
{
    using namespace Genesis;

    if ( g_pGame->GetCurrentSector() == nullptr || g_pGame->GetPlayer() == nullptr )
        return;

    Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
    if ( pPlayerShip )
    {
        DockingState dockingState = pPlayerShip->GetDockingState();
        m_pRoot->Show( dockingState != DockingState::Docking && dockingState != DockingState::Docked );

        UpdatePhaseBar( pPlayerShip );
        UpdateShieldBar( pPlayerShip );
        UpdateAbilities( pPlayerShip );

        const float energyCapacity = pPlayerShip->GetEnergyCapacity();
        m_pPower->Show( energyCapacity > 0.0f );
        if ( energyCapacity > 0.0f )
        {
            const float energyRatio = pPlayerShip->GetEnergy() / energyCapacity;
            m_pPower->SetPosition( 40.0f, 512.0f - 512.0f * energyRatio );
            m_pPower->SetSize( 16.0f, 512.0f * energyRatio );
        }
    }
}

void Hotbar::UpdateAbilities( Ship* pPlayerShip )
{
    using namespace Genesis;

    AddonModuleList addonModules = pPlayerShip->GetAddonModules();
    addonModules.sort( AddonModuleSortPredicate );

    addonModules.remove_if(
        []( const AddonModule* pAddonModule ) {
            return ( (AddonInfo*)( pAddonModule->GetModuleInfo() ) )->GetCategory() == AddonCategory::QuantumStateAlternator;
        } );

    const size_t addonsCount = addonModules.size();
    int abilityCount = static_cast<int>( addonsCount );
    if ( abilityCount > sNumShipAddons )
        abilityCount = sNumShipAddons;

    const bool hasRammingSpeed = g_pGame->GetPlayer()->GetPerks()->IsEnabled( Perk::RammingSpeed );
    int rammingSpeedAbilityIndex = -1;
    if ( hasRammingSpeed )
    {
        rammingSpeedAbilityIndex = abilityCount;
        abilityCount++;
    }

    const bool hasAlternator = pPlayerShip->GetQuantumStateAlternator() != nullptr;
    int alternatorAbilityIndex = -1;
    if ( hasAlternator )
    {
        alternatorAbilityIndex = abilityCount;
        abilityCount++;
    }

    if ( abilityCount > sMaxAbilityCount )
    {
        abilityCount = sMaxAbilityCount;
    }

    int keyNameIndex = 0;
    AddonModuleList::const_iterator it = addonModules.begin();
    for ( int i = 0; i < static_cast<int>( abilityCount ); ++i )
    {
        AbilityData& ability = m_Abilities[ i ];

        if ( i  == rammingSpeedAbilityIndex )
        {
            ability.key = "TAB";
            ability.name = "Ramming speed";
            ability.inCooldown = pPlayerShip->GetRammingSpeedCooldown() > 0.0f;
            ability.isActive = pPlayerShip->IsRammingSpeedEnabled();
            ability.cooldownRatio = pPlayerShip->GetRammingSpeedCooldown() / RammingSpeedCooldown;
            ability.isUsable = !ability.inCooldown;
            ability.pIcon = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/icons/navigation.png" );
        }
        else if ( i == alternatorAbilityIndex )
        {
            ability.key = "Space";
            ability.name = "Switch quantum phase";
            ability.inCooldown = false;
            ability.isActive = false;
            ability.cooldownRatio = 1.0f;
            ability.isUsable = true;
            ability.pIcon = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( "data/ui/icons/quantum_alternator.png" );
        }
        else
        {
            Addon* pAddon = ( *it )->GetAddon();
            AddonInfo* pAddonInfo = static_cast<AddonInfo*>( ( *it )->GetModuleInfo() );

            ability.key = sKeyNames[ keyNameIndex++ ];
            ability.name = pAddonInfo->GetFullName();
            ability.inCooldown = pAddon->GetCooldown() > 0.0f;
            ability.isActive = pAddon->IsActive();
            ability.cooldownRatio = pAddon->GetCooldown() / pAddonInfo->GetCooldown();
            ability.isUsable = pAddon->CanUse();
            ability.pIcon = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( pAddonInfo->GetIcon() );

            ++it;
        }
    }

    for ( int i = 0; i < sMaxAbilityCount; ++i )
    {
        const AbilityData& ability = m_Abilities[ i ];

        if ( i >= abilityCount ) // Hide all unused pAddonModule icons
        {
            m_pAddonGui[ i ]->Show( false );
            m_pTextGui[ i ]->Show( false );
            m_pCooldownGui[ i ]->Show( false );
            continue;
        }

        m_pAddonGui[ i ]->Show( true );
        m_pTextGui[ i ]->Show( true );

        // Displays the module's name when hovering the icon
        if ( m_pAddonGui[ i ]->IsMouseInside() )
        {
            m_pTextGui[ i ]->SetText( ability.name );
        }
        else
        {
            m_pTextGui[ i ]->SetText( ability.key );
        }

        if ( ability.inCooldown )
        {
            m_pCooldownGui[ i ]->SetSize( 40.0f * ability.cooldownRatio, 4.0f );
            m_pCooldownGui[ i ]->Show( true );
        }
        else
        {
            m_pCooldownGui[ i ]->Show( false );
        }

        if ( ability.pIcon )
        {
            m_pAddonGui[ i ]->SetTexture( ability.pIcon );
        }

        if ( ability.inCooldown )
            m_pAddonGui[ i ]->SetColour( 1.0f, 1.0f, 1.0f, 0.33f );
        else if ( ability.isActive )
            m_pAddonGui[ i ]->SetColour( 0.0f, 1.0f, 0.25f, 1.0f );
        else if ( ability.isUsable == false )
            m_pAddonGui[ i ]->SetColour( 1.0f, 0.0f, 0.0f, 0.33f );
        else
            m_pAddonGui[ i ]->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    }
}

} // namespace Hexterminate