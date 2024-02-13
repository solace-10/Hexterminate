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

#include <gui/gui.h>

#include "stringaux.h"
#include "ui/fonts.h"

namespace Hexterminate::UI
{

static bool sFontsLoaded = false;
static std::vector<Genesis::ResourceFont*> sFonts;

void Fonts::Load()
{
    using namespace Genesis;

    if ( sFontsLoaded )
    {
        return;
    }

    for ( const auto& entry : std::filesystem::directory_iterator( "data/fonts" ) )
    {
        if ( entry.path().extension() == ".fnt" )
        {
#ifdef _WIN32
            std::wstring wpath( entry.path().c_str() );
            ResourceFont* pFont = FrameWork::GetResourceManager()->GetResource<ResourceFont*>( ToString( wpath ) );
#else
            ResourceFont* pFont = FrameWork::GetResourceManager()->GetResource<ResourceFont*>( entry.path().c_str() );
#endif
            SDL_assert( pFont != nullptr );
            sFonts.push_back( pFont );
        }
    }

    sFontsLoaded = true;
}

Genesis::ResourceFont* Fonts::Get( const std::string& fontName )
{
    Load();

    for ( auto& pFont : sFonts )
    {
        if ( pFont->GetFilename().GetName() == fontName )
        {
            return pFont;
        }
    }

    Genesis::FrameWork::GetLogger()->LogWarning( "Couldn't find font '%s'.", fontName.c_str() );
    return nullptr;
}

const FontVector& Fonts::Get()
{
    Load();
    return sFonts;
}

} // namespace Hexterminate::UI
