#include "colony_construction.h"

#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "avatar.h"
#include "basecamp.h"
#include "character.h"
#include "clzones.h"
#include "construction.h"
#include "construction_group.h"
#include "coordinates.h"
#include "crafting.h"
#include "faction.h"
#include "faction_camp.h"
#include "game.h"
#include "map.h"
#include "memory_fast.h"
#include "messages.h"
#include "mission_companion.h"
#include "npc.h"
#include "options.h"
#include "output.h"
#include "overmapbuffer.h"
#include "player_activity.h"
#include "point.h"
#include "popup.h"
#include "recipe.h"
#include "requirements.h"
#include "string_formatter.h"
#include "translations.h"
#include "type_id.h"
#include "uilist.h"

static const activity_id ACT_MOVE_LOOT( "ACT_MOVE_LOOT" );
static const activity_id ACT_MULTIPLE_CONSTRUCTION( "ACT_MULTIPLE_CONSTRUCTION" );
static const faction_id faction_your_followers( "your_followers" );
static const zone_type_id zone_type_CONSTRUCTION_BLUEPRINT( "CONSTRUCTION_BLUEPRINT" );

bool is_cull_colony_world()
{
    return get_option<bool>( "CULL_COLONY" );
}

static basecamp *find_player_colony_camp()
{
    avatar &player_character = get_avatar();
    std::optional<basecamp *> here = overmap_buffer.find_camp( player_character.pos_abs_omt().xy() );
    if( here ) {
        return *here;
    }
    for( const tripoint_abs_omt &camp_omt : player_character.camps ) {
        std::optional<basecamp *> bcp = overmap_buffer.find_camp( camp_omt.xy() );
        if( bcp ) {
            return *bcp;
        }
    }
    // Last resort: bootstrap if somehow missing (new colony / migrated save).
    std::optional<basecamp *> created = talk_function::found_colony_camp(
                                            player_character.pos_abs_omt(), faction_your_followers, _( "Colony" ) );
    return created ? *created : nullptr;
}

static faction_id colony_fac()
{
    Character &pc = get_player_character();
    return pc.get_faction() ? pc.get_faction()->id : faction_your_followers;
}

static construction_id finalize_blueprint_construction( construction_id con_index )
{
    if( !con_index.is_valid() ) {
        return construction_id( -1 );
    }
    std::set<construction_id> skip_index;
    blueprint_options probe;
    return probe.get_final_construction( get_constructions(), con_index, skip_index );
}

static void place_or_configure_tile_blueprint( basecamp &camp )
{
    map &here = get_map();
    avatar &player_character = get_avatar();
    const faction_id fac = colony_fac();

    construction_id con_index = finalize_blueprint_construction( construction_menu( true ) );
    if( !con_index.is_valid() ) {
        return;
    }
    const construction &chosen = con_index.obj();
    auto options = make_shared_fast<blueprint_options>( chosen.post_terrain, chosen.group, con_index );

        // Prefer configuring an existing empty colony construction zone if the player made one.
        zone_manager &mgr = zone_manager::get_manager();
        zone_data *target_zone = nullptr;
        for( zone_manager::ref_zone_data &ref : mgr.get_zones( fac ) ) {
            zone_data &z = ref.get();
            if( z.get_type() != zone_type_CONSTRUCTION_BLUEPRINT || !z.get_enabled() ) {
                continue;
            }
            const blueprint_options *opts = dynamic_cast<const blueprint_options *>( &z.get_options() );
            // Seeded / player zones with empty mark are unconfigured placeholders.
            if( opts && opts->get_mark().empty() ) {
                target_zone = &z;
                break;
            }
        }

        if( target_zone ) {
            // zone_data options aren't replaceable in place — recreate at the same bounds.
            const std::string name = options->get_zone_name_suggestion().empty()
                                     ? _( "Colony construction" ) : options->get_zone_name_suggestion();
            const tripoint_abs_ms start = target_zone->get_start_point();
            const tripoint_abs_ms end = target_zone->get_end_point();
            mgr.remove( *target_zone );
            mgr.add( name, zone_type_CONSTRUCTION_BLUEPRINT, fac, false, true, start, end, options, true );
            add_msg( m_good, _( "Configured colony construction zone for %s." ),
                     chosen.group.obj().name() );
        } else {
        static_popup hint;
        hint.on_top( true );
        hint.message( "%s", _( "Select first corner of the construction blueprint zone." ) );

        tripoint_bub_ms center = player_character.pos_bub() + player_character.view_offset;
        const look_around_result first =
            g->look_around( /*show_window=*/false, center, center, false, true, false );
        if( !first.position ) {
            return;
        }

        hint.message( "%s", _( "Select second corner of the construction blueprint zone." ) );
        const look_around_result second =
            g->look_around( /*show_window=*/false, center, *first.position, true, true, false );
        if( !second.position ) {
            return;
        }

        const tripoint_abs_ms start = here.get_abs( tripoint_bub_ms(
                                          std::min( first.position->x(), second.position->x() ),
                                          std::min( first.position->y(), second.position->y() ),
                                          std::min( first.position->z(), second.position->z() ) ) );
        const tripoint_abs_ms end = here.get_abs( tripoint_bub_ms(
                                        std::max( first.position->x(), second.position->x() ),
                                        std::max( first.position->y(), second.position->y() ),
                                        std::max( first.position->z(), second.position->z() ) ) );

        std::string name = options->get_zone_name_suggestion();
        if( name.empty() ) {
            name = _( "Colony blueprint" );
        }
        mgr.add( name, zone_type_CONSTRUCTION_BLUEPRINT, fac, false, true, start, end, options, true );
        add_msg( m_good, _( "Blueprint zone placed for %s." ), chosen.group.obj().name() );
    }

    // Ensure haul + construction priorities remain active after queueing work.
    for( npc_ptr &guy : camp.get_npcs_assigned() ) {
        if( !guy ) {
            continue;
        }
        if( guy->job.get_priority_of_job( ACT_MULTIPLE_CONSTRUCTION ) <= 0 ) {
            guy->job.set_task_priority( ACT_MULTIPLE_CONSTRUCTION, 2 );
        }
        if( guy->job.get_priority_of_job( ACT_MOVE_LOOT ) <= 0 ) {
            guy->job.set_task_priority( ACT_MOVE_LOOT, 3 );
        }
    }
    add_msg( m_info, _( "Residents will haul real materials and complete the blueprint." ) );
}

