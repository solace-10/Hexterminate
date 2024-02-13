// Copyright 2015 Pedro Nunes
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

#include "misc/mathaux.h"
#include <gui/gui.h>
#include <list>

namespace Hexterminate
{

class TutorialNextButton;
class PointOfInterest;
class PointOfInterestTarget;

///////////////////////////////////////////////////////////////////////////////
// TutorialStep
///////////////////////////////////////////////////////////////////////////////

class TutorialStep
{
public:
    TutorialStep( const std::string& content );

    void SetPointOfInterest( const glm::vec2& position );
    void SetPointOfInterest( PointOfInterestTarget* pPointOfInterest );

    bool HasPointOfInterest() const;
    const glm::vec2& GetPointOfInterest() const;

    const std::string& GetContent() const;

private:
    std::string m_Content;
    bool m_HasPointOfInterest;
    glm::vec2 m_PointOfInterest;
    PointOfInterestTarget* m_pPointOfInterest;
};

inline void TutorialStep::SetPointOfInterest( const glm::vec2& position )
{
    m_pPointOfInterest = nullptr;
    m_PointOfInterest = position;
    m_HasPointOfInterest = true;
}

inline void TutorialStep::SetPointOfInterest( PointOfInterestTarget* pPointOfInterest )
{
    m_pPointOfInterest = pPointOfInterest;
    m_HasPointOfInterest = ( pPointOfInterest != nullptr );
}

inline bool TutorialStep::HasPointOfInterest() const
{
    return m_HasPointOfInterest;
}

inline const std::string& TutorialStep::GetContent() const
{
    return m_Content;
}

typedef std::list<TutorialStep> TutorialStepList;

///////////////////////////////////////////////////////////////////////////////
// TutorialWindow
///////////////////////////////////////////////////////////////////////////////

class TutorialWindow
{
public:
    TutorialWindow();
    ~TutorialWindow();
    void Update( float delta );

    bool IsActive() const;
    void AddStep( const TutorialStep& step );
    void NextStep();
    void Clear();

private:
    void SetContent( const std::string& content );

    Genesis::Gui::Panel* m_pMainPanel;
    Genesis::Gui::Text* m_pContent;
    Genesis::Gui::Text* m_pTitle;
    Genesis::Gui::Image* m_pIcon;
    TutorialNextButton* m_pNextButton;
    PointOfInterest* m_pPointOfInterest;

    TutorialStepList m_Steps;

    int m_LineLength;
    bool m_Dirty;

    Genesis::ResourceSound* m_pSFX;
    float m_Delay;
};

inline bool TutorialWindow::IsActive() const
{
    return ( m_Steps.empty() == false );
}

///////////////////////////////////////////////////////////////////////////////
// TutorialNextButton
///////////////////////////////////////////////////////////////////////////////

class TutorialNextButton : public Genesis::Gui::Button
{
public:
    TutorialNextButton( TutorialWindow* pTutorialWindow );
    virtual void OnPress();

private:
    TutorialWindow* m_pTutorialWindow;
};

} // namespace Hexterminate