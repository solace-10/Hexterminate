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

#include <algorithm>
#include <sstream>
#include <stdarg.h>

#include "globals.h"
#include "hexterminate.h"
#include "menus/eva.h"
#include "menus/panelshipyard.h"
#include "menus/table.h"
#include "menus/tablerow.h"
#include "player.h"
#include "ship/inventory.h"
#include "shipyard/shipyard.h"
#include "stringaux.h"
#include <configuration.h>
#include <genesis.h>
#include <gui/gui.h>

#ifdef _DEBUG
#include "menus/panelshipyarddebug.h"
#endif

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////
// PanelShipyard
///////////////////////////////////////////////////////////////////////////

PanelShipyard::PanelShipyard()
    : m_pTableWindow( nullptr )
    , m_pTableTitle( nullptr )
    , m_SelectedGroup( ModuleType::Engine )
    , m_pButtonUndock( nullptr )
    , m_pScrollingElement( nullptr )
{
    for ( int i = 0; i < (int)ModuleType::Count; ++i )
    {
        m_pIcons[ i ] = nullptr;
        m_pTable[ i ] = nullptr;
    }

    CreateTableWindow();
    CreateButtonUndock();

    g_pGame->SetInputBlocked( true );
}

PanelShipyard::~PanelShipyard()
{
    delete m_pTableWindow;
    Genesis::FrameWork::GetGuiManager()->RemoveElement( m_pButtonUndock );

    g_pGame->SetInputBlocked( false );
}

void PanelShipyard::CreateButtonUndock()
{
    m_pButtonUndock = new ButtonUndock();
    m_pButtonUndock->SetPosition( 8, Genesis::Configuration::GetScreenHeight() - 40 );
    m_pButtonUndock->SetSize( m_pTableWindow->GetMainPanel()->GetWidth(), 32 );
    m_pButtonUndock->SetColour( EVA_BUTTON_COLOUR_BACKGROUND );
    m_pButtonUndock->SetHoverColour( EVA_BUTTON_COLOUR_HOVER );
    m_pButtonUndock->SetBorderColour( EVA_BUTTON_COLOUR_BORDER );
    m_pButtonUndock->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    m_pButtonUndock->SetFont( EVA_FONT );
    m_pButtonUndock->SetText( "Undock" );
    Genesis::FrameWork::GetGuiManager()->AddElement( m_pButtonUndock );
}

