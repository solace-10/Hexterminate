// Copyright 2018 Pedro Nunes
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

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <filesystem>
#include <sstream>

#include <configuration.h>
#include <genesis.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl.h>

#include "hexterminate.h"
#include "menus/popup.h"
#include "player.h"
#include "savegameheader.h"
#include "savegamestorage.h"
#include "sector/galaxy.h"
#include "stringaux.h"
#include "xmlaux.h"

namespace Hexterminate
{

SaveGameStorage::SaveGameStorage()
    : m_DebugWindowOpen( false )
    , m_CloudStorageActive( false )
    , m_pXmlReadThread( nullptr )
    , m_Ready( false )
{
    Genesis::ImGuiImpl::RegisterMenu( "Game", "Save game storage", &m_DebugWindowOpen );

#if USE_STEAM
    m_pSteamRemoteStorage = SteamRemoteStorage();
    m_CloudStorageActive = m_pSteamRemoteStorage && m_pSteamRemoteStorage->IsCloudEnabledForApp() && m_pSteamRemoteStorage->IsCloudEnabledForAccount();

    if ( Genesis::FrameWork::GetCommandLineParameters()->HasParameter( "--use-local-saves" ) )
    {
        Genesis::FrameWork::GetLogger()->LogInfo("%s", "SaveGameStorage using local saves due to presence of --use-local-saves.");
        m_CloudStorageActive = false;
    }

    if ( m_CloudStorageActive )
    {
        Genesis::FrameWork::GetLogger()->LogInfo("%s", "SaveGameStorage using Steam remote storage.");
        const int filecount = m_pSteamRemoteStorage->GetFileCount();
        Genesis::FrameWork::GetLogger()->LogInfo("Steam remote storage contains %d files.", filecount);
        for ( int i = 0; i < filecount; ++i )
        {
            int fileSize = 0;
            const char* pFilename = m_pSteamRemoteStorage->GetFileNameAndSize( i, &fileSize );
            std::shared_ptr<StorageFile> pStorageFile = std::make_shared<StorageFile>();
            pStorageFile->filename = ToWString( pFilename );
            m_StorageFiles.push_back( pStorageFile );
            Genesis::FrameWork::GetLogger()->LogInfo("Save in remote storage: %s", pFilename);
        }
    }
#else
    Genesis::FrameWork::GetLogger()->LogInfo( "Using local saves as no cloud storage is available." );
#endif

    if ( m_CloudStorageActive == false )
    {
        std::filesystem::path saveGameDirectory = Genesis::Configuration::GetSystemSaveGameFolder() / "Hexterminate";
        if ( std::filesystem::exists( saveGameDirectory ) )
        {
            for ( const auto& filename : std::filesystem::directory_iterator( saveGameDirectory ) )
            {
                std::shared_ptr<StorageFile> pStorageFile = std::make_shared<StorageFile>();
                pStorageFile->filename = filename.path();
                m_StorageFiles.push_back( pStorageFile );
            }
        }
    }

    m_pXmlReadThread = SDL_CreateThread( &SaveGameStorage::sXmlReadThreadMain, "Save game storage - reader thread", this );
}

void SaveGameStorage::UpdateDebugUI()
{
    if ( m_DebugWindowOpen )
    {
        ImGui::SetNextWindowSize( ImVec2( 600.0f, 800.0f ) );
        ImGui::Begin( "Save game storage", &m_DebugWindowOpen );

        ImGui::Columns( 2 );

        ImGui::Text( "Filename" );
        ImGui::NextColumn();
        ImGui::Text( "Loaded" );
        ImGui::NextColumn();
        ImGui::Separator();

        for ( auto& pStorageFile : m_StorageFiles )
        {
            std::string filename = ToString( pStorageFile->filename );
            ImGui::Text( "%s", filename.c_str() );
            ImGui::NextColumn();
            ImGui::Text( "%s", pStorageFile->pSaveGameHeader == nullptr ? "false" : "true" );
            ImGui::NextColumn();
        }

        ImGui::Columns( 1 );

        ImGui::End();
    }
}

bool SaveGameStorage::SaveGame( bool killSave /* = false */ )
{
    using namespace Genesis;

    if ( g_pGame->GetState() != GameState::GalaxyView )
    {
        g_pGame->RaiseInteractiveWarning( "Save failed: attempting to save game while not in the galaxy view." );
        return false;
    }

    std::filesystem::path filename = GetSaveGameFileName();
    std::filesystem::path fullPath;

    if ( m_CloudStorageActive == false )
    {
        std::filesystem::path systemSaveGameFolder = Genesis::Configuration::GetSystemSaveGameFolder();
        std::filesystem::path gameSaveGameFolder = systemSaveGameFolder / "Hexterminate";

        if ( !CreateSaveGameFolder( gameSaveGameFolder ) )
        {
            g_pGame->RaiseInteractiveWarning( "Save failed: couldn't create save game folder." );
            return false;
        }

        fullPath = gameSaveGameFolder / filename;
        Genesis::FrameWork::GetLogger()->LogInfo( "Attempting to save to %s", ToString( fullPath ).c_str() );
    }

    tinyxml2::XMLDocument xmlDoc;
    CreateSaveGameData( xmlDoc, killSave );

    if ( m_CloudStorageActive )
    {
        return SaveToRemoteStorage( xmlDoc );
    }
    else
    {
        return SaveToLocalStorage( xmlDoc, fullPath );
    }
}

bool SaveGameStorage::SaveToLocalStorage( tinyxml2::XMLDocument& xmlDoc, const std::filesystem::path& fullPath )
{
    using namespace tinyxml2;

    FILE* fp = nullptr;

#ifdef _WIN32
    errno_t err = _wfopen_s( &fp, fullPath.c_str(), L"wb" );
    if ( err != 0 )
    {
        char errorMessage[ 256 ];
        strerror_s( errorMessage, 256, err );

        std::stringstream ss;
        ss << "Save failed: " << errorMessage;
        g_pGame->RaiseInteractiveWarning( ss.str() );
        return false;
    }
#else
    fp = fopen( fullPath.c_str(), "wb" );
    if ( fp == nullptr )
    {
        std::stringstream ss;
        ss << "Save failed: error " << errno;
        g_pGame->RaiseInteractiveWarning( ss.str() );
        return false;
    }
#endif

    XMLError saveResult = xmlDoc.SaveFile( fp );

    if ( saveResult == XMLError::XML_SUCCESS )
    {
        Genesis::FrameWork::GetLogger()->LogInfo( "Game saved successfully!" );
        fclose( fp );
        return true;
    }
    else
    {
        std::stringstream ss;
        ss << "Save failed: TinyXML error code " << static_cast<unsigned int>( saveResult );
        g_pGame->RaiseInteractiveWarning( ss.str() );
        fclose( fp );
        return false;
    }

    return false;
}

bool SaveGameStorage::SaveToRemoteStorage( tinyxml2::XMLDocument& xmlDoc )
{
    bool success = false;
    std::filesystem::path filename = GetSaveGameFileName();

#if USE_STEAM
    // Generates a buffer with the XML contents which can then be passed to Steam.
    using namespace tinyxml2;
    XMLPrinter printer;
    xmlDoc.Print( &printer );

    std::string steamFilename = ToString( filename );
    success = m_pSteamRemoteStorage->FileWrite( steamFilename.c_str(), printer.CStr(), printer.CStrSize() );
    {
        UpdateStorageFiles( filename, xmlDoc );
        return true;
    }
#else
    SDL_assert( false ); // Not implemented
#endif

    if ( success )
    {
        UpdateStorageFiles( filename, xmlDoc );
    }

    return success;
}

bool SaveGameStorage::LoadGame( const std::filesystem::path& filename, tinyxml2::XMLDocument& xmlDoc )
{
    return XmlRead( filename, xmlDoc );
}

int SaveGameStorage::sXmlReadThreadMain( void* pData )
{
    using namespace tinyxml2;
    SaveGameStorage* pSaveGameStorage = reinterpret_cast<SaveGameStorage*>( pData );
    for ( StorageFileList::iterator it = pSaveGameStorage->m_StorageFiles.begin(); it != pSaveGameStorage->m_StorageFiles.end(); )
    {
        auto pStorageFile = *it;
        if ( pStorageFile->pSaveGameHeader == nullptr )
        {
            tinyxml2::XMLDocument xmlDoc;
            pSaveGameStorage->XmlRead( pStorageFile->filename, xmlDoc );

            SaveGameHeaderSharedPtr pSaveGameHeader = std::make_shared<SaveGameHeader>( pStorageFile->filename );
            if ( pSaveGameHeader->Read( xmlDoc ) )
            {
                pStorageFile->pSaveGameHeader = pSaveGameHeader;
                it++;
            }
            else
            {
                it = pSaveGameStorage->m_StorageFiles.erase( it );
            }
        }
    }

    pSaveGameStorage->m_Ready.store( true );

    return 0;
}

bool SaveGameStorage::XmlRead( const std::filesystem::path& filename, tinyxml2::XMLDocument& xmlDoc )
{
#if USE_STEAM
    if ( m_CloudStorageActive )
    {
        std::string steamFilename = ToString( filename );
        const int fileSize = m_pSteamRemoteStorage->GetFileSize( steamFilename.c_str() );
        if ( fileSize == 0 )
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "SteamWorks gave a file size of 0 for save game '%s', skipping this save. This is likely a hiccup from Steam.", steamFilename.c_str() );
            return false;
        }
        std::vector<char> fileData;
        fileData.resize( fileSize );
        int bytesRead = m_pSteamRemoteStorage->FileRead( steamFilename.c_str(), &fileData[ 0 ], fileSize );
        if ( bytesRead != fileSize )
        {
            return false;
        }

