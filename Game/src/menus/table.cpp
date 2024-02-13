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

#include <cassert>

#include <genesis.h>
#include <gui/gui.h>
#include <rendersystem.h>
#include <shader.h>
#include <shadercache.h>
#include <vertexbuffer.h>

#include "menus/eva.h"
#include "menus/table.h"
#include "menus/tablerow.h"
#include <inputmanager.h>

namespace Hexterminate
{

#define TABLE_COLOUR_ROW_1 glm::vec4( 1.00f, 1.00f, 1.00f, 0.05f )
#define TABLE_COLOUR_ROW_2 glm::vec4( 0.00f, 0.00f, 0.00f, 0.20f )

Table::Table()
    : m_PositionsDirty( false )
    , m_ContentsDirty( false )
    , m_MousePressedToken( Genesis::InputManager::sInvalidInputCallbackToken )
    , m_pShader( nullptr )
    , m_pVertexBuffer( nullptr )
{
    using namespace Genesis;

    ResourceFont* pFont = EVA_FONT;
    m_RowHeight = ceilf( pFont->GetLineHeight() * 1.5f );
    SDL_assert( m_RowHeight > 0.0f );

    m_MousePressedToken = FrameWork::GetInputManager()->AddMouseCallback( std::bind( &Table::OnMousePressedCallback, this ), MouseButton::Left, ButtonState::Pressed );

    m_pShader = FrameWork::GetRenderSystem()->GetShaderCache()->Load( "untextured_vertex_coloured" );
    m_pVertexBuffer = new VertexBuffer( GeometryType::Triangle, VBO_POSITION | VBO_COLOUR );
}

Table::~Table()
{
    for ( auto& pRow : m_Rows )
    {
        delete pRow;
    }

    Genesis::FrameWork::GetInputManager()->RemoveMouseCallback( m_MousePressedToken );

    delete m_pVertexBuffer;
}

void Table::Render()
{
    using namespace Genesis;

    if ( m_ContentsDirty )
    {
        UpdateContents();
    }

    if ( m_PositionsDirty )
    {
        UpdatePositions();
    }

    size_t maxRows = static_cast<size_t>( mSize.y / m_RowHeight );
    glm::vec2 pos = GetPositionAbsolute();
    glm::vec4 colour[ 2 ] = { TABLE_COLOUR_ROW_1, TABLE_COLOUR_ROW_2 };

    PositionData posData;
    ColourData colourData;

    const size_t numVertices = maxRows * 6;
    posData.reserve( numVertices );
    colourData.reserve( numVertices );

    for ( size_t i = 0; i < maxRows; ++i )
    {
        const float y1 = pos.y + i * m_RowHeight;
        const float y2 = y1 + m_RowHeight;
        posData.emplace_back( pos.x, y1, 0.0f ); // 0
        posData.emplace_back( pos.x, y2, 0.0f ); // 1
        posData.emplace_back( pos.x + mSize.x, y2, 0.0f ); // 2
        posData.emplace_back( pos.x, y1, 0.0f ); // 0
        posData.emplace_back( pos.x + mSize.x, y2, 0.0f ); // 2
        posData.emplace_back( pos.x + mSize.x, y1, 0.0f ); // 3

        const glm::vec4& rowColour = colour[ i % 2 ];
        for ( int j = 0; j < 6; ++j )
        {
            colourData.push_back( rowColour );
        }
    }

    m_pVertexBuffer->CopyPositions( posData );
    m_pVertexBuffer->CopyColours( colourData );
    m_pShader->Use();
    m_pVertexBuffer->Draw( static_cast<uint32_t>( numVertices ) );

    Gui::GuiElement::Render();
}

size_t Table::AddRow( TableRow* pRow )
{
    // Make sure we aren't trying to add a row with the incorrect number of elements
    if ( !m_Dimensions.empty() )
    {
        SDL_assert( pRow->GetContents().size() == m_Dimensions.size() );
    }

    // Length of the columns may need to be updated if one of the new strings is too long
    bool dimensionsChanged = UpdateDimensions( pRow );

    dimensionsChanged = true;

    m_Rows.push_back( pRow );

    // Don't update the positions immediately, wait until the next time we need to render as we
    // are likely doing consecutive AddRows()
    if ( dimensionsChanged )
        m_PositionsDirty = true;

    m_ContentsDirty = true;

    return m_Rows.size();
}

bool Table::UpdateDimensions( TableRow* pRow )
{
    const TableRowContents& rowContents = pRow->GetContents();
    if ( rowContents.size() > m_Dimensions.size() )
        m_Dimensions.resize( rowContents.size(), 0 );

    bool changed = false;

    Genesis::ResourceFont* pFont = pRow->GetFont();
    for ( unsigned int i = 0; i < rowContents.size(); ++i )
    {
        float length = pFont->GetTextLength( rowContents.at( i ) );
        if ( m_Dimensions[ i ] < length )
        {
            m_Dimensions[ i ] = length;
            changed = true;
        }
    }
    return changed;
}

void Table::UpdatePositions()
{
    Genesis::ResourceFont* pFont = EVA_FONT;
    float verticalAlignment = ( m_RowHeight - pFont->GetLineHeight() ) / 2.0f;
    float baseOffsetX = 12.0f;
    float gapX = 24.0f;
    float offsetX = baseOffsetX;
    float offsetY = 0.0f;
    for ( unsigned int i = 0; i < m_Text.size(); ++i )
    {
        int j = i % m_Dimensions.size();
        if ( i > 0 && j == 0 )
        {
            offsetX = baseOffsetX;
            offsetY += m_RowHeight;
        }

        m_Text[ i ]->SetPosition( offsetX, floor( offsetY + verticalAlignment ) );
        m_Text[ i ]->SetSize( m_Dimensions[ j ], m_RowHeight );
        offsetX += m_Dimensions[ j ] + gapX;
    }

    m_PositionsDirty = false;
}

void Table::UpdateContents()
{
    if ( !m_ContentsDirty )
        return;

    for ( auto& pChild : mChildren )
    {
        delete pChild;
    }

    mChildren.clear();
    m_Text.clear();
    m_Dimensions.clear();

    for ( TableRow* pRow : m_Rows )
    {
        for ( const std::string& text : pRow->GetContents() )
        {
            Genesis::Gui::Text* pTextElement = new Genesis::Gui::Text();
            pTextElement->SetMultiLine( false );
            pTextElement->SetSize( 256, 32 );
            pTextElement->SetPosition( 8.0f, 8.0f );
            pTextElement->SetColour( pRow->GetColour() );
            pTextElement->SetFont( pRow->GetFont() );
            pTextElement->SetText( text );
            m_Text.push_back( pTextElement );
            AddElement( pTextElement );
        }

        UpdateDimensions( pRow );
    }

    m_ContentsDirty = false;
}

void Table::Update( float delta )
{
    using namespace Genesis;

    // Prevents selection of a row which is not currently visible due to the table contents
    // having been clipped or a parent element hidden. This is particularly important when
    // a table is a child of a scrolling element.
    const glm::vec2& mousePos = FrameWork::GetInputManager()->GetMousePosition();
    for ( Gui::GuiElement* pParentElement = this; pParentElement != nullptr; pParentElement = pParentElement->GetParent() )
    {
        if ( pParentElement->IsVisible() == false )
            return;

        const glm::vec2& parentAbsPos = pParentElement->GetPositionAbsolute();
        if ( mousePos.x < parentAbsPos.x || mousePos.x > parentAbsPos.x + pParentElement->GetSize().x )
            return;
        else if ( mousePos.y < parentAbsPos.y || mousePos.y > parentAbsPos.y + pParentElement->GetSize().y )
            return;
    }

    const glm::vec2& tableAbsPos = GetPositionAbsolute();
    int rowIndex = (int)( ( mousePos.y - tableAbsPos.y ) / GetRowHeight() );
    if ( rowIndex > 0 && rowIndex < (int)GetRowCount() )
    {
        TableRow* pRow = GetRow( rowIndex );
        if ( pRow != nullptr )
        {
            pRow->OnHover();
        }
    }
}

void Table::OnMousePressedCallback()
{
    using namespace Genesis;

    // Prevents selection of a row which is not currently visible due to the table contents
    // having been clipped or a parent element hidden. This is particularly important when
    // a table is a child of a scrolling element.
    const glm::vec2& mousePos = FrameWork::GetInputManager()->GetMousePosition();
    for ( Gui::GuiElement* pParentElement = this; pParentElement != nullptr; pParentElement = pParentElement->GetParent() )
    {
        if ( pParentElement->IsVisible() == false )
            return;

        const glm::vec2& parentAbsPos = pParentElement->GetPositionAbsolute();
        if ( mousePos.x < parentAbsPos.x || mousePos.x > parentAbsPos.x + pParentElement->GetSize().x )
            return;
        else if ( mousePos.y < parentAbsPos.y || mousePos.y > parentAbsPos.y + pParentElement->GetSize().y )
            return;
    }

    const glm::vec2& tableAbsPos = GetPositionAbsolute();
    int rowIndex = (int)( ( mousePos.y - tableAbsPos.y ) / GetRowHeight() );
    if ( rowIndex > 0 && rowIndex < (int)GetRowCount() )
    {
        TableRow* pRow = GetRow( rowIndex );
        if ( pRow != nullptr )
        {
            pRow->OnPress();
        }
    }
}

} // namespace Hexterminate