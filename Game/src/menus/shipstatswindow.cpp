// Copyright 2024 Pedro Nunes
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

#include "menus/shipstatswindow.h"

#include <sstream>

#include <configuration.h>
#include <genesis.h>

#include "fleet/fleet.h"
#include "hexterminate.h"
#include "player.h"
#include "ship/shield.h"
#include "ship/shipinfo.h"
#include "stringaux.h"
#include "ui/icon.h"
#include "ui/image.h"
#include "ui/panel.h"
#include "ui/rootelement.h"
#include "ui/text.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// ShipStatsWindow
/////////////////////////////////////////////////////////////////////

ShipStatsWindow::ShipStatsWindow()
    : UI::Window( "Ship stats window", true )
{
    AddSection( "Weapons", m_Weaponry );
    AddEntry( m_Weaponry, "Kinetic", m_WeaponryKinetic );
    AddEntry( m_Weaponry, "Energy", m_WeaponryEnergy );
    AddEntry( m_Weaponry, "Exotic", m_WeaponryExotic );

    AddSection( "Shields", m_Shields );
    AddEntry( m_Shields, "Capacity", m_ShieldsCapacity );
    AddEntry( m_Shields, "Recharge", m_ShieldsRecharge );
    AddEntry( m_Shields, "Efficiency", m_ShieldsEfficiency );

    AddSection( "Capacitor", m_Capacitor );
    AddEntry( m_Capacitor, "Capacity", m_CapacitorCapacity );
    AddEntry( m_Capacitor, "Recharge", m_CapacitorRecharge );
    AddEntry( m_Capacitor, "Weapons", m_CapacitorWeapons );
    AddEntry( m_Capacitor, "Addons", m_CapacitorAddons );

    AddSection( "Energy grid", m_Grid );
    AddEntry( m_Grid, "Used", m_GridUsed );
    AddEntry( m_Grid, "Available", m_GridAvailable );

    AddSection( "Navigation", m_Navigation );
    AddEntry( m_Navigation, "Linear", m_NavigationLinear );
    AddEntry( m_Navigation, "Angular", m_NavigationAngular );
}

ShipStatsWindow::~ShipStatsWindow()
{
}

void ShipStatsWindow::Show( bool state )
{
    UI::Window::Show( state );
}

void ShipStatsWindow::OnShipConfigurationChanged()
{
    UpdateWeaponryStats();
    UpdateShieldStats();
    UpdateCapacitorStats();
    UpdateEnergyGridStats();
    UpdateNavigationStats();
}

void ShipStatsWindow::AddSection( const std::string& baseName, Section& section )
{
    section.pPanel = std::make_shared<UI::Panel>( baseName + " panel" );
    GetContentPanel()->Add( section.pPanel );
    section.pSubtitle = std::make_shared<UI::Text>( baseName + " subtitle" );
    section.pPanel->Add( section.pSubtitle );
}

void ShipStatsWindow::AddEntry( Section& section, const std::string& baseName, Entry& entry )
{
    entry.pIcon = std::make_shared<UI::Icon>( baseName + " icon" );
    section.pPanel->Add( entry.pIcon );
    entry.pText = std::make_shared<UI::Text>( baseName + " text" );
    section.pPanel->Add( entry.pText );
    entry.pValue = std::make_shared<UI::Text>( baseName + " value" );
    section.pPanel->Add( entry.pValue );
}

void ShipStatsWindow::UpdateWeaponryStats()
{
    float kinetic = 0.0f;
    float energy = 0.0f;
    float exotic = 0.0f;

    Ship* pShip = g_pGame->GetPlayer()->GetShip();
    const WeaponModuleList& weaponModules = pShip->GetWeaponModules();
    for ( auto& pWeaponModule : weaponModules )
    {
        const WeaponInfo* pWeaponInfo = static_cast<WeaponInfo*>( pWeaponModule->GetModuleInfo() );
        const DamageType type = pWeaponInfo->GetDamageType();
        const float dps = pWeaponInfo->GetDPS( pShip );
        if ( type == DamageType::Kinetic )
        {
            kinetic += dps;
        }
        else if ( type == DamageType::Energy )
        {
            energy += dps;
        }
        else if ( type == DamageType::TrueDamage )
        {
            exotic += dps;
        }
    }

    const float bonusMultiplier = CalculateBonusMultiplier( TowerBonus::Damage );
    m_WeaponryKinetic.pValue->SetText( ToString( floor( kinetic * bonusMultiplier ) ) + " DPS" );
    m_WeaponryEnergy.pValue->SetText( ToString( floor( energy * bonusMultiplier ) ) + " DPS" );
    m_WeaponryExotic.pValue->SetText( ToString( floor( exotic * bonusMultiplier ) ) + " DPS" );
}