struct upgrade_choice {
    point_rel_omt dir;
    basecamp_upgrade upgrade;
    bool materials_ok = false;
    std::string detail;
};

static std::vector<upgrade_choice> collect_upgrades( basecamp &camp )
{
    std::vector<upgrade_choice> out;
    camp.form_crafting_inventory();
    for( const auto &dir_pair : base_camps::all_directions ) {
        const point_rel_omt &dir = dir_pair.first;
        for( const basecamp_upgrade &upgrade : camp.available_upgrades( dir ) ) {
            upgrade_choice choice;
            choice.dir = dir;
            choice.upgrade = upgrade;
            choice.detail = camp.om_upgrade_description( upgrade.bldg, upgrade.args, true );
            const recipe &making = recipe_id( upgrade.bldg ).obj();
            if( making.is_blueprint() ) {
                auto req_it = making.blueprint_build_reqs().reqs_by_parameters.find( upgrade.args );
                if( req_it != making.blueprint_build_reqs().reqs_by_parameters.end() ) {
                    const requirement_data &reqs = req_it->second.consolidated_reqs;
                    choice.materials_ok = reqs.can_make_with_inventory(
                                              camp.crafting_inventory(), making.get_component_filter(), 1,
                                              craft_flags::none, false );
                }
            }
            out.push_back( std::move( choice ) );
        }
    }
    std::sort( out.begin(), out.end(),
    []( const upgrade_choice & a, const upgrade_choice & b ) {
        return a.upgrade.name.translated_lt( b.upgrade.name );
    } );
    return out;
}

static void queue_camp_upgrade( basecamp &camp )
{
    std::vector<upgrade_choice> choices = collect_upgrades( camp );
    if( choices.empty() ) {
        popup( _( "No camp expansions are available yet.  Finish prerequisites or survey an expansion." ) );
        return;
    }

    uilist menu;
    menu.title = _( "Queue camp expansion" );
    menu.desc_enabled = true;
    int i = 0;
    for( const upgrade_choice &choice : choices ) {
        const std::string dir_abbr =
            base_camps::all_directions.at( choice.dir ).bracket_abbr.translated();
        std::string label = string_format( "%s %s", dir_abbr, choice.upgrade.name.translated() );
        if( choice.upgrade.in_progress ) {
            label += _( " (in progress)" );
        } else if( !choice.materials_ok ) {
            label += _( " (need materials)" );
        } else if( !choice.upgrade.avail ) {
            label += _( " (unavailable)" );
        }
        const bool enabled = choice.upgrade.avail && !choice.upgrade.in_progress;
        menu.addentry_desc( i++, enabled, MENU_AUTOASSIGN, label, choice.detail );
    }
    menu.query();
    if( menu.ret < 0 || static_cast<size_t>( menu.ret ) >= choices.size() ) {
        return;
    }

    const upgrade_choice &picked = choices[menu.ret];
    if( !picked.materials_ok ) {
        popup( _( "Camp stocks are missing required materials.  Haul real items into colony storage "
                  "(LOOT_UNSORTED / CAMP_STORAGE), then try again." ) );
        return;
    }

    mission_id miss_id;
    miss_id.id = Camp_Upgrade;
    miss_id.parameters = picked.upgrade.bldg;
    miss_id.mapgen_args = picked.upgrade.args;
    miss_id.dir = picked.dir;
    camp.start_upgrade( miss_id );
}

