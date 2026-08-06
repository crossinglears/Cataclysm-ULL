#include "colony_combat.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "avatar.h"
#include "basecamp.h"
#include "character.h"
#include "clzones.h"
#include "colony_construction.h"
#include "coordinates.h"
#include "creature.h"
#include "debug.h"
#include "faction.h"
#include "faction_camp.h"
#include "game.h"
#include "map.h"
#include "messages.h"
#include "npc.h"
#include "npctalk.h"
#include "options.h"
#include "output.h"
#include "overmapbuffer.h"
#include "point.h"
#include "popup.h"
#include "string_formatter.h"
#include "translations.h"
#include "type_id.h"
#include "uilist.h"

static const faction_id faction_your_followers( "your_followers" );
static const zone_type_id zone_type_CONSTRUCTION_BLUEPRINT( "CONSTRUCTION_BLUEPRINT" );
static const zone_type_id zone_type_NPC_RETREAT( "NPC_RETREAT" );

enum class colony_tactical_order : int {
    hold = 0,
    retreat,
    defend,
    avoid,
    escort
};

static const char *order_name( colony_tactical_order order )
{
    switch( order ) {
        case colony_tactical_order::hold:
            return _( "Hold position" );
        case colony_tactical_order::retreat:
            return _( "Retreat / fall back" );
        case colony_tactical_order::defend:
            return _( "Defend perimeter" );
        case colony_tactical_order::avoid:
            return _( "Avoid combat" );
        case colony_tactical_order::escort:
            return _( "Escort leader" );
    }
    return _( "Unknown" );
}

static const char *order_desc( colony_tactical_order order )
{
    switch( order ) {
        case colony_tactical_order::hold:
            return _( "Guards hold in place (engagement: no move).  Fight only what enters range." );
        case colony_tactical_order::retreat:
            return _( "Disengage and fall back to the NPC_RETREAT rally zone.  Do not seek fights." );
        case colony_tactical_order::defend:
            return _( "Engage hostiles freely while holding the line.  Camp residents resume duties." );
        case colony_tactical_order::avoid:
            return _( "Forbid engagement.  Survivors refuse to start fights." );
        case colony_tactical_order::escort:
            return _( "Follow the leader closely and fight nearby threats." );
    }
    return "";
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
    std::optional<basecamp *> created = talk_function::found_colony_camp(
                                            player_character.pos_abs_omt(), faction_your_followers, _( "Colony" ) );
    return created ? *created : nullptr;
}

static faction_id colony_fac()
{
    Character &pc = get_player_character();
    return pc.get_faction() ? pc.get_faction()->id : faction_your_followers;
}

static std::vector<npc *> camp_residents( basecamp &camp )
{
    std::vector<npc *> out;
    for( const npc_ptr &guy : camp.get_npcs_assigned() ) {
        if( guy ) {
            out.push_back( guy.get() );
        }
    }
    return out;
}

static std::optional<tripoint_abs_ms> nearest_retreat( const tripoint_abs_ms &from,
        const faction_id &fac )
{
    return zone_manager::get_manager().get_nearest( zone_type_NPC_RETREAT, from,
            60, fac );
}

static void clear_combat_stance_flags( npc &guy )
{
    guy.rules.clear_flag( ally_rule::forbid_engage );
    guy.rules.clear_flag( ally_rule::follow_close );
    guy.rules.clear_flag( ally_rule::hold_the_line );
    guy.rules.clear_flag( ally_rule::follow_distance_2 );
}

static void apply_order_to_npc( npc &guy, colony_tactical_order order,
                                const std::optional<tripoint_abs_ms> &rally )
{
    clear_combat_stance_flags( guy );

    switch( order ) {
        case colony_tactical_order::hold:
            guy.rules.engagement = combat_engagement::NO_MOVE;
            guy.rules.set_flag( ally_rule::hold_the_line );
            talk_function::assign_guard( guy );
            guy.set_guard_pos( guy.pos_abs() );
            break;
        case colony_tactical_order::retreat: {
            guy.rules.engagement = combat_engagement::NONE;
            guy.rules.set_flag( ally_rule::follow_close );
            const tripoint_abs_ms target = rally ? *rally : guy.pos_abs();
            talk_function::assign_guard( guy );
            guy.set_guard_pos( target );
            break;
        }
        case colony_tactical_order::defend:
            guy.rules.engagement = combat_engagement::ALL;
            guy.rules.set_flag( ally_rule::hold_the_line );
            // Resume camp work when not actively fighting.
            if( guy.mission == NPC_MISSION_GUARD_ALLY || guy.mission == NPC_MISSION_NULL ||
                guy.is_following() || guy.is_leader() ) {
                talk_function::return_to_camp_duties( guy );
            }
            break;
        case colony_tactical_order::avoid:
            guy.rules.engagement = combat_engagement::NONE;
            guy.rules.set_flag( ally_rule::forbid_engage );
            if( guy.is_guarding() ) {
                talk_function::return_to_camp_duties( guy );
            }
            break;
        case colony_tactical_order::escort:
            guy.rules.engagement = combat_engagement::CLOSE;
            guy.rules.set_flag( ally_rule::follow_close );
            guy.rules.set_flag( ally_rule::follow_distance_2 );
            talk_function::stop_guard( guy );
            break;
    }
}

