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

#include <sstream>

#include <genesis.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl.h>

#include "blackboard.h"

namespace Hexterminate
{

/////////////////////////////////////////////////////////////////////
// Blackboard
/////////////////////////////////////////////////////////////////////

Blackboard::Blackboard()
    : m_DebugUIOpen( false )
{
    Genesis::ImGuiImpl::RegisterMenu( "Game", "Blackboard", &m_DebugUIOpen );
}

void Blackboard::Add( const std::string& text, int value /* = 1 */ )
{
    m_Map[ text ] = value;
}

bool Blackboard::Exists( const std::string& text ) const
{
    return ( m_Map.find( text ) != m_Map.cend() );
}

int Blackboard::Get( const std::string& text ) const
{
    BlackboardMap::const_iterator it = m_Map.find( text );
    if ( it == m_Map.cend() )
    {
        return 0;
    }
    else
    {
        return it->second;
    }
}

void Blackboard::Clear()
{
    m_Map.clear();
}

bool Blackboard::Write( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement* pRootElement )
{
    using namespace tinyxml2;

    XMLElement* pElement = xmlDoc.NewElement( "Blackboard" );
    pRootElement->LinkEndChild( pElement );

    for ( auto& pair : m_Map )
    {
        XMLElement* pPairElement = xmlDoc.NewElement( "Pair" );
        pElement->LinkEndChild( pPairElement );

        std::stringstream value;
        value << pair.second;
        pPairElement->SetAttribute( "name", pair.first.c_str() );
        pPairElement->SetAttribute( "value", value.str().c_str() );
    }

    return true;
}

bool Blackboard::Read( tinyxml2::XMLElement* pRootElement )
{
    using namespace tinyxml2;

    Clear();

    for ( XMLElement* pChildElement = pRootElement->FirstChildElement(); pChildElement != nullptr; pChildElement = pChildElement->NextSiblingElement() )
    {
        const char* pName = pChildElement->Attribute( "name" );
        if ( pName == nullptr )
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "Empty 'name' attribute in Blackboard::Read(), skipping" );
            continue;
        }

        std::string name( pChildElement->Attribute( "name" ) );
        int value = 0;
        pChildElement->QueryIntAttribute( "value", &value );
        Add( name, value );
    }

    return true;
}

void Blackboard::UpdateDebugUI()
{
    if ( Genesis::ImGuiImpl::IsEnabled() && m_DebugUIOpen )
    {
        std::string tagToRemove;

        ImGui::SetNextWindowSize( ImVec2( 400.0f, 400.0f ) );
        ImGui::Begin( "Blackboard", &m_DebugUIOpen );
        ImGui::Columns( 3 );
        ImGui::Separator();
        ImGui::Text( "Tag" );
        ImGui::NextColumn();
        ImGui::Text( "Value" );
        ImGui::NextColumn();
        ImGui::Text( "Remove tag" );
        ImGui::NextColumn();

        ImGui::Separator();

        int id = 0;
        for ( auto& pair : m_Map )
        {
            ImGui::Text( "%s", pair.first.c_str() );
            ImGui::NextColumn();
            ImGui::Text( "%d", pair.second );
            ImGui::NextColumn();

            ImGui::PushID( id++ );
            if ( ImGui::Button( "Remove" ) )
            {
                tagToRemove = pair.first;
            }
            ImGui::PopID();
            ImGui::NextColumn();
        }
        ImGui::End();

        if ( !tagToRemove.empty() )
        {
            m_Map.erase( tagToRemove );
        }
    }
}

} // namespace Hexterminate