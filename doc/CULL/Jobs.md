# Jobs

## Concept

Every meaningful colony task is a **job** (or maps onto one): mining, woodcutting, farming, cooking, hunting, tailoring, mechanics, electronics, cleaning, hauling, guard duty, medical care, construction support, and so on.

Survivors claim jobs from colony priorities. The player adjusts priority and availability; the colony does not require assigning every task to a named person by default (manual assignment can be a later refinement).

## Selection factors

- Skills / relevant stats
- Equipment on hand or reachable
- Priority set by the player
- Whether the job is currently available (materials, zones, blueprints)
- Distance and safety

## Two work channels (locked default)

| Channel | Use for | Existing substrate |
|---------|---------|-------------------|
| **Local / on-map** | Work inside or near the settlement with visible map interaction | Zones + `npc::job_data` + `ACT_MULTIPLE_*` in `src/activity_item_handling.cpp` |
| **Off-map / abstracted travel** | Expeditions and distant camp missions | Companion missions (`src/mission_companion.*`, [FACTION_MISSIONS.md](../JSON/FACTION_MISSIONS.md)) |

Do not invent a third job queue until both channels are proven insufficient; record any exception in [TechnicalDesign.md](TechnicalDesign.md).

## Local multi-activities (extend)

Examples already in the engine family:

- `ACT_MULTIPLE_CONSTRUCTION`
- Farm / chop / butcher / loot / craft style multi-activities
- Zone-driven work (`src/clzones.*` and related JSON)

Camp residents already use `job_data` priorities for these activities. CULL’s job board should drive and expose that layer rather than bypass it.

## Phase 3 — shipped

### Colony job board

- **Entry:** Factions screen (`#`) → camp tab → `j` / Confirm → **Colony job board** (`basecamp::colony_job_board_ui`).
- Sets **colony-wide** priorities on every assigned resident’s `npc::job` (`job_data::set_task_priority`).
- Links to existing **per-worker** `job_assignment_ui` for fine control.
- Resident list on the camp tab shows top enabled job verb + priority.

### Defaults on Start Colony

`talk_function::seed_colony_local_work` (from `game::seed_colony_start`) assigns:

| Activity | Default priority |
|----------|------------------|
| `ACT_MOVE_LOOT` (haul) | 3 |
| `ACT_MULTIPLE_CONSTRUCTION` | 2 |
| `ACT_MULTIPLE_FARM` | 2 |
| `ACT_MULTIPLE_CHOP_TREES` / `PLANKS` / `BUTCHER` | 1 |

### Starter zones

Same bootstrap places faction zones on the camp OMT (refine via zone manager `Y`):

- `CAMP_STORAGE` / `CAMP_FOOD`
- `LOOT_UNSORTED` (hauling)
- `FARM_PLOT` / `CONSTRUCTION_BLUEPRINT` (options empty until player configures seeds/blueprints)

Residents with `NPC_MISSION_CAMP_RESIDENT` + `assigned_camp` + `has_job()` claim work through the behavior tree (`camp_work` → `find_job_to_perform`).

### Guard duty (Phase 6)

Guard posts are **missions**, not `job_data` activities: `NPC_MISSION_GUARD_ALLY` + `guard_pos` via Colony Defense. Standing combat orders adjust `npc_follower_rules` / engagement for all residents. See [Combat.md](Combat.md).

## Off-map missions (extend)

Gather materials, hunting, recruiting, patrol, camp upgrades, crafting missions, and similar companion mission kinds remain the pattern for “send people away and get a result later.” See [Exploration.md](Exploration.md).

## Player-facing job board (design)

- List job categories with priority controls — **done** (Phase 3 colony board)
- Show demand (queued work) vs supply (qualified idle survivors) — supply count shown; demand still zone/blueprint driven
- Link shortages to resources or missing zones/blueprints ([Resources.md](Resources.md), [Construction.md](Construction.md))

## Related docs

- [SurvivorAI.md](SurvivorAI.md)
- [ColonyManagement.md](ColonyManagement.md)
- [PLAYER_ACTIVITY.md](../PLAYER_ACTIVITY.md) (upstream activity concepts)
