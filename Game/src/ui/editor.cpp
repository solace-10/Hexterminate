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

#include <genesis.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl.h>

#include "hexterminate.h"
#include "ui/editor.h"
#include "ui/rootelement.h"

namespace Hexterminate::UI
{

Editor::Editor()
    : m_IsOpen( false )
{
    Genesis::ImGuiImpl::RegisterMenu( "Tools", "UI editor", &m_IsOpen );
}

Editor::~Editor()
{
}

void Editor::UpdateDebugUI()
{
    RootElement* pRoot = g_pGame->GetUIRoot();
    if ( pRoot == nullptr )
    {
        return;
    }

    if ( Genesis::ImGuiImpl::IsEnabled() && m_IsOpen )
    {
        ImGui::Begin( "UI editor", &m_IsOpen );

        if ( ImGui::Button( "Save" ) )
        {
            pRoot->Save();
        }
        ImGui::SameLine();
        if ( ImGui::Button( "Reload" ) )
        {
            pRoot->Load();
        }

        ImGui::BeginChild( "Hierarchy", ImVec2( ImGui::GetWindowContentRegionWidth() * 0.4f, -1.0f ), true );
        for ( auto& pChild : pRoot->GetChildren() )
        {
            RenderHierarchy( pChild );
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild( "Properties", ImVec2( -1.0f, -1.0f ), true );
        ElementSharedPtr pElement = m_pSelectedElement.lock();
        if ( pElement != nullptr )
        {
            if ( pElement->IsEditable() == false )
            {
                ImGui::TextDisabled( "This element is not editable." );
            }
            else if ( pElement->IsDynamic() )
            {
                ImGui::TextDisabled( "This element is dynamic and can't be modified." );
            }
            else
            {
                pElement->RenderProperties();
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }
}

void Editor::RenderHierarchy( ElementSharedPtr pElement )
{
    ImGuiTreeNodeFlags extraNodeFlags = 0;
    ElementSharedPtr pSelectedElement = m_pSelectedElement.lock();
    if ( pSelectedElement.get() == pElement.get() )
    {
        extraNodeFlags = ImGuiTreeNodeFlags_Selected;
    }

    if ( pElement->GetChildren().empty() ) // Leaf node?
    {
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | extraNodeFlags;
        ImGui::TreeNodeEx( pElement->GetName().c_str(), nodeFlags, "%s", pElement->GetName().c_str() );

        if ( ImGui::IsItemClicked() )
        {
            m_pSelectedElement = pElement;
        }
    }
    else
    {
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | extraNodeFlags;
        const bool nodeOpen = ImGui::TreeNodeEx( pElement->GetName().c_str(), nodeFlags, "%s", pElement->GetName().c_str() );
        if ( ImGui::IsItemClicked() )
        {
            m_pSelectedElement = pElement;
        }

        if ( nodeOpen )
        {
            for ( auto& pChild : pElement->GetChildren() )
            {
                RenderHierarchy( pChild );
            }

            ImGui::TreePop();
        }
    }
}

} // namespace Hexterminate::UI
