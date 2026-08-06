#pragma once
#ifndef CATA_SRC_COLONY_COMBAT_H
#define CATA_SRC_COLONY_COMBAT_H

/**
 * Phase 6 leadership combat panel:
 * standing tactical orders, camp guard posts, retreat/rally zones,
 * and links to perimeter construction.
 */
void colony_combat_menu();

/**
 * CULL Phase 6 bootstrap: NPC_RETREAT rally zone and default defend
 * engagement / ally rules on assigned residents.
 */
void seed_colony_defense( class basecamp &camp );

#endif // CATA_SRC_COLONY_COMBAT_H
