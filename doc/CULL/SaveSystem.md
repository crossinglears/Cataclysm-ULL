# Save System

## Principle

Persist CULL on the **existing world save layout** and JSON serializers. Prefer extending serialized fields on camps, NPCs, factions, and world/game state over a parallel save format.

## What already serializes

Relevant pieces already flow through savegame code (notably `src/savegame_json.cpp`, `src/savegame.cpp`):

- Avatar / characters
- NPCs (including mission and activity-related state)
- Factions
- Basecamps (overmap camp load/store; see overmap unserialize paths)
- Activities and related runtime state
- World folder structure via `src/worldfactory.*` and `PATH_INFO` save paths (`src/path_info.cpp`)

CULL should treat these as the persistence backbone for colony residents, stock, and expansions.

## CULL-specific state (expected)

When implementation adds colony-mode data, prefer:

1. Flags or mode markers on the world/game so loads know the save is a CULL run
2. Extra fields on `basecamp` / faction / NPC where the data naturally belongs (job board priorities, tactical standing orders, expedition queue)
3. Versioned JSON with migrations consistent with existing save migration practices (`savegame_legacy` patterns where applicable)

Avoid a second `save/` tree or binary blob that duplicates item inventories already on the map or in camp storage.

### Phase 1 marker

- World option **`CULL_COLONY`** (bool, hidden in options UI, stored in `worldoptions.json`).
- Set to true when **Start Colony** succeeds; Play filters to these worlds.
- Colony save file is still the normal avatar `.sav` (invisible god-mode observer) plus master/NPC/overmap data.

### Phase 2 camp + residents

- Settlement persists as a normal **basecamp** on the overmap (`basecamp::serialize` / overmap camp load).
- Residents persist via NPC fields already saved: `assigned_camp`, `mission` (`NPC_MISSION_CAMP_RESIDENT`), faction, and follower list. `basecamp::assigned_npcs` is rebuilt on load via `validate_assignees()` (not a separate serialized roster).
- Avatar `camps` set records owned camp OMTs (written when `map::add_camp` runs).

### Phase 3 jobs + zones + stock

| State | Persistence |
|-------|-------------|
| Job priorities | `npc::job` / `job_data` on each resident |
| Work zones | Zone manager (same as player zones) |
| Starter camp food | Faction `fac_food_supply` |

No new save format — load a `CULL_COLONY` world restores camp, residents, jobs, and zones normally.

## New game / templates

- Colony setup results write a normal world save that continues with standard load/continue from **Play**.
- Character preset templates from the archived solo path are not a live New Game feature; World “Character to Template” was removed from the menu in Phase 1.

## Compatibility

- **Legacy solo saves** (no `CULL_COLONY`): hidden from Play; still manageable/deletable under World.
  - **Phase 7 policy (locked): refuse-with-message.** No automatic migration into colony mode.
  - World menu labels non-colony worlds `[legacy]` and offers “Why can't I play this?” explaining the policy.
  - Players delete/reset legacy worlds or start a new colony from New Game.
- CULL saves should fail gracefully or ignore unknown future fields using the same tolerant JSON habits as the rest of the project.
- Do not break upstream field names casually; extend carefully and note migrations in this doc when they ship.

### Phase 7 setup persistence

| State | Persistence |
|-------|-------------|
| Difficulty (spawn / monster speed / resilience) | World options written at Start Colony |
| Start location | Avatar `start_location` (normal new-game path) |
| Starter gear crates | Real items via `basecamp::place_results` |
| Background mix | Applied when NPCs `randomize` with chosen `npc_class` |

## Related docs

- [TechnicalDesign.md](TechnicalDesign.md)
- [ColonyManagement.md](ColonyManagement.md)
- [UI.md](UI.md)
