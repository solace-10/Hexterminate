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

#include <fstream>

#include "ui/design.h"

namespace Hexterminate
{
namespace UI
{

    Design::Design()
    {
        m_Path = "data/ui/design.json";
    }

    void Design::Save()
    {
        std::ofstream file( m_Path, std::ios::out );
        if ( file.is_open() )
        {
            file << m_Data.dump( 4 ) << std::endl;
            file.close();
        }
    }

    void Design::Load()
    {
        m_Data = {};

        if ( std::filesystem::exists( m_Path ) == false || std::filesystem::file_size( m_Path ) == 0 )
        {
            return;
        }

        std::ifstream file( m_Path, std::ios::in );
        if ( file.is_open() )
        {
            file >> m_Data;
            file.close();
        }
    }

    json& Design::Get( const std::string& path )
    {
        return m_Data[ nlohmann::json_pointer<json>( path ) ];
    }

} // namespace UI
} // namespace Hexterminate