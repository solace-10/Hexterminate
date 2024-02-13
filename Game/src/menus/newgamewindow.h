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

#pragma once

#include <any>
#include <vector>

#include "sector/galaxycreationinfo.h"
#include "ship/ship.h"
#include "ui/types.fwd.h"
#include "ui/window.h"

namespace Hexterminate
{

class NewGameWindow : public UI::Window
{
public:
    NewGameWindow();

    virtual void Update() override;
    virtual void Reset() override;

private:
    enum class PageId
    {
        GameMode,
        ShipSelection,
        Customisation,
        FactionPresence,
        StartNewGame,
        Count
    };

    void OnPageSwitchButtonPressed( const std::any& userData );
    void OnGameModeButtonPressed( const std::any& userData );
    void OnShipButtonPressed( const std::any& userData );

    void Select( PageId pageId );

    void CreateGameModeSelectionPage();
    void CreateShipSelectionPage();
    void CreateCustomisationPage();
    void CreateFactionPresencePage();

    UI::ElementSharedPtr CreateGameModeInfoPanel( const std::string& name ) const;
    UI::ElementSharedPtr CreateShipInfoPanel( const std::string& name ) const;
    UI::ElementSharedPtr CreateShipDetailsPanel();
    UI::ElementSharedPtr CreateDifficultyPanel();
    UI::ElementSharedPtr CreateFactionPresencePanel( FactionId factionId );
    UI::ElementSharedPtr CreatePreferencesPanel();

    void StartNewGame();
    void SetFactionPresence( FactionId factionId, FactionPresence presence );

    std::vector<UI::ElementSharedPtr> m_Pages;
    PageId m_CurrentPage;

    UI::ToggleGroupSharedPtr m_pGameModeToggleGroup;
    UI::ToggleGroupSharedPtr m_pShipToggleGroup;

    UI::ButtonSharedPtr m_pButtonNext;
    UI::ButtonSharedPtr m_pButtonPrevious;

    ShipCustomisationData m_ShipCustomisationData;
    std::string m_CompanionShipTemplate;
    GalaxyCreationInfo m_GalaxyCreationInfo;

    UI::InputAreaSharedPtr m_pShipNameInputArea;
    UI::InputAreaSharedPtr m_pCaptainNameInputArea;
    UI::RadioButtonSharedPtr m_pNormalDifficultyRadioButton;
    UI::CheckboxSharedPtr m_pTipsCheckbox;
};

} // namespace Hexterminate
