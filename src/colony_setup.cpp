#include "colony_setup.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "avatar.h"
#include "basecamp.h"
#include "calendar.h"
#include "colony_combat.h"
#include "colony_construction.h"
#include "colony_expeditions.h"
#include "coordinates.h"
#include "debug.h"
#include "faction.h"
#include "faction_camp.h"
#include "game.h"
#include "item.h"
#include "item_group.h"
#include "npc.h"
#include "options.h"
#include "output.h"
#include "overmapbuffer.h"
#include "popup.h"
#include "start_location.h"
#include "string_formatter.h"
#include "translations.h"
#include "uilist.h"
#include "worldfactory.h"

static const item_group_id
item_group_cull_colony_gear_sparse( "cull_colony_gear_sparse" );
static const item_group_id
item_group_cull_colony_gear_standard( "cull_colony_gear_standard" );
static const item_group_id
item_group_cull_colony_gear_well_supplied( "cull_colony_gear_well_supplied" );

static const npc_class_id NC_COWBOY( "NC_COWBOY" );
static const npc_class_id NC_DOCTOR( "NC_DOCTOR" );
static const npc_class_id NC_FARMER( "NC_FARMER" );
static const npc_class_id NC_HUNTER( "NC_HUNTER" );
static const npc_class_id NC_SOLDIER( "NC_SOLDIER" );
static const npc_class_id NC_BOUNTY_HUNTER( "NC_BOUNTY_HUNTER" );

static const start_location_id start_location_sloc_field( "sloc_field" );
static const start_location_id start_location_sloc_fort( "sloc_fort" );
static const start_location_id start_location_sloc_house_boarded( "sloc_house_boarded" );
static const start_location_id start_location_sloc_refugee_center( "sloc_refugee_center" );
static const start_location_id start_location_sloc_shelter_safe( "sloc_shelter_safe" );

const std::vector<colony_start_location_choice> &colony_start_location_choices()
{
    static const std::vector<colony_start_location_choice> choices = {
        {
            start_location_sloc_shelter_safe,
            to_translation( "Evacuation Shelter" ),
            to_translation( "A standard shelter start — defensible and familiar." )
        },
        {
            start_location_sloc_field,
            to_translation( "Open Field" ),
            to_translation( "Room to expand farms and walls, but little cover." )
        },
        {
            start_location_sloc_house_boarded,
            to_translation( "Boarded House" ),
            to_translation( "Suburban foothold with scavengable neighborhoods nearby." )
        },
        {
            start_location_sloc_fort,
            to_translation( "Bastion Fort" ),
            to_translation( "Strong walls from day one; fewer soft resources." )
        },
        {
            start_location_sloc_refugee_center,
            to_translation( "Refugee Center" ),
            to_translation( "Crowded ruins — contacts and loot, plus early pressure." )
        },
    };
    return choices;
}

std::string colony_background_mix_name( colony_background_mix mix )
{
    switch( mix ) {
        case colony_background_mix::farmstead:
            return _( "Farmstead" );
        case colony_background_mix::scavengers:
            return _( "Scavengers" );
        case colony_background_mix::militia:
            return _( "Militia" );
        case colony_background_mix::medical:
            return _( "Medical" );
        case colony_background_mix::mixed:
        default:
            return _( "Mixed" );
    }
}

std::string colony_background_mix_desc( colony_background_mix mix )
{
    switch( mix ) {
        case colony_background_mix::farmstead:
            return _( "Farmers and rural survivors — stronger early food production." );
        case colony_background_mix::scavengers:
            return _( "Hunters and wanderers — better at foraging and light combat." );
        case colony_background_mix::militia:
            return _( "Soldiers and gun hands — stronger defense, weaker farming." );
        case colony_background_mix::medical:
            return _( "Doctors and caregivers — trauma care, thinner combat skills." );
        case colony_background_mix::mixed:
        default:
            return _( "A balanced cross-section of common survivor backgrounds." );
    }
}

std::string colony_gear_level_name( colony_gear_level gear )
{
    switch( gear ) {
        case colony_gear_level::sparse:
            return _( "Sparse" );
        case colony_gear_level::well_supplied:
            return _( "Well supplied" );
        case colony_gear_level::standard:
        default:
            return _( "Standard" );
    }
}

