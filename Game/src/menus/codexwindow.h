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

#include <filesystem>
#include <map>
#include <memory>
#include <vector>

#include "ui/types.fwd.h"
#include "ui/window.h"

namespace Hexterminate
{

class CodexCategory;
class CodexEntry;
using CodexEntryUniquePtr = std::unique_ptr<CodexEntry>;
using CodexEntryVector = std::vector<CodexEntry*>;

///////////////////////////////////////////////////////////////////////////////
// CodexWindow
///////////////////////////////////////////////////////////////////////////////

class CodexWindow : public UI::Window
{
public:
    CodexWindow();

    void ShowEntry( const std::string& content );

private:
    void LoadEntries();
    void LoadEntry( const std::filesystem::path& path );
    void RefreshUI();
    void AddEntry( const std::string& category, const std::string& name, const std::string& content );
    CodexCategory& GetCategory( const std::string& name );

    using Categories = std::map<std::string, CodexCategory>;
    Categories m_Categories;

    UI::PanelSharedPtr m_pCategoriesPanel;
    UI::PanelSharedPtr m_pContentPanel;

    UI::ScrollingElementSharedPtr m_pCategoriesArea;

    UI::ImageSharedPtr m_pModuleWireframe;
    UI::TextSharedPtr m_pContentText;
    UI::ToggleGroupSharedPtr m_pEntryToggleGroup;
};

///////////////////////////////////////////////////////////////////////////////
// CodexCategory
///////////////////////////////////////////////////////////////////////////////

class CodexCategory
{
public:
    CodexCategory();
    CodexCategory( const std::string& name, UI::Element* pParent );
    UI::Button* GetButton() const;
    void AddEntry( const std::string& name, const std::string& content, UI::Element* pParent, CodexWindow* pCodexWindow, UI::ToggleGroupSharedPtr pEntryToggleGroup );
    CodexEntryVector GetEntries();

private:
    UI::ButtonSharedPtr m_pButton;

    using Entries = std::map<std::string, CodexEntryUniquePtr>;
    Entries m_Entries;
};

///////////////////////////////////////////////////////////////////////////////
// CodexEntry
///////////////////////////////////////////////////////////////////////////////

class CodexEntry
{
public:
    CodexEntry();
    CodexEntry( const std::string& name, const std::string& content, UI::Element* pParent, CodexWindow* pCodexWindow, UI::ToggleGroupSharedPtr pEntryToggleGroup );
    UI::Button* GetButton() const;

private:
    void OnButtonPressed();

    UI::ButtonSharedPtr m_pButton;
    std::string m_Content;
    CodexWindow* m_pCodexWindow;
};

} // namespace Hexterminate