static void show_construction_status( basecamp &camp )
{
    zone_manager &mgr = zone_manager::get_manager();
    const faction_id fac = colony_fac();

    std::string report;
    report += string_format( _( "Camp: %s\n" ), camp.camp_name() );

    int configured = 0;
    int empty = 0;
    for( const zone_manager::ref_zone_data &ref : mgr.get_zones( fac ) ) {
        const zone_data &z = ref.get();
        if( z.get_type() != zone_type_CONSTRUCTION_BLUEPRINT || !z.get_enabled() ) {
            continue;
        }
        const blueprint_options *opts = dynamic_cast<const blueprint_options *>( &z.get_options() );
        if( opts && !opts->get_mark().empty() ) {
            configured++;
            report += string_format( _( "• Tile blueprint: %s\n" ), z.get_name() );
        } else {
            empty++;
            report += string_format( _( "• Unconfigured construction zone: %s\n" ), z.get_name() );
        }
    }
    if( configured + empty == 0 ) {
        report += _( "• No construction blueprint zones.\n" );
    }

    int ready = 0;
    int short_mats = 0;
    int in_progress = 0;
    for( const upgrade_choice &choice : collect_upgrades( camp ) ) {
        if( choice.upgrade.in_progress ) {
            in_progress++;
            report += string_format( _( "• In progress: %s\n" ), choice.upgrade.name.translated() );
        } else if( choice.materials_ok && choice.upgrade.avail ) {
            ready++;
        } else if( choice.upgrade.avail ) {
            short_mats++;
            report += string_format( _( "• Waiting on materials: %s\n" ),
                                     choice.upgrade.name.translated() );
        }
    }
    report += string_format(
                  _( "\nConfigured tile blueprints: %d\nUnconfigured zones: %d\n"
                     "Camp upgrades ready: %d\nCamp upgrades short materials: %d\n"
                     "Camp upgrades in progress: %d\n" ),
                  configured, empty, ready, short_mats, in_progress );

    int builders = 0;
    int haulers = 0;
    for( const npc_ptr &guy : camp.get_npcs_assigned() ) {
        if( !guy ) {
            continue;
        }
        if( guy->job.get_priority_of_job( ACT_MULTIPLE_CONSTRUCTION ) > 0 ) {
            builders++;
        }
        if( guy->job.get_priority_of_job( ACT_MOVE_LOOT ) > 0 ) {
            haulers++;
        }
    }
    report += string_format( _( "Residents: construction %d / haul %d\n" ), builders, haulers );
    popup( "%s", report );
}

void colony_construction_menu()
{
    if( !is_cull_colony_world() ) {
        popup( _( "Colony Construction is only available in colony worlds." ) );
        return;
    }

    basecamp *camp = find_player_colony_camp();
    if( !camp ) {
        popup( _( "No colony camp found.  Start a new colony game first." ) );
        return;
    }

    while( true ) {
        uilist menu;
        menu.title = _( "Colony Construction" );
        menu.desc_enabled = true;
        menu.addentry_desc( 0, true, 'b', _( "Place / configure tile blueprint" ),
                            _( "Choose a construction and assign it to a blueprint zone.  Residents "
                               "gather real materials and complete it via ACT_MULTIPLE_CONSTRUCTION." ) );
        menu.addentry_desc( 1, true, 'e', _( "Queue camp expansion" ),
                            _( "Start a basecamp blueprint upgrade using camp stocks and companions." ) );
        menu.addentry_desc( 2, true, 's', _( "Construction status" ),
                            _( "List tile blueprints, upgrades, and material shortages." ) );
        menu.addentry_desc( 3, true, 'z', _( "Manage haul / storage zones" ),
                            _( "Open the zone manager for material stockpiles and haul targets." ) );
        menu.addentry_desc( 4, true, 'j', _( "Colony job board" ),
                            _( "Adjust construction / haul priorities for residents." ) );
        menu.addentry_desc( 5, true, 'c', _( "Open full camp missions" ),
                            _( "Faction camp mission board (gather materials, upgrades, crafting)." ) );
        menu.query();
        if( menu.ret < 0 ) {
            break;
        }
        switch( menu.ret ) {
            case 0:
                place_or_configure_tile_blueprint( *camp );
                break;
            case 1:
                queue_camp_upgrade( *camp );
                break;
            case 2:
                show_construction_status( *camp );
                break;
            case 3:
                zone_manager_ui::display_zone_manager();
                camp->form_storage_zones( get_map(), get_avatar().pos_abs() );
                break;
            case 4:
                camp->colony_job_board_ui();
                break;
            case 5: {
                npc *speaker = nullptr;
                for( const npc_ptr &guy : camp->get_npcs_assigned() ) {
                    if( guy ) {
                        speaker = guy.get();
                        break;
                    }
                }
                if( !speaker ) {
                    for( npc *guy : g->allies() ) {
                        if( guy ) {
                            speaker = guy;
                            break;
                        }
                    }
                }
                if( !speaker ) {
                    popup( _( "No colony survivors are available to open the camp board." ) );
                    break;
                }
                talk_function::basecamp_mission( *speaker );
                break;
            }
            default:
                break;
        }
    }
}
