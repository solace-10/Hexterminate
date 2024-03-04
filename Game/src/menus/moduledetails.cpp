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

#include "menus/moduledetails.h"
#include "globals.h"
#include "hexterminate.h"
#include "math/misc.h"
#include "menus/eva.h"
#include "player.h"
#include "ship/moduleinfo.h"
#include "stringaux.h"
#include <genesis.h>
#include <sstream>

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// ModuleDetails
///////////////////////////////////////////////////////////////////////////////

ModuleDetails::ModuleDetails()
    : m_pModuleInfo( nullptr )
    , m_pMainPanel( nullptr )
    , m_pIcon( nullptr )
    , m_pTitleText( nullptr )
    , m_pContentsText( nullptr )
    , m_pIconOnly( nullptr )
    , m_InIconOnlyMode( false )
    , m_HideNextFrame( false )
{
    CreateElements();
}

ModuleDetails::~ModuleDetails()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr )
    {
        pGuiManager->RemoveElement( m_pMainPanel );
        pGuiManager->RemoveElement( m_pIconOnly );
    }
}

void ModuleDetails::Update( float delta )
{
    PlaceAtCursor();

    if ( m_HideNextFrame > 2 )
    {
        m_pMainPanel->Show( false );
        m_pIconOnly->Show( false );
    }
}

void ModuleDetails::SetModuleInfo( ModuleInfo* pModuleInfo, bool iconOnly /* = false */ )
{
    if ( m_pModuleInfo != pModuleInfo || m_InIconOnlyMode != iconOnly )
    {
        m_pModuleInfo = pModuleInfo;

        if ( m_pModuleInfo != nullptr )
        {
            if ( iconOnly )
            {
                m_pMainPanel->Show( false );
                m_pIconOnly->Show( true );
                m_InIconOnlyMode = true;

                SetIcon();
            }
            else
            {
                m_pMainPanel->Show( true );
                m_pIconOnly->Show( false );
                m_InIconOnlyMode = false;

                SetTitle();
                SetIcon();
                SetContents();
            }

            m_HideNextFrame = 0;
        }
    }

    if ( m_pModuleInfo == nullptr )
    {
        m_HideNextFrame++;
    }
}

void ModuleDetails::PlaceAtCursor()
{
    // Positions are slightly offset so they don't overlap the cursor.
    if ( m_pMainPanel->IsVisible() )
    {
        const glm::vec2& mousePosition = Genesis::FrameWork::GetInputManager()->GetMousePosition();
        m_pMainPanel->SetPosition( mousePosition.x + 8.0f, mousePosition.y + 8.0f );
    }
    else if ( m_pIconOnly->IsVisible() )
    {
        const glm::vec2& mousePosition = Genesis::FrameWork::GetInputManager()->GetMousePosition();
        m_pIconOnly->SetPosition( mousePosition.x + 8.0f, mousePosition.y + 4.0f );
    }
}

