// Copyright 2015 Pedro Nunes
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

#include "menus/panelshipstats.h"
#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/table.h"
#include "menus/tablerow.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/module.h"
#include "ship/moduleinfo.h"
#include "ship/shield.h"
#include "stringaux.h"
#include <configuration.h>
#include <genesis.h>

#include <sstream>

namespace Hexterminate
{

PanelShipStats::PanelShipStats()
    : m_pMainPanel( nullptr )
    , m_pTitle( nullptr )
    , m_pTable( nullptr )
    , m_pWeaponryTurretRow( nullptr )
    , m_pWeaponryFixedRow( nullptr )
    , m_pShieldCapacityRow( nullptr )
    , m_pShieldRechargeRow( nullptr )
    , m_pShieldEfficiencyRow( nullptr )
    , m_pEnergyCapacityRow( nullptr )
    , m_pEnergyRechargeRow( nullptr )
    , m_pWeaponsEnergyUsageRow( nullptr )
    , m_pAddonsEnergyUsageRow( nullptr )
    , m_pGridRow( nullptr )
    , m_pGridUsedRow( nullptr )
    , m_pGridAvailableRow( nullptr )
    , m_PowerGrid( 0.0f )
    , m_PowerGridUsage( 0.0f )
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();

    const float panelWidth = 300.0f;
    const float screenWidth = static_cast<float>( Genesis::Configuration::GetScreenWidth() );
    const float outerBorder = 8.0f;
    const float innerBorder = 8.0f;

    m_pMainPanel = new Genesis::Gui::Panel();
    m_pMainPanel->SetPosition( screenWidth - panelWidth - outerBorder, outerBorder );
    m_pMainPanel->SetColor( 0.0f, 0.0f, 0.0f, 0.25f );
    m_pMainPanel->SetBorderColor( 1.0f, 1.0f, 1.0f, 0.25f );
    m_pMainPanel->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    pGuiManager->AddElement( m_pMainPanel );

    m_pTitle = new Genesis::Gui::Text();
    m_pTitle->SetSize( 256.0f, 16.0f );
    m_pTitle->SetPosition( innerBorder, innerBorder );
    m_pTitle->SetColor( EVA_TEXT_COLOR );
    m_pTitle->SetFont( EVA_FONT );
    m_pTitle->SetText( "Shipyard > Overview" );
    m_pMainPanel->AddElement( m_pTitle );

    m_pTable = new Table();
    m_pTable->SetPosition( innerBorder, 48.0f );

    AddWeaponryRows();
    AddEmptyRow();
    AddShieldRows();
    AddEmptyRow();
    AddCapacitorRows();
    AddEmptyRow();
    AddPowerGridRows();

    // Resize window to fit all the rows we need
    const float tableHeight = m_pTable->GetRowCount() * m_pTable->GetRowHeight();
    m_pTable->SetSize( panelWidth - innerBorder * 2.0f, tableHeight );
    m_pMainPanel->SetSize( panelWidth, 48.0f + tableHeight + 8.0f );

    m_pMainPanel->AddElement( m_pTable );

    OnShipConfigurationChanged();
}

PanelShipStats::~PanelShipStats()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr )
    {
        pGuiManager->RemoveElement( m_pMainPanel );
    }
}

void PanelShipStats::AddEmptyRow()
{
    TableRow* pEmptyRow = new TableRow();
    pEmptyRow->Add( "" );
    pEmptyRow->Add( "" );
    m_pTable->AddRow( pEmptyRow );
}

void PanelShipStats::AddTitleRow( const std::string& title )
{
    TableRow* pTitleRow = new TableRow();
    pTitleRow->Add( title );
    pTitleRow->Add( "" );
    m_pTable->AddRow( pTitleRow );
}

void PanelShipStats::AddWeaponryRows()
{
    AddTitleRow( "Weaponry" );

    m_pWeaponryTurretRow = new TableRow();
    m_pWeaponryTurretRow->Add( "- Turrets:" );
    m_pWeaponryTurretRow->Add( "0 dps" );
    m_pTable->AddRow( m_pWeaponryTurretRow );

    m_pWeaponryFixedRow = new TableRow();
    m_pWeaponryFixedRow->Add( "- Fixed:" );
    m_pWeaponryFixedRow->Add( "0 dps" );
    m_pTable->AddRow( m_pWeaponryFixedRow );
}

