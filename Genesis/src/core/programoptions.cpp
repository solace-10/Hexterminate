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

#include "core/programoptions.h"

#include "genesis.h"
#include "logger.h"

namespace Genesis
{

ProgramOptions::ProgramOptions( int argc, char* argv[] )
{
    // Skipping argv[0] as that's the executable.
    for ( int i = 1; i < argc; i++ )
    {
        std::string arg( argv[ i ] );
        size_t separator = arg.find_first_of( '=' );
        if ( separator == std::string::npos )
        {
            Set<bool>( arg, true );
        }
        else if ( separator == 0 || separator == arg.length() )
        {
            FrameWork::GetLogger()->LogError( "Invalid argument %d, format must be 'name=value'.", i - 1 );
        }
        else
        {
            std::string optionName = arg.substr( 0, separator );
            std::string value = arg.substr( separator + 1 );
            if ( !TryParseBool( optionName, value ) && !TryParseInt( optionName, value ) )
            {
                FrameWork::GetLogger()->LogError( "Invalid value for %s.", optionName.c_str() );
            }
        }
    }
}

std::optional<bool> ProgramOptions::GetBool( const std::string& optionName ) const
{
    auto it = m_Options.find( optionName );
    if ( it == m_Options.end() )
    {
        return std::nullopt;
    }
    else
    {
        if ( it->second.type() != typeid( bool ) )
        {
            FrameWork::GetLogger()->LogError( "Program option %s is not a boolean.", optionName.c_str() );
            return std::nullopt;
        }
        else
        {
            return std::any_cast<bool>( it->second );
        }
    }
}

std::optional<int> ProgramOptions::GetInt( const std::string& optionName ) const
{
    auto it = m_Options.find( optionName );
    if ( it == m_Options.end() )
    {
        return std::nullopt;
    }
    else
    {
        if ( it->second.type() != typeid( int ) )
        {
            FrameWork::GetLogger()->LogError( "Program option %s is not an integer.", optionName.c_str() );
            return std::nullopt;
        }
        else
        {
            return std::any_cast<int>( it->second );
        }
    }
}

bool ProgramOptions::TryParseBool( const std::string& optionName, const std::string& value )
{
    if ( value == "true" || value == "false" )
    {
        Set<bool>( optionName, ( value == "true" ) );
        return true;
    }
    else
    {
        return false;
    }
}

bool ProgramOptions::TryParseInt( const std::string& optionName, const std::string& value )
{
    try
    {
        Set<int>( optionName, std::stoi( value ) );
        return true;

    } catch ( const std::invalid_argument& ex )
    {
        FrameWork::GetLogger()->LogError( "Invalid argument for %s: %s", optionName.c_str(), ex.what() );
    } catch ( const std::out_of_range& )
    {
        FrameWork::GetLogger()->LogError( "Value out of range for %s.", optionName.c_str() );
    }

    return false;
}

} // namespace Genesis
