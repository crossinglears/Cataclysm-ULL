#include "colony_expeditions.h"

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "avatar.h"
#include "basecamp.h"
#include "calendar.h"
#include "colony_construction.h"
#include "coordinates.h"
#include "enums.h"
#include "faction_camp.h"
#include "game.h"
#include "game_constants.h"
#include "item.h"
#include "memory_fast.h"
#include "messages.h"
#include "mission_companion.h"
#include "npc.h"
#include "output.h"
#include "overmapbuffer.h"
#include "overmap_ui.h"
#include "point.h"
#include "popup.h"
#include "string_formatter.h"
#include "translations.h"
#include "type_id.h"
#include "uilist.h"

static const skill_id skill_survival( "survival" );

namespace
{

constexpr const char *loot_prefix = "colony_loot_";

struct loot_target {
    std::string param_suffix;
    std::string omt_match;
    translation name;
    translation desc;
};

static const std::vector<loot_target> loot_targets = {
    {
        "pharm", "s_pharm",
        to_translation( "Pharmacy" ),
        to_translation( "Raid a nearby pharmacy for medicine and medical supplies." )
    },
    {
        "hardware", "s_hardware",
        to_translation( "Hardware store" ),
        to_translation( "Raid a nearby hardware store for tools and building materials." )
    },
    {
        "gun", "s_gun",
        to_translation( "Gun store" ),
        to_translation( "Raid a nearby gun store for weapons and ammunition." )
    },
    {
        "house", "house",
        to_translation( "House" ),
        to_translation( "Scavenge a nearby house for mixed supplies." )
    },
};

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
    return nullptr;
}

static mission_id hub_mission( mission_kind kind, const std::string &parameters = "" )
{
    return { kind, parameters, {}, base_camps::base_dir };
}

static void run_mission( basecamp &camp, mission_kind kind, bool ret,
                         const std::string &parameters = "" )
{
    camp.handle_mission( { hub_mission( kind, parameters ), ret } );
}

static std::string worker_summary( basecamp &camp, const mission_id &miss_id )
{
    const comp_list workers = camp.get_mission_workers( miss_id );
    if( workers.empty() ) {
        return _( "None active." );
    }
    std::string out;
    for( const npc_ptr &guy : workers ) {
        if( !guy ) {
            continue;
        }
        std::string eta = _( "ready" );
        if( guy->companion_mission_time_ret > calendar::turn ) {
            eta = to_string( guy->companion_mission_time_ret - calendar::turn );
        }
        out += string_format( _( "• %s — %s\n" ), guy->get_name(), eta );
    }
    return out;
}

static void show_expedition_status( basecamp &camp )
{
    camp.ensure_colony_expedition_provides();
    camp.reset_camp_workers();

    std::string report = _( "Active colony expeditions:\n\n" );
    report += _( "Scout:\n" ) + worker_summary( camp, hub_mission( Camp_Scouting ) ) + "\n";
    report += _( "Loot:\n" );
    bool any_loot = false;
    for( const loot_target &tgt : loot_targets ) {
        const mission_id mid = hub_mission( Camp_Gather_Materials, std::string( loot_prefix ) +
                                            tgt.param_suffix );
        const comp_list workers = camp.get_mission_workers( mid );
        if( workers.empty() ) {
            continue;
        }
        any_loot = true;
        report += string_format( _( "%s:\n" ), tgt.name.translated() );
        report += worker_summary( camp, mid );
    }
    if( !any_loot ) {
        report += _( "None active.\n" );
    }
    report += std::string( "\n" ) + _( "Hunt:\n" ) +
              worker_summary( camp, hub_mission( Camp_Hunting ) ) + "\n";
    report += _( "Recruit:\n" ) + worker_summary( camp, hub_mission( Camp_Recruiting ) );

    report += string_format( _( "\nCamp food endurance (~moderate work): %d days\n" ),
                             camp.camp_food_supply_days( MODERATE_EXERCISE ) );
    popup( "%s", report );
}

