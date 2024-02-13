// Copyright 2016 Pedro Nunes
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

#include <unordered_map>
#include <string>

namespace Genesis
{

class Shader;

///////////////////////////////////////////////////////////////////////////////
// ShaderCache
// Caches shader programs. Compiling shader programs is expensive and must be
// done at load time, so any shader files should be added to this cache before
// they are referenced for rendering.
///////////////////////////////////////////////////////////////////////////////

class ShaderCache
{
public:
    ShaderCache();
    ~ShaderCache();

    Shader* Load( const std::string& programName );

private:
    typedef std::unordered_map<std::string, Shader*> ShaderMap;

    ShaderMap m_ProgramCache;
};
}
