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

#include "fleet/fleet.fwd.h"
#include "globals.h"
#include "menus/detailswindow.h"
#include "sector/sectorinfo.h"
#include <gui/gui.h>

namespace Hexterminate
{

class SectorInfo;
class DeployProbeButton;
class DeployStarfortButton;
class EnterSectorButton;
class Fleet;
class PointOfInterest;

///////////////////////////////////////////////////////////////////////////////
// SectorDetails
// Window that shows up in the Galaxy screen, when the player's fleet is
// stopped on a sector, providing information about the sector's contents and
// a button to enter the sector.
///////////////////////////////////////////////////////////////////////////////

class SectorDetails : public DetailsWindow
{
public:
    SectorDetails();
    virtual ~SectorDetails();
    void SetSectorInfo( SectorInfo* pSectorInfo );
    SectorInfo* GetSectorInfo() const;

    virtual void SetAnchor( float x, float y ) override;
    virtual void Show( bool state ) override;

private:
    Genesis::Color GetThreatRatingColour( ThreatRating rating ) const;
    std::string GetThreatRatingText( ThreatRating rating ) const;

    SectorInfo* m_pSectorInfo;
    Genesis::Gui::Text* m_pTitle;
    DeployProbeButton* m_pDeployProbeButton;
    DeployStarfortButton* m_pDeployStarfortButton;
    EnterSectorButton* m_pEnterSectorButton;
    Genesis::Gui::Panel* m_pThreatBackground;
    PointOfInterest* m_pPointOfInterest;
};

///////////////////////////////////////////////////////////////////////////////
// GenericSectorButton
///////////////////////////////////////////////////////////////////////////////

class GenericSectorButton : public Genesis::Gui::Button
{
public:
    GenericSectorButton();
    void SetSectorInfo( SectorInfo* pSectorInfo );

protected:
    SectorInfo* m_pSectorInfo;
};

///////////////////////////////////////////////////////////////////////////////
// DeployProbeButton
///////////////////////////////////////////////////////////////////////////////

class DeployProbeButton : public GenericSectorButton
{
public:
    DeployProbeButton();
    virtual void Update( float delta ) override;
    virtual void OnPress() override;

private:
    bool m_CanDeploy;
    int m_Cost;
};

///////////////////////////////////////////////////////////////////////////////
// DeployStarfortButton
///////////////////////////////////////////////////////////////////////////////

class DeployStarfortButton : public GenericSectorButton
{
public:
    DeployStarfortButton();
    virtual void Update( float delta ) override;
    virtual void OnPress() override;

private:
    bool m_CanDeploy;
    int m_Cost;
};

///////////////////////////////////////////////////////////////////////////////
// EnterSectorButton
///////////////////////////////////////////////////////////////////////////////

class EnterSectorButton : public GenericSectorButton
{
public:
    EnterSectorButton();
    virtual void OnPress() override;
};

} // namespace Hexterminate