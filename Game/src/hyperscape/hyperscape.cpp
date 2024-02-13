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

#include <imgui/imgui.h>
#include <imgui/imgui_impl.h>

#include "hyperscape/hyperscape.h"
#include "hyperscape/hyperscapelocation.h"
#include "hyperscape/hyperscaperep.h"
#include "hyperscape/silverthread.h"
#include "hyperscape/starfield.h"
#include "sector/starinfo.h"

namespace Hexterminate
{

//-----------------------------------------------------------------------------
// Hyperscape
//-----------------------------------------------------------------------------

Hyperscape::Hyperscape()
    : m_DebugWindowOpen( false )
{
    m_pRep = std::make_unique<HyperscapeRep>( this );
    m_pRep->Initialise();

    m_pSilverThread = std::make_unique<SilverThread>();
    m_pSilverThread->Scan( 1 );
    m_pSilverThread->JumpToScannedLocation( 0 );
    m_pSilverThread->CallStation();
    m_pSilverThread->Scan( 1 );
    m_pSilverThread->JumpToScannedLocation( 0 );
    m_pSilverThread->Scan( 2 );

    m_pStarfield = std::make_unique<Starfield>();

    Genesis::ImGuiImpl::RegisterMenu( "Game", "Hyperscape", &m_DebugWindowOpen );
}

Hyperscape::~Hyperscape()
{
    m_pRep->RemoveFromScene();
    Genesis::ImGuiImpl::UnregisterMenu( "Game", "Hyperscape" );
}

void Hyperscape::Update( float delta )
{
    m_pStarfield->Update( delta );
    UpdateDebugUI();
}

void Hyperscape::Show( bool state )
{
    m_pRep->Show( state );
}

bool Hyperscape::IsVisible() const
{
    return m_pRep->IsVisible();
}

bool Hyperscape::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    bool state = true;

    tinyxml2::XMLElement* pHyperscapeElement = xmlDoc.NewElement( "Hyperscape" );
    pRootElement->LinkEndChild( pHyperscapeElement );

    return state;
}

bool Hyperscape::Read( tinyxml2::XMLElement* pRootElement )
{
    int version = 1;

    if ( version != GetVersion() )
    {
        UpgradeFromVersion( version );
    }

    return true;
}

int Hyperscape::GetVersion() const
{
    return 1;
}

void Hyperscape::UpdateDebugUI()
{
#ifdef _DEBUG
    {
        ImGui::SetNextWindowSize( ImVec2( 600.0f, 800.0f ) );
        ImGui::Begin( "Hyperscape", &m_DebugWindowOpen );

        static bool sSilverThreadOpen = true;
        int id = 0;
        if ( ImGui::CollapsingHeader( "Silver thread", &sSilverThreadOpen, ImGuiTreeNodeFlags_DefaultOpen ) )
        {
            const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
            if ( ImGui::BeginTable( "svtable", 4, flags ) )
            {
                ImGui::TableSetupColumn( "Action" );
                ImGui::TableSetupColumn( "Depth" );
                ImGui::TableSetupColumn( "Status" );
                ImGui::TableSetupColumn( "Seed" );
                ImGui::TableHeadersRow();

                const HyperscapeLocationVector& locations = m_pSilverThread->GetLocations();
                for ( auto it = locations.rbegin(); it != locations.rend(); ++it )
                {
                    bool locationsChanged = false;
                    const HyperscapeLocation& location = *it;
                    const bool isCurrent = ( m_pSilverThread->GetCurrentLocation() == location );
                    if ( location.GetType() == HyperscapeLocation::Type::Visited )
                    {
                        ImGui::TableNextColumn();
                        if ( isCurrent )
                        {
                            ImGui::PushID( id++ );
                            if ( ImGui::Button( "Call station" ) )
                            {
                                m_pSilverThread->CallStation();
                            }
                            ImGui::PopID();
                        }

                        ImGui::TableNextColumn();
                        ImGui::Text( "%u", location.GetDepth() );

                        ImGui::TableNextColumn();
                        if ( isCurrent )
                        {
                            ImGui::TextColored( ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), "Current" );
                        }
                        else
                        {
                            ImGui::Text( "Visited" );
                        }

                        ImGui::TableNextColumn();
                        ImGui::Text( "%u", location.GetSeed() );
                    }
                    else if ( location.GetType() == HyperscapeLocation::Type::Scanned )
                    {
                        ImGui::TableNextColumn();
                        ImGui::PushID( id++ );
                        if ( ImGui::Button( "Jump" ) )
                        {
                            m_pSilverThread->JumpToScannedLocation( location );
                            m_pSilverThread->Scan( 2 );
                            locationsChanged = true;
                        }
                        ImGui::PopID();

                        ImGui::TableNextColumn();
                        ImGui::Text( "%u", location.GetDepth() );
                        ImGui::TableNextColumn();
                        ImGui::TextColored( ImVec4( 0.0f, 1.0f, 1.0f, 1.0f ), "Scanned" );
                        ImGui::TableNextColumn();
                        ImGui::Text( "%u", location.GetSeed() );
                    }
                    else if ( location.GetType() == HyperscapeLocation::Type::Station )
                    {
                        ImGui::TableNextColumn(); // Action
                        ImGui::TableNextColumn();
                        ImGui::Text( "%u", location.GetDepth() );
                        ImGui::TableNextColumn();
                        ImGui::TextColored( ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ), isCurrent ? "Station / Current" : "Station" );
                        ImGui::TableNextColumn();
                        ImGui::Text( "%u", location.GetSeed() );
                    }

                    if ( locationsChanged )
                    {
                        break;
                    }
                }

                ImGui::EndTable();
            }
        }

        ImGui::End();
    }
#endif
}

void Hyperscape::UpgradeFromVersion( int version )
{
}

} // namespace Hexterminate
