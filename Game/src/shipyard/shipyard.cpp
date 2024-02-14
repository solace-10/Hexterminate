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

#include <configuration.h>
#include <genesis.h>
#include <inputmanager.h>
#include <resources/resourcemodel.h>
#include <resources/resourcesound.h>
#include <shader.h>
#include <shaderuniform.h>

#include "hexterminate.h"
#include "menus/contextualtips.h"
#include "menus/eva.h"
#include "menus/moduledetails.h"
#include "menus/paneldocking.h"
#include "menus/panelshipstats.h"
#include "menus/panelshipyard.h"
#include "perks.h"
#include "player.h"
#include "sector/sector.h"
#include "ship/hexgrid.h"
#include "ship/hyperspacecore.h"
#include "ship/inventory.h"
#include "ship/module.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"
#include "shipyard/shipyard.h"

#ifdef _DEBUG
#include "menus/panelshipyarddebug.h"
#endif

namespace Hexterminate
{

Shipyard::Shipyard( const glm::vec3& spawnPosition )
    : m_pDockedShip( nullptr )
    , m_Position( spawnPosition )
    , m_SelectedX( -1 )
    , m_SelectedY( -1 )
    , m_MaxY( 0 )
    , m_BaseModelShowTimer( 0.0f )
    , m_pPanelDocking( new PanelDocking() )
    , m_pPanel( nullptr )
    , m_pPanelShipStats( nullptr )
    , m_pModuleDetails( nullptr )
    , m_pGrabbedModule( nullptr )
    , m_DockingRange( 200.0f )
    , m_MinConstructionX( 0 )
    , m_MaxConstructionX( sHexGridWidth )
    , m_MinConstructionY( 0 )
    , m_MaxConstructionY( sHexGridHeight )
    , m_pAssemblySFX( nullptr )
    , m_pDisassemblySFX( nullptr )
{
    using namespace Genesis;

#ifdef _DEBUG
    m_pPanelDebug = nullptr;
    m_DebugKeyPressedToken = InputManager::sInvalidInputCallbackToken;

    m_DebugKeyPressedToken = FrameWork::GetInputManager()->AddKeyboardCallback( std::bind( &Shipyard::ToggleDebugMode, this ), SDL_SCANCODE_F11, ButtonState::Pressed );
#endif

    InitialiseModels();

    SetConstructionDimensions();

    LoadSFX();

    m_pModuleDetails = new ModuleDetails();
}

Shipyard::~Shipyard()
{
#ifdef _DEBUG
    if ( Genesis::FrameWork::GetInputManager() != nullptr )
    {
        delete m_pPanelDebug;
        Genesis::FrameWork::GetInputManager()->RemoveKeyboardCallback( m_DebugKeyPressedToken );
    }
#endif

    delete m_pPanel;
    delete m_pPanelShipStats;
    delete m_pModuleDetails;
}

void Shipyard::LoadSFX()
{
    using namespace Genesis;
    using namespace std::literals;

    m_pAssemblySFX = (ResourceSound*)FrameWork::GetResourceManager()->GetResource( "data/sfx/shipyard_assembly.wav" );
    if ( m_pAssemblySFX != nullptr )
    {
        m_pAssemblySFX->Initialise( SOUND_FLAG_FX );
        m_pAssemblySFX->SetInstancingLimit( 100ms );
    }

    m_pDisassemblySFX = (ResourceSound*)FrameWork::GetResourceManager()->GetResource( "data/sfx/shipyard_disassembly.wav" );
    if ( m_pDisassemblySFX != nullptr )
    {
        m_pDisassemblySFX->Initialise( SOUND_FLAG_FX );
        m_pDisassemblySFX->SetInstancingLimit( 100ms );
    }
}

void Shipyard::PlaySFX( Genesis::ResourceSound* pSFX )
{
    if ( pSFX != nullptr )
    {
        Genesis::FrameWork::GetSoundManager()->CreateSoundInstance( pSFX, Genesis::Sound::SoundBus::Type::SFX, m_Position, 300.0f );
    }
}

void Shipyard::InitialiseModels()
{
    using namespace Genesis;

    m_pBaseModel = (ResourceModel*)FrameWork::GetResourceManager()->GetResource( "data/models/misc/dockhex/module.tmf" );
    SDL_assert_release( m_pBaseModel != nullptr );
    m_pBaseModel->SetFlipAxis( false );

    m_pShipyardModel = (ResourceModel*)FrameWork::GetResourceManager()->GetResource( "data/models/misc/shipyard/model.tmf" );
    SDL_assert_release( m_pShipyardModel != nullptr );
    m_pShipyardModel->SetFlipAxis( false );

    MaterialList& shipyardMaterials = m_pShipyardModel->GetMaterials();
    for ( Material* pMaterial : shipyardMaterials )
    {
        // Sets the ambient colour for all the shipyard's materials
        // This needs to match the behaviour of the ship's modules to achieve a consistent illumination across the scene.
        ShaderUniform* pAmbientUniform = pMaterial->shader->RegisterUniform( "k_a", ShaderUniformType::FloatVector4, false );
        if ( pAmbientUniform != nullptr )
        {
            const Background* pBackground = g_pGame->GetCurrentSector()->GetBackground();
            pAmbientUniform->Set( pBackground->GetAmbientColour() );
        }
    }
}

void Shipyard::SetConstructionDimensions()
{
    Perks* pPerks = g_pGame->GetPlayer()->GetPerks();
    if ( pPerks->IsEnabled( Perk::DreadnaughtConstruction ) )
    {
        m_MinConstructionX = 0;
        m_MaxConstructionX = 4;
        m_MinConstructionY = 0;
        m_MaxConstructionY = 18;
    }
    else if ( pPerks->IsEnabled( Perk::BattleshipConstruction ) )
    {
        m_MinConstructionX = 0;
        m_MaxConstructionX = 3;
        m_MinConstructionY = 0;
        m_MaxConstructionY = 18;
    }
    else if ( pPerks->IsEnabled( Perk::BattlecruiserConstruction ) )
    {
        m_MinConstructionX = 0;
        m_MaxConstructionX = 3;
        m_MinConstructionY = 2;
        m_MaxConstructionY = 15;
    }
    else if ( pPerks->IsEnabled( Perk::GunshipConstruction ) )
    {
        m_MinConstructionX = 1;
        m_MaxConstructionX = 3;
        m_MinConstructionY = 3;
        m_MaxConstructionY = 14;
    }

    SDL_assert( m_MaxConstructionX <= sHexGridWidth );
    SDL_assert( m_MaxConstructionY <= sHexGridHeight );
}

bool Shipyard::CanBeUsed() const
{
    Ship* pPlayerShip = g_pGame->GetPlayer()->GetShip();
    Sector* pCurrentSector = g_pGame->GetCurrentSector();
    return ( pPlayerShip != nullptr && m_pDockedShip == nullptr && pCurrentSector->IsPlayerVictorious() && glm::distance( pPlayerShip->GetTowerPosition(), GetPosition() ) < GetDockingRange() && pPlayerShip->GetHyperspaceCore()->IsCharging() == false && pPlayerShip->GetHyperspaceCore()->IsJumping() == false );
}

void Shipyard::Update( float delta )
{
    UpdateSelection();
    UpdateInput();

    if ( m_pDockedShip != nullptr && m_pDockedShip->GetDockingState() == DockingState::Docked && m_pDockedShip->IsModuleEditLocked() )
    {
        m_BaseModelShowTimer += delta;
        if ( m_BaseModelShowTimer > 0.1f && m_MaxY < m_MaxConstructionY )
        {
            m_MaxY++;
            m_BaseModelShowTimer = 0.0f;
        }
    }
    else
    {
        m_MaxY = 0;
        m_BaseModelShowTimer = 0.0f;
    }

    // The IsShipCaptureModeActive() is here to make sure that the docking text isn't visible even if the shipyard can be used,
    // so that screenshots can be taken immediately after leaving the shipyard without having to move.
    m_pPanelDocking->Show( CanBeUsed() && !g_pGame->IsShipCaptureModeActive() );

    if ( m_pModuleDetails != nullptr )
    {
        m_pModuleDetails->Update( delta );
    }
}

void Shipyard::UpdateSelection()
{
    m_SelectedX = m_SelectedY = -1;

    if ( m_pDockedShip == nullptr || m_pDockedShip->GetDockingState() != DockingState::Docked || m_pDockedShip->IsModuleEditLocked() == false )
        return;

    const float sphereRadius = sModuleWidth / 2.0f;

    const glm::vec2& mousePosition = Genesis::FrameWork::GetInputManager()->GetMousePosition();
    const glm::vec3 selectedPosition = Genesis::FrameWork::GetRenderSystem()->Raycast( mousePosition );

    for ( unsigned int y = m_MinConstructionY; y < m_MaxConstructionY; ++y )
    {
        for ( unsigned int x = m_MinConstructionX; x < m_MaxConstructionX; ++x )
        {
            if ( y % 2 && x == m_MaxConstructionX - 1 )
                continue;

            glm::vec3 modulePos = Module::GetLocalPosition( m_pDockedShip, x, y ) + m_Position;
            if ( glm::distance( selectedPosition, modulePos ) < sphereRadius )
            {
                m_SelectedX = x;
                m_SelectedY = y;
                break;
            }
        }
    }
}

void Shipyard::UpdateInput()
{
    if ( m_pDockedShip == nullptr || m_pDockedShip->GetDockingState() != DockingState::Docked || m_pDockedShip->IsModuleEditLocked() == false )
        return;

    bool pressed = Genesis::FrameWork::GetInputManager()->IsMouseButtonPressed( Genesis::MouseButton::Left );
    bool pressedRight = Genesis::FrameWork::GetInputManager()->IsMouseButtonPressed( Genesis::MouseButton::Right );

    const ModuleHexGrid& hexGrid = m_pDockedShip->GetModuleHexGrid();

    // Right clicking on a module removes it outright
    if ( pressedRight && m_SelectedX != -1 && m_SelectedY != -1 && GetGrabbedModule() == nullptr )
    {
        Module* pModule = hexGrid.Get( m_SelectedX, m_SelectedY );
        if ( pModule != nullptr )
        {
            ModuleInfo* pModuleInfo = pModule->GetModuleInfo();
            m_pDockedShip->RemoveModule( m_SelectedX, m_SelectedY );
            m_pPanelShipStats->OnShipConfigurationChanged();
            g_pGame->GetPlayer()->GetInventory()->AddModule( pModuleInfo->GetName() );
            m_pPanel->UpdateModule( pModuleInfo );
            PlaySFX( m_pDisassemblySFX );
        }
    }
    // Grabbed a module from the ship area
    else if ( pressed && m_SelectedX != -1 && m_SelectedY != -1 && GetGrabbedModule() == nullptr )
    {
        Module* pModule = hexGrid.Get( m_SelectedX, m_SelectedY );
        if ( pModule != nullptr )
        {
            SetGrabbedModule( pModule->GetModuleInfo() );
            m_pDockedShip->RemoveModule( m_SelectedX, m_SelectedY );
            m_pPanelShipStats->OnShipConfigurationChanged();
            PlaySFX( m_pDisassemblySFX );
        }
    }
    // Dropped a module on the ship area
    else if ( !pressed && m_SelectedX != -1 && m_SelectedY != -1 && GetGrabbedModule() != nullptr )
    {
        if ( hexGrid.Get( m_SelectedX, m_SelectedY ) == nullptr )
        {
            Module* pModule = m_pDockedShip->AddModule( GetGrabbedModule(), m_SelectedX, m_SelectedY );
            if ( pModule != nullptr )
            {
                m_pPanelShipStats->OnShipConfigurationChanged();
                PlaySFX( m_pAssemblySFX );
                pModule->TriggerAssemblyEffect();
            }
        }
        else
        {
            m_pPanelShipStats->OnShipConfigurationChanged();
            g_pGame->GetPlayer()->GetInventory()->AddModule( GetGrabbedModule()->GetName() );
        }
        SetGrabbedModule( nullptr );
    }
    // Dropped the module outside of the ship area
    else if ( !pressed && GetGrabbedModule() != nullptr )
    {
        g_pGame->GetPlayer()->GetInventory()->AddModule( GetGrabbedModule()->GetName() );
        m_pPanelShipStats->OnShipConfigurationChanged();
        SetGrabbedModule( nullptr );
    }
    // Player is holding a module
    else if ( m_pGrabbedModule != nullptr )
    {
        m_pModuleDetails->SetModuleInfo( m_pGrabbedModule, true );
    }
    // Hovering the mouse on the ship
    else if ( m_SelectedX != -1 && m_SelectedY != -1 )
    {
        Module* pModule = hexGrid.Get( m_SelectedX, m_SelectedY );
        if ( pModule != nullptr )
        {
            m_pModuleDetails->SetModuleInfo( pModule->GetModuleInfo(), false );
        }
        else
        {
            m_pModuleDetails->SetModuleInfo( nullptr );
        }
    }
    // Cursor not on the ship and the player isn't holding a module
    else if ( m_pGrabbedModule == nullptr )
    {
        m_pModuleDetails->SetModuleInfo( nullptr );
    }
}

void Shipyard::Render()
{
    using namespace Genesis;

    glEnable( GL_DEPTH_TEST );

    m_pShipyardModel->Render( glm::translate( m_Position ) );

    glDisable( GL_DEPTH_TEST );

    RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
    pRenderSystem->SetBlendMode( BlendMode::Blend );

    for ( unsigned int y = m_MinConstructionY; y < m_MaxY; ++y )
    {
        for ( unsigned int x = m_MinConstructionX; x < m_MaxConstructionX; ++x )
        {
            if ( y % 2 && x == m_MaxConstructionX - 1 )
            {
                continue;
            }

            if ( m_pDockedShip && m_pDockedShip->GetModuleHexGrid().Get( x, y ) )
            {
                continue;
            }

            float offsetX = static_cast<float>( x ) * sModuleHorizontalSpacing;
            if ( y % 2 == 1 )
            {
                offsetX += sModuleHorizontalSpacing / 2.0f;
            }

            glm::vec3 translation = glm::vec3( offsetX, static_cast<float>( y * sModuleHalfHeight ), 0.0f ) - m_pDockedShip->GetCentreOfMass();
            const glm::mat4 modelMatrix = glm::translate( translation );
            m_pBaseModel->Render( modelMatrix );
            if ( x == static_cast<unsigned int>( m_SelectedX ) && y == static_cast<unsigned int>( m_SelectedY ) )
            {
                m_pBaseModel->Render( modelMatrix );
            }
        }
    }

    pRenderSystem->SetBlendMode( BlendMode::Disabled );
}

bool Shipyard::Dock( Ship* pShip )
{
    if ( CanBeUsed() == false )
        return false;

    m_pDockedShip = pShip;
    pShip->Dock( this );

    if ( m_pPanel == nullptr )
    {
        m_pPanel = new PanelShipyard();
    }

    if ( m_pPanelShipStats == nullptr )
    {
        m_pPanelShipStats = new PanelShipStats();
    }

    return true;
}

bool Shipyard::Undock()
{
    if ( m_pDockedShip != nullptr && m_pDockedShip->GetDockingState() == DockingState::Docked )
    {
        if ( ValidateDockedShip() == false )
        {
            return false;
        }

        if ( GetGrabbedModule() != nullptr )
        {
            g_pGame->GetPlayer()->GetInventory()->AddModule( GetGrabbedModule()->GetName() );
            SetGrabbedModule( nullptr );
        }
        else
        {
            // Make sure that the module details is set to null, otherwise if the player is hovering above
            // a module the details will remain visible after undocking.
            m_pModuleDetails->SetModuleInfo( nullptr );
        }

        StoreHexGrid();

        m_pDockedShip->Undock();
        m_pDockedShip = nullptr;

        delete m_pPanel;
        m_pPanel = nullptr;

        delete m_pPanelShipStats;
        m_pPanelShipStats = nullptr;

#ifdef _DEBUG
        delete m_pPanelDebug;
        m_pPanelDebug = nullptr;
#endif

        m_BaseModelShowTimer = 0.0f;
        m_MaxY = 0;

        return true;
    }

    return false;
}

bool Shipyard::ValidateDockedShip()
{
    if ( m_pDockedShip == nullptr )
    {
        return false;
    }
    else
    {
        int numAddons = 0;
        int numTowers = 0;
        int numEngines = 0;
        int numReactors = 0;
        float powerGrid = 0.0f;

        int x1, y1, x2, y2;
        const ModuleHexGrid& moduleHexGrid = m_pDockedShip->GetModuleHexGrid();
        moduleHexGrid.GetBoundingBox( x1, y1, x2, y2 );

        glm::vec3 moduleLocalPos( 0.0f );
        for ( int x = x1; x <= x2; ++x )
        {
            for ( int y = y1; y <= y2; ++y )
            {
                Module* pModule = moduleHexGrid.Get( x, y );
                if ( pModule != nullptr )
                {
                    ModuleType type = pModule->GetModuleInfo()->GetType();
                    if ( type == ModuleType::Tower )
                    {
                        numTowers++;
                    }
                    else if ( type == ModuleType::Engine )
                    {
                        numEngines++;
                    }
                    else if ( type == ModuleType::Reactor )
                    {
                        numReactors++;

                        // ReactorInfo* pReactorInfo = static_cast<ReactorInfo*>( pModule->GetModuleInfo() );
                        //energyGenerated += pReactorInfo->GetCapacitorRechargeRate();
                    }
                    else if ( type == ModuleType::Addon )
                    {
                        numAddons++;
                    }

                    powerGrid += pModule->GetModuleInfo()->GetPowerGrid( m_pDockedShip );
                }
            }
        }

        if ( numTowers < 1 )
        {
            g_pGame->AddFleetCommandIntel( "The ship needs to have a Command Bridge before it can undock, Captain." );
            return false;
        }
        else if ( numTowers > 1 )
        {
            g_pGame->AddFleetCommandIntel( "The ship can only have one Command Bridge, Captain." );
            return false;
        }
        else if ( numEngines == 0 )
        {
            g_pGame->AddFleetCommandIntel( "The ship needs to have at least one engine before it can undock." );
            return false;
        }
        else if ( numReactors == 0 )
        {
            g_pGame->AddFleetCommandIntel( "The ship is unpowered and needs at least one reactor." );
            return false;
        }
        else if ( powerGrid < 0.0f )
        {
            g_pGame->AddFleetCommandIntel( "The ship is using more of the power grid than it can handle, Captain." );
            return false;
        }
        else if ( numAddons > 6 )
        {
            g_pGame->AddFleetCommandIntel( "Our ships can only support a maximum of 6 addon modules. You'll have to remove some before undocking." );
            return false;
        }
    }

    return true;
}

void Shipyard::StoreHexGrid()
{
    // After the ship is modified in the shipyard, we need to make sure that the inventory's hexgrid actually reflects these changes,
    // otherwise they won't be saved.
    if ( m_pDockedShip == g_pGame->GetPlayer()->GetShip() )
    {
        Inventory* pInventory = g_pGame->GetPlayer()->GetInventory();
        for ( int x = 0; x < sHexGridWidth; ++x )
        {
            for ( int y = 0; y < sHexGridHeight; ++y )
            {
                Module* pModule = m_pDockedShip->GetModuleHexGrid().Get( x, y );
                if ( pModule == nullptr )
                {
                    pInventory->GetHexGrid()->Set( x, y, nullptr );
                }
                else
                {
                    pInventory->GetHexGrid()->Set( x, y, pModule->GetModuleInfo() );
                }
            }
        }
    }
}

void Shipyard::SetGrabbedModule( ModuleInfo* pModuleInfo )
{
    if ( pModuleInfo != nullptr )
    {
        m_pPanel->UpdateModule( pModuleInfo );
        m_pGrabbedModule = pModuleInfo;
        m_pModuleDetails->SetModuleInfo( pModuleInfo, true );

        ProvideContextualTip();
    }
    else
    {
        if ( m_pGrabbedModule != nullptr )
        {
            m_pPanel->UpdateModule( m_pGrabbedModule );
        }

        m_pModuleDetails->SetModuleInfo( nullptr );
        m_pGrabbedModule = nullptr;
    }
}

void Shipyard::ProvideContextualTip()
{
    if ( m_pGrabbedModule != nullptr )
    {
        const std::string& contextualTipTag = m_pGrabbedModule->GetContextualTip();
        if ( contextualTipTag.empty() == false )
        {
            ContextualTips::Present( contextualTipTag );
        }
    }
}

void Shipyard::SetModuleDetails( ModuleInfo* pModuleInfo )
{
    m_pModuleDetails->SetModuleInfo( pModuleInfo );
}

#ifdef _DEBUG

void Shipyard::LoadFromFile( const std::string& filename )
{
    if ( m_pDockedShip == nullptr )
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Attempting to load file with no ship docked" );
        return;
    }

