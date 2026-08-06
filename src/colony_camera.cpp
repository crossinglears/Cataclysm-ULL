#include "colony_camera.h"

#include "action.h"
#include "avatar.h"
#include "cata_utility.h"
#include "colony_toolbar.h"
#include "coordinates.h"
#include "game.h"
#include "map.h"
#include "map_scale_constants.h"
#include "options.h"
#include "panels.h"
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
        shutdown_colony_toolbar();
        return;
    }
    panel_manager::get_manager().set_current_layout( "cull_colony_sidebar" );
    ensure_colony_toolbar();
}

static void colony_apply_view_delta( avatar &pc, const point_rel_ms &delta )
{
    // One tile per key — free camera, no fog reveal (seen_cache stays on parked body).
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
