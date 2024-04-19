// Copyright 2024 Pedro Nunes
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

#include <any>
#include <optional>
#include <string>
#include <unordered_map>

#include "genesis.h"

namespace Genesis
{

class ProgramOptions
{
public:
    ProgramOptions( int argc, char* argv[] );
    ~ProgramOptions() {}

    std::optional<bool> GetBool( const std::string& optionName ) const;
    std::optional<int> GetInt( const std::string& optionName ) const;

private:
    bool TryParseBool( const std::string& optionName, const std::string& value );
    bool TryParseInt( const std::string& optionName, const std::string& value );

    template <typename T>
    void Set( const std::string& optionName, T value )
    {
        m_Options[ optionName ] = value;
    }

    std::unordered_map<std::string, std::any> m_Options;
};

} // namespace Genesis
