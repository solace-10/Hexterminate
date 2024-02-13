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

#include <sstream>

#ifdef _WIN32
#include <ShlObj.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#include <SDL.h>

#include "beginexternalheaders.h"
#include "tinyxml2.h"
#include "endexternalheaders.h"

#include "configuration.h"
#include "genesis.h"
#include "logger.h"
#include "xml.h"

#define CONFIG_FILENAME "config.xml"

namespace Genesis
{

bool Configuration::m_Fullscreen = false;
std::bitset<static_cast<size_t>( Genesis::RenderSystem::PostProcessEffect::Count )> Configuration::m_PostProcessEffects;
unsigned int Configuration::m_ScreenWidth = 800u;
unsigned int Configuration::m_ScreenHeight = 600u;
unsigned int Configuration::m_MultiSampleSamples = 4u;
unsigned int Configuration::m_MasterVolume = 100u;
unsigned int Configuration::m_MusicVolume = 50u;
unsigned int Configuration::m_SFXVolume = 100u;
bool Configuration::m_Outlines = true;
bool Configuration::m_FireToggle = false;

void WriteXmlElement( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement& parentElement, const std::string& name, const std::string& content )
{
    using namespace tinyxml2;

	XMLElement* pElement = xmlDoc.NewElement( name.c_str() );
	pElement->SetText( content.c_str() );
    parentElement.InsertEndChild( pElement );
}

void WriteXmlElement( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement& parentElement, const std::string& name, bool content )
{
    using namespace tinyxml2;

    XMLElement* pElement = xmlDoc.NewElement( name.c_str() );
    pElement->SetText( content ? "true" : "false" );
    parentElement.InsertEndChild( pElement );
}

void WriteXmlElement( tinyxml2::XMLDocument& xmlDoc, tinyxml2::XMLElement& parentElement, const std::string& name, unsigned int content )
{
    using namespace tinyxml2;

	XMLElement* pElement = xmlDoc.NewElement( name.c_str() );

	std::stringstream ss;
	ss << content;

	pElement->SetText( ss.str().c_str() );
    parentElement.InsertEndChild( pElement );
}

void Configuration::Load()
{
    EnsureValidResolution();

    using namespace tinyxml2;
    tinyxml2::XMLDocument doc;
    if ( doc.LoadFile( CONFIG_FILENAME ) == tinyxml2::XML_SUCCESS )
    {
        bool bleachBypass = true;
        bool glow = true;
        bool vignette = true;

        XMLElement* pElemConfiguration = doc.FirstChildElement();
        for ( XMLElement* pElemEntry = pElemConfiguration->FirstChildElement(); pElemEntry; pElemEntry = pElemEntry->NextSiblingElement() )
        {
			Xml::Serialise( pElemEntry, "Fullscreen", m_Fullscreen );
			Xml::Serialise( pElemEntry, "PostProcessBleachBypass", bleachBypass );
            Xml::Serialise( pElemEntry, "PostProcessGlow", glow );
            Xml::Serialise( pElemEntry, "PostProcessVignette", vignette );
			Xml::Serialise( pElemEntry, "MasterVolume", (int&)m_MasterVolume );
			Xml::Serialise( pElemEntry, "MusicVolume", (int&)m_MusicVolume );
			Xml::Serialise( pElemEntry, "SFXVolume", (int&)m_SFXVolume );
			Xml::Serialise( pElemEntry, "Outlines", m_Outlines );
			Xml::Serialise( pElemEntry, "FireToggle", m_FireToggle );
        }

        EnablePostProcessEffect( Genesis::RenderSystem::PostProcessEffect::BleachBypass, bleachBypass );
        EnablePostProcessEffect( Genesis::RenderSystem::PostProcessEffect::Glow, glow );
        EnablePostProcessEffect( Genesis::RenderSystem::PostProcessEffect::Vignette, vignette );

        FrameWork::GetLogger()->LogInfo( "Config file loaded." );
    }
    else
    {
        CreateDefaultFile();
    }
}

void Configuration::Save()
{
    using namespace tinyxml2;

    tinyxml2::XMLDocument xmlDoc;
    XMLElement* pRoot = xmlDoc.NewElement( "Configuration" );
    xmlDoc.InsertFirstChild( pRoot );

	WriteXmlElement( xmlDoc, *pRoot, "Fullscreen", IsFullscreen() );
	WriteXmlElement( xmlDoc, *pRoot, "PostProcessBleachBypass", IsPostProcessingEffectEnabled( Genesis::RenderSystem::PostProcessEffect::BleachBypass ) );
    WriteXmlElement( xmlDoc, *pRoot, "PostProcessGlow", IsPostProcessingEffectEnabled( Genesis::RenderSystem::PostProcessEffect::Glow ) );
    WriteXmlElement( xmlDoc, *pRoot, "PostProcessVignette", IsPostProcessingEffectEnabled( Genesis::RenderSystem::PostProcessEffect::Vignette ) );
	WriteXmlElement( xmlDoc, *pRoot, "MasterVolume", GetMasterVolume() );
	WriteXmlElement( xmlDoc, *pRoot, "MusicVolume", GetMusicVolume() );
	WriteXmlElement( xmlDoc, *pRoot, "SFXVolume", GetSFXVolume() );
	WriteXmlElement( xmlDoc, *pRoot, "Outlines", GetOutlines() );
	WriteXmlElement( xmlDoc, *pRoot, "FireToggle", GetFireToggle() );

    xmlDoc.SaveFile( CONFIG_FILENAME );
}

void Configuration::CreateDefaultFile()
{
    FrameWork::GetLogger()->LogInfo( "Creating default config file." );
    SetDefaultValues();
    Save();
}

void Configuration::SetDefaultValues()
{
    EnsureValidResolution();

    m_Fullscreen = false;

    for ( size_t i = 0; i < static_cast<size_t>( Genesis::RenderSystem::PostProcessEffect::Count ); ++i )
    {
        m_PostProcessEffects[ i ] = true;
    }

    m_MultiSampleSamples = 4u;

	m_MasterVolume = 100u;
	m_MusicVolume = 50u;
	m_SFXVolume = 100u;

	m_Outlines = true;
	m_FireToggle = false;
}

void Configuration::EnsureValidResolution()
{
    SDL_DisplayMode dm;
    if ( SDL_GetDesktopDisplayMode( 0, &dm ) != 0 )
    {
        FrameWork::GetLogger()->LogError( "SDL_GetDesktopDisplayMode failed: %s", SDL_GetError() );
    }
    else if ( dm.w <= 0 )
    {
        FrameWork::GetLogger()->LogError( "Invalid screen width: %d.", dm.w );
    }
    else if ( dm.h <= 0 )
    {
        FrameWork::GetLogger()->LogError( "Invalid screen height: %d.", dm.h );
    }

    m_ScreenWidth = static_cast<unsigned int>( dm.w );
    m_ScreenHeight = static_cast<unsigned int>( dm.h );
}

std::filesystem::path Configuration::GetSystemSaveGameFolder()
{
#ifdef _WIN32
    // Return %USERPROFILE%\Saved Games for Windows Vista or newer
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb762188%28v=vs.85%29.aspx
    PWSTR pKnownFolderPath = nullptr;
    HRESULT result = SHGetKnownFolderPath( FOLDERID_SavedGames, 0, nullptr, &pKnownFolderPath );
    SDL_assert_release( result == S_OK );
    std::wstring folder( pKnownFolderPath );
    CoTaskMemFree( pKnownFolderPath );
    return folder;
#else
    struct passwd *pw = getpwuid( getuid() );
    const char* pHomeDir = pw->pw_dir;
    std::filesystem::path folder = std::filesystem::path( pHomeDir ) / ".local" / "share";
    return folder;
#endif
}

}