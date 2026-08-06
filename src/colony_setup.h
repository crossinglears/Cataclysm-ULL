#pragma once
#ifndef CATA_SRC_COLONY_SETUP_H
#define CATA_SRC_COLONY_SETUP_H

#include <optional>
#include <string>
#include <vector>

#include "translation.h"
#include "type_id.h"

struct WORLD;
class avatar;
class basecamp;

/** Phase 7: curated starting-background mixes for starter residents. */
enum class colony_background_mix {
    mixed = 0,
    farmstead,
    scavengers,
    militia,
    medical,
};

/** Phase 7: how well-stocked the colony is at boot. */
enum class colony_gear_level {
    sparse = 0,
    standard,
    well_supplied,
};

/** Phase 7: world threat / spawn difficulty presets. */
enum class colony_difficulty {
    easy = 0,
    normal,
    hard,
    brutal,
};

/**
 * Colony New Game setup choices (Phase 7).
 * Applied before world boot and while seeding the settlement.
 */
struct colony_setup_options {
    static constexpr int min_survivors = 2;
    static constexpr int max_survivors = 12;
    static constexpr int default_survivors = 4;

    int survivor_count = default_survivors;
    colony_background_mix backgrounds = colony_background_mix::mixed;
    colony_gear_level gear = colony_gear_level::standard;
    start_location_id location = start_location_id( "sloc_shelter_safe" );
    colony_difficulty difficulty = colony_difficulty::normal;

    std::string summary() const;
};

struct colony_start_location_choice {
    start_location_id id;
    translation name;
    translation desc;
};

const std::vector<colony_start_location_choice> &colony_start_location_choices();

std::string colony_background_mix_name( colony_background_mix mix );
std::string colony_background_mix_desc( colony_background_mix mix );
std::string colony_gear_level_name( colony_gear_level gear );
std::string colony_gear_level_desc( colony_gear_level gear );
std::string colony_difficulty_name( colony_difficulty diff );
std::string colony_difficulty_desc( colony_difficulty diff );

/** Interactive setup UI. Returns nullopt if the player cancels. */
std::optional<colony_setup_options> colony_setup_ui();

/** Apply spawn / monster difficulty world options from the chosen preset. */
void apply_colony_difficulty_to_world( WORLD &world, colony_difficulty diff );

/** Pin the leadership proxy to the chosen start location (not random). */
void apply_colony_setup_to_avatar( avatar &pc, const colony_setup_options &opts );

/** Pick an NPC class for starter resident index (0-based among NPC residents). */
npc_class_id pick_colony_starter_class( const colony_setup_options &opts, int resident_index );

/** Item group used for camp starter gear crates. */
item_group_id colony_starter_gear_group( colony_gear_level gear );

/** Kcal per survivor seeded into faction food stock. */
int colony_starter_food_kcal_per_survivor( colony_gear_level gear, colony_difficulty diff );

/** Drop starter tools / materials into camp storage. */
void seed_colony_starter_gear( basecamp &camp, const colony_setup_options &opts );

/**
 * Phase 7 leadership hub: jump to jobs / construction / expeditions / defense / factions.
 */
void colony_hub_menu();

#endif // CATA_SRC_COLONY_SETUP_H
