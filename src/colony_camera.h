#pragma once
#ifndef CATA_SRC_COLONY_CAMERA_H
#define CATA_SRC_COLONY_CAMERA_H

#include "coords_fwd.h"

class avatar;
enum action_id : int;

/** True when the active world is a CULL colony (god-mode leadership). */
bool is_colony_god_mode();

/**
 * Configure the engine proxy avatar as an invisible immortal observer.
 * Safe to call on new game and on load.
 */
void ensure_colony_god_avatar( avatar &pc );

/** Force the slim colony sidebar layout (place/weather/time/wind + log + minimap). */
void ensure_colony_sidebar();

/**
 * Pan the camera without moving the body or revealing fog of war.
 * Returns true if the action was handled as god-mode camera.
 */
bool colony_god_handle_move( action_id act, avatar &pc );

/**
 * Change viewed Z freely (no stairs / ceiling checks). Body stays put.
 * Returns true if handled.
 */
bool colony_god_handle_vertical( action_id act, avatar &pc );

/** Reset XY camera offset (Z view offset kept). */
void colony_god_center_view( avatar &pc );

#endif // CATA_SRC_COLONY_CAMERA_H