void ShipStatsWindow::UpdateShieldStats()
{
    float shieldCapacity = 0.0f;
    float shieldRecharge = 0.0f;

    Ship* pShip = g_pGame->GetPlayer()->GetShip();
    const ShieldModuleList& shieldModules = pShip->GetShieldModules();
    float shieldEfficiency = Shield::CalculateEfficiency( shieldModules );

    for ( auto& pShieldModule : shieldModules )
    {
        ShieldInfo* pShieldInfo = static_cast<ShieldInfo*>( pShieldModule->GetModuleInfo() );
        shieldCapacity += pShieldInfo->GetCapacity() * shieldEfficiency;
        shieldRecharge += pShieldInfo->GetPeakRecharge() * shieldEfficiency;
    }

    float bonusMultiplier = CalculateBonusMultiplier( TowerBonus::Shields );
    shieldCapacity *= bonusMultiplier;
    shieldRecharge *= bonusMultiplier;

    m_ShieldsCapacity.pValue->SetText( ToString( floor( shieldCapacity ) ) + "GW" );
    m_ShieldsRecharge.pValue->SetText( ToString( floor( shieldRecharge ) ) + "GW/s" );
    m_ShieldsEfficiency.pValue->SetText( ToString( floor( shieldEfficiency * 100.0f ) ) + "%" );
}

void ShipStatsWindow::UpdateCapacitorStats()
{
    float capacity = 0.0f;
    float recharge = 0.0f;

    Ship* pShip = g_pGame->GetPlayer()->GetShip();
    const ReactorModuleList& reactorModules = pShip->GetReactorModules();
    for ( auto& pReactorModule : reactorModules )
    {
        ReactorInfo* pReactorInfo = static_cast<ReactorInfo*>( pReactorModule->GetModuleInfo() );
        capacity += pReactorInfo->GetCapacitorStorage();
        recharge += pReactorInfo->GetCapacitorRechargeRate();
    }

    float addonsEnergyPerSecond = 0.0f;
    for ( auto& pModule : pShip->GetAddonModules() )
    {
        AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pModule->GetModuleInfo() );

        if ( pAddonInfo->GetType() == AddonActivationType::Trigger )
        {
            SDL_assert( pAddonInfo->GetCooldown() > 0.0f );
            addonsEnergyPerSecond += pAddonInfo->GetActivationCost() / pAddonInfo->GetCooldown();
        }
        else if ( pAddonInfo->GetType() == AddonActivationType::Toggle )
        {
            addonsEnergyPerSecond += pAddonInfo->GetActivationCost();
        }
    }

    float weaponsEnergyPerSecond = 0.0f;
    for ( auto& pModule : pShip->GetWeaponModules() )
    {
        WeaponInfo* pWeaponInfo = static_cast<WeaponInfo*>( pModule->GetModuleInfo() );
        weaponsEnergyPerSecond += pWeaponInfo->GetActivationCost( pShip ) * pWeaponInfo->GetRateOfFire( pShip );
    }

    m_CapacitorCapacity.pValue->SetText( ToString( floor( capacity ) ) + "GW" );
    m_CapacitorRecharge.pValue->SetText( ToString( floor( recharge ) ) + "GW/s" );
    m_CapacitorWeapons.pValue->SetText( ToString( floor( addonsEnergyPerSecond ) ) + "GW/s" );
    m_CapacitorAddons.pValue->SetText( ToString( floor( weaponsEnergyPerSecond ) ) + "GW/s" );
}

