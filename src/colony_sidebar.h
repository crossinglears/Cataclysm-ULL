#pragma once
#ifndef CATA_SRC_COLONY_SIDEBAR_H
#define CATA_SRC_COLONY_SIDEBAR_H

/**
 * In-world ImGui sidebar for CULL colony worlds (place/weather/time/wind,
 * message log, local overmap). Replaces the traditional cull_colony_sidebar
 * panel draw while still using that layout for terrain width reservation.
 */
void ensure_colony_imgui_sidebar();
void shutdown_colony_imgui_sidebar();

/** True when the ImGui colony sidebar is live (traditional panels should not draw). */
bool colony_imgui_sidebar_active();

#endif // CATA_SRC_COLONY_SIDEBAR_H