static void recover_ready( basecamp &camp )
{
    camp.ensure_colony_expedition_provides();
    camp.reset_camp_workers();

    struct recover_entry {
        mission_id mid;
        std::string label;
        bool ready = false;
    };
    std::vector<recover_entry> entries;

    auto add_kind = [&]( mission_kind kind, const std::string & label,
    const std::string & parameters = "" ) {
        const mission_id mid = hub_mission( kind, parameters );
        for( const npc_ptr &guy : camp.get_mission_workers( mid ) ) {
            if( !guy ) {
                continue;
            }
            recover_entry e;
            e.mid = mid;
            e.label = string_format( "%s — %s", label, guy->get_name() );
            e.ready = guy->companion_mission_time_ret <= calendar::turn;
            if( !e.ready ) {
                e.label += string_format( _( " (ETA %s)" ),
                                          to_string( guy->companion_mission_time_ret - calendar::turn ) );
            }
            entries.push_back( e );
        }
    };

    add_kind( Camp_Scouting, _( "Scout" ) );
    for( const loot_target &tgt : loot_targets ) {
        add_kind( Camp_Gather_Materials, string_format( _( "Loot %s" ), tgt.name.translated() ),
                  std::string( loot_prefix ) + tgt.param_suffix );
    }
    add_kind( Camp_Hunting, _( "Hunt" ) );
    add_kind( Camp_Recruiting, _( "Recruit" ) );

    if( entries.empty() ) {
        popup( _( "No expeditions are currently in the field." ) );
        return;
    }

    uilist menu;
    menu.title = _( "Recover expedition" );
    menu.desc_enabled = true;
    for( size_t i = 0; i < entries.size(); i++ ) {
        menu.addentry_desc( static_cast<int>( i ), entries[i].ready, 0, entries[i].label,
                            entries[i].ready
                            ? _( "Companion is ready to return with results." )
                            : _( "Still traveling / working." ) );
    }
    menu.query();
    if( menu.ret < 0 || static_cast<size_t>( menu.ret ) >= entries.size() ) {
        return;
    }
    const recover_entry &picked = entries[menu.ret];
    camp.handle_mission( { picked.mid, true } );
}

static void start_scout( basecamp &camp )
{
    camp.ensure_colony_expedition_provides();
    popup( _( "Plan a scout path on the overmap.  Map knowledge is revealed along the route; "
              "expect encounters." ) );
    run_mission( camp, Camp_Scouting, false );
}

static void start_hunt( basecamp &camp )
{
    camp.ensure_colony_expedition_provides();
    run_mission( camp, Camp_Hunting, false );
}

static void start_recruit( basecamp &camp )
{
    camp.ensure_colony_expedition_provides();
    run_mission( camp, Camp_Recruiting, false );
}

static time_duration estimate_round_trip( int omt_dist )
{
    // Rough: 30 minutes per OMT tile each way, floor of 4 hours.
    return std::max( 4_hours, time_duration::from_minutes( 30 * omt_dist * 2 ) );
}

