#include "faction/neutralfaction.h"
#include "globals.h"
#include "hexterminate.h"
#include "sector/galaxy.h"
#include "sector/sector.h"

namespace Hexterminate
{

NeutralFaction::NeutralFaction( const FactionInfo& info )
    : Faction( info, FactionId::Neutral )
{
}

void NeutralFaction::ProcessTurn()
{
    // Doesn't call Faction::ProcessTurn() on purpose, as we don't want to build shipyards / gather resources.
    // As such, we need to explicitly tell it it to process the sectors.
    for ( auto& pSectorInfo : GetControlledSectors() )
    {
        pSectorInfo->ProcessTurn();
    }
}

} // namespace Hexterminate