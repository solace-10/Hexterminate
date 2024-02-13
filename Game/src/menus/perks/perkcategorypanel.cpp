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

#include "menus/perks/perkcategorypanel.h"
#include "hexterminate.h"
#include "menus/eva.h"
#include "misc/gui.h"
#include "player.h"

namespace Hexterminate
{

PerkCategoryPanel::PerkCategoryPanel()
    : m_pTitle( nullptr )
    , m_PreviousUnlocked( true )
{
}

void PerkCategoryPanel::Init( const std::string& category )
{
    m_pTitle = GuiExtended::CreateText( 8, 8, 256, 32, "> " + category, this );
}

void PerkCategoryPanel::Show( bool state )
{
    Genesis::Gui::Image::Show( state );

    for ( PerkPanel* pPerk : m_Perks )
    {
        pPerk->Show( state );
    }
}

void PerkCategoryPanel::AddPerk( Perk perk, const std::string& name, const std::string& description, const std::string& icon, int cost )
{
    PerkState state = m_PreviousUnlocked ? PerkState::Locked : PerkState::Disabled;

    const int panelHeight = 106;
    const int panelWidth = 188;

    PerkPanel* pPanel = new PerkPanel();
    pPanel->SetSize( panelWidth, panelHeight );
    pPanel->SetColour( 0.0f, 0.0f, 0.0f, 0.5f );
    pPanel->SetBorderColour( 1.0f, 1.0f, 1.0f, 0.25f );
    pPanel->SetBorderMode( Genesis::Gui::PANEL_BORDER_ALL );
    pPanel->SetPosition( 4 + static_cast<int>( m_Perks.size() ) * ( panelWidth + 4 ), 32 );
    pPanel->Init( perk, name, description, icon, cost, state );
    AddElement( pPanel );
    m_Perks.push_back( pPanel );

    m_PreviousUnlocked = ( state == PerkState::Enabled );
}

void PerkCategoryPanel::HandleGameEvent( GameEvent* pEvent )
{
    if ( pEvent->GetType() == GameEventType::PerkAcquired )
    {
        m_PreviousUnlocked = true;

        for ( auto& pPerkPanel : m_Perks )
        {
            PerkState state;
            if ( g_pGame->GetPlayer()->GetPerks()->IsEnabled( pPerkPanel->GetPerk() ) )
            {
                state = PerkState::Enabled;
            }
            else
            {
                state = m_PreviousUnlocked ? PerkState::Locked : PerkState::Disabled;
            }

            pPerkPanel->SetState( state );

            m_PreviousUnlocked = ( state == PerkState::Enabled );
        }
    }
}

} // namespace Hexterminate