void ShipStatsWindow::UpdateEnergyGridStats()
{
// float energyCapacity = 0.0f;
//     float energyRecharge = 0.0f;

//     Ship* pShip = g_pGame->GetPlayer()->GetShip();
//     const ReactorModuleList& reactorModules = pShip->GetReactorModules();
//     for ( auto& pReactorModule : reactorModules )
//     {
//         ReactorInfo* pReactorInfo = static_cast<ReactorInfo*>( pReactorModule->GetModuleInfo() );
//         energyCapacity += pReactorInfo->GetCapacitorStorage();
//         energyRecharge += pReactorInfo->GetCapacitorRechargeRate();
//     }

//     for ( auto& pModule : pShip->GetReactorModules() )
//     {
//         m_PowerGrid += pModule->GetModuleInfo()->GetPowerGrid( pShip );
//     }

//     for ( auto& pModule : pShip->GetShieldModules() )
//     {
//         m_PowerGridUsage += -pModule->GetModuleInfo()->GetPowerGrid( pShip );
//     }

//     float addonsEnergyPerSecond = 0.0f;
//     for ( auto& pModule : pShip->GetAddonModules() )
//     {
//         AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pModule->GetModuleInfo() );
//         m_PowerGridUsage += -pAddonInfo->GetPowerGrid( pShip );

//         if ( pAddonInfo->GetType() == AddonActivationType::Trigger )
//         {
//             SDL_assert( pAddonInfo->GetCooldown() > 0.0f );
//             addonsEnergyPerSecond += pAddonInfo->GetActivationCost() / pAddonInfo->GetCooldown();
//         }
//         else if ( pAddonInfo->GetType() == AddonActivationType::Toggle )
//         {
//             addonsEnergyPerSecond += pAddonInfo->GetActivationCost();
//         }
//     }

//     float weaponsEnergyPerSecond = 0.0f;
//     for ( auto& pModule : pShip->GetWeaponModules() )
//     {
//         WeaponInfo* pWeaponInfo = static_cast<WeaponInfo*>( pModule->GetModuleInfo() );
//         weaponsEnergyPerSecond += pWeaponInfo->GetActivationCost( pShip ) * pWeaponInfo->GetRateOfFire( pShip );
//     }

//     m_pEnergyCapacityRow->Set( 1, ToString( floor( energyCapacity ) ) + "u" );
//     m_pEnergyRechargeRow->Set( 1, ToString( floor( energyRecharge ) ) + "u/s" );
//     m_pAddonsEnergyUsageRow->Set( 1, ToString( floor( addonsEnergyPerSecond ) ) + "u/s" );
//     m_pWeaponsEnergyUsageRow->Set( 1, ToString( floor( weaponsEnergyPerSecond ) ) + "u/s" );

//     m_pGridRow->Set( 1, ToString( floor( m_PowerGrid ) ) + "u" );
//     m_pGridUsedRow->Set( 1, ToString( floor( m_PowerGridUsage ) ) + "u" );
//     m_pGridAvailableRow->Set( 1, ToString( floor( m_PowerGrid - m_PowerGridUsage ) ) + "u" );

//     Genesis::Color clr = ( m_PowerGrid >= m_PowerGridUsage ) ? Genesis::Color( 1.0f, 1.0f, 1.0f ) : Genesis::Color( 1.0f, 0.0f, 0.0f );
//     m_pGridUsedRow->SetColour( clr );
//     m_pGridAvailableRow->SetColour( clr );
}

void ShipStatsWindow::UpdateNavigationStats()
{

}

float ShipStatsWindow::CalculateBonusMultiplier( TowerBonus towerBonus ) const
{
    // Usually the bridge bonus is stored in the sector but this doesn't work correctly when
    // we are editting the ship as it is possible to have multiple bridges affecting the same
    // bonus. This creates an invalid ship which can't be undocked but we should still show
    // the correct stats.
    float multiplier = 1.0f;

    Ship* pShip = g_pGame->GetPlayer()->GetShip();
    const TowerModuleList& towerModules = pShip->GetTowerModules();
    for ( auto& pTowerModule : towerModules )
    {
        TowerInfo* pTowerInfo = static_cast<TowerInfo*>( pTowerModule->GetModuleInfo() );
        if ( pTowerInfo->GetBonusType() == towerBonus )
        {
            multiplier *= pTowerInfo->GetBonusMagnitude();
        }
    }

    return multiplier;
}

} // namespace Hexterminate
