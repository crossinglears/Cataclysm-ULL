#pragma once
#ifndef CATA_SRC_COLONY_CONSTRUCTION_H
#define CATA_SRC_COLONY_CONSTRUCTION_H

/** True when the active world is a CULL colony save. */
bool is_cull_colony_world();

/**
 * Phase 4 leadership construction panel:
 * place/configure tile blueprints, queue camp expansions, review material status.
 */
void colony_construction_menu();

#endif // CATA_SRC_COLONY_CONSTRUCTION_H
