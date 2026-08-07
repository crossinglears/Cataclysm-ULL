#include "colony_camera.h"

#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "action.h"
#include "avatar.h"
#include "basecamp.h"
#include "cata_utility.h"
#include "character_id.h"
#include "colony_sidebar.h"
#include "colony_toolbar.h"
#include "coordinates.h"
#include "faction.h"
#include "game.h"
#include "game_constants.h"
#include "map.h"
#include "map_scale_constants.h"
#include "mission_companion.h"
#include "npc.h"
#include "options.h"
#include "overmapbuffer.h"
#include "panels.h"
#include "popup.h"
#include "translations.h"
#include "type_id.h"

static const trait_id trait_DEBUG_CLOAK( "DEBUG_CLOAK" );
static const trait_id trait_DEBUG_LS( "DEBUG_LS" );
static const trait_id trait_DEBUG_NODMG( "DEBUG_NODMG" );
static const trait_id trait_DEBUG_NOSCENT( "DEBUG_NOSCENT" );
static const trait_id trait_DEBUG_NOTEMP( "DEBUG_NOTEMP" );

bool is_colony_god_mode()
{
    return get_option<bool>( "CULL_COLONY" );
}

void ensure_colony_god_avatar( avatar &pc )
{
    if( !is_colony_god_mode() ) {
        return;
    }

    // Invisible immortal observer — not a survivor on the map.
    const trait_id god_traits[] = {
        trait_DEBUG_CLOAK,
        trait_DEBUG_LS,
        trait_DEBUG_NODMG,
        trait_DEBUG_NOTEMP,
        trait_DEBUG_NOSCENT,
    };
    for( const trait_id &tr : god_traits ) {
        if( !pc.has_trait( tr ) ) {
            pc.set_mutation( tr );
        }
    }

    // Keep hunger/thirst/sleepiness from ever interrupting leadership.
    pc.set_hunger( 0 );
    pc.set_thirst( 0 );
    pc.set_sleepiness( 0 );
    pc.set_stamina( pc.get_stamina_max() );

    ensure_colony_sidebar();
}

void ensure_colony_sidebar()
{
    if( !is_colony_god_mode() ) {
        shutdown_colony_imgui_sidebar();
        shutdown_colony_toolbar();
        return;
    }
    // Keep traditional layout for terrain width reservation; content is ImGui.
    panel_manager::get_manager().set_current_layout( "cull_colony_sidebar" );
    ensure_colony_imgui_sidebar();
    ensure_colony_toolbar();
}

static void colony_apply_view_delta( avatar &pc, const point_rel_ms &delta )
{
    // One tile per key — free camera; unseen tiles stay fogged until a team member sees them.
    pc.view_offset += tripoint_rel_ms( delta, 0 );
}

bool colony_god_handle_move( action_id act, avatar &pc )
{
    if( !is_colony_god_mode() ) {
        return false;
    }

    switch( act ) {
        case ACTION_MOVE_FORTH:
        case ACTION_MOVE_FORTH_RIGHT:
        case ACTION_MOVE_RIGHT:
        case ACTION_MOVE_BACK_RIGHT:
        case ACTION_MOVE_BACK:
        case ACTION_MOVE_BACK_LEFT:
        case ACTION_MOVE_LEFT:
        case ACTION_MOVE_FORTH_LEFT:
            colony_apply_view_delta( pc, get_delta_from_movement_action( act, iso_rotate::yes ) );
            return true;
        default:
            return false;
    }
}

bool colony_god_handle_vertical( action_id act, avatar &pc )
{
    if( !is_colony_god_mode() ) {
        return false;
    }
    if( act != ACTION_MOVE_UP && act != ACTION_MOVE_DOWN ) {
        return false;
    }

    map &here = get_map();
    const int dz = act == ACTION_MOVE_UP ? 1 : -1;
    const int old_view_z = pc.posz() + pc.view_offset.z();
    const int new_view_z = clamp( old_view_z + dz, -OVERMAP_DEPTH, OVERMAP_HEIGHT );
    if( new_view_z == old_view_z ) {
        return true;
    }

    pc.view_offset.z() = new_view_z - pc.posz();
    here.invalidate_map_cache( new_view_z );
    here.build_map_cache( pc.posz() );
    here.invalidate_visibility_cache();
    return true;
}

void colony_god_center_view( avatar &pc )
{
    pc.view_offset.x() = g->driving_view_offset.x();
    pc.view_offset.y() = g->driving_view_offset.y();
    // Keep current Z peek so centering XY does not yank the floor view.
}

void colony_god_focus_on( avatar &pc, const tripoint_abs_ms &target )
{
    const tripoint_rel_ms delta = target - pc.pos_abs();
    pc.view_offset = delta;
    map &here = get_map();
    here.invalidate_map_cache( pc.posz() + pc.view_offset.z() );
    here.invalidate_visibility_cache();
}

bool colony_is_team_member( const npc &guy )
{
    if( guy.is_hallucination() ) {
        return false;
    }
    if( guy.is_player_ally() ) {
        return true;
    }
    return guy.assigned_camp.has_value();
}

std::vector<std::pair<tripoint_abs_ms, int>> colony_team_vision_sources()
{
    std::vector<std::pair<tripoint_abs_ms, int>> sources;
    if( !is_colony_god_mode() ) {
        return sources;
    }

    map &here = get_map();
    std::set<character_id> seen;
    for( npc &guy : g->all_npcs() ) {
        if( !colony_is_team_member( guy ) ) {
            continue;
        }
        if( !seen.insert( guy.getID() ).second ) {
            continue;
        }
        const tripoint_abs_ms abs = guy.pos_abs();
        if( !here.inbounds( abs ) ) {
            continue;
        }
        const int range = guy.unimpaired_range();
        if( range <= 0 ) {
            continue;
        }
        sources.emplace_back( abs, range );
    }
    return sources;
}

void colony_open_unified_crafting()
{
    if( !is_colony_god_mode() ) {
        return;
    }

    Character &player_character = get_player_character();
    if( !player_character.get_faction() ) {
        popup( _( "You have no colony faction." ) );
        return;
    }

    g->validate_camps();
    basecamp *camp = nullptr;
    for( const tripoint_abs_omt &elem : player_character.camps ) {
        std::optional<basecamp *> found = overmap_buffer.find_camp( elem.xy() );
        if( !found ) {
            continue;
        }
        if( ( *found )->get_owner() != player_character.get_faction()->id ) {
            continue;
        }
        camp = *found;
        break;
    }

    if( !camp ) {
        popup( _( "No colony camp is available for crafting." ) );
        return;
    }
    if( camp->get_npcs_assigned().empty() ) {
        popup( _( "There's nobody assigned to work at the camp!" ) );
        return;
    }

    const mission_id miss_id = { Camp_Crafting, "", {}, base_camps::base_dir };
    camp->start_crafting( miss_id );
}
