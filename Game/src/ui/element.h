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

#include <list>
#include <memory>
#include <string>

#include <glm/vec2.hpp>

#include "json.hpp"
#include "ui/types.fwd.h"

using json = nlohmann::json;

namespace Genesis::Gui
{
class Panel;
}

namespace Hexterminate::UI
{

using ElementList = std::list<ElementSharedPtr>;

// clang-format off
enum ElementFlags : uint32_t
{
    ElementFlags_None            = 0,
    ElementFlags_DynamicPosition = 1,
    ElementFlags_DynamicSize     = 1 << 1,
    ElementFlags_NoSerialize     = 1 << 2
};
// clang-format on

class Element
{
public:
    Element( const std::string& name );
    virtual ~Element();

    virtual void Update();
    virtual void RenderProperties();

    virtual bool IsResizeable() const;
    virtual void SetSize( const glm::ivec2& size );
    virtual void SetSize( int width, int height );
    virtual void SetPosition( const glm::ivec2& position );
    virtual void SetPosition( int x, int y );
    int GetWidth() const;
    int GetHeight() const;
    glm::ivec2 GetSize() const;
    glm::ivec2 GetPosition() const;
    void GetPosition( int& x, int& y );
    bool IsHovered() const;
    bool IsAcceptingInput() const;
    bool IsPopupElement() const;
    void SetPopupElement( bool isPopupElement );
    bool IsEditable() const;
    void SetEditable( bool isEditable );
    bool IsDynamic() const;
    void SetDynamic( bool isDynamic );
    uint32_t GetFlags() const;
    bool HasFlag( ElementFlags flag ) const;
    void SetFlags( uint32_t flags );
    bool IsVisible() const;

    void SetParent( Element* pParentElement );
    Element* GetParent() const;

    virtual void Add( ElementSharedPtr pElement );
    virtual void Remove( ElementSharedPtr pElement );
    const ElementList& GetChildren() const;
    const std::string& GetName() const;
    virtual void Show( bool state );

    // This shouldn't be exposed, but it is necessary until all the
    // Genesis::Gui elements have been ported to the new system.
    Genesis::Gui::Panel* GetPanel() const;

    virtual Design* GetDesign() const;

protected:
    virtual void SaveProperties( json& properties );
    virtual void LoadProperties( const json& properties );

    const std::string& GetPath() const;
    bool IsPathResolved() const;
    void ResolvePath( Element* pParentElement );
    void LoadFromDesign( Design* pDesign );
    void SaveInternal( json& data, bool saveProperties = true );

    void InputIntExt( const char* label, int* v, bool isReadOnly = false );

private:
    std::string m_Name;
    std::string m_Path;
    Genesis::Gui::Panel* m_pPanel;
    Design* m_pDesign;
    Element* m_pParent;
    ElementList m_Children;
    ElementList m_ChildrenToRemove;

    bool m_IsPopupElement;
    bool m_IsEditable;
    bool m_IsDynamic;
    bool m_AnchorTop;
    bool m_AnchorLeft;
    bool m_AnchorBottom;
    bool m_AnchorRight;
    int m_PaddingRight;
    int m_PaddingBottom;
    uint32_t m_Flags;
};

inline bool Element::IsResizeable() const
{
    return true;
}

inline const std::string& Element::GetName() const
{
    return m_Name;
}

inline const std::string& Element::GetPath() const
{
    return m_Path;
}

inline bool Element::IsPathResolved() const
{
    return m_Path.empty() == false;
}

inline Genesis::Gui::Panel* Element::GetPanel() const
{
    return m_pPanel;
}

inline const ElementList& Element::GetChildren() const
{
    return m_Children;
}

inline bool Element::IsPopupElement() const
{
    return m_IsPopupElement;
}

inline void Element::SetPopupElement( bool isPopupElement )
{
    m_IsPopupElement = isPopupElement;
}

inline bool Element::IsEditable() const
{
    return m_IsEditable;
}

inline void Element::SetEditable( bool state )
{
    m_IsEditable = state;
}

inline bool Element::IsDynamic() const
{
    return m_IsDynamic;
}

inline void Element::SetDynamic( bool state )
{
    m_IsDynamic = state;
}

inline uint32_t Element::GetFlags() const
{
    return m_Flags;
}

inline bool Element::HasFlag( ElementFlags flag ) const 
{
    return ( m_Flags & flag ) == flag;
}

inline void Element::SetFlags( uint32_t flags )
{
    m_Flags = flags;
}

inline Design* Element::GetDesign() const
{
    return m_pDesign;
}

inline void Element::SetParent( Element* pParentElement )
{
    m_pParent = pParentElement;
}

inline Element* Element::GetParent() const
{
    return m_pParent;
}

} // namespace Hexterminate::UI