std::string colony_gear_level_desc( colony_gear_level gear )
{
    switch( gear ) {
        case colony_gear_level::sparse:
            return _( "Minimal tools and thin larder.  Lean early game." );
        case colony_gear_level::well_supplied:
            return _( "Extra tools, materials, and food to bootstrap facilities." );
        case colony_gear_level::standard:
        default:
            return _( "A workable toolkit and several days of camp rations." );
    }
}

std::string colony_difficulty_name( colony_difficulty diff )
{
    switch( diff ) {
        case colony_difficulty::easy:
            return _( "Easy" );
        case colony_difficulty::hard:
            return _( "Hard" );
        case colony_difficulty::brutal:
            return _( "Brutal" );
        case colony_difficulty::normal:
        default:
            return _( "Normal" );
    }
}

std::string colony_difficulty_desc( colony_difficulty diff )
{
    switch( diff ) {
        case colony_difficulty::easy:
            return _( "Fewer spawns, slower monsters, larger starter food stock." );
        case colony_difficulty::hard:
            return _( "Denser spawns and faster enemies." );
        case colony_difficulty::brutal:
            return _( "Heavy spawn pressure, fast and tough monsters." );
        case colony_difficulty::normal:
        default:
            return _( "Default Cataclysm spawn and monster settings." );
    }
}

std::string colony_setup_options::summary() const
{
    std::string loc_name = location.is_valid() ? location->name() : location.str();
    return string_format(
               _( "%d survivors · %s · %s gear · %s · %s" ),
               survivor_count,
               colony_background_mix_name( backgrounds ),
               colony_gear_level_name( gear ),
               loc_name,
               colony_difficulty_name( difficulty ) );
}

std::optional<colony_setup_options> colony_setup_ui()
{
    colony_setup_options opts;
    while( true ) {
        uilist menu;
        menu.title = _( "Colony setup" );
        menu.desc_enabled = true;
        menu.text = opts.summary();
        menu.addentry_desc( 0, true, 'n',
                            string_format( _( "Survivors: %d" ), opts.survivor_count ),
                            string_format( _( "Starting group size (%d–%d).  Includes the leadership proxy." ),
                                           colony_setup_options::min_survivors,
                                           colony_setup_options::max_survivors ) );
        menu.addentry_desc( 1, true, 'b',
                            string_format( _( "Backgrounds: %s" ),
                                           colony_background_mix_name( opts.backgrounds ) ),
                            colony_background_mix_desc( opts.backgrounds ) );
        menu.addentry_desc( 2, true, 'g',
                            string_format( _( "Starting gear: %s" ),
                                           colony_gear_level_name( opts.gear ) ),
                            colony_gear_level_desc( opts.gear ) );
        menu.addentry_desc( 3, true, 'l',
                            string_format( _( "Location: %s" ),
                                           opts.location.is_valid() ? opts.location->name() : opts.location.str() ),
                            _( "Where the colony founds its first settlement." ) );
        menu.addentry_desc( 4, true, 'd',
                            string_format( _( "Difficulty: %s" ),
                                           colony_difficulty_name( opts.difficulty ) ),
                            colony_difficulty_desc( opts.difficulty ) );
        menu.addentry_desc( 5, true, 's', _( "Start colony" ),
                            _( "Create or pick a world and begin with these settings." ) );
        menu.addentry_desc( 6, true, 'q', _( "Cancel" ),
                            _( "Return to the main menu." ) );
        menu.query();
        if( menu.ret < 0 || menu.ret == 6 ) {
            return std::nullopt;
        }
        if( menu.ret == 5 ) {
            return opts;
        }
        if( menu.ret == 0 ) {
            int count = opts.survivor_count;
            if( query_int( count, false, _( "Starting survivors (%d–%d):" ),
                           colony_setup_options::min_survivors,
                           colony_setup_options::max_survivors ) ) {
                opts.survivor_count = std::clamp( count,
                                             colony_setup_options::min_survivors,
                                             colony_setup_options::max_survivors );
            }
        } else if( menu.ret == 1 ) {
            uilist bg;
            bg.title = _( "Background mix" );
            bg.desc_enabled = true;
            const colony_background_mix values[] = {
                colony_background_mix::mixed,
                colony_background_mix::farmstead,
                colony_background_mix::scavengers,
                colony_background_mix::militia,
                colony_background_mix::medical,
            };
            for( size_t i = 0; i < sizeof( values ) / sizeof( values[0] ); i++ ) {
                bg.addentry_desc( static_cast<int>( i ), true, MENU_AUTOASSIGN,
                                  colony_background_mix_name( values[i] ),
                                  colony_background_mix_desc( values[i] ) );
            }
            bg.query();
            if( bg.ret >= 0 && static_cast<size_t>( bg.ret ) < sizeof( values ) / sizeof( values[0] ) ) {
                opts.backgrounds = values[bg.ret];
            }
        } else if( menu.ret == 2 ) {
            uilist gear;
            gear.title = _( "Starting gear" );
            gear.desc_enabled = true;
            const colony_gear_level values[] = {
                colony_gear_level::sparse,
                colony_gear_level::standard,
                colony_gear_level::well_supplied,
            };
            for( size_t i = 0; i < sizeof( values ) / sizeof( values[0] ); i++ ) {
                gear.addentry_desc( static_cast<int>( i ), true, MENU_AUTOASSIGN,
                                    colony_gear_level_name( values[i] ),
                                    colony_gear_level_desc( values[i] ) );
            }
            gear.query();
            if( gear.ret >= 0 &&
                static_cast<size_t>( gear.ret ) < sizeof( values ) / sizeof( values[0] ) ) {
                opts.gear = values[gear.ret];
            }
        } else if( menu.ret == 3 ) {
            uilist loc;
            loc.title = _( "Starting location" );
            loc.desc_enabled = true;
            const auto &choices = colony_start_location_choices();
            for( size_t i = 0; i < choices.size(); i++ ) {
                loc.addentry_desc( static_cast<int>( i ), true, MENU_AUTOASSIGN,
                                   choices[i].name.translated(),
                                   choices[i].desc.translated() );
            }
            loc.query();
            if( loc.ret >= 0 && static_cast<size_t>( loc.ret ) < choices.size() ) {
                opts.location = choices[loc.ret].id;
            }
        } else if( menu.ret == 4 ) {
            uilist diff;
            diff.title = _( "Colony difficulty" );
            diff.desc_enabled = true;
            const colony_difficulty values[] = {
                colony_difficulty::easy,
                colony_difficulty::normal,
                colony_difficulty::hard,
                colony_difficulty::brutal,
            };
            for( size_t i = 0; i < sizeof( values ) / sizeof( values[0] ); i++ ) {
                diff.addentry_desc( static_cast<int>( i ), true, MENU_AUTOASSIGN,
                                    colony_difficulty_name( values[i] ),
                                    colony_difficulty_desc( values[i] ) );
            }
            diff.query();
            if( diff.ret >= 0 &&
                static_cast<size_t>( diff.ret ) < sizeof( values ) / sizeof( values[0] ) ) {
                opts.difficulty = values[diff.ret];
            }
        }
    }
}