void PanelShipyard::CreateTableWindow()
{
    using namespace Genesis;

    unsigned int screenHeight = Configuration::GetScreenHeight();

    m_pTableWindow = new EvaWindow( 8, 8, 658, screenHeight - 56, false );

    m_pTableTitle = new Gui::Text();
    m_pTableTitle->SetSize( 642, 32 );
    m_pTableTitle->SetPosition( 8, 8 );
    m_pTableTitle->SetColour( EVA_TEXT_COLOUR );
    m_pTableTitle->SetFont( EVA_FONT );
    m_pTableTitle->SetText( "Shipyard > Inventory" );
    m_pTableWindow->GetMainPanel()->AddElement( m_pTableTitle );

    float iconSize = 64.0f;
    float iconSpacing = 8.0f;
    float iconBarWidth = iconSize * (int)ModuleType::Count + iconSpacing * ( (int)ModuleType::Count - 1 );

    const std::string iconFile[ (int)ModuleType::Count ] = {
        "engine.png",
        "armour.png",
        "shield.png",
        "weapon.png",
        "reactor.png",
        "addon.png",
        "bridge.png"
    };

    for ( int i = 0; i < (int)ModuleType::Count; ++i )
    {
        ButtonModule* pIcon = new ButtonModule( this, static_cast<ModuleType>( i ) );
        m_pIcons[ i ] = pIcon;

        pIcon->SetSize( iconSize, iconSize );
        pIcon->SetPosition( 325.0f - iconBarWidth / 2.0f + i * ( iconSize + iconSpacing ), 32.0f );
        pIcon->SetColour( 1.0f, 1.0f, 1.0f, 0.4f );
        pIcon->SetTexture( (ResourceImage*)FrameWork::GetResourceManager()->GetResource( std::string( "data/ui/icons/" ) + iconFile[ i ] ) );
        m_pTableWindow->GetMainPanel()->AddElement( pIcon );
    }

    Genesis::Gui::GuiElement* pOwner = m_pTableWindow->GetMainPanel();

    m_pScrollingElement = new Genesis::Gui::ScrollingElement();
    m_pScrollingElement->SetPosition( 8, 104 );
    m_pScrollingElement->SetSize( 642, screenHeight - 168 );
    m_pScrollingElement->Init();
    pOwner->AddElement( m_pScrollingElement );

    CreateTable( ModuleType::Engine, 2, "Thrust", "Torque" );
    CreateTable( ModuleType::Armour, 3, "Armour", "KR", "ER" );
    CreateTable( ModuleType::Shield, 3, "Capacity", "Recharge", "Grid" );
    CreateTable( ModuleType::Weapon, 3, "Type", "DPS", "Range" );
    CreateTable( ModuleType::Reactor, 3, "Capacitor", "Recharge", "Grid" );
    CreateTable( ModuleType::Addon, 1, "Type" );
    CreateTable( ModuleType::Tower, 1, "Description" );

    FillTables();

    for ( int i = 0; i < (int)ModuleType::Count; ++i )
    {
        Table* pTable = m_pTable[ i ];
        if ( pTable != nullptr )
        {
            pTable->Show( false ); // the correct one to display will be chosen in SetSelectedGroup()
            pTable->SetPosition( 0, 0 );
            pTable->SetSize( m_pScrollingElement->GetWidth() - m_pScrollingElement->GetScrollbarWidth(), static_cast<int>( pTable->GetRowCount() ) * static_cast<int>( pTable->GetRowHeight() ) );
            m_pScrollingElement->AddElement( pTable );
        }
    }

    SetSelectedGroup( ModuleType::Engine );
}

TableRow* PanelShipyard::CreateCommonRow() const
{
    TableRow* pTitleRow = new TableRow();
    pTitleRow->Add( "Name" );
    pTitleRow->Add( "#" );

    return pTitleRow;
}

// It would be nice to use a initializer_list, but Visual Studio Express 2012 doesn't support it yet...
void PanelShipyard::CreateTable( ModuleType type, int columns, ... )
{
    Table* pTable = new Table();
    m_pTable[ (int)type ] = pTable;

    TableRow* pTitleRow = CreateCommonRow();

    va_list ap;
    va_start( ap, columns );
    for ( int i = 1; i <= columns; ++i )
    {
        char* columnTitle = va_arg( ap, char* ); // char* as va_arg doesn't support non-POD types
        pTitleRow->Add( columnTitle );
    }
    va_end( ap );

    pTable->AddRow( pTitleRow );
}