void PanelShipStats::AddShieldRows()
{
    AddTitleRow( "Shield" );

    m_pShieldCapacityRow = new TableRow();
    m_pShieldCapacityRow->Add( "- Capacity:" );
    m_pShieldCapacityRow->Add( "0 u" );
    m_pTable->AddRow( m_pShieldCapacityRow );

    m_pShieldRechargeRow = new TableRow();
    m_pShieldRechargeRow->Add( "- Recharge:" );
    m_pShieldRechargeRow->Add( "0 u/s" );
    m_pTable->AddRow( m_pShieldRechargeRow );

    m_pShieldEfficiencyRow = new TableRow();
    m_pShieldEfficiencyRow->Add( "- Efficiency:" );
    m_pShieldEfficiencyRow->Add( "100%" );
    m_pTable->AddRow( m_pShieldEfficiencyRow );
}

void PanelShipStats::AddCapacitorRows()
{
    AddTitleRow( "Capacitor" );

    m_pEnergyCapacityRow = new TableRow();
    m_pEnergyCapacityRow->Add( "- Capacity:" );
    m_pEnergyCapacityRow->Add( "0 u" );
    m_pTable->AddRow( m_pEnergyCapacityRow );

    m_pEnergyRechargeRow = new TableRow();
    m_pEnergyRechargeRow->Add( "- Recharge:" );
    m_pEnergyRechargeRow->Add( "0 u/s" );
    m_pTable->AddRow( m_pEnergyRechargeRow );

    m_pWeaponsEnergyUsageRow = new TableRow();
    m_pWeaponsEnergyUsageRow->Add( "- Usage (weapons):" );
    m_pWeaponsEnergyUsageRow->Add( "0 u/s" );
    m_pTable->AddRow( m_pWeaponsEnergyUsageRow );

    m_pAddonsEnergyUsageRow = new TableRow();
    m_pAddonsEnergyUsageRow->Add( "- Usage (addons):" );
    m_pAddonsEnergyUsageRow->Add( "0 u/s" );
    m_pTable->AddRow( m_pAddonsEnergyUsageRow );
}

void PanelShipStats::AddPowerGridRows()
{
    AddTitleRow( "Power grid" );

    m_pGridRow = new TableRow();
    m_pGridRow->Add( "- Grid:" );
    m_pGridRow->Add( "0" );
    m_pTable->AddRow( m_pGridRow );

    m_pGridUsedRow = new TableRow();
    m_pGridUsedRow->Add( "- Used:" );
    m_pGridUsedRow->Add( "0" );
    m_pTable->AddRow( m_pGridUsedRow );

    m_pGridAvailableRow = new TableRow();
    m_pGridAvailableRow->Add( "- Available:" );
    m_pGridAvailableRow->Add( "0" );
    m_pTable->AddRow( m_pGridAvailableRow );
}

void PanelShipStats::OnShipConfigurationChanged()
{
    // Reset the energy usage and update it if needed inside the various Update*Stats()
    m_PowerGrid = 0.0f;
    m_PowerGridUsage = 0.0f;

    UpdateWeaponryStats();
    UpdateShieldStats();
    UpdateReactorStats();

    m_pTable->NotifyContentUpdated();
}

void PanelShipStats::UpdateWeaponryStats()
{
    float fixedWeaponDPS = 0.0f;
    float turretWeaponDPS = 0.0f;

    const Ship* pShip = g_pGame->GetPlayer()->GetShip();
    for ( auto& pWeaponModule : pShip->GetModules<WeaponModule>() )
    {
        WeaponInfo* pWeaponInfo = static_cast<WeaponInfo*>( pWeaponModule->GetModuleInfo() );
        if ( pWeaponInfo->GetBehaviour() == WeaponBehaviour::Fixed )
        {
            fixedWeaponDPS += pWeaponInfo->GetDPS( pShip );
        }
        else if ( pWeaponInfo->GetBehaviour() == WeaponBehaviour::Turret )
        {
            turretWeaponDPS += pWeaponInfo->GetDPS( pShip );
        }
    }

    float bonusMultiplier = CalculateBonusMultiplier( TowerBonus::Damage );
    fixedWeaponDPS *= bonusMultiplier;
    turretWeaponDPS *= bonusMultiplier;

    m_pWeaponryFixedRow->Set( 1, ToString( floor( fixedWeaponDPS ) ) + " dps" );
    m_pWeaponryTurretRow->Set( 1, ToString( floor( turretWeaponDPS ) ) + " dps" );
}

