# Roadmap

Phased delivery. Update this file when milestones complete or reorder. Day-to-day checkboxes live in [TODO.md](TODO.md).

## Phase 0 — Documentation

- Establish `doc/CULL/` as design source of truth
- Cross-link existing CDDA systems (basecamp, jobs, missions, saves, menu)
- Root README pointer to these docs

## Phase 1 — Mode entry stub (done)

- New Game = colony sim options only; Play = your colony save only ([UI.md](UI.md), [TechnicalDesign.md](TechnicalDesign.md))
- Remove all solo avatar New Game / play entries; archive character-creation path (keep [Gameplay.md](Gameplay.md) reference)
- Minimal colony setup → start world with colony state
- World option `CULL_COLONY`; god-mode observer avatar + starter camp residents

## Phase 2 — Camp as colony (done)

- Bootstrap starting settlement via `talk_function::found_colony_camp` (extended basecamp)
- Starter NPCs assigned as `NPC_MISSION_CAMP_RESIDENT` on the camp
- Rough leadership view on factions UI: food stock, residents, expansions, next upgrade
- Persistence: basecamp + NPC `assigned_camp` via existing savegame JSON

## Phase 3 — Jobs and local AI (done)

- Job priorities driving `job_data` / `ACT_MULTIPLE_*` / zones ([Jobs.md](Jobs.md), [SurvivorAI.md](SurvivorAI.md))
- Hauling, farming, construction support as playable local loops
- Needs handling (eat/rest) under autonomy
- Colony job board on Factions camp tab

## Phase 4 — Construction pipeline (done)

- Blueprint placement → material gather/haul → completion ([Construction.md](Construction.md))
- Prefer basecamp blueprint/expansion recipes for major buildings
- Colony Construction panel: `src/colony_construction.*`, action `colony_construction`

## Phase 5 — Expeditions (done)

- Colony Expeditions panel on companion missions ([Exploration.md](Exploration.md))
- Scout / loot / hunt / recruit; loot uses `loot_building` → camp stocks; recruit grows roster
- `src/colony_expeditions.*`, action `colony_expeditions`; `ensure_colony_expedition_provides`

## Phase 6 — Combat orders and defense (done)

- Colony Defense panel: standing tactical orders, guard posts, `NPC_RETREAT` rally zones ([Combat.md](Combat.md))
- Integrates with guard assignment (`assign_guard` / `return_to_camp_duties`) and perimeter builds via Colony Construction
- `seed_colony_defense` on Start Colony

## Phase 7 — Polish and content (done)

- Colony setup UI: survivors, backgrounds, gear, location, difficulty (`src/colony_setup.*`)
- Leadership hub + factions UX pass (`ACTION_COLONY_HUB`, camp overview / menus)
- Starter gear content packs (`data/json/cull/colony_starter_gear.json`); facilities/missions stay on basecamp + companion missions
- Legacy solo saves: **refuse-with-message** (no auto-migrate); World menu labels `[legacy]` / `[colony]`
- Roster display caps + large-colony job board notice
- Solo creator archived in-place (`newcharacter.cpp` / `character_creator_ui.h` markers; still compiles)

## Tracking notes

- Avatar/camera: **god-mode observer** — free camera pan / free Z; no body walk; no survivor possession ([UI.md](UI.md), [TechnicalDesign.md](TechnicalDesign.md))
- Any deviation from locked defaults in TechnicalDesign must be written there first