static void apply_standing_order( basecamp &camp, colony_tactical_order order )
{
    const faction_id fac = colony_fac();
    std::optional<tripoint_abs_ms> rally =
        nearest_retreat( get_avatar().pos_abs(), fac );

    if( order == colony_tactical_order::retreat && !rally ) {
        popup( _( "No NPC_RETREAT rally zone found.  Place one under Defense zones first." ) );
        return;
    }

    int applied = 0;
    for( npc *guy : camp_residents( camp ) ) {
        apply_order_to_npc( *guy, order, rally );
        applied++;
    }

    if( applied == 0 ) {
        popup( _( "No camp residents to receive orders." ) );
        return;
    }

    add_msg( m_good, _( "Standing order set: %s (%d survivors)." ),
             order_name( order ), applied );
}

static colony_tactical_order infer_order( const npc &guy )
{
    if( guy.rules.has_flag( ally_rule::forbid_engage ) ) {
        return colony_tactical_order::avoid;
    }
    if( guy.is_following() && guy.rules.has_flag( ally_rule::follow_close ) ) {
        return colony_tactical_order::escort;
    }
    if( guy.rules.engagement == combat_engagement::NONE &&
        guy.rules.has_flag( ally_rule::follow_close ) ) {
        return colony_tactical_order::retreat;
    }
    if( guy.rules.engagement == combat_engagement::NO_MOVE && guy.is_guarding() ) {
        return colony_tactical_order::hold;
    }
    return colony_tactical_order::defend;
}

static void choose_standing_order( basecamp &camp )
{
    uilist menu;
    menu.title = _( "Standing tactical orders" );
    menu.desc_enabled = true;
    menu.addentry_desc( static_cast<int>( colony_tactical_order::hold ), true, 'h',
                        order_name( colony_tactical_order::hold ),
                        order_desc( colony_tactical_order::hold ) );
    menu.addentry_desc( static_cast<int>( colony_tactical_order::retreat ), true, 'r',
                        order_name( colony_tactical_order::retreat ),
                        order_desc( colony_tactical_order::retreat ) );
    menu.addentry_desc( static_cast<int>( colony_tactical_order::defend ), true, 'd',
                        order_name( colony_tactical_order::defend ),
                        order_desc( colony_tactical_order::defend ) );
    menu.addentry_desc( static_cast<int>( colony_tactical_order::avoid ), true, 'a',
                        order_name( colony_tactical_order::avoid ),
                        order_desc( colony_tactical_order::avoid ) );
    menu.addentry_desc( static_cast<int>( colony_tactical_order::escort ), true, 'e',
                        order_name( colony_tactical_order::escort ),
                        order_desc( colony_tactical_order::escort ) );
    menu.query();
    if( menu.ret < 0 ) {
        return;
    }
    apply_standing_order( camp, static_cast<colony_tactical_order>( menu.ret ) );
}

static bool pick_zone_corners( tripoint_abs_ms &start, tripoint_abs_ms &end,
                               const std::string &first_hint, const std::string &second_hint )
{
    map &here = get_map();
    avatar &player_character = get_avatar();
    tripoint_bub_ms center = player_character.pos_bub() + player_character.view_offset;

    static_popup hint;
    hint.on_top( true );
    hint.message( "%s", first_hint );

    const look_around_result first =
        g->look_around( /*show_window=*/false, center, center, false, true, false );
    if( !first.position ) {
        return false;
    }

    hint.message( "%s", second_hint );
    const look_around_result second =
        g->look_around( /*show_window=*/false, center, *first.position, true, true, false );
    if( !second.position ) {
        return false;
    }

    start = here.get_abs( tripoint_bub_ms(
                              std::min( first.position->x(), second.position->x() ),
                              std::min( first.position->y(), second.position->y() ),
                              std::min( first.position->z(), second.position->z() ) ) );
    end = here.get_abs( tripoint_bub_ms(
                            std::max( first.position->x(), second.position->x() ),
                            std::max( first.position->y(), second.position->y() ),
                            std::max( first.position->z(), second.position->z() ) ) );
    return true;
}

