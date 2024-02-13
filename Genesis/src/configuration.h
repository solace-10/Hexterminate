// Copyright 2017 Pedro Nunes
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

#include <bitset>
#include <filesystem>
#include <rendersystem.h>
#include <string>

#include <SDL_assert.h>

namespace Genesis
{

class Configuration
{
public:
    static void Load();
    static void Save();

    static unsigned int GetScreenWidth();
    static unsigned int GetScreenHeight();
    static bool IsFullscreen();
	static void SetFullscreen( bool value );
	static bool IsPostProcessingEffectEnabled( RenderSystem::PostProcessEffect effect );
	static void EnablePostProcessEffect( RenderSystem::PostProcessEffect effect, bool enabled );
    static unsigned int GetMultiSampleSamples();
    static std::filesystem::path GetSystemSaveGameFolder();

	static unsigned int GetMasterVolume();
	static unsigned int GetMusicVolume();
	static unsigned int GetSFXVolume();
	static void SetMasterVolume( unsigned int value );
	static void SetMusicVolume( unsigned int value );
	static void SetSFXVolume( unsigned int value );

	static bool GetOutlines();
	static void SetOutline( bool state );
	static bool GetFireToggle();
	static void SetFireToggle( bool state );

private:
    static void CreateDefaultFile();
    static void SetDefaultValues();
	static void EnsureValidResolution();

    static unsigned int m_ScreenWidth;
    static unsigned int m_ScreenHeight;
    static bool m_Fullscreen;
	static std::bitset<static_cast<size_t>( Genesis::RenderSystem::PostProcessEffect::Count )> m_PostProcessEffects;
    static unsigned int m_MultiSampleSamples;
	static unsigned int m_MasterVolume;
	static unsigned int m_MusicVolume;
	static unsigned int m_SFXVolume;
	static bool m_Outlines;
	static bool m_FireToggle;
};

inline unsigned int Configuration::GetScreenWidth()
{
    return m_ScreenWidth;
}

inline unsigned int Configuration::GetScreenHeight()
{
    return m_ScreenHeight;
}

inline bool Configuration::IsFullscreen()
{
    return m_Fullscreen;
}

inline void Configuration::SetFullscreen( bool state )
{
	m_Fullscreen = state;
}

inline bool Configuration::IsPostProcessingEffectEnabled( RenderSystem::PostProcessEffect effect )
{
	return m_PostProcessEffects[ static_cast<size_t>( effect) ];
}

inline void Configuration::EnablePostProcessEffect( RenderSystem::PostProcessEffect effect, bool enabled )
{
	m_PostProcessEffects[ static_cast<size_t>( effect ) ] = enabled;
}

inline unsigned int Configuration::GetMultiSampleSamples()
{
    return m_MultiSampleSamples;
}

inline unsigned int Configuration::GetMasterVolume()
{
	return m_MasterVolume;
}

inline unsigned int Configuration::GetMusicVolume()
{
	return m_MusicVolume;
}

inline unsigned int Configuration::GetSFXVolume()
{
	return m_SFXVolume;
}

inline void Configuration::SetMasterVolume( unsigned int value )
{
	SDL_assert( value <= 100u );
	m_MasterVolume = value;
}

inline void Configuration::SetMusicVolume( unsigned int value )
{
	SDL_assert( value <= 100u );
	m_MusicVolume = value;
}

inline void Configuration::SetSFXVolume( unsigned int value )
{
	SDL_assert( value <= 100u );
	m_SFXVolume = value;
}

inline bool Configuration::GetOutlines()
{
	return m_Outlines;
}

inline void Configuration::SetOutline( bool state )
{
	m_Outlines = state;
}

inline bool Configuration::GetFireToggle()
{
	return m_FireToggle;
}

inline void Configuration::SetFireToggle( bool state )
{
	m_FireToggle = state;
}

}
