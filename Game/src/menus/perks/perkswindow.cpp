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

#include "menus/perks/perkswindow.h"

#include <configuration.h>
#include <genesis.h>

#include "hexterminate.h"
#include "menus/buttonevent.h"
#include "menus/eva.h"
#include "misc/gui.h"
#include "player.h"

namespace Hexterminate
{

PerksWindow::PerksWindow()
    : m_pMainPanel( nullptr )
    , m_pButtonClose( nullptr )
    , m_pTitle( nullptr )
    , m_pScrollingElement( nullptr )
{
}

PerksWindow::~PerksWindow()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr && m_pMainPanel != nullptr )
    {
        pGuiManager->RemoveElement( m_pMainPanel );
        m_pMainPanel = nullptr;
    }
}

void PerksWindow::Init()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();

    const int panelWidth = 800;
    const int panelHeight = 600;

    m_pMainPanel = new Genesis::Gui::Panel();
    m_pMainPanel->SetSize( panelWidth, panelHeight );
    m_pMainPanel->SetColour( EVA_COLOUR_BACKGROUND );
    m_pMainPanel->SetBorderColour( EVA_COLOUR_BORDER );
    m_pMainPanel->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    m_pMainPanel->SetPosition(
        (int)( ( Genesis::Configuration::GetScreenWidth() - panelWidth ) / 2.0f ),
        (int)( ( Genesis::Configuration::GetScreenHeight() - panelHeight ) / 2.0f ) );
    m_pMainPanel->Show( false );
    pGuiManager->AddElement( m_pMainPanel );

    m_pButtonClose = new ButtonEvent( this, Genesis::Gui::GuiEvent::Close );
    m_pButtonClose->SetPosition( panelWidth - 32, 0 );
    m_pButtonClose->SetSize( 32, 32 );
    m_pButtonClose->SetColour( 0.0f, 0.0f, 0.0f, 0.0f );
    m_pButtonClose->SetHoverColour( 0.0f, 0.0f, 0.0f, 0.0f );
    m_pButtonClose->SetIconColour( EVA_BUTTON_ICON_COLOUR );
    m_pButtonClose->SetIconHoverColour( EVA_BUTTON_ICON_COLOUR_HOVER );
    m_pButtonClose->SetIcon( "data/ui/icons/close.png" );
    m_pButtonClose->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    m_pMainPanel->AddElement( m_pButtonClose );

    m_pTitle = GuiExtended::CreateText( 8, 8, 256, 32, "> Perks", m_pMainPanel );

    m_pScrollingElement = new Genesis::Gui::ScrollingElement();
    m_pScrollingElement->SetPosition( 0, 36 );
    m_pScrollingElement->SetSize( panelWidth - 4, panelHeight - 40 );
    m_pScrollingElement->Init();
    m_pMainPanel->AddElement( m_pScrollingElement );

    InitCategories();
}

void PerksWindow::InitCategories()
{
    static const int sTier1PerkCost = 2;
    static const int sTier2PerkCost = 5;
    static const int sTier3PerkCost = 10;
    static const int sTier4PerkCost = 20;

    PerkCategoryPanel* pShipyard = AddCategory( "Shipyard", "data/ui/perks/shipyard.png" );
    pShipyard->AddPerk( Perk::GunshipConstruction, "Gunships", "You are capable of building and commanding gunships.", "data/ui/icons/shipyard.png", sTier1PerkCost );
    pShipyard->AddPerk( Perk::BattlecruiserConstruction, "Battlecruisers", "Gunships can only take you so far. The additional space of a battlecruiser lets you pack several more modules at the shipyard.", "data/ui/icons/shipyard.png", sTier2PerkCost );
    pShipyard->AddPerk( Perk::BattleshipConstruction, "Battleships", "Battleships are the workhorse of the Empire, usually ponderous but carrying a frightening amount of firepower. Your shipyard space is expanded further.", "data/ui/icons/shipyard.png", sTier3PerkCost );
    pShipyard->AddPerk( Perk::DreadnaughtConstruction, "Dreadnaughts", "The ability to create the largest ships in the galaxy is now yours to command.", "data/ui/icons/shipyard.png", sTier4PerkCost );

    PerkCategoryPanel* pArmour = AddCategory( "Armour", "data/ui/perks/armour.png" );
    pArmour->AddPerk( Perk::LighterMaterials, "Lighter materials", "Decrease armour weight by 20%.", "data/ui/icons/armour.png", sTier1PerkCost );
    pArmour->AddPerk( Perk::Nanobots, "Nanobots", "Armour modules slowly get repaired over time.", "data/ui/icons/armour.png", sTier2PerkCost );
    pArmour->AddPerk( Perk::ReinforcedBulkheads, "Reinforced bulkheads", "All modules have an increased 1000hp.", "data/ui/icons/armour.png", sTier3PerkCost );
    pArmour->AddPerk( Perk::Unbroken, "Unbroken", "When an armour module is about to be destroyed, it becomes invulnerable for 10s. This effect can only happen once every 30 seconds.", "data/ui/icons/armour.png", sTier4PerkCost );

    PerkCategoryPanel* pEmpire = AddCategory( "Empire", "data/ui/perks/empire.png" );
    pEmpire->AddPerk( Perk::ReclaimedSectors, "Reclaimed sectors", "Any sector you capture immediately gets a strengthened regional fleet.", "data/ui/icons/empire.png", sTier1PerkCost );
    pEmpire->AddPerk( Perk::SharedGlory, "Shared glory", "Gain influence from sectors captured by allied fleets.", "data/ui/icons/empire.png", sTier2PerkCost );
    pEmpire->AddPerk( Perk::SwordOfTheEmpire, "Sword of the Empire", "The Empire prioritises reinforcements towards your fleet, assisting you with on-going battles.", "data/ui/icons/empire.png", sTier3PerkCost );
    pEmpire->AddPerk( Perk::UnityIsStrength, "Unity is Strength", "Imperial sectors increase your influence, further fueling your expansion.", "data/ui/icons/empire.png", sTier4PerkCost );

    PerkCategoryPanel* pEnergyWeapons = AddCategory( "Energy weapons", "data/ui/perks/energy.png" );
    pEnergyWeapons->AddPerk( Perk::AdvancedHeatsinks, "Advanced heatsinks", "+15% rate of fire and -20% energy used.", "data/ui/icons/energy.png", sTier1PerkCost );
    pEnergyWeapons->AddPerk( Perk::PhaseSynchronisation, "Phase synchronisation", "+15% damage to enemy shields.", "data/ui/icons/energy.png", sTier2PerkCost );
    pEnergyWeapons->AddPerk( Perk::Disruption, "Disruption", "An enemy module hit by an energy weapon has a chance of being disabled temporarily (EMP).", "data/ui/icons/energy.png", sTier3PerkCost );
    pEnergyWeapons->AddPerk( Perk::Overload, "Overload", "Ion cannons have a chance of temporarily disabling enemy shields.", "data/ui/icons/energy.png", sTier4PerkCost );

    PerkCategoryPanel* pFleet = AddCategory( "Fleet", "data/ui/perks/fleet.png" );
    pFleet->AddPerk( Perk::SupportRequest, "Support request", "The maximum size of your fleet is increased to 4 ships.", "data/ui/icons/fleet.png", sTier1PerkCost );
    pFleet->AddPerk( Perk::PrimaryFleet, "Primary fleet", "The maximum size of your fleet is increased to 5 ships.", "data/ui/icons/fleet.png", sTier2PerkCost );
    pFleet->AddPerk( Perk::PriorityRequisition, "Priority requisition", "Tier II ships are now available for requisition.", "data/ui/icons/fleet.png", sTier3PerkCost );
    pFleet->AddPerk( Perk::PrototypeAccess, "Prototype access", "Tier III ships can now be requisitioned, giving you access to the most advanced ships the Empire is capable of producing.", "data/ui/icons/fleet.png", sTier4PerkCost );

    PerkCategoryPanel* pKineticWeapons = AddCategory( "Kinetic weapons", "data/ui/perks/kinetic.png" );
    pKineticWeapons->AddPerk( Perk::MagneticLoaders, "Magnetic loaders", "Experimental magnetic autoloaders grant you an additional 15% rate of fire.", "data/ui/icons/kinetic.png", sTier1PerkCost );
    pKineticWeapons->AddPerk( Perk::AdvancedElectrocoils, "Adv. electrocoils", "Advanced electrocoils give your weapons a 20% increased range.", "data/ui/icons/kinetic.png", sTier2PerkCost );
    pKineticWeapons->AddPerk( Perk::PlasmaWarheads, "Plasma warheads", "Enemy modules taking damage repair at 30% efficiency.", "data/ui/icons/kinetic.png", sTier3PerkCost );
    pKineticWeapons->AddPerk( Perk::Siegebreaker, "Siegebreaker", "Each projectile that hits an enemy increases the rate of fire for that weapon. Missing clears all of the weapon's accumulated stacks.", "data/ui/icons/kinetic.png", sTier4PerkCost );

    PerkCategoryPanel* pNavigation = AddCategory( "Navigation", "data/ui/perks/navigation.png" );
    pNavigation->AddPerk( Perk::EngineRemapping, "Engine remapping", "Fleet engineers remap the engine modules of your flagship, increasing its top speed by 10%.", "data/ui/icons/navigation.png", sTier1PerkCost );
    pNavigation->AddPerk( Perk::EvasionProtocols, "Evasion protocols", "Your ship can now use its sideway thrusters to attempt to dodge incoming projectiles. Double tap 'Q' or 'E' for this.", "data/ui/icons/navigation.png", sTier2PerkCost );
    pNavigation->AddPerk( Perk::ThrustVectoring, "Thrust vectoring", "Your flagship strafes and reverses at 60% of forward thrust (up from 40%).", "data/ui/icons/navigation.png", sTier3PerkCost );
    pNavigation->AddPerk( Perk::RammingSpeed, "Ramming speed", "Your flagship will now deal damage on collision when using the ramming speed ability.", "data/ui/icons/navigation.png", sTier4PerkCost );

    PerkCategoryPanel* pShield = AddCategory( "Shield", "data/ui/perks/shield.png" );
    pShield->AddPerk( Perk::FrequencyCycling, "Frequency cycling", "Your shield takes -15% damage from Energy weapons.", "data/ui/icons/shield.png", sTier1PerkCost );
    pShield->AddPerk( Perk::KineticHardening, "Kinetic hardening", "Your shield takes -15% damage from Kinetic weapons.", "data/ui/icons/shield.png", sTier2PerkCost );
    pShield->AddPerk( Perk::Superconductors, "Superconductors", "Shields modules require 30% less energy to fit.", "data/ui/icons/shield.png", sTier3PerkCost );
    pShield->AddPerk( Perk::EmergencyCapacitors, "Emergency capacitors", "If your shield is taken offline, it is immediately restored with 60% capacity. This effect can only happen once every 120 seconds.", "data/ui/icons/shield.png", sTier4PerkCost );
}