static void place_retreat_zone( basecamp &/*camp*/ )
{
    tripoint_abs_ms start;
    tripoint_abs_ms end;
    if( !pick_zone_corners( start, end,
                            _( "Select first corner of the retreat / rally zone." ),
                            _( "Select second corner of the retreat / rally zone." ) ) ) {
        return;
    }

    zone_manager &mgr = zone_manager::get_manager();
    mgr.add( _( "Colony rally" ), zone_type_NPC_RETREAT, colony_fac(), false, true,
             start, end, nullptr, true );
    add_msg( m_good, _( "Retreat / rally zone placed.  Retreat orders and fleeing AI use it." ) );
}

static void assign_guard_post( basecamp &camp )
{
    std::vector<npc *> residents = camp_residents( camp );
    if( residents.empty() ) {
        popup( _( "No camp residents available to guard." ) );
        return;
    }

    uilist pick;
    pick.title = _( "Assign guard post" );
    for( size_t i = 0; i < residents.size(); i++ ) {
        npc *guy = residents[i];
        std::string label = guy->get_name();
        if( guy->is_guarding() ) {
            label += _( " (already guarding)" );
        }
        pick.addentry( static_cast<int>( i ), true, 0, label );
    }
    pick.query();
    if( pick.ret < 0 || static_cast<size_t>( pick.ret ) >= residents.size() ) {
        return;
    }
    npc &guy = *residents[pick.ret];

    avatar &player_character = get_avatar();
    tripoint_bub_ms center = player_character.pos_bub() + player_character.view_offset;
    static_popup hint;
    hint.on_top( true );
    hint.message( "%s", _( "Select guard post position." ) );
    const look_around_result result =
        g->look_around( /*show_window=*/false, center, center, false, true, false );
    if( !result.position ) {
        return;
    }

    clear_combat_stance_flags( guy );
    guy.rules.engagement = combat_engagement::ALL;
    guy.rules.set_flag( ally_rule::hold_the_line );
    talk_function::assign_guard( guy );
    guy.set_guard_pos( get_map().get_abs( *result.position ) );
    add_msg( m_good, _( "%s will hold that post and defend the area." ), guy.get_name() );
}

static void recall_guards( basecamp &camp )
{
    int recalled = 0;
    for( npc *guy : camp_residents( camp ) ) {
        if( guy->is_guarding() ) {
            talk_function::return_to_camp_duties( *guy );
            clear_combat_stance_flags( *guy );
            guy->rules.engagement = combat_engagement::ALL;
            guy->rules.set_flag( ally_rule::hold_the_line );
            recalled++;
        }
    }
    if( recalled == 0 ) {
        popup( _( "No guards are currently posted." ) );
        return;
    }
    add_msg( m_good, _( "Recalled %d guard(s) to camp duties (defend stance)." ), recalled );
}

static void show_defense_status( basecamp &camp )
{
    const faction_id fac = colony_fac();
    zone_manager &mgr = zone_manager::get_manager();
    const tripoint_abs_ms origin = project_to<coords::ms>( camp.camp_omt_pos() );

    int retreat_zones = 0;
    for( const zone_manager::ref_zone_data &ref : mgr.get_zones( fac ) ) {
        const zone_data &z = ref.get();
        if( z.get_type() == zone_type_NPC_RETREAT && z.get_enabled() ) {
            retreat_zones++;
        }
    }

    int guards = 0;
    int workers = 0;
    int following = 0;
    std::string stance_summary = _( "none" );
    const std::vector<npc *> residents = camp_residents( camp );
    if( !residents.empty() ) {
        stance_summary = order_name( infer_order( *residents.front() ) );
    }

    std::string roster;
    for( npc *guy : residents ) {
        std::string role = _( "worker" );
        if( guy->is_following() || guy->is_leader() ) {
            role = _( "escort" );
            following++;
        } else if( guy->is_guarding() ) {
            role = _( "guard" );
            guards++;
        } else {
            workers++;
        }
        roster += string_format( _( "  %s — %s [%s]\n" ), guy->get_name(), role,
                                 order_name( infer_order( *guy ) ) );
    }

    int blueprint_zones = 0;
    for( const zone_manager::ref_zone_data &ref : mgr.get_zones( fac ) ) {
        const zone_data &z = ref.get();
        if( z.get_type() == zone_type_CONSTRUCTION_BLUEPRINT && z.get_enabled() ) {
            blueprint_zones++;
        }
    }

    // Nearby hostiles on the loaded map (rough threat read).
    int hostiles = 0;
    for( Creature *critter : g->get_creatures_if( []( const Creature & c ) {
    return c.is_monster() || c.is_npc();
    } ) ) {
        if( !critter ) {
            continue;
        }
        if( get_avatar().attitude_to( *critter ) == Creature::Attitude::HOSTILE &&
            rl_dist( get_avatar().pos_bub(), critter->pos_bub() ) <= 30 ) {
            hostiles++;
        }
    }

    std::string report;
    report += string_format( _( "Camp: %s\n" ), camp.camp_name() );
    report += string_format( _( "Inferred standing order: %s\n" ), stance_summary );
    report += string_format( _( "Residents: %d (guards %d / workers %d / escorting %d)\n" ),
                             static_cast<int>( residents.size() ), guards, workers, following );
    report += string_format( _( "Retreat / rally zones: %d\n" ), retreat_zones );
    report += string_format( _( "Construction blueprint zones (perimeter builds): %d\n" ),
                             blueprint_zones );
    report += string_format( _( "Nearby hostiles (loaded map, ≤30 tiles): %d\n\n" ), hostiles );
    if( roster.empty() ) {
        report += _( "No assigned residents.\n" );
    } else {
        report += _( "Roster:\n" );
        report += roster;
    }
    report += string_format( _( "\nCamp OMT origin ~ %s\n" ), origin.to_string() );
    if( !nearest_retreat( get_avatar().pos_abs(), fac ) ) {
        report += _( "\nWarning: no retreat zone in range — place one before issuing Retreat.\n" );
    }

    popup( "%s", report );
}