static void start_loot( basecamp &camp )
{
    camp.ensure_colony_expedition_provides();

    uilist type_menu;
    type_menu.title = _( "Loot expedition target" );
    type_menu.desc_enabled = true;
    for( size_t i = 0; i < loot_targets.size(); i++ ) {
        type_menu.addentry_desc( static_cast<int>( i ), true, 0,
                                 loot_targets[i].name.translated(),
                                 loot_targets[i].desc.translated() );
    }
    type_menu.addentry_desc( static_cast<int>( loot_targets.size() ), true, 'm',
                             _( "Choose destination on map" ),
                             _( "Pick any overmap tile within range; loot whatever is there." ) );
    type_menu.query();
    if( type_menu.ret < 0 ) {
        return;
    }

    const bool free_pick = type_menu.ret == static_cast<int>( loot_targets.size() );
    const loot_target *tgt = free_pick ? nullptr : &loot_targets[type_menu.ret];
    const std::string param = free_pick
                              ? std::string( loot_prefix ) + "any"
                              : std::string( loot_prefix ) + tgt->param_suffix;

    // Only one loot party of a given flavor at a time (matches camp mission slots).
    camp.reset_camp_workers();
    const mission_id miss_id = hub_mission( Camp_Gather_Materials, param );
    if( !camp.get_mission_workers( miss_id ).empty() ) {
        popup( _( "A loot expedition of that type is already in the field." ) );
        return;
    }

    tripoint_abs_omt dest = tripoint_abs_omt::invalid;
    if( free_pick ) {
        dest = ui::omap::choose_point( _( "Select a loot destination (max 60 tiles away)." ) );
        if( dest.is_invalid() ) {
            return;
        }
        const int dist = rl_dist( camp.camp_omt_pos().xy(), dest.xy() );
        if( dist < 1 || dist > 60 ) {
            popup( _( "Choose a tile between 1 and 60 overmap tiles from camp." ) );
            return;
        }
        if( dest.z() != camp.camp_omt_pos().z() ) {
            popup( _( "Loot destinations must be on the same z-level as the camp." ) );
            return;
        }
    } else {
        dest = overmap_buffer.find_closest( camp.camp_omt_pos(), tgt->omt_match, 60, false,
                                            ot_match_type::contains );
        if( dest.is_invalid() ) {
            popup( _( "No matching %s found within 60 tiles of the colony." ),
                   tgt->name.translated() );
            return;
        }
        if( !query_yn( _( "Send a party to loot %s at %s?" ),
                       tgt->name.translated(), dest.to_string() ) ) {
            return;
        }
    }

    const int dist = rl_dist( camp.camp_omt_pos().xy(), dest.xy() );
    const time_duration travel = estimate_round_trip( dist );
    if( !query_yn( _( "Loot expedition estimate:\nDestination: %s\nRound-trip: ~%s\nProceed?" ),
                   overmap_buffer.ter( dest ).id().str(), to_string( travel ) ) ) {
        return;
    }

    npc_ptr comp = camp.start_mission( miss_id, travel, true,
                                       _( "departs on a loot expedition…" ), false, {},
                                       skill_survival, 0, MODERATE_EXERCISE );
    if( comp ) {
        comp->companion_mission_points.clear();
        comp->companion_mission_points.push_back( dest );
        add_msg( m_good, _( "%s heads out to loot %s." ),
                 comp->get_name(), overmap_buffer.ter( dest ).id().str() );
    }
}

} // namespace

void colony_expeditions_menu()
{
    if( !is_cull_colony_world() ) {
        popup( _( "Colony Expeditions are only available in colony worlds." ) );
        return;
    }

    basecamp *camp = find_player_colony_camp();
    if( !camp ) {
        popup( _( "No colony camp found.  Start a new colony game first." ) );
        return;
    }
    camp->ensure_colony_expedition_provides();

    while( true ) {
        uilist menu;
        menu.title = _( "Colony Expeditions" );
        menu.desc_enabled = true;
        menu.addentry_desc( 0, true, 's', _( "Scout" ),
                            _( "Send a companion along a planned overmap path.  Reveals terrain; "
                               "risk of combat encounters." ) );
        menu.addentry_desc( 1, true, 'l', _( "Loot" ),
                            _( "Raid a pharmacy, hardware store, gun store, or house.  Real items "
                               "return to camp stocks." ) );
        menu.addentry_desc( 2, true, 'h', _( "Hunt" ),
                            _( "Send a hunter for animal corpses (meat / hides) into camp storage." ) );
        menu.addentry_desc( 3, true, 'r', _( "Recruit" ),
                            _( "Search for survivors who may join the colony roster." ) );
        menu.addentry_desc( 4, true, 'c', _( "Recover expedition" ),
                            _( "Bring back companions whose missions have finished." ) );
        menu.addentry_desc( 5, true, 'a', _( "Expedition status" ),
                            _( "List active scout / loot / hunt / recruit parties and ETAs." ) );
        menu.addentry_desc( 6, true, 'm', _( "Open full camp missions" ),
                            _( "Faction camp mission board (gather, craft, fortify, etc.)." ) );
        menu.query();
        if( menu.ret < 0 ) {
            break;
        }
        switch( menu.ret ) {
            case 0:
                start_scout( *camp );
                break;
            case 1:
                start_loot( *camp );
                break;
            case 2:
                start_hunt( *camp );
                break;
            case 3:
                start_recruit( *camp );
                break;
            case 4:
                recover_ready( *camp );
                break;
            case 5:
                show_expedition_status( *camp );
                break;
            case 6: {
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
