#include <configuration.h>
#include <genesis.h>
#include <imgui/imgui_impl.h>
#include <inputmanager.h>
#include <scene/scene.h>
#include <sound/soundmanager.h>
#include <window.h>

#include "hexterminate.h"
#include "player.h"
#include "sector/sector.h"
#include "sector/sectorcamera.h"
#include "ship/module.h"
#include "ship/ship.h"
#include "shipyard/shipyard.h"

namespace Hexterminate
{

static const float ZOOM_MINIMUM = 0.4f;
static const float ZOOM_MAXIMUM = 5.0f;

float clamp( float fv, float fmin, float fmax )
{
    if ( fv < fmin )
        return fmin;
    else if ( fv > fmax )
        return fmax;
    else
        return fv;
}

SectorCamera::SectorCamera()
    : m_ZoomMult( 2.75f )
    , m_ZoomMultTarget( m_ZoomMult )
    , m_Initialised( false )
{
    m_MouseWheelEventToken = Genesis::FrameWork::GetInputManager()->AddMouseWheelCallback( std::bind( &SectorCamera::OnMouseWheelCallback, this, std::placeholders::_1, std::placeholders::_2 ) );
}

SectorCamera::~SectorCamera()
{
    Genesis::FrameWork::GetInputManager()->RemoveMouseWheelCallback( m_MouseWheelEventToken );
}

void SectorCamera::Update( float delta )
{
    // Do not update the camera while imgui is open, as otherwise it will keep
    // following the cursor.
    if ( Genesis::ImGuiImpl::IsEnabled() )
    {
        return;
    }

    Genesis::Scene* pScene = Genesis::FrameWork::GetScene();
    const Player* pPlayer = g_pGame->GetPlayer();
    const Ship* pShip = ( pPlayer != nullptr ) ? pPlayer->GetShip() : nullptr;

    // Update our camera zooming. m_ZoomMultTarget is our target zoom, set by the
    // mouse wheel in OnMouseWheelCallback(). The actual zooming factor is m_ZoomMult,
    // which approaches the target every frame, providing a smooth camera movement.
    if ( pShip != nullptr )
    {
        if ( pShip->GetDockingState() == DockingState::Undocked )
        {
            m_ZoomMultTarget = clamp( m_ZoomMultTarget, ZOOM_MINIMUM, ZOOM_MAXIMUM );
        }
        else
        {
            m_ZoomMultTarget = 1.1f;
        }
    }

    if ( delta > fabs( m_ZoomMultTarget - m_ZoomMult ) )
        m_ZoomMult = m_ZoomMultTarget;
    else if ( m_ZoomMult > m_ZoomMultTarget + 0.01f )
        m_ZoomMult -= delta;
    else if ( m_ZoomMult < m_ZoomMultTarget - 0.01f )
        m_ZoomMult += delta;

    if ( pShip == nullptr )
    {
        // If there is no player ship, set the camera to the centre of the sector
        pScene->GetCamera()->SetPosition( 0.0f, 0.01f, 200.0f );
        pScene->GetCamera()->SetTargetPosition( 0.0f, 0.0f, 0.0f );
    }
    else if ( pShip->GetDockingState() == DockingState::Undocked )
    {
        // Otherwise, anchor the camera on the centre of the player ship but allow it to pan up to a
        // certain range away from it.
        glm::vec3 shipCentre = pShip->GetCentre( TransformSpace::World );
        Genesis::InputManager* inputManager = Genesis::FrameWork::GetInputManager();
        const glm::vec2& mousePosition = inputManager->GetMousePosition();

        glm::vec2 offset(
            ( mousePosition.x - Genesis::FrameWork::GetWindow()->GetWidth() / 2.0f ),
            ( -mousePosition.y + Genesis::FrameWork::GetWindow()->GetHeight() / 2.0f ) );
        float len = glm::length( offset );

        float offsetFactor = 0.0f;
        if ( len > 0.0f )
        {
            offset = glm::normalize( offset );
            offsetFactor = len / ( Genesis::FrameWork::GetWindow()->GetWidth() / 2.0f );
            if ( offsetFactor > 1.0f )
                offsetFactor = 1.0f;
        }

        // Scale the offset so that the ship always remains visible, even across multiple zoom settings.
        offset.x *= 100.0f * offsetFactor * m_ZoomMult;
        offset.y *= 100.0f * offsetFactor * m_ZoomMult;

        // Smoothen the camera's position
        float cameraEndX = shipCentre.x + offset.x;
        float cameraEndY = shipCentre.y + offset.y;

        Genesis::Camera* pCamera = pScene->GetCamera();

        const float interpolationFactor = clamp( m_Initialised ? delta : 1.0f, 0.0f, 1.0f );
        float cameraX = pCamera->GetPosition().x + ( cameraEndX - pCamera->GetPosition().x ) * interpolationFactor;
        float cameraY = pCamera->GetPosition().y + ( cameraEndY - pCamera->GetPosition().y ) * interpolationFactor;

        pCamera->SetPosition( cameraX, cameraY, 200.0f * m_ZoomMult );
        pCamera->SetTargetPosition( cameraX, cameraY, 0.0f );

        m_Initialised = true;
    }
    else if ( pShip->GetShipyard() != nullptr )
    {
        const glm::vec3& shipyardPosition = pShip->GetShipyard()->GetPosition();
        glm::vec3 cameraPosition = shipyardPosition - glm::vec3( 0.0f, -70.0f, 0.0f ) - pShip->GetCentreOfMass();

        pScene->GetCamera()->SetPosition( cameraPosition.x, cameraPosition.y, 220.0f * m_ZoomMult );
        pScene->GetCamera()->SetTargetPosition( cameraPosition.x, cameraPosition.y, 0.0f );
    }

    UpdateListener( delta );
    UpdateBorders();
}

void SectorCamera::UpdateListener( float delta )
{
    using namespace Genesis;

    const Scene* pScene = FrameWork::GetScene();
    const Camera* pCamera = pScene->GetCamera();

    const glm::vec3& position = pCamera->GetPosition();
    glm::vec3 forward = pCamera->GetTargetPosition() - position;
    glm::vec3 up( 0.0f, 1.0f, 0.0f );
    glm::vec3 velocity( 0.0f, 0.0f, 0.0f );

    FrameWork::GetSoundManager()->SetListener( position, velocity, glm::normalize( forward ), up );
}

void SectorCamera::OnMouseWheelCallback( float x, float y )
{
    if ( y > 0.0f )
    {
        m_ZoomMultTarget -= 0.1f;
    }
    else if ( y < 0.0f )
    {
        m_ZoomMultTarget += 0.1f;
    }
}

void SectorCamera::UpdateBorders()
{
    using namespace Genesis;
    const Camera* pCamera = FrameWork::GetScene()->GetCamera();

    const glm::vec3& position = pCamera->GetPosition();
    const float farDist = position.z;
    const float screenRatio = static_cast<float>( Configuration::GetScreenWidth() ) / static_cast<float>( Configuration::GetScreenHeight() );
    const float fovY = 45.0f;

    const float hh = tanf( fovY / 2.0f ) * farDist;
    m_BorderTopLeft.y = position.y + hh;
    m_BorderBottomRight.y = position.y - hh;

    const float hw = hh * screenRatio;
    m_BorderTopLeft.x = position.x - hw;
    m_BorderBottomRight.x = position.x + hw;
}

void SectorCamera::GetBorders( glm::vec2& topLeft, glm::vec2& bottomRight ) const
{
    topLeft = m_BorderTopLeft;
    bottomRight = m_BorderBottomRight;
}

} // namespace Hexterminate