void seed_colony_defense( basecamp &camp )
{
    faction *fac = g->faction_manager_ptr->get( camp.get_owner() );
    if( !fac ) {
        debugmsg( "seed_colony_defense: camp has no faction" );
        return;
    }

    zone_manager &mgr = zone_manager::get_manager();
    const tripoint_abs_ms omt_origin = project_to<coords::ms>( camp.camp_omt_pos() );
    const faction_id fac_id = fac->id;

    // Central rally / fall-back zone for retreat orders and careful_retreat AI.
    if( !mgr.has_near( zone_type_NPC_RETREAT, omt_origin + point( 12, 12 ), 24, fac_id ) ) {
        mgr.add( _( "Colony rally" ), zone_type_NPC_RETREAT, fac_id, false, true,
                 omt_origin + point( 10, 10 ), omt_origin + point( 13, 13 ), nullptr, true );
    }

    for( const npc_ptr &guy : camp.get_npcs_assigned() ) {
        if( !guy ) {
            continue;
        }
        clear_combat_stance_flags( *guy );
        guy->rules.engagement = combat_engagement::ALL;
        guy->rules.set_flag( ally_rule::hold_the_line );
        guy->rules.clear_flag( ally_rule::forbid_engage );
    }
}

void colony_combat_menu()
{
    if( !is_cull_colony_world() ) {
        popup( _( "Colony Defense is only available in colony worlds." ) );
        return;
    }

    basecamp *camp = find_player_colony_camp();
    if( !camp ) {
        popup( _( "No colony camp found.  Start a new colony game first." ) );
        return;
    }

    while( true ) {
        uilist menu;
        menu.title = _( "Colony Defense" );
        menu.desc_enabled = true;
        menu.addentry_desc( 0, true, 'o', _( "Standing tactical orders" ),
                            _( "Hold, retreat, defend, avoid combat, or escort — applied to all "
                               "camp residents via engagement rules, ally flags, and guard posts." ) );
        menu.addentry_desc( 1, true, 'g', _( "Assign guard post" ),
                            _( "Pick a resident and a map position.  They hold and fight (defend)." ) );
        menu.addentry_desc( 2, true, 'c', _( "Recall guards to duties" ),
                            _( "Send posted guards back to camp work under a defend stance." ) );
        menu.addentry_desc( 3, true, 'r', _( "Place retreat / rally zone" ),
                            _( "Create an NPC_RETREAT zone for fall-back orders and fleeing AI." ) );
        menu.addentry_desc( 4, true, 'p', _( "Perimeter construction" ),
                            _( "Open Colony Construction to place wall / barricade blueprints "
                               "and queue camp expansions that fortify the perimeter." ) );
        menu.addentry_desc( 5, true, 's', _( "Defense status" ),
                            _( "Guards, standing order, rally zones, blueprints, nearby threats." ) );
        menu.addentry_desc( 6, true, 'j', _( "Colony job board" ),
                            _( "Adjust work priorities (haul, farm, construction)." ) );
        menu.query();
        if( menu.ret < 0 ) {
            break;
        }
        switch( menu.ret ) {
            case 0:
                choose_standing_order( *camp );
                break;
            case 1:
                assign_guard_post( *camp );
                break;
            case 2:
                recall_guards( *camp );
                break;
            case 3:
                place_retreat_zone( *camp );
                break;
            case 4:
                colony_construction_menu();
                break;
            case 5:
                show_defense_status( *camp );
                break;
            case 6:
                camp->colony_job_board_ui();
                break;
            default:
                break;
        }
    }
}
