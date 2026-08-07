#pragma once
#ifndef CATA_SRC_COLONY_CAMERA_H
#define CATA_SRC_COLONY_CAMERA_H

#include <utility>
#include <vector>

#include "coords_fwd.h"

class avatar;
class npc;
enum action_id : int;

/** True when the active world is a CULL colony (god-mode leadership). */
bool is_colony_god_mode();

/**
 * Configure the engine proxy avatar as an invisible immortal observer.
 * Safe to call on new game and on load.
 */
void ensure_colony_god_avatar( avatar &pc );

/** Force the slim colony sidebar (ImGui: place/weather/time/wind + log + map). */
void ensure_colony_sidebar();

/**
 * Pan the camera without moving the body or revealing unseen tiles.
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

/** Pan camera so @p target is centered (no control / possession). */
void colony_god_focus_on( avatar &pc, const tripoint_abs_ms &target );

/**
 * On-map camp residents and player-ally followers that contribute team FOV.
 * Each pair is absolute position + vision range for camera_cache casting.
 */
std::vector<std::pair<tripoint_abs_ms, int>> colony_team_vision_sources();

/** True if this NPC is a camp resident or player-faction ally (roster / FOV). */
bool colony_is_team_member( const npc &guy );

/**
 * Open unified colony crafting: recipes/skills from camp assignees, materials
 * from camp stock, then assign a companion craft mission.
 */
void colony_open_unified_crafting();

#endif // CATA_SRC_COLONY_CAMERA_H
