#include "misc/gui.h"
#include "menus/eva.h"
#include <genesis.h>
#include <gui/gui.h>

namespace Hexterminate
{

namespace GuiExtended
{

    Genesis::Gui::Text* CreateText( int x, int y, int w, int h, const std::string& text, Genesis::Gui::GuiElement* pParent, bool bold /* = false */, bool multiline /* = false */ )
    {
        Genesis::Gui::Text* pTextElement = new Genesis::Gui::Text();
        pTextElement->SetSize( w, h );
        pTextElement->SetPosition( x, y );
        pTextElement->SetColour( EVA_TEXT_COLOUR );
        pTextElement->SetFont( bold ? EVA_FONT_BOLD : EVA_FONT );
        pTextElement->SetMultiLine( multiline );
        pTextElement->SetText( text );
        AddToParent( pTextElement, pParent );
        return pTextElement;
    }

    void AddToParent( Genesis::Gui::GuiElement* pElement, Genesis::Gui::GuiElement* pParent )
    {
        if ( pElement == nullptr )
        {
            return;
        }

        if ( pParent == nullptr )
        {
            Genesis::FrameWork::GetGuiManager()->AddElement( pElement );
        }
        else
        {
            pParent->AddElement( pElement );
        }
    }

} // namespace GuiExtended

} // namespace Hexterminate