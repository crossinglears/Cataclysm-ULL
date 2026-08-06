#pragma once
#ifndef CATA_SRC_COLONY_TOOLBAR_H
#define CATA_SRC_COLONY_TOOLBAR_H

#include <string>

/**
 * In-world ImGui leadership toolbar for CULL colony worlds.
 * Shows clickable buttons that fire the same DEFAULTMODE actions as keybinds.
 */
void ensure_colony_toolbar();
void shutdown_colony_toolbar();

/**
 * If a toolbar button was clicked on the last ImGui draw, return that action
 * id (e.g. "colony_hub") and clear it. Empty string if none.
 */
std::string colony_toolbar_poll_action();

#endif // CATA_SRC_COLONY_TOOLBAR_H