void apply_colony_difficulty_to_world( WORLD &world, colony_difficulty diff )
{
    float spawn = 1.0f;
    int speed = 100;
    int resilience = 100;
    switch( diff ) {
        case colony_difficulty::easy:
            spawn = 0.5f;
            speed = 90;
            resilience = 90;
            break;
        case colony_difficulty::hard:
            spawn = 1.5f;
            speed = 110;
            resilience = 110;
            break;
        case colony_difficulty::brutal:
            spawn = 2.0f;
            speed = 125;
            resilience = 125;
            break;
        case colony_difficulty::normal:
        default:
            break;
    }
    world.WORLD_OPTIONS["SPAWN_DENSITY"].setValue( spawn );
    world.WORLD_OPTIONS["MONSTER_SPEED"].setValue( speed );
    world.WORLD_OPTIONS["MONSTER_RESILIENCE"].setValue( resilience );
}

void apply_colony_setup_to_avatar( avatar &pc, const colony_setup_options &opts )
{
    pc.random_start_location = false;
    if( opts.location.is_valid() ) {
        pc.start_location = opts.location;
    } else {
        pc.start_location = start_location_sloc_shelter_safe;
        pc.random_start_location = true;
    }
}

npc_class_id pick_colony_starter_class( const colony_setup_options &opts, int resident_index )
{
    switch( opts.backgrounds ) {
        case colony_background_mix::farmstead: {
            static const npc_class_id pool[] = { NC_FARMER, NC_FARMER, NC_HUNTER, NC_COWBOY };
            return pool[resident_index % 4];
        }
        case colony_background_mix::scavengers: {
            static const npc_class_id pool[] = { NC_HUNTER, NC_COWBOY, NC_HUNTER, NC_BOUNTY_HUNTER };
            return pool[resident_index % 4];
        }
        case colony_background_mix::militia: {
            static const npc_class_id pool[] = { NC_SOLDIER, NC_BOUNTY_HUNTER, NC_SOLDIER, NC_COWBOY };
            return pool[resident_index % 4];
        }
        case colony_background_mix::medical: {
            static const npc_class_id pool[] = { NC_DOCTOR, NC_DOCTOR, NC_FARMER, NC_HUNTER };
            return pool[resident_index % 4];
        }
        case colony_background_mix::mixed:
        default: {
            static const npc_class_id pool[] = {
                NC_FARMER, NC_HUNTER, NC_SOLDIER, NC_DOCTOR, NC_COWBOY, NC_BOUNTY_HUNTER
            };
            return pool[resident_index % 6];
        }
    }
}

