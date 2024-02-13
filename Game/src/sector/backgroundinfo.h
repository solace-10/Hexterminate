// Copyright 2015 Pedro Nunes
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

#pragma once

#include "color.h"
#include <string>
#include <vector>

namespace Hexterminate
{

class BackgroundInfo;
typedef std::vector<BackgroundInfo> BackgroundInfoVector;

class BackgroundInfo
{
public:
    BackgroundInfo( int id, const std::string& filename, const Genesis::Color& ambient )
        : m_Id( id )
        , m_Filename( filename )
        , m_Ambient( ambient )
    {
    }

    inline int GetId() const { return m_Id; }
    inline const std::string& GetFilename() const { return m_Filename; }
    inline const Genesis::Color& GetAmbientColour() const { return m_Ambient; }

private:
    int m_Id;
    std::string m_Filename;
    Genesis::Color m_Ambient;
};

} // namespace Hexterminate