void PanelShipyard::FillTables()
{
    std::vector<ModuleInfo*> modulesByRarity[ (int)ModuleRarity::Count ];

    Ship* pShip = g_pGame->GetPlayer()->GetShip();
    const InventoryItemMap& itemMap = g_pGame->GetPlayer()->GetInventory()->GetItems();
    for ( auto& item : itemMap )
    {
        ModuleInfo* pModuleInfo = g_pGame->GetModuleInfoManager()->GetModuleByName( item.first );
        if ( pModuleInfo == nullptr )
        {
            Genesis::FrameWork::GetLogger()->LogError( "Attempting to add to inventory non-existent module '%s'", item.first.c_str() );
            continue;
        }
        else
        {
            modulesByRarity[ (int)pModuleInfo->GetRarity() ].push_back( pModuleInfo );
        }
    }

    // Display items sorted first by rarity (rarest first) then by name.
    for ( int i = (int)ModuleRarity::Count - 1; i >= 0; i-- )
    {
        std::sort(
            modulesByRarity[ i ].begin(),
            modulesByRarity[ i ].end(),
            []( ModuleInfo* pModuleInfoA, ModuleInfo* pModuleInfoB ) { return pModuleInfoA->GetFullName() < pModuleInfoB->GetFullName(); } );

        for ( auto& pModuleInfo : modulesByRarity[ i ] )
        {
            TableRowInventory* pItemRow = new TableRowInventory( this, pModuleInfo );
            pItemRow->SetColour( ModuleRarityToColour( pModuleInfo->GetRarity() ) );
            pItemRow->Add( pModuleInfo->GetFullName() );
            pItemRow->Add( ToString( itemMap.find( pModuleInfo->GetName() )->second.quantity ) ); // quantity

            ModuleType moduleType = pModuleInfo->GetType();
            if ( moduleType == ModuleType::Engine )
            {
                EngineInfo* pEngineInfo = static_cast<EngineInfo*>( pModuleInfo );
                pItemRow->Add( ToString( pEngineInfo->GetThrust() ) );
                pItemRow->Add( ToString( pEngineInfo->GetTorque() ) );
            }
            else if ( moduleType == ModuleType::Weapon )
            {
                WeaponInfo* pWeaponInfo = static_cast<WeaponInfo*>( pModuleInfo );
                pItemRow->Add( ToString( pWeaponInfo->GetSystem() ) );
                pItemRow->Add( ToString( static_cast<int>( pWeaponInfo->GetDPS( pShip ) ) ) );
                pItemRow->Add( ToString( static_cast<int>( pWeaponInfo->GetRange( pShip ) ) ) );
            }
            else if ( moduleType == ModuleType::Armour )
            {
                ArmourInfo* pArmourInfo = static_cast<ArmourInfo*>( pModuleInfo );
                pItemRow->Add( ToString( pArmourInfo->GetHealth( pShip ) ) );
                pItemRow->Add( ToStringPercentage( pArmourInfo->GetKineticResistance() ) );
                pItemRow->Add( ToStringPercentage( pArmourInfo->GetEnergyResistance() ) );
            }
            else if ( moduleType == ModuleType::Shield )
            {
                ShieldInfo* pShieldInfo = static_cast<ShieldInfo*>( pModuleInfo );
                pItemRow->Add( ToString( pShieldInfo->GetCapacity() ) );
                pItemRow->Add( ToString( pShieldInfo->GetPeakRecharge() ) );
                pItemRow->Add( ToString( pShieldInfo->GetPowerGrid( pShip ) ) );
            }
            else if ( moduleType == ModuleType::Reactor )
            {
                ReactorInfo* pReactorInfo = static_cast<ReactorInfo*>( pModuleInfo );
                pItemRow->Add( ToString( pReactorInfo->GetCapacitorStorage() ) );
                pItemRow->Add( ToString( pReactorInfo->GetCapacitorRechargeRate() ) );
                pItemRow->Add( ToString( pReactorInfo->GetPowerGrid( pShip ) ) );
            }
            else if ( moduleType == ModuleType::Addon )
            {
                AddonInfo* pAddonInfo = static_cast<AddonInfo*>( pModuleInfo );
                pItemRow->Add( ToString( pAddonInfo->GetCategory() ) );
            }
            else if ( moduleType == ModuleType::Tower )
            {
                TowerInfo* pTowerInfo = static_cast<TowerInfo*>( pModuleInfo );
                pItemRow->Add( pTowerInfo->GetShortDescription() );
            }

            pItemRow->SetUserData( pModuleInfo );
            m_pTable[ (int)moduleType ]->AddRow( pItemRow );
        }
    }
}

void PanelShipyard::SetTitleFromGroup( ModuleType type )
{
    if ( type == ModuleType::Addon )
    {
        m_pTableTitle->SetText( "Shipyard > Inventory > Addons" );
    }
    else if ( type == ModuleType::Armour )
    {
        m_pTableTitle->SetText( "Shipyard > Inventory > Armour" );
    }
    else if ( type == ModuleType::Engine )
    {
        m_pTableTitle->SetText( "Shipyard > Inventory > Engines" );
    }
    else if ( type == ModuleType::Reactor )
    {
        m_pTableTitle->SetText( "Shipyard > Inventory > Reactors" );
    }
    else if ( type == ModuleType::Shield )
    {
        m_pTableTitle->SetText( "Shipyard > Inventory > Shield generators" );
    }
    else if ( type == ModuleType::Tower )
    {
        m_pTableTitle->SetText( "Shipyard > Inventory > Bridges" );
    }
    else if ( type == ModuleType::Weapon )
    {
        m_pTableTitle->SetText( "Shipyard > Inventory > Weapons" );
    }
}

