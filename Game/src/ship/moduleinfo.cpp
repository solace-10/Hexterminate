// Copyright 2014 Pedro Nunes
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

#include <filesystem>
#include <string>
#include <vector>

#include <genesis.h>
#include <logger.h>
#include <math/misc.h>
#include <resources/resourcesound.h>
#include <xml.h>

#include "globals.h"
#include "hexterminate.h"
#include "ship/moduleinfo.h"
#include "stringaux.h"
#include "xmlaux.h"

namespace Hexterminate
{

///////////////////////////////////////////////////////////////////////////////
// Misc functions
///////////////////////////////////////////////////////////////////////////////

Genesis::Color ModuleRarityToColour( ModuleRarity rarity )
{
    using Genesis::Color;

    if ( rarity == ModuleRarity::Trash )
        return Color( 0.8f, 0.8f, 0.8f );
    else if ( rarity == ModuleRarity::Common )
        return Color( 1.0f, 1.0f, 1.0f );
    else if ( rarity == ModuleRarity::Uncommon )
        return Color( 0.0f, 1.0f, 0.0f );
    else if ( rarity == ModuleRarity::Rare )
        return Color( 0.4f, 0.4f, 1.0f );
    else if ( rarity == ModuleRarity::Artifact )
        return Color( 1.0f, 0.5f, 0.0f );
    else if ( rarity == ModuleRarity::Legendary )
        return Color( 1.0f, 0.0f, 0.0f );
    else
        return Color( 0.0f, 0.0f, 0.0f );
}

///////////////////////////////////////////////////////////////////////////////
// ModuleInfoManager
///////////////////////////////////////////////////////////////////////////////

ModuleInfoManager::ModuleInfoManager()
{
    using namespace tinyxml2;

    // Run through all the files in data/xml/modules and load all the modules that are listed
    for ( const auto& moduleFile : std::filesystem::directory_iterator( "data/xml/modules" ) )
    {
        if ( moduleFile.path().extension() != ".xml" )
        {
            continue;
        }

        FILE* fp = nullptr;

#ifdef _WIN32
        errno_t err = _wfopen_s( &fp, moduleFile.path().c_str(), L"rb" );
        SDL_assert_release( err == 0 );
#else
        fp = fopen( moduleFile.path().c_str(), "rb" );
        SDL_assert_release( fp != nullptr );
#endif

        tinyxml2::XMLDocument doc;
        XMLError fileLoaded = doc.LoadFile( fp );
        SDL_assert_release( fileLoaded == XML_SUCCESS );

        XMLElement* pRootElement = doc.FirstChildElement();
        for ( XMLElement* pModuleEntry = pRootElement->FirstChildElement(); pModuleEntry; pModuleEntry = pModuleEntry->NextSiblingElement() )
        {
            // Find the type of the module that needs to be created. Each type will load any elements that it needs.
            ModuleType moduleType = ModuleType::Invalid;

            for ( XMLElement* pElement = pModuleEntry->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
            {
                if ( strcmp( pElement->Value(), "Type" ) == 0 )
                {
                    moduleType = StringToModuleType( pElement->GetText() );
                    break;
                }
            }

            SDL_assert_release( moduleType != ModuleType::Invalid ); // If this asserts then we didn't find a "Type" tag in this module...

            ModuleInfo* pModule = CreateModuleInfo( moduleType, pModuleEntry );
            m_Modules.insert( std::pair<std::string, ModuleInfo*>( pModule->GetName(), pModule ) );
        }

        fclose( fp );
    }
}

ModuleInfoManager::~ModuleInfoManager()
{
    for ( auto& moduleInfoPair : m_Modules )
    {
        delete moduleInfoPair.second;
    }
}

ModuleType ModuleInfoManager::StringToModuleType( const std::string& str ) const
{
    if ( str == "Engine" )
        return ModuleType::Engine;
    else if ( str == "Armour" )
        return ModuleType::Armour;
    else if ( str == "Shield" )
        return ModuleType::Shield;
    else if ( str == "Weapon" )
        return ModuleType::Weapon;
    else if ( str == "Reactor" )
        return ModuleType::Reactor;
    else if ( str == "Addon" )
        return ModuleType::Addon;
    else if ( str == "Tower" )
        return ModuleType::Tower;
    else
        return ModuleType::Invalid;
}

ModuleInfo* ModuleInfoManager::CreateModuleInfo( ModuleType moduleType, tinyxml2::XMLElement* pElement )
{
    SDL_assert_release( moduleType != ModuleType::Invalid );
    if ( moduleType == ModuleType::Engine )
        return new EngineInfo( pElement );
    else if ( moduleType == ModuleType::Armour )
        return new ArmourInfo( pElement );
    else if ( moduleType == ModuleType::Shield )
        return new ShieldInfo( pElement );
    else if ( moduleType == ModuleType::Weapon )
        return new WeaponInfo( pElement );
    else if ( moduleType == ModuleType::Reactor )
        return new ReactorInfo( pElement );
    else if ( moduleType == ModuleType::Addon )
        return new AddonInfo( pElement );
    else if ( moduleType == ModuleType::Tower )
        return new TowerInfo( pElement );
    else
        return nullptr;
}

ModuleInfo* ModuleInfoManager::GetModuleByName( const std::string& str ) const
{
    std::map<std::string, ModuleInfo*>::const_iterator it = m_Modules.find( str );
    if ( it == m_Modules.end() )
        return nullptr;
    else
        return it->second;
}

ModuleInfoVector ModuleInfoManager::GetAllModules() const
{
    ModuleInfoVector allModules;
    for ( auto& module : m_Modules )
    {
        allModules.push_back( module.second );
    }
    return allModules;
}

///////////////////////////////////////////////////////////////////////////////
// ModuleInfo
///////////////////////////////////////////////////////////////////////////////

ModuleInfo::ModuleInfo( tinyxml2::XMLElement* pModuleElement )
    : m_Name( "" )
    , m_FullName( "" )
    , m_Description( "" )
    , m_Icon( "" )
    , m_Type( ModuleType::Invalid )
    , m_ActivationCost( 0.0f )
    , m_Model( "" )
    , m_Health( BaseModuleHealth )
    , m_Rarity( ModuleRarity::Common )
    , m_OverlayColour( 0.0f, 0.0f, 0.0f, 0.0f )
    , m_PowerGrid( 0.0f )
{
    using namespace Genesis;

    float extraHealth = 0.0f;

    for ( tinyxml2::XMLElement* pElement = pModuleElement->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
    {
        Xml::Serialise( pElement, "Name", m_Name );
        Xml::Serialise( pElement, "FullName", m_FullName );
        Xml::Serialise( pElement, "Description", m_Description );
        Xml::Serialise( pElement, "ActivationCost", m_ActivationCost );
        Xml::Serialise( pElement, "Model", m_Model );
        Xml::Serialise( pElement, "ExtraHealth", extraHealth );
        Xml::Serialise( pElement, "Rarity", m_Rarity );
        Xml::Serialise( pElement, "OverlayColour", m_OverlayColour );
        Xml::Serialise( pElement, "ContextualTip", m_ContextualTip );
        Xml::Serialise( pElement, "PowerGrid", m_PowerGrid );
    }

    // Fallback for all modules which don't have a model yet
    if ( m_Model == "" )
    {
        Genesis::FrameWork::GetLogger()->LogInfo( "No model set for '%s'", m_Name.c_str() );
        m_Model = "data/models/armour/medium/t1/module.tmf";
    }

    m_Health += extraHealth;

    // If we are using an overlay colour, we bump the intensity fractionally to make
    // the pattern more dynamic.
    if ( m_OverlayColour.a > 0.0f && m_OverlayColour.a <= 0.9f )
    {
        m_OverlayColour.a += gRand( 0.0f, 0.1f );
    }

    SDL_assert_release( m_Name != "" );
    SDL_assert_release( m_Health > 0.0f );
}

float ModuleInfo::GetHealth( const Ship* pShip ) const
{
    if ( pShip != nullptr && pShip->HasPerk( Perk::ReinforcedBulkheads ) )
    {
        return m_Health + 1000.0f;
    }
    else
    {
        return m_Health;
    }
}

///////////////////////////////////////////////////////////////////////////////
// EngineInfo
///////////////////////////////////////////////////////////////////////////////

EngineInfo::EngineInfo( tinyxml2::XMLElement* pModuleElement )
    : ModuleInfo( pModuleElement )
    , m_Thrust( 0.0f )
    , m_Torque( 0.0f )
    , m_IsStabilised( false )
    , m_IsEMPResistant( false )
    , m_ReducesHyperspaceChargeup( false )
{
    using namespace Genesis;

    m_Type = ModuleType::Engine;
    m_Icon = "data/ui/icons/engine.png";

    for ( tinyxml2::XMLElement* pElement = pModuleElement->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
    {
        Xml::Serialise( pElement, "Thrust", m_Thrust );
        Xml::Serialise( pElement, "Torque", m_Torque );

        std::string bonus;
        if ( Xml::Serialise( pElement, "Bonus", bonus ) )
        {
            if ( bonus == "Stabilised" )
            {
                m_IsStabilised = true;
            }
            else if ( bonus == "EMPR" )
            {
                m_IsEMPResistant = true;
            }
            else if ( bonus == "Hyperspace" )
            {
                m_ReducesHyperspaceChargeup = true;
            }
        }
    }
    SDL_assert_release( m_Thrust > 0.0f );
    SDL_assert_release( m_Torque > 0.0f );
}

Module* EngineInfo::CreateModule()
{
    return new EngineModule( this );
}

bool EngineInfo::HasBonus( EngineBonus bonus ) const
{
    if ( bonus == EngineBonus::Stabilised )
    {
        return m_IsStabilised;
    }
    else if ( bonus == EngineBonus::EMPResistant )
    {
        return m_IsEMPResistant;
    }
    else if ( bonus == EngineBonus::ReducesHyperspaceChargeup )
    {
        return m_ReducesHyperspaceChargeup;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// ArmourInfo
///////////////////////////////////////////////////////////////////////////////

ArmourInfo::ArmourInfo( tinyxml2::XMLElement* pModuleElement )
    : ModuleInfo( pModuleElement )
    , m_MassMultiplier( 1.0f )
    , m_KineticResistance( 0.0f )
    , m_EnergyResistance( 0.0f )
    , m_Regenerative( false )
    , m_RammingProw( false )
{
    using namespace Genesis;

    m_Type = ModuleType::Armour;
    m_Icon = "data/ui/icons/armour.png";

    for ( tinyxml2::XMLElement* pElement = pModuleElement->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
    {
        Xml::Serialise( pElement, "MassMultiplier", m_MassMultiplier );
        Xml::Serialise( pElement, "KineticResistance", m_KineticResistance );
        Xml::Serialise( pElement, "EnergyResistance", m_EnergyResistance );
        Xml::Serialise( pElement, "Regenerative", m_Regenerative );
        Xml::Serialise( pElement, "Ramming", m_RammingProw );
    }

    SDL_assert_release( m_KineticResistance >= 0.0f && m_KineticResistance <= 1.0f );
    SDL_assert_release( m_EnergyResistance >= 0.0f && m_EnergyResistance <= 1.0f );
}

Module* ArmourInfo::CreateModule()
{
    return new ArmourModule( this );
}

float ArmourInfo::GetMassMultiplier( Ship* pShip ) const
{
    float massMultiplier = m_MassMultiplier;

    if ( pShip->HasPerk( Perk::LighterMaterials ) )
    {
        massMultiplier *= 0.8f;
    }

    return massMultiplier;
}

///////////////////////////////////////////////////////////////////////////////
// ShieldInfo
///////////////////////////////////////////////////////////////////////////////

ShieldInfo::ShieldInfo( tinyxml2::XMLElement* pModuleElement )
    : ModuleInfo( pModuleElement )
    , m_Capacity( 0.0f )
    , m_PeakRecharge( 0.0f )
    , m_Overtuned( false )
{
    using namespace Genesis;

    m_Type = ModuleType::Shield;
    m_Icon = "data/ui/icons/shield.png";

    for ( tinyxml2::XMLElement* pElement = pModuleElement->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
    {
        Xml::Serialise( pElement, "Capacity", m_Capacity );
        Xml::Serialise( pElement, "PeakRechargeRate", m_PeakRecharge );
        Xml::Serialise( pElement, "Overtuned", m_Overtuned );
    }

    SDL_assert_release( m_Capacity > 0.0f );
    SDL_assert_release( m_PeakRecharge >= 0.0f );
}

Module* ShieldInfo::CreateModule()
{
    return new ShieldModule( this );
}

float ShieldInfo::GetPowerGrid( const Ship* pShip ) const
{
    float powerGrid = m_PowerGrid;

    if ( pShip->HasPerk( Perk::Superconductors ) )
    {
        powerGrid *= 0.7f;
    }

    return powerGrid;
}

///////////////////////////////////////////////////////////////////////////////
// TowerInfo
///////////////////////////////////////////////////////////////////////////////

TowerInfo::TowerInfo( tinyxml2::XMLElement* pModuleElement )
    : ModuleInfo( pModuleElement )
    , m_BonusType( TowerBonus::None )
    , m_BonusMagnitude( 0.0f )
{
    using namespace Genesis;

    m_Type = ModuleType::Tower;
    m_Icon = "data/ui/icons/bridge.png";

    for ( tinyxml2::XMLElement* pElement = pModuleElement->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
    {
        Xml::Serialise( pElement, "BonusMagnitude", m_BonusMagnitude );
        Xml::Serialise( pElement, "Bonus", m_BonusType );
        Xml::Serialise( pElement, "ShortDescription", m_ShortDescription );
    }

    SDL_assert_release( m_BonusMagnitude >= 0.0f );
}

Module* TowerInfo::CreateModule()
{
    return new TowerModule( this );
}

///////////////////////////////////////////////////////////////////////////////
// WeaponInfo
///////////////////////////////////////////////////////////////////////////////

WeaponInfo::WeaponInfo( tinyxml2::XMLElement* pModuleElement )
    : ModuleInfo( pModuleElement )
    , m_Behaviour( WeaponBehaviour::Invalid )
    , m_System( WeaponSystem::Invalid )
    , m_Rof( 0.0f )
    , m_Damage( 0.0f )
    , m_Burst( 1 )
    , m_RayLength( 20.0f )
    , m_Speed( 20.0f )
    , m_Range( 100.0f )
    , m_Tracking( 0.0f )
    , m_IsSwarm( false )
    , m_BeamWidth( 1.0f )
    , m_BeamLifetime( 1.0f )
    , m_pOnFireSFX( nullptr )
    , m_pOnHitSFX( nullptr )
    , m_OnHitSFXDistance( 150.0f )
    , m_DamageType( DamageType::Kinetic )
    , m_MuzzleflashScale( 1.0f )
{
    using namespace Genesis;

    m_Type = ModuleType::Weapon;
    m_Icon = "data/ui/icons/weapon.png";

    std::string onFireSFX;
    std::string onHitSFX;

    for ( tinyxml2::XMLElement* pElement = pModuleElement->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
    {
        Xml::Serialise( pElement, "WeaponModel", m_WeaponModel );
        Xml::Serialise( pElement, "Rof", m_Rof );
        Xml::Serialise( pElement, "Damage", m_Damage );
        Xml::Serialise( pElement, "Burst", m_Burst );
        Xml::Serialise( pElement, "RayLength", m_RayLength );
        Xml::Serialise( pElement, "Speed", m_Speed );
        Xml::Serialise( pElement, "Range", m_Range );
        Xml::Serialise( pElement, "Behaviour", m_Behaviour );
        Xml::Serialise( pElement, "Tracking", m_Tracking );
        Xml::Serialise( pElement, "Swarm", m_IsSwarm );
        Xml::Serialise( pElement, "BeamWidth", m_BeamWidth );
        Xml::Serialise( pElement, "BeamLifetime", m_BeamLifetime );
        Xml::Serialise( pElement, "BeamColourR", m_BeamColour.r );
        Xml::Serialise( pElement, "BeamColourG", m_BeamColour.g );
        Xml::Serialise( pElement, "BeamColourB", m_BeamColour.b );
        Xml::Serialise( pElement, "OnFireSFX", onFireSFX );
        Xml::Serialise( pElement, "OnHitSFX", onHitSFX );
        Xml::Serialise( pElement, "OnHitSFXDistance", m_OnHitSFXDistance );
        Xml::Serialise( pElement, "MuzzleflashScale", m_MuzzleflashScale );
        Xml::Serialise( pElement, "MuzzleflashColourR", m_MuzzleflashColour.r );
        Xml::Serialise( pElement, "MuzzleflashColourG", m_MuzzleflashColour.g );
        Xml::Serialise( pElement, "MuzzleflashColourB", m_MuzzleflashColour.b );

        if ( strcmp( pElement->Value(), "System" ) == 0 )
        {
            const std::string systems[ static_cast<size_t>( WeaponSystem::Count ) ] = {
                "Projectile",
                "Missile",
                "Rocket",
                "Torpedo",
                "Antiproton",
                "Lance",
                "Ion",
                "Universal"
            };

            for ( int i = 0; i < static_cast<int>( WeaponSystem::Count ); ++i )
            {
                if ( strcmp( pElement->GetText(), systems[ i ].c_str() ) == 0 )
                {
                    m_System = static_cast<WeaponSystem>( i );
                    break;
                }
            }
        }
    }

    SDL_assert_release( m_Behaviour != WeaponBehaviour::Invalid );
    SDL_assert_release( m_System != WeaponSystem::Invalid );
    SDL_assert_release( m_Rof > 0.0f );
    SDL_assert_release( m_Damage > 0.0f );
    SDL_assert_release( m_Range > 0.0f );
    SDL_assert_release( m_Tracking >= 0.0f );
    SDL_assert_release( m_BeamWidth > 0.0f );
    SDL_assert_release( m_BeamColour.r >= 0.0f && m_BeamColour.r <= 1.0f );
    SDL_assert_release( m_BeamColour.g >= 0.0f && m_BeamColour.g <= 1.0f );
    SDL_assert_release( m_BeamColour.b >= 0.0f && m_BeamColour.b <= 1.0f );
    SDL_assert_release( m_OnHitSFXDistance > 0.0f );
    SDL_assert_release( m_MuzzleflashScale > 0.0f );
    SDL_assert_release( m_MuzzleflashColour.r >= 0.0f && m_MuzzleflashColour.r <= 1.0f );
    SDL_assert_release( m_MuzzleflashColour.g >= 0.0f && m_MuzzleflashColour.g <= 1.0f );
    SDL_assert_release( m_MuzzleflashColour.b >= 0.0f && m_MuzzleflashColour.b <= 1.0f );

    m_pOnFireSFX = Genesis::ResourceSound::LoadAs3D( onFireSFX );
    m_pOnHitSFX = Genesis::ResourceSound::LoadAs3D( onHitSFX );

    if ( m_System == WeaponSystem::Lance || m_System == WeaponSystem::Ion )
    {
        m_DamageType = DamageType::Energy;
    }
    else if ( m_System == WeaponSystem::Antiproton )
    {
        m_DamageType = DamageType::TrueDamage;
    }
}

Module* WeaponInfo::CreateModule()
{
    return new WeaponModule( this );
}

float WeaponInfo::GetRateOfFire( Ship* pShip ) const
{
    float rateOfFire = m_Rof;

    if ( pShip != nullptr )
    {
        if ( pShip->HasPerk( Perk::MagneticLoaders ) && GetDamageType() == DamageType::Kinetic )
        {
            rateOfFire *= 1.15f;
        }
        else if ( pShip->HasPerk( Perk::AdvancedHeatsinks ) && GetDamageType() == DamageType::Energy )
        {
            rateOfFire *= 1.15f;
        }
    }

    return rateOfFire;
}

float WeaponInfo::GetRange( Ship* pShip ) const
{
    float range = m_Range;

    if ( pShip != nullptr && pShip->HasPerk( Perk::AdvancedElectrocoils ) && GetDamageType() == DamageType::Kinetic )
    {
        range *= 1.2f;
    }

    return range;
}

float WeaponInfo::GetActivationCost( Ship* pShip ) const
{
    float activationCost = m_ActivationCost;

    if ( pShip != nullptr && pShip->HasPerk( Perk::AdvancedHeatsinks ) && GetDamageType() == DamageType::Energy )
    {
        activationCost *= 0.8f;
    }

    return activationCost;
}

///////////////////////////////////////////////////////////////////////////////
// ReactorInfo
///////////////////////////////////////////////////////////////////////////////

ReactorInfo::ReactorInfo( tinyxml2::XMLElement* pModuleElement )
    : ModuleInfo( pModuleElement )
    , m_CapacitorStorage( 0.0f )
    , m_CapacitorRechargeRate( 0.0f )
    , m_Variant( ReactorVariant::Standard )
{
    using namespace Genesis;

    m_Type = ModuleType::Reactor;
    m_Icon = "data/ui/icons/reactor.png";

    for ( tinyxml2::XMLElement* pElement = pModuleElement->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
    {
        Xml::Serialise( pElement, "CapacitorStorage", m_CapacitorStorage );
        Xml::Serialise( pElement, "CapacitorRechargeRate", m_CapacitorRechargeRate );
        Xml::Serialise( pElement, "Variant", m_Variant );
    }

    SDL_assert_release( m_CapacitorStorage > 0.0f );
    SDL_assert_release( m_CapacitorRechargeRate > 0.0f );
}

Module* ReactorInfo::CreateModule()
{
    return new ReactorModule( this );
}

///////////////////////////////////////////////////////////////////////////////
// AddonInfo
///////////////////////////////////////////////////////////////////////////////

AddonInfo::AddonInfo( tinyxml2::XMLElement* pModuleElement )
    : ModuleInfo( pModuleElement )
    , m_Category( AddonCategory::Invalid )
    , m_ActivationType( AddonActivationType::Invalid )
    , m_Cooldown( 0.0f )
{
    using namespace Genesis;

    m_Type = ModuleType::Addon;
    m_Icon = "data/ui/icons/addon.png";

    for ( tinyxml2::XMLElement* pElement = pModuleElement->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
    {
        if ( strcmp( pElement->Value(), "Category" ) == 0 && pElement->GetText() != nullptr )
        {
            if ( strcmp( pElement->GetText(), "ModuleRepairer" ) == 0 )
            {
                m_Category = AddonCategory::ModuleRepairer;
                m_Icon = "data/ui/icons/armourrepairer.png";
            }
            else if ( strcmp( pElement->GetText(), "HangarBay" ) == 0 )
            {
                m_Category = AddonCategory::HangarBay;
            }
            else if ( strcmp( pElement->GetText(), "DroneBay" ) == 0 )
            {
                m_Category = AddonCategory::DroneBay;
            }
            else if ( strcmp( pElement->GetText(), "MissileInterceptor" ) == 0 )
            {
                m_Category = AddonCategory::MissileInterceptor;
                m_Icon = "data/ui/icons/intercept.png";
            }
            else if ( strcmp( pElement->GetText(), "ParticleAccelerator" ) == 0 )
            {
                m_Category = AddonCategory::ParticleAccelerator;
                m_Icon = "data/ui/icons/particle_accelerator.png";
            }
            else if ( strcmp( pElement->GetText(), "PhaseBarrier" ) == 0 )
            {
                m_Category = AddonCategory::PhaseBarrier;
                m_Icon = "data/ui/icons/phasebarrier.png";
            }
            else if ( strcmp( pElement->GetText(), "FuelInjector" ) == 0 )
            {
                m_Category = AddonCategory::FuelInjector;
            }
            else if ( strcmp( pElement->GetText(), "EngineDisruptor" ) == 0 )
            {
                m_Category = AddonCategory::EngineDisruptor;
            }
            else if ( strcmp( pElement->GetText(), "QuantumStateAlternator" ) == 0 )
            {
                m_Category = AddonCategory::QuantumStateAlternator;
                m_Icon = "data/ui/icons/quantum_alternator.png";
            }
        }
        else if ( strcmp( pElement->Value(), "ActivationType" ) == 0 && pElement->GetText() != nullptr )
        {
            if ( strcmp( pElement->GetText(), "Trigger" ) == 0 )
                m_ActivationType = AddonActivationType::Trigger;
            else if ( strcmp( pElement->GetText(), "Toggle" ) == 0 )
                m_ActivationType = AddonActivationType::Toggle;
        }

        Xml::Serialise( pElement, "Cooldown", m_Cooldown );
        Xml::Serialise( pElement, "Parameter", m_Parameter );
    }

    if ( m_Cooldown <= 0.0f )
        m_Cooldown = 0.0f;

    if ( m_Category == AddonCategory::Invalid )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Failed to serialise 'Category' on addon '%s'", GetName().c_str() );
    }
    else if ( m_ActivationType == AddonActivationType::Invalid )
    {
        Genesis::FrameWork::GetLogger()->LogError( "Failed to serialise 'ActivationType' on addon '%s'", GetName().c_str() );
    }
}

Module* AddonInfo::CreateModule()
{
    return new AddonModule( this );
}

} // namespace Hexterminate