void PanelShipStats::UpdateShieldStats()
{
    float shieldCapacity = 0.0f;
    float shieldRecharge = 0.0f;

    const Ship* pShip = g_pGame->GetPlayer()->GetShip();
    auto shieldModules = pShip->GetModules<ShieldModule>();
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

    m_pShieldCapacityRow->Set( 1, ToString( floor( shieldCapacity ) ) + "u" );
    m_pShieldRechargeRow->Set( 1, ToString( floor( shieldRecharge ) ) + "u/s" );
    m_pShieldEfficiencyRow->Set( 1, ToString( floor( shieldEfficiency * 100.0f ) ) + "%" );
}

void PanelShipStats::UpdateReactorStats()
{
    float energyCapacity = 0.0f;
    float energyRecharge = 0.0f;

    const Ship* pShip = g_pGame->GetPlayer()->GetShip();
    const auto& reactorModules = pShip->GetModules<ReactorModule>();
    for ( auto& pReactorModule : reactorModules )
    {
        ReactorInfo* pReactorInfo = static_cast<ReactorInfo*>( pReactorModule->GetModuleInfo() );
        energyCapacity += pReactorInfo->GetCapacitorStorage();
        energyRecharge += pReactorInfo->GetCapacitorRechargeRate();
        m_PowerGrid += pReactorInfo->GetPowerGrid( pShip );
    }

    for ( auto& pModule : pShip->GetModules<ShieldModule>() )
    {
        m_PowerGridUsage += -pModule->GetModuleInfo()->GetPowerGrid( pShip );
    }

    float addonsEnergyPerSecond = 0.0f;
    for ( auto& pModule : pShip->GetModules<AddonModule>() )
    {
        AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pModule->GetModuleInfo() );
        m_PowerGridUsage += -pAddonInfo->GetPowerGrid( pShip );

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
    for ( auto& pModule : pShip->GetModules<WeaponModule>() )
    {
        WeaponInfo* pWeaponInfo = static_cast<WeaponInfo*>( pModule->GetModuleInfo() );
        weaponsEnergyPerSecond += pWeaponInfo->GetActivationCost( pShip ) * pWeaponInfo->GetRateOfFire( pShip );
    }

    m_pEnergyCapacityRow->Set( 1, ToString( floor( energyCapacity ) ) + "u" );
    m_pEnergyRechargeRow->Set( 1, ToString( floor( energyRecharge ) ) + "u/s" );
    m_pAddonsEnergyUsageRow->Set( 1, ToString( floor( addonsEnergyPerSecond ) ) + "u/s" );
    m_pWeaponsEnergyUsageRow->Set( 1, ToString( floor( weaponsEnergyPerSecond ) ) + "u/s" );

    m_pGridRow->Set( 1, ToString( floor( m_PowerGrid ) ) + "u" );
    m_pGridUsedRow->Set( 1, ToString( floor( m_PowerGridUsage ) ) + "u" );
    m_pGridAvailableRow->Set( 1, ToString( floor( m_PowerGrid - m_PowerGridUsage ) ) + "u" );

    Genesis::Color clr = ( m_PowerGrid >= m_PowerGridUsage ) ? Genesis::Color( 1.0f, 1.0f, 1.0f ) : Genesis::Color( 1.0f, 0.0f, 0.0f );
    m_pGridUsedRow->SetColor( clr );
    m_pGridAvailableRow->SetColor( clr );
}

float PanelShipStats::CalculateBonusMultiplier( TowerBonus towerBonus ) const
{
    // Usually the bridge bonus is stored in the sector but this doesn't work correctly when
    // we are editting the ship as it is possible to have multiple bridges affecting the same
    // bonus. This creates an invalid ship which can't be undocked but we should still show
    // the correct stats.
    float multiplier = 1.0f;

    const Ship* pShip = g_pGame->GetPlayer()->GetShip();
    for ( auto& pTowerModule : pShip->GetModules<TowerModule>() )
    {
        TowerInfo* pTowerInfo = static_cast<TowerInfo*>( pTowerModule->GetModuleInfo() );
        if ( pTowerInfo->GetBonusType() == towerBonus )
        {
            multiplier *= pTowerInfo->GetBonusMagnitude();
        }
    }

    return multiplier;
}

void PanelShipStats::Show( bool state )
{
    m_pMainPanel->Show( state );
}

} // namespace Hexterminate