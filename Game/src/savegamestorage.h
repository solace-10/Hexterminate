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

#pragma once

#include <atomic>
#include <filesystem>

namespace Hexterminate
{

class SaveGameHeader;
using SaveGameHeaderVector = std::vector<std::shared_ptr<SaveGameHeader>>;
GENESIS_DECLARE_SMART_PTR( SaveGameHeader );

class SaveGameStorage
{
public:
    SaveGameStorage();
    void UpdateDebugUI();
    bool SaveGame( bool killSave = false );
    bool LoadGame( const std::filesystem::path& filename, tinyxml2::XMLDocument& xmlDoc );

    bool Exists( const std::string& captainName, const std::string& shipName ) const;
    void GetSaveGameHeaders( SaveGameHeaderVector& saveGameHeaders ) const;
    bool IsReady() const;

private:
    std::filesystem::path GetSaveGameFileName() const;
    std::filesystem::path GetSaveGameFileName( const std::string& captainName, const std::string& shipName ) const;
    static int sXmlReadThreadMain( void* pData );
    bool XmlRead( const std::filesystem::path& filename, tinyxml2::XMLDocument& xmlDoc );
    bool CreateSaveGameFolder( const std::filesystem::path& folder );
    void CreateSaveGameData( tinyxml2::XMLDocument& xmlDoc, bool killSave );
    bool SaveToLocalStorage( tinyxml2::XMLDocument& xmlDoc, const std::filesystem::path& fullPath );
    bool SaveToRemoteStorage( tinyxml2::XMLDocument& xmlDoc );
    void UpdateStorageFiles( const std::filesystem::path& filename, tinyxml2::XMLDocument& xmlDoc );

#if USE_STEAM
    ISteamRemoteStorage* m_pSteamRemoteStorage;
#endif

    struct StorageFile
    {
        std::filesystem::path filename;
        SaveGameHeaderSharedPtr pSaveGameHeader;
    };
    using StorageFileList = std::list<std::shared_ptr<StorageFile>>;
    StorageFileList m_StorageFiles;
    bool m_DebugWindowOpen;
    bool m_CloudStorageActive;
    SDL_Thread* m_pXmlReadThread;
    std::atomic_bool m_Ready;
};

} // namespace Hexterminate