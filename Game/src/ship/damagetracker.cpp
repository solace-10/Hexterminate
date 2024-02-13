// Copyright 2017 Pedro Nunes
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
#include <sstream>

#include "faction/faction.h"
#include "globals.h"
#include "hexterminate.h"
#include "ship/damagetracker.h"
#include "ship/ship.h"
#include "ship/shipinfo.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// DamageTracker
/////////////////////////////////////////////////////////////////////

int DamageTracker::sID = 0;

DamageTracker::DamageTracker( Ship* pShip )
    : m_pShip( pShip )
    , m_ID( sID++ )
{
    for ( int i = 0; i < static_cast<int>( FactionId::Count ); ++i )
    {
        m_Damage[ i ] = 0.0f;
    }

    DamageTrackerDebugWindow::AddDamageTracker( this );
}

DamageTracker::~DamageTracker()
{
    DamageTrackerDebugWindow::RemoveDamageTracker( this );
}

Ship* DamageTracker::GetShip() const
{
    return m_pShip;
}

float DamageTracker::GetDamage( FactionId id ) const
{
    return m_Damage[ static_cast<int>( id ) ];
}

void DamageTracker::AddDamage( FactionId id, float amount )
{
    m_Damage[ static_cast<int>( id ) ] += amount;
}

void DamageTracker::GetHighestContributor( FactionId& id, float& amount ) const
{
    float highestAmount = -1.0f;
    for ( int i = 0; i < static_cast<int>( FactionId::Count ); ++i )
    {
        if ( m_Damage[ i ] > highestAmount )
        {
            id = static_cast<FactionId>( i );
            amount = m_Damage[ i ];
            highestAmount = m_Damage[ i ];
        }
    }
}

int DamageTracker::GetID() const
{
    return m_ID;
}

/////////////////////////////////////////////////////////////////////
// DamageTrackerDebugWindow
/////////////////////////////////////////////////////////////////////

bool DamageTrackerDebugWindow::m_Open = false;
std::list<DamageTracker*> DamageTrackerDebugWindow::m_Trackers;

void DamageTrackerDebugWindow::Register()
{
    Genesis::ImGuiImpl::RegisterMenu( "Sector", "Damage tracker", &m_Open );
}

void DamageTrackerDebugWindow::Unregister()
{
    Genesis::ImGuiImpl::UnregisterMenu( "Sector", "Damage tracker" );
}

void DamageTrackerDebugWindow::Update()
{
    using namespace Genesis;
    if ( m_Open )
    {
        ImGui::SetNextWindowSize( ImVec2( 400.0f, 400.0f ) );
        ImGui::Begin( "Damage tracker", &m_Open );

        for ( auto& pTracker : m_Trackers )
        {
            Ship* pShip = pTracker->GetShip();
            std::stringstream ss;
            ss << "[" << pShip->GetFaction()->GetName() << "]: ";
            if ( pShip->GetShipInfo() == nullptr )
            {
                ss << "Player ship";
            }
            else
            {
                ss << pShip->GetShipInfo()->GetName();
            }

            if ( pShip->IsDestroyed() )
            {
                ss << " (destroyed)";
            }

            ImGui::PushID( pTracker->GetID() );
            if ( ImGui::CollapsingHeader( ss.str().data() ) )
            {
                FactionId highestContributor;
                float highestContributorDamage;
                pTracker->GetHighestContributor( highestContributor, highestContributorDamage );

                for ( int idx = 0; idx < static_cast<int>( FactionId::Count ); ++idx )
                {
                    FactionId factionId = static_cast<FactionId>( idx );
                    const float damage = pTracker->GetDamage( factionId );
                    if ( damage > 0.0f )
                    {
                        bool overrideColour = false;
                        if ( ( factionId == FactionId::Player && damage > highestContributorDamage * ShipRewardThreshold ) || ( factionId == highestContributor ) )
                        {
                            overrideColour = true;
                            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
                        }

                        ImGui::Columns( 2 );
                        ImGui::Text( "%s", g_pGame->GetFaction( factionId )->GetName().data() );
                        ImGui::NextColumn();
                        ImGui::Text( "%d", static_cast<int>( damage ) );
                        ImGui::NextColumn();
                        ImGui::Columns( 1 );

                        if ( overrideColour )
                        {
                            ImGui::PopStyleColor();
                        }
                    }
                }
            }
            ImGui::PopID();
        }

        ImGui::End();
    }
}

void DamageTrackerDebugWindow::AddDamageTracker( DamageTracker* pTracker )
{
    m_Trackers.push_back( pTracker );
}

void DamageTrackerDebugWindow::RemoveDamageTracker( DamageTracker* pTracker )
{
    m_Trackers.remove( pTracker );
}

} // namespace Hexterminate