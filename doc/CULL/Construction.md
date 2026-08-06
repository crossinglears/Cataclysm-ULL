# Construction

## Player-facing flow

1. Player places or selects a **blueprint** (planned structure / expansion).
2. Workers gather required materials from stocks or the field.
3. Workers haul materials to the site.
4. Construction activity runs (survivors, not player micromanagement).
5. Structure finishes and becomes part of the settlement.

This is planning and logistics, not tile-by-tile player construction as the primary loop.

## Phase 4 — Colony Construction panel (shipped)

Open **Colony Construction** (`ACTION_COLONY_CONSTRUCTION`, also in the action menu under Info). Requires a `CULL_COLONY` world with a bootstrapped basecamp.

| Action | What it does |
|--------|----------------|
| **Place / configure tile blueprint** | `construction_menu( true )` → mark a `CONSTRUCTION_BLUEPRINT` zone with real construction id/group. Residents with `ACT_MULTIPLE_CONSTRUCTION` + haul priorities gather items and build. |
| **Queue camp expansion** | Lists `basecamp::available_upgrades`, shows material status from camp crafting inventory, starts `basecamp::start_upgrade` (companion upgrade mission + mapgen). |
| **Construction status** | Summarizes configured tile zones, upgrade readiness / shortages, and resident job supply. |
| **Manage haul / storage zones** | Zone manager for `CAMP_STORAGE` / `LOOT_UNSORTED` logistics. |
| **Colony job board** | Colony-wide priorities (`basecamp::colony_job_board_ui`). |
| **Full camp missions** | Existing faction camp mission board (gather materials, upgrades, crafting). |

Materials are always **real CDDA items** in camp/map zones — never abstract meters.

**Phase 6:** Colony Defense opens this panel for **perimeter** walls / barricades (and camp fortification upgrades) alongside guard posts and `NPC_RETREAT` rally zones — see [Combat.md](Combat.md).

## Two construction layers (locked default)

| Layer | When | Substrate |
|-------|------|-----------|
| **Camp blueprints / expansions** | Planned colony buildings and modular upgrades | Basecamp recipes with `construction_blueprint`, provides/requires, update mapgen — [BASECAMP.md](../JSON/BASECAMP.md), `data/json/recipes/basecamps/`, `data/json/mapgen/basecamps/` |
| **Tile construction + zones** | Ad-hoc walls, furniture, repairs, local builds | `src/construction.*`, construction JSON, `ACT_MULTIPLE_CONSTRUCTION` + construction zones |

Colony “Build here” orders prefer the camp blueprint pipeline for major facilities and the construction/zone pipeline for local work.

## Reuse / extend

- Blueprint recipe fields and expansion graph: [BASECAMP.md](../JSON/BASECAMP.md)
- Companion upgrade missions: `Camp_Upgrade` and related kinds in `src/mission_companion.*`
- Construction menu blueprint path: `construction_menu( bool blueprint )` in `src/construction.h`
- Colony panel entry: `src/colony_construction.*`, action `colony_construction`
- Do not invent a disconnected “building HP bar” that ignores real components unless Resources/UI later define a **display aggregate** over real items

## Materials and jobs

Material demand ties to [Resources.md](Resources.md). Gathering, hauling, and building steps are jobs under [Jobs.md](Jobs.md). Survivors pick tools and paths per [SurvivorAI.md](SurvivorAI.md). Phase 3 seeds haul/storage zones and default construction/haul `job_data` priorities on residents.

## Related docs

- [ColonyManagement.md](ColonyManagement.md)
- [Jobs.md](Jobs.md)
- [Resources.md](Resources.md)
- [UI.md](UI.md)