    // Clear the current ship
    m_pDockedShip->ClearModules();

    // Load from file and create the modules in the correct places
    ModuleInfoManager* pModuleInfoManager = g_pGame->GetModuleInfoManager();
    std::ifstream fs( filename );
    if ( fs.is_open() )
    {
        int x, y;
        std::string moduleName;
        while ( fs.good() )
        {
            if ( fs >> x >> y >> moduleName )
            {
                ModuleInfo* pModuleInfo = pModuleInfoManager->GetModuleByName( moduleName );
                if ( pModuleInfo != nullptr )
                {
                    m_pDockedShip->AddModule( pModuleInfo, x, y );
                }
            }
            else
            {
                break;
            }
        }
        fs.close();

        m_pPanelShipStats->OnShipConfigurationChanged();
    }
    else
    {
        Genesis::FrameWork::GetLogger()->LogWarning( "Could not load HexGrid template from '%s'", filename.c_str() );
    }
}

void Shipyard::SaveToFile( const std::string& filename )
{
    Genesis::Logger* pLogger = Genesis::FrameWork::GetLogger();
    if ( m_pDockedShip == nullptr )
    {
        pLogger->LogWarning( "Attempting to save file with no ship docked" );
        return;
    }
    else if ( filename == "" )
    {
        pLogger->LogWarning( "No file loaded to save to" );
        return;
    }

    int points = ShipInfo::sCalculatePoints( m_pDockedShip->GetModuleHexGrid().GetUsedSlots() );
    ShipType type = ShipInfo::sCalculateShipType( m_pDockedShip->GetModuleHexGrid().GetUsedSlots() );
    pLogger->LogInfo( "Storing '%s'", filename.c_str() );
    pLogger->LogInfo( "- Points: %d", points );
    if ( type == ShipType::Gunship )
        pLogger->LogInfo( "Type: Gunship" );
    else if ( type == ShipType::Battlecruiser )
        pLogger->LogInfo( "Type: Battlecruiser" );
    else if ( type == ShipType::Capital )
        pLogger->LogInfo( "Type: Capital" );

    std::ofstream fs( filename, std::ios::out | std::ios::trunc );
    if ( fs.is_open() )
    {
        int x1, y1, x2, y2;
        m_pDockedShip->GetModuleHexGrid().GetBoundingBox( x1, y1, x2, y2 );
        if ( m_pDockedShip->GetModuleHexGrid().GetUsedSlots() > 0 )
        {
            for ( int x = x1; x <= x2; ++x )
            {
                for ( int y = y1; y <= y2; ++y )
                {
                    Module* pModule = m_pDockedShip->GetModuleHexGrid().Get( x, y );
                    if ( pModule != nullptr )
                    {
                        fs << x << " " << y << " " << pModule->GetModuleInfo()->GetName() << std::endl;
                    }
                }
            }
        }

        fs.close();
    }
}

void Shipyard::ToggleDebugMode()
{
    if ( m_pDockedShip == nullptr )
        return;

    if ( m_pPanelDebug == nullptr )
    {
        m_pPanelDebug = new PanelShipyardDebug();

        // Give the player 99 of each item
        ModuleInfoManager* pModuleInfoManager = g_pGame->GetModuleInfoManager();
        ModuleInfoVector moduleInfos = pModuleInfoManager->GetAllModules();
        Inventory* pInventory = g_pGame->GetPlayer()->GetInventory();
        for ( auto& pModuleInfo : moduleInfos )
        {
            pInventory->AddModule( pModuleInfo->GetName(), 99 );
        }

        // Enable the ship construction perks so we aren't restricted to gunship-sized ships
        Perks* pPerks = g_pGame->GetPlayer()->GetPerks();
        pPerks->Enable( Perk::BattlecruiserConstruction );
        pPerks->Enable( Perk::BattleshipConstruction );
        pPerks->Enable( Perk::DreadnaughtConstruction );

        SetConstructionDimensions();

        delete m_pPanel;
        m_pPanel = new PanelShipyard();
    }
    else
    {
        delete m_pPanelDebug;
        m_pPanelDebug = nullptr;
    }
}

#endif

} // namespace Hexterminate