        return ( xmlDoc.Parse( fileData.data(), fileSize ) == tinyxml2::XML_SUCCESS );
    }
#endif // USE_STEAM

    FILE* fp = nullptr;

#ifdef _WIN32
    errno_t err = _wfopen_s( &fp, filename.c_str(), L"rb" );
    if ( err != 0 )
    {
        char errorMessage[ 256 ];
        strerror_s( errorMessage, 256, err );
        std::stringstream popupMessage;
        popupMessage << "Couldn't open file: " << errorMessage;
        g_pGame->GetPopup()->Show( PopupMode::Ok, popupMessage.str() );
        return false;
    }
#else
    fp = fopen( filename.c_str(), "rb" );
    if ( fp == nullptr )
    {
        std::stringstream popupMessage;
        popupMessage << "Couldn't open file: " << errno;
        g_pGame->GetPopup()->Show( PopupMode::Ok, popupMessage.str() );
        return false;
    }
#endif

    tinyxml2::XMLError xmlError = xmlDoc.LoadFile( fp );
    if ( xmlError != tinyxml2::XMLError::XML_SUCCESS )
    {
        std::stringstream ss;
        ss << "Load failed: TinyXML error code " << static_cast<unsigned int>( xmlError );
        g_pGame->RaiseInteractiveWarning( ss.str() );
        return false;
    }

    fclose( fp );
    return true;
}

void SaveGameStorage::UpdateStorageFiles( const std::filesystem::path& filename, tinyxml2::XMLDocument& xmlDoc )
{
    std::shared_ptr<StorageFile> pActiveStorageFile = nullptr;
    for ( auto& pStorageFile : m_StorageFiles )
    {
        if ( pStorageFile->filename == filename )
        {
            pActiveStorageFile = pStorageFile;
            break;
        }
    }

    if ( pActiveStorageFile == nullptr )
    {
        pActiveStorageFile = std::make_shared<StorageFile>();
        pActiveStorageFile->filename = filename;
        pActiveStorageFile->pSaveGameHeader = std::make_unique<SaveGameHeader>( filename );
        m_StorageFiles.push_back( pActiveStorageFile );
    }

    SDL_assert( pActiveStorageFile->pSaveGameHeader != nullptr );
    pActiveStorageFile->pSaveGameHeader->Read( xmlDoc );
}

void SaveGameStorage::CreateSaveGameData( tinyxml2::XMLDocument& xmlDoc, bool killSave )
{
    using namespace tinyxml2;
    XMLElement* pRootElement = xmlDoc.NewElement( "Hexterminate" );
    xmlDoc.InsertFirstChild( pRootElement );

    bool result = true;
    Xml::Write( xmlDoc, pRootElement, "Alive", !killSave );
    Xml::Write( xmlDoc, pRootElement, "Difficulty", ToString( g_pGame->GetDifficulty() ) );
    Xml::Write( xmlDoc, pRootElement, "GameMode", ToString( g_pGame->GetGameMode() ) );
    result &= g_pGame->GetPlayer()->Write( xmlDoc, pRootElement );
    result &= g_pGame->GetBlackboard()->Write( xmlDoc, pRootElement );
    result &= g_pGame->GetGalaxy()->Write( xmlDoc, pRootElement );

    XMLElement* pFactionsElement = xmlDoc.NewElement( "Factions" );
    pRootElement->LinkEndChild( pFactionsElement );

    for ( int i = 0; i < static_cast<int>( FactionId::Count ); ++i )
    {
        g_pGame->GetFaction( static_cast<FactionId>( i ) )->Write( xmlDoc, pFactionsElement );
    }

    SDL_assert( result );
}