PerkCategoryPanel* PerksWindow::AddCategory( const std::string& category, const std::string& backgroundImageFilename )
{
    using namespace Genesis;

    ResourceImage* pBackgroundImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( backgroundImageFilename );

    const int panelHeight = 144;
    PerkCategoryPanel* pPanel = new PerkCategoryPanel();
    pPanel->SetSize( m_pScrollingElement->GetWidth() - m_pScrollingElement->GetScrollbarWidth() - 8, panelHeight );
    pPanel->SetColour( 1.0f, 1.0f, 1.0f, 1.0f );
    pPanel->SetBorderColour( EVA_COLOUR_BORDER );
    pPanel->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    pPanel->SetPosition( 4, static_cast<int>( m_Categories.size() ) * ( panelHeight + 4 ) );
    pPanel->SetTexture( pBackgroundImage );

    pPanel->Init( category );

    m_pScrollingElement->AddElement( pPanel );

    m_Categories.push_back( pPanel );

    return pPanel;
}

void PerksWindow::Show( bool state )
{
    if ( m_pMainPanel != nullptr )
    {
        m_pMainPanel->Show( state );
        g_pGame->SetInputBlocked( state );

        for ( PerkCategoryPanel* pPanel : m_Categories )
        {
            pPanel->Show( state );
        }
    }

    if ( state )
    {
        GameEventManager::Broadcast( new GameEvent( GameEventType::PerkAcquired ) );
        m_pScrollingElement->UpdateScrollingAreaHeight();
        g_pGame->Pause();
    }
    else
    {
        g_pGame->Unpause();
    }
}

bool PerksWindow::IsVisible() const
{
    return m_pMainPanel ? m_pMainPanel->IsVisible() : false;
}

bool PerksWindow::HandleEvent( Genesis::Gui::GuiEvent event )
{
    using namespace Genesis::Gui;
    if ( event == GuiEvent::Close )
    {
        Show( false );
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace Hexterminate
