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

#include <filesystem>
#include <string>
#include <vector>

#include <configuration.h>
#include <genesis.h>
#include <gui/gui.h>
#include <imgui/imgui.h>

#include "ui/design.h"
#include "ui/rootelement.h"

namespace Hexterminate::UI
{

RootElement::RootElement( const std::filesystem::path& designPath )
    : Element( designPath.stem().u8string() )
    , m_pDesign( nullptr )
{
    m_pDesign = std::make_unique<Design>( designPath );
    m_pDesign->Load();

    GetPanel()->SetWidth( static_cast<float>( Genesis::Configuration::GetScreenWidth() ) );
    GetPanel()->SetHeight( static_cast<float>( Genesis::Configuration::GetScreenHeight() ) );
    Genesis::FrameWork::GetGuiManager()->AddElement( GetPanel() );
}

RootElement::~RootElement()
{
    Genesis::FrameWork::GetGuiManager()->RemoveElement( GetPanel() );
}

void RootElement::Save()
{
    if ( GetChildren().size() > 0 )
    {
        m_pDesign->Clear();
    }

    json& rootData = m_pDesign->Get( "" );
    SaveInternal( rootData, false );
    m_pDesign->Save();
}

void RootElement::Load()
{
    m_pDesign->Load();
}

} // namespace Hexterminate::UI
