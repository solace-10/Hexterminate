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

#include "ui/element.h"

namespace Genesis
{
class ResourceImage;

namespace Gui
{
    class Image;
} // namespace Gui
} // namespace Genesis

namespace Hexterminate::UI
{

class Image : public Element
{
public:
    enum class BlendMode
    {
        Disabled,
        Blend,
        Add,
        Multiply,
        Screen
    };

    Image( const std::string& name, const std::string& path = "" );
    virtual ~Image() override;

    virtual void RenderProperties() override;
    virtual bool IsResizeable() const override;
    virtual void SetSize( int width, int height ) override;

    void SetAutoSize( bool autoSize );
    void SetBlendMode( BlendMode blendMode );
    void SetColour( float r, float g, float b, float a );
    void SetPath( const std::string& filename );
    void SetShader( const std::string& shaderName );

protected:
    virtual void SaveProperties( json& properties ) override;
    virtual void LoadProperties( const json& properties ) override;
    void LoadResources();

    Genesis::Gui::Image* m_pImage;
    Genesis::ResourceImage* m_pResource;
    std::string m_Path;
    bool m_AutoSize;
};

inline void Image::SetAutoSize( bool autoSize )
{
    m_AutoSize = autoSize;
}

inline bool Image::IsResizeable() const
{
    return false;
}

} // namespace Hexterminate::UI