void PanelShipyard::SetSelectedGroup( ModuleType moduleType )
{
    unsigned int newIdx = static_cast<unsigned int>( moduleType );
    unsigned int oldIdx = static_cast<unsigned int>( m_SelectedGroup );
    m_pTable[ oldIdx ]->Show( false );
    m_pTable[ newIdx ]->Show( true );

    m_pIcons[ oldIdx ]->SetColour( 1.0f, 1.0f, 1.0f, 0.4f );
    m_pIcons[ newIdx ]->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );

    m_SelectedGroup = moduleType;

    m_pScrollingElement->UpdateScrollingAreaHeight();
    m_pScrollingElement->Reset();

    SetTitleFromGroup( moduleType );
}

void PanelShipyard::UpdateModule( ModuleInfo* pModuleInfo )
{
    const size_t idx = static_cast<size_t>( pModuleInfo->GetType() );
    const size_t rowCount = m_pTable[ idx ]->GetRowCount();
    for ( size_t i = 0; i < rowCount; ++i )
    {
        TableRow* pRow = m_pTable[ idx ]->GetRow( i );
        if ( pRow->GetUserData() == pModuleInfo )
        {
            const InventoryItemMap& items = g_pGame->GetPlayer()->GetInventory()->GetItems();
            InventoryItemMap::const_iterator it = items.find( pModuleInfo->GetName() );
            if ( it != items.cend() )
            {
                static const unsigned int sQuantityIdx = 1; // Hardcoded value of the known position of where the item's "quantity" is in the row
                pRow->Set( sQuantityIdx, ToString( it->second.quantity ) );
                m_pTable[ idx ]->NotifyContentUpdated();
            }

            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// ButtonModule
///////////////////////////////////////////////////////////////////////////////

ButtonModule::ButtonModule( PanelShipyard* pOwner, ModuleType type )
    : m_pOwner( pOwner )
    , m_ModuleType( type )
{
}

void ButtonModule::OnPress()
{
    m_pOwner->SetSelectedGroup( m_ModuleType );
}

///////////////////////////////////////////////////////////////////////////////
// ButtonUndock
///////////////////////////////////////////////////////////////////////////////

ButtonUndock::ButtonUndock()
{
}

void ButtonUndock::OnPress()
{
    Shipyard* pShipyard = g_pGame->GetPlayer()->GetShip()->GetShipyard();
    if ( pShipyard != nullptr )
    {
        pShipyard->Undock();
    }
}

///////////////////////////////////////////////////////////////////////////////
// TableRowInventory
///////////////////////////////////////////////////////////////////////////////

TableRowInventory::TableRowInventory( PanelShipyard* pOwner, ModuleInfo* pModuleInfo )
    : m_pModuleInfo( pModuleInfo )
{
}

void TableRowInventory::OnPress()
{
    Inventory* pInventory = g_pGame->GetPlayer()->GetInventory();
    Shipyard* pShipyard = g_pGame->GetPlayer()->GetShip()->GetShipyard();
    SDL_assert( pShipyard != nullptr );

    if ( pInventory->GetModuleCount( m_pModuleInfo->GetName() ) > 0 && pShipyard->GetGrabbedModule() == nullptr )
    {
        pInventory->RemoveModule( m_pModuleInfo->GetName() );

        pShipyard->SetGrabbedModule( m_pModuleInfo );
    }
}

void TableRowInventory::OnHover()
{
    Ship* pShip = g_pGame->GetPlayer()->GetShip();
    Shipyard* pShipyard = pShip->GetShipyard();

    if ( pShipyard->GetGrabbedModule() == nullptr && pShip->GetDockingState() == DockingState::Docked )
    {
        pShipyard->SetModuleDetails( m_pModuleInfo );
    }
    else
    {
        pShipyard->SetModuleDetails( nullptr );
    }
}

} // namespace Hexterminate