void ModuleDetails::CreateElements()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();

    m_pMainPanel = new Genesis::Gui::Panel();
    m_pMainPanel->SetSize( 320.0f, 68.0f );
    m_pMainPanel->SetPosition( 0.0f, 0.0f );
    m_pMainPanel->SetColor( 0.0f, 0.0f, 0.0f, 0.8f );
    m_pMainPanel->SetBorderColor( EVA_COLOR_BORDER );
    m_pMainPanel->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    m_pMainPanel->SetDepth( 10 );
    pGuiManager->AddElement( m_pMainPanel );

    m_pTitleText = new Genesis::Gui::Text();
    m_pTitleText->SetSize( 256.0f, 16.0f );
    m_pTitleText->SetPosition( 80.0f, 8.0f );
    m_pTitleText->SetColor( EVA_TEXT_COLOR );
    m_pTitleText->SetFont( EVA_FONT );
    m_pTitleText->SetMultiLine( false );
    m_pTitleText->SetText( "" );
    m_pMainPanel->AddElement( m_pTitleText );

    m_pContentsText = new Genesis::Gui::Text();
    m_pContentsText->SetSize( 256.0f, 32.0f );
    m_pContentsText->SetPosition( 80.0f, 28.0f );
    m_pContentsText->SetColor( EVA_TEXT_COLOR );
    m_pContentsText->SetFont( EVA_FONT );
    m_pContentsText->SetText( "" );
    m_pMainPanel->AddElement( m_pContentsText );

    m_pIcon = new Genesis::Gui::Image();
    m_pIcon->SetSize( 64.0f, 64.0f );
    m_pIcon->SetPosition( 8.0f, 8.0f );
    m_pIcon->SetColor( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pMainPanel->AddElement( m_pIcon );

    m_pIconOnly = new Genesis::Gui::Image();
    m_pIconOnly->SetSize( 64.0f, 64.0f );
    m_pIconOnly->SetPosition( 8.0f, 8.0f );
    m_pIconOnly->SetColor( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pIconOnly->SetDepth( 10 );
    pGuiManager->AddElement( m_pIconOnly );

    m_pMainPanel->Show( false );
    m_pIconOnly->Show( false );
}

void ModuleDetails::SetTitle()
{
    m_pTitleText->SetText( m_pModuleInfo->GetFullName() );
    m_pTitleText->SetColor( ModuleRarityToColor( m_pModuleInfo->GetRarity() ) );
}

void ModuleDetails::SetIcon()
{
    using namespace Genesis;
    ResourceImage* pIconImage = (ResourceImage*)FrameWork::GetResourceManager()->GetResource( m_pModuleInfo->GetIcon() );

    if ( m_InIconOnlyMode )
    {
        m_pIconOnly->SetTexture( pIconImage );
    }
    else
    {
        m_pIcon->SetTexture( pIconImage );
    }
}

void ModuleDetails::SetContents()
{
    // Make sure the item's title always fits
    const float titleWidth = m_pTitleText->GetPosition().x + m_pTitleText->GetText().length() * 8.0f + 8.0f;
    const float panelWidth = gMax( 344.0f, titleWidth );
    m_pMainPanel->SetWidth( panelWidth );

    std::stringstream contentsText;
    contentsText << "Rarity: " << ToString( m_pModuleInfo->GetRarity() ) << "\n";

    Ship* pShip = g_pGame->GetPlayer()->GetShip();
    ModuleType type = m_pModuleInfo->GetType();
    if ( type == ModuleType::Engine )
    {
        EngineInfo* pEngineInfo = static_cast<EngineInfo*>( m_pModuleInfo );
        contentsText << "Armour: " << pEngineInfo->GetHealth( pShip ) << "\n"
                     << "Thrust: " << pEngineInfo->GetThrust() << "\n"
                     << "Torque: " << pEngineInfo->GetTorque() << "\n";
    }
    else if ( type == ModuleType::Armour )
    {
        ArmourInfo* pArmourInfo = static_cast<ArmourInfo*>( m_pModuleInfo );
        contentsText << "Armour: " << pArmourInfo->GetHealth( pShip ) << "\n"
                     << "Mass: " << static_cast<int>( BaseModuleMass * pArmourInfo->GetMassMultiplier( pShip ) ) << "\n"
                     << "Resistance (kinetic): " << ToStringPercentage( pArmourInfo->GetKineticResistance() ) << "\n"
                     << "Resistance (energy): " << ToStringPercentage( pArmourInfo->GetEnergyResistance() ) << "\n"
                     << "Regenerative: " << ( pArmourInfo->IsRegenerative() ? "Yes" : "No" ) << "\n"
                     << "Ramming prow: " << ( pArmourInfo->IsRammingProw() ? "Yes" : "No" ) << "\n";
    }
    else if ( type == ModuleType::Shield )
    {
        ShieldInfo* pShieldInfo = static_cast<ShieldInfo*>( m_pModuleInfo );
        contentsText << "Capacity: " << pShieldInfo->GetCapacity() << " u\n"
                     << "Peak recharge: " << pShieldInfo->GetPeakRecharge() << " u/s\n"
                     << "Power grid: " << pShieldInfo->GetPowerGrid( pShip ) << " u/s\n";
    }
    else if ( type == ModuleType::Weapon )
    {
        WeaponInfo* pWeaponInfo = static_cast<WeaponInfo*>( m_pModuleInfo );
        contentsText << "Type: " << ToString( pWeaponInfo->GetSystem() ) << "\n"
                     << "Damage type: " << ToString( pWeaponInfo->GetDamageType() ) << "\n"
                     << "DPS: " << static_cast<int>( pWeaponInfo->GetDPS( pShip ) ) << "\n"
                     << "Range: " << pWeaponInfo->GetRange( pShip ) << "\n"
                     << "Energy cost: " << static_cast<int>( pWeaponInfo->GetActivationCost( pShip ) * pWeaponInfo->GetRateOfFire( pShip ) ) << " u/s\n";
    }
    else if ( type == ModuleType::Reactor )
    {
        ReactorInfo* pReactorInfo = static_cast<ReactorInfo*>( m_pModuleInfo );
        contentsText << "Power grid: " << pReactorInfo->GetPowerGrid( pShip ) << "u\n"
                     << "Capacitor storage: " << pReactorInfo->GetCapacitorStorage() << "u\n"
                     << "Capacitor recharge: " << pReactorInfo->GetCapacitorRechargeRate() << " u/s\n";

        ReactorVariant variant = pReactorInfo->GetVariant();
        if ( variant == ReactorVariant::HighCapacity )
        {
            contentsText << "Variant: High capacity\n";
        }
        else if ( variant == ReactorVariant::Unstable )
        {
            contentsText << "Variant: Unstable\n";
        }
    }

    if ( m_pModuleInfo->GetDescription().empty() == false )
    {
        contentsText << "Description:\n"
                     << m_pModuleInfo->GetDescription();
    }

    // Ensure we don't end the module details with a '\n' as that will cause an incorrect line count when
    // we resize the panel.
    std::string contents = contentsText.str();
    if (contents.empty() == false && contents[contents.size() - 1] == '\n')
    {
        contents.pop_back();
    }

    const float contentsWidth = panelWidth - m_pContentsText->GetPosition().x - 16.0f;
    m_pContentsText->SetWidth( contentsWidth );
    m_pContentsText->SetText( contents );

    // Panel height needs to be set after we figure out how many lines we actually have to show
    const float lineHeight = m_pContentsText->GetFont()->GetLineHeight();
    const float panelHeight = gMax( 80.0f, m_pContentsText->GetPosition().y + lineHeight * m_pContentsText->GetLineCount() + 4.0f );
    m_pMainPanel->SetHeight( panelHeight );
}

} // namespace Hexterminate