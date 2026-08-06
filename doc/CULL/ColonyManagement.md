# Colony Management

## Role of the settlement

In CULL the settlement is the primary focus of play: housing, food production, power, storage, workshops, farms, medical care, defense, roads, and logistics. The base should grow and specialize over time as blueprints complete and jobs run.

## Design intent

- Treat the player colony as an **extended faction basecamp**, not a second parallel camp framework.
- Surface camp inventory, expansions, work hours, and provides/requires in colony UI language (still backed by camp data).
- Let ad-hoc tile work (zones, construction) coexist with planned expansions (blueprints / mapgen updates).

## Facility examples

- Housing and bunks
- Kitchens and food production
- Farms and water
- Storage and logistics (hauling, roads)
- Workshops (tailoring, mechanics, electronics, etc.)
- Medical facilities
- Power generation
- Defense (walls, watch posts, staging areas)

## Reuse / extend

| Existing piece | Role for CULL |
|----------------|---------------|
| [BASECAMP.md](../JSON/BASECAMP.md) | Blueprint recipes, modular expansions, mapgen updates |
| `src/basecamp.*` | Camp state, inventory, expansions, work-day hours |
| `src/faction_camp.*` | Start camp, mission tabs, farm ops UI patterns |
| `data/json/recipes/basecamps/` | Upgrade / expansion recipes (`construction_blueprint`, provides/requires) |
| `data/json/mapgen/basecamps/` | Visual / structural results of upgrades |
| Overmap camp APIs (`overmapbuffer` camp helpers) | Locate and persist camps |

Do **not** invent a separate “colony building” type that ignores basecamp provides/requires and companion upgrade missions unless TechnicalDesign later records a hard blocker.

## Player-facing management

Players manage:

- Expansion priorities and blueprint placement ([Construction.md](Construction.md))
- Job priorities and staffing ([Jobs.md](Jobs.md))
- Stock awareness ([Resources.md](Resources.md))
- Expeditions and defense posture ([Exploration.md](Exploration.md), [Combat.md](Combat.md))

Day-to-day fetching, crafting steps, and pathing stay with survivors ([SurvivorAI.md](SurvivorAI.md)).

## Phase 2 — bootstrap (shipped)

On **Start Colony**, `game::seed_colony_start`:

1. Spawns starter follower NPCs (god-mode observer avatar is not a survivor).
2. Calls `talk_function::found_colony_camp` at the proxy OMT — picks a terrain-compatible faction base type when available, applies blueprint mapgen, defines a `basecamp` owned by `your_followers`, named “Colony”.
3. Assigns starters as `NPC_MISSION_CAMP_RESIDENT` via `basecamp::add_assignee`.

Rough leadership UI: factions screen (**Your faction** tab) shows food stock / endurance, residents, expansions, and next upgrade when `CULL_COLONY` is set.

## Phase 3 — local work (shipped)

`talk_function::seed_colony_local_work` adds starter zones, faction food stock (Phase 7 gear/difficulty scaled), and default `job_data` priorities. Colony job board: `basecamp::colony_job_board_ui` from the factions camp tab. See [Jobs.md](Jobs.md) and [SurvivorAI.md](SurvivorAI.md).

## Phase 7 — setup + content (shipped)

- New Game setup UI configures survivors, background mix (`npc_class`), gear crates (`data/json/cull/colony_starter_gear.json`), start location, and difficulty world options.
- Facilities remain basecamp expansion recipes ([BASECAMP.md](../JSON/BASECAMP.md)); expedition missions remain companion missions with early provides from `ensure_colony_expedition_provides`.
- Starter tools/materials drop into camp via `place_results` so construction and workshops can begin with real items.

## Related docs

- [Construction.md](Construction.md)
- [Jobs.md](Jobs.md)
- [TechnicalDesign.md](TechnicalDesign.md)
- [SaveSystem.md](SaveSystem.md)