std::filesystem::path SaveGameStorage::GetSaveGameFileName() const
{
    SDL_assert( g_pGame->GetPlayer() != nullptr );

    const std::string& captainName = g_pGame->GetPlayer()->GetShipCustomisationData().m_CaptainName;
    const std::string& shipName = g_pGame->GetPlayer()->GetShipCustomisationData().m_ShipName;

    return GetSaveGameFileName( captainName, shipName );
}

std::filesystem::path SaveGameStorage::GetSaveGameFileName( const std::string& captainName, const std::string& shipName ) const
{
    std::stringstream ss;
    ss << captainName << " - " << shipName << ".xml";
    std::string filename( ss.str() );

    // These characters aren't supported by the operative system. Having them as part of the filename would
    // cause the save game to fail, so we replace them with a '_'
    const std::string invalidCharacters( "\\/*?\"<>|" );
    const size_t filenameLength = filename.length();
    const size_t invalidCharactersLength = invalidCharacters.length();
    for ( size_t i = 0; i < filenameLength; ++i )
    {
        for ( size_t j = 0; j < invalidCharactersLength; ++j )
        {
            if ( filename[ i ] == invalidCharacters[ j ] )
            {
                filename[ i ] = '_';
            }
        }
    }

    // Make the filename lower case. This makes the behaviour consistent between Windows, Linux and Steam.
    // We wrap the call of tolower in a lambda as otherwise we'd trigger a conversion form int to char warning.
    std::transform( filename.begin(), filename.end(), filename.begin(), []( char c ) { return static_cast<char>( std::tolower( c ) ); } );

    return filename;
}

bool SaveGameStorage::CreateSaveGameFolder( const std::filesystem::path& folder )
{
    std::error_code errorCode;
    bool state = std::filesystem::create_directories( folder, errorCode );
    if ( state == false )
    {
        if ( errorCode.value() == 0 ) // It can fail because the directory already exists... and return a success error code.
        {
            return true;
        }
        else
        {
            Genesis::FrameWork::GetLogger()->LogWarning( "Could not create save game directory: %s.", errorCode.message().c_str() );
            return false;
        }
    }

    return state;
}

bool SaveGameStorage::Exists( const std::string& captainName, const std::string& shipName ) const
{
    while ( IsReady() == false )
        ; // Sorry.

    for ( auto& pStorageFile : m_StorageFiles )
    {
        SaveGameHeader* pSaveGameHeader = pStorageFile->pSaveGameHeader.get();
        SDL_assert( pSaveGameHeader != nullptr );
        if ( pSaveGameHeader->GetCaptainName() == captainName && pSaveGameHeader->GetShipName() == shipName )
        {
            return true;
        }
    }

    return false;
}

void SaveGameStorage::GetSaveGameHeaders( SaveGameHeaderVector& saveGameHeaders ) const
{
    saveGameHeaders.reserve( m_StorageFiles.size() );
    for ( auto& pStorageFile : m_StorageFiles )
    {
        if ( pStorageFile->pSaveGameHeader != nullptr )
        {
            saveGameHeaders.push_back( pStorageFile->pSaveGameHeader );
        }
    }
}

bool SaveGameStorage::IsReady() const
{
    return m_Ready.load();
}

} // namespace Hexterminate