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

#include "requests/requestmanager.h"

#include <genesis.h>

#include "hexterminate.h"
#include "requests/arbiterrequest.h"
#include "requests/arbiterrequestinfo.h"
#include "requests/campaignrequest.h"
#include "requests/campaignrequestinfo.h"
#include "requests/defendrequest.h"
#include "requests/defendrequestinfo.h"
#include "requests/expandrequest.h"
#include "requests/expandrequestinfo.h"
#include "requests/imperialrequest.h"
#include "requests/infinitewarrequest.h"
#include "requests/infinitewarrequestinfo.h"
#include "requests/invasionrequest.h"
#include "requests/invasionrequestinfo.h"
#include "requests/strongholdrequest.h"
#include "requests/strongholdrequestinfo.h"

namespace Hexterminate
{

static const float sRequestManagerInstancingTimer = 1.0f;

RequestManager::RequestManager()
    : m_InstancingTimer( 0.0f )
{
    for ( auto& active : m_Active )
        active = 0;

    for ( auto& time : m_StartTime )
        time = 0.0f;

    m_Infos.push_back( new CampaignRequestInfo );
    m_Infos.push_back( new ExpandRequestInfo );
    m_Infos.push_back( new StrongholdRequestInfo );
    m_Infos.push_back( new DefendRequestInfo );
    m_Infos.push_back( new InvasionRequestInfo );
    m_Infos.push_back( new ArbiterRequestInfo );
    m_Infos.push_back( new InfiniteWarRequestInfo );
}

RequestManager::~RequestManager()
{
    for ( auto& pInfo : m_Infos )
    {
        delete pInfo;
    }
}

void RequestManager::Update( float delta )
{
    // The request manager is on hold until the tutorial finishes, otherwise we'll start getting events
    // happening while the player is busy learning the game's ropes.
    if ( g_pGame->IsTutorialActive() )
    {
        return;
    }

    m_InstancingTimer -= delta;
    if ( m_InstancingTimer <= 0.0f )
    {
        for ( auto& requestInfo : m_Infos )
        {
            if ( requestInfo->GetAllowedGameMode() == ImperialRequestInfo::AllowedGameMode::Campaign && g_pGame->GetGameMode() != GameMode::Campaign )
            {
                continue;
            }
            else if ( requestInfo->GetAllowedGameMode() == ImperialRequestInfo::AllowedGameMode::InfiniteWar && g_pGame->GetGameMode() != GameMode::InfiniteWar )
            {
                continue;
            }

            ImperialRequestSharedPtr request = requestInfo->TryInstantiate( this );
            if ( request )
            {
                SDL_assert( request->GetType() != ImperialRequestType::Invalid );
                SDL_assert( request->GetType() == requestInfo->GetType() );
                m_Requests.push_back( request );
                m_StartTime[ (int)request->GetType() ] = request->GetStartTime();
                m_Active[ (int)request->GetType() ]++;

                request->OnBegin();
            }
        }

        m_InstancingTimer = sRequestManagerInstancingTimer;
    }

    for ( auto& pRequest : m_Requests )
    {
        pRequest->Update( delta );
    }

    ClearRemovedRequests();
}

void RequestManager::Remove( ImperialRequestWeakPtr pRequestWeakPtr )
{
    ImperialRequestSharedPtr pRequest = pRequestWeakPtr.lock();
    SDL_assert( pRequest != nullptr );
    if ( pRequest != nullptr )
    {
        const int idx = (int)pRequest->GetType();
        m_Active[ idx ]--;
        SDL_assert_release( m_Active[ idx ] >= 0 );

        // Requests are added to a removal list to make sure they are safely removed, in case
        // the Remove() happens while m_Requests is being iterated on.
        m_RequestsToRemove.push_back( pRequest );
    }
}

void RequestManager::ClearRemovedRequests()
{
    for ( auto& pRequestToRemove : m_RequestsToRemove )
    {
        for ( ImperialRequestList::iterator it = m_Requests.begin(), itEnd = m_Requests.end(); it != itEnd; ++it )
        {
            if ( *it == pRequestToRemove )
            {
                m_Requests.erase( it );
                break;
            }
        }
    }

    m_RequestsToRemove.clear();
}

int RequestManager::GetActiveCount( ImperialRequestType type ) const
{
    int count = 0;
    for ( auto& pRequest : m_Requests )
    {
        if ( pRequest->GetType() == type )
            count++;
    }

    return count;
}

float RequestManager::GetPreviousStartTime( ImperialRequestType type ) const
{
    return m_StartTime[ (int)type ];
}

} // namespace Hexterminate
