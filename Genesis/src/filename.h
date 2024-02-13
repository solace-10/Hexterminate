// Copyright 2021 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <string>

namespace Genesis
{

class Filename
{
public:
    Filename( const char* pFilename );
    Filename( const std::string& filename );

    const std::string& GetExtension() const;
    const std::string& GetDirectory() const;
    const std::string& GetFullPath() const;
    const std::string& GetName() const;

private:
    void Initialise( const std::string& filename );

    std::string ResolvePath( const std::string& filename ) const;
    std::string m_Extension;
    std::string m_Directory;
    std::string m_FullPath;
    std::string m_Name;
};

inline const std::string& Filename::GetExtension() const
{
    return m_Extension;
}

inline const std::string& Filename::GetDirectory() const
{
    return m_Directory;
}

inline const std::string& Filename::GetFullPath() const
{
    return m_FullPath;
}

inline const std::string& Filename::GetName() const
{
    return m_Name;
}
}