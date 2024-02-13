#include "menus/detailswindow.h"
#include "faction/faction.h"
#include "hexterminate.h"
#include "menus/eva.h"
#include <genesis.h>

namespace Hexterminate
{

DetailsWindow::DetailsWindow()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();

    m_AnchorX = m_AnchorY = 0.0f;
    m_NumLines = 1;

    m_pMainPanel = new Genesis::Gui::Panel();
    m_pMainPanel->SetSize( 256.0f, 16.0f );
    m_pMainPanel->SetPosition( 0.0f, 0.0f );
    m_pMainPanel->SetColour( 0.0f, 0.0f, 0.0f, 0.75f );
    m_pMainPanel->SetBorderColour( 1.0f, 1.0f, 1.0f, 1.0f );
    m_pMainPanel->SetBorderMode( Genesis::Gui::PANEL_BORDER_NONE );
    pGuiManager->AddElement( m_pMainPanel );

    m_pText = new Genesis::Gui::Text();
    m_pText->SetSize( 256.0f, 16.0f );
    m_pText->SetPosition( 8.0f, 2.0f );
    m_pText->SetColour( EVA_TEXT_COLOUR );
    m_pText->SetFont( EVA_FONT );
    m_pText->SetText( "" );
    m_pMainPanel->AddElement( m_pText );
}

DetailsWindow::~DetailsWindow()
{
    Genesis::Gui::GuiManager* pGuiManager = Genesis::FrameWork::GetGuiManager();
    if ( pGuiManager != nullptr )
    {
        pGuiManager->RemoveElement( m_pMainPanel );
    }
}

void DetailsWindow::Update( float delta )
{
    m_pMainPanel->SetPosition( m_AnchorX, m_AnchorY );
}

void DetailsWindow::Show( bool state )
{
    m_pMainPanel->Show( state );
}

void DetailsWindow::SetContents( const std::string& contents )
{
    m_NumLines = 1;
    size_t len = contents.length();
    int lineLength = 0;
    int maxLineLength = 0;
    for ( size_t i = 0; i < len; ++i )
    {
        if ( contents[ i ] == '\n' )
        {
            if ( lineLength > maxLineLength )
            {
                maxLineLength = lineLength;
            }
            lineLength = 0;
            m_NumLines++;
        }
        else
        {
            lineLength++;
        }
    }

    float textSizeX = UsesDynamicLineLength() ? ( maxLineLength * 8.0f ) : 256.0f;
    float textSizeY = m_pText->GetFont()->GetLineHeight() * m_NumLines;

    m_pText->SetSize( textSizeX, textSizeY );
    m_pText->SetText( contents );

    m_pMainPanel->SetSize( textSizeX + 4.0f, textSizeY + 2.0f );
}

void DetailsWindow::SetAnchor( float x, float y )
{
    m_AnchorX = floor( x );
    m_AnchorY = floor( y );
}

} // namespace Hexterminate