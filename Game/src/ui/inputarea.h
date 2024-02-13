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

#include "ui/panel.h"

namespace Genesis::Gui
{
class InputArea;
}

namespace Hexterminate::UI
{

class InputArea : public Panel
{
public:
    InputArea( const std::string& name );

    virtual void Update() override;
    virtual void RenderProperties() override;
    virtual void SetSize( int width, int height ) override;

    const std::string& GetText() const;
    void SetText( const std::string& text );

protected:
    virtual void SaveProperties( json& properties ) override;
    virtual void LoadProperties( const json& properties ) override;

private:
    Genesis::Gui::InputArea* m_pInputArea;
    std::string m_DefaultText;
};

} // namespace Hexterminate::UI