item_group_id colony_starter_gear_group( colony_gear_level gear )
{
    switch( gear ) {
        case colony_gear_level::sparse:
            return item_group_cull_colony_gear_sparse;
        case colony_gear_level::well_supplied:
            return item_group_cull_colony_gear_well_supplied;
        case colony_gear_level::standard:
        default:
            return item_group_cull_colony_gear_standard;
    }
}

int colony_starter_food_kcal_per_survivor( colony_gear_level gear, colony_difficulty diff )
{
    int base = 12000;
    switch( gear ) {
        case colony_gear_level::sparse:
            base = 6000;
            break;
        case colony_gear_level::well_supplied:
            base = 20000;
            break;
        case colony_gear_level::standard:
        default:
            break;
    }
    if( diff == colony_difficulty::easy ) {
        base = static_cast<int>( base * 1.5 );
    } else if( diff == colony_difficulty::brutal ) {
        base = static_cast<int>( base * 0.75 );
    }
    return std::max( 1000, base );
}

void seed_colony_starter_gear( basecamp &camp, const colony_setup_options &opts )
{
    const item_group_id group = colony_starter_gear_group( opts.gear );
    if( !item_group::group_is_defined( group ) ) {
        debugmsg( "Missing colony starter gear group %s", group.str() );
        return;
    }
    // One crate per 4 survivors (minimum one) so larger colonies get more tools.
    const int crates = std::max( 1, ( opts.survivor_count + 3 ) / 4 );
    for( int i = 0; i < crates; i++ ) {
        for( const item &it : item_group::items_from( group, calendar::turn ) ) {
            camp.place_results( it );
        }
    }
}

void colony_hub_menu()
{
    if( !get_option<bool>( "CULL_COLONY" ) ) {
        popup( _( "Colony leadership tools are only available in colony worlds." ) );
        return;
    }

    while( true ) {
        uilist menu;
        menu.title = _( "Colony leadership" );
        menu.desc_enabled = true;
        menu.addentry_desc( 0, true, 'f', _( "Colony overview (Factions)" ),
                            _( "Camp stock, residents, expansions, and management menu." ) );
        menu.addentry_desc( 1, true, 'j', _( "Job board" ),
                            _( "Set colony-wide work priorities for residents." ) );
        menu.addentry_desc( 2, true, 'b', _( "Construction" ),
                            _( "Place blueprints and queue camp upgrades." ) );
        menu.addentry_desc( 3, true, 'e', _( "Expeditions" ),
                            _( "Plan scout, loot, hunt, and recruit missions." ) );
        menu.addentry_desc( 4, true, 'd', _( "Defense" ),
                            _( "Standing orders, guards, and retreat zones." ) );
        menu.addentry_desc( 5, true, 'q', _( "Close" ), _( "Return to the game." ) );
        menu.query();
        if( menu.ret < 0 || menu.ret == 5 ) {
            break;
        }
        if( menu.ret == 0 ) {
            g->faction_manager_ptr->display();
            break;
        }
        if( menu.ret == 1 ) {
            g->validate_camps();
            avatar &you = get_avatar();
            basecamp *camp = nullptr;
            for( const tripoint_abs_omt &elem : you.camps ) {
                if( std::optional<basecamp *> p = overmap_buffer.find_camp( elem.xy() ) ) {
                    camp = *p;
                    break;
                }
            }
            if( camp == nullptr ) {
                popup( _( "No colony camp found.  Open Construction to found one, or start a new colony." ) );
            } else {
                camp->colony_job_board_ui();
            }
            break;
        }
        if( menu.ret == 2 ) {
            colony_construction_menu();
            break;
        }
        if( menu.ret == 3 ) {
            colony_expeditions_menu();
            break;
        }
        if( menu.ret == 4 ) {
            colony_combat_menu();
            break;
        }
    }
}
