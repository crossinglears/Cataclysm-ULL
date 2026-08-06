# Exploration

## Concept

The player **sends expeditions**; they do not manually walk the overmap as a single avatar. Missions resolve through AI and existing companion-mission patterns, then return people, loot, information, or failure states.

## Phase 5 — Colony Expeditions panel

Open **Colony Expeditions** (`ACTION_COLONY_EXPEDITIONS`, also under Info in the action menu, or Factions → camp → **Colony expeditions**). Requires a `CULL_COLONY` world with a bootstrapped basecamp.

| Action | Companion mission | Outcome |
|--------|-------------------|---------|
| **Scout** | `Camp_Scouting` | Path pick on overmap; reveals terrain; combat risk |
| **Loot** | `Camp_Gather_Materials` + `colony_loot_*` params | Destination raid via `loot_building`; real items into camp storage |
| **Hunt** | `Camp_Hunting` | Animal corpses into camp stocks |
| **Recruit** | `Camp_Recruiting` | Chance to add a new resident to the roster |
| **Recover** | mission return handlers | Bring back finished parties |
| **Status** | — | Lists active parties and ETAs |

Colony camps get scout / hunt / recruit / gathering **provides** via `basecamp::ensure_colony_expedition_provides` (also seeded from `seed_colony_local_work`), so early colonies can launch these without waiting on specific upgrades.

Loot targets: pharmacy (`s_pharm`), hardware (`s_hardware`), gun store (`s_gun`), house, or a free overmap pick. Round-trip time scales with distance (minimum 4 hours).

## Example expedition types

- Scout nearby town or overmap special — **Phase 5**
- Loot pharmacy / hardware / gun store — **Phase 5**
- Hunt animals — **Phase 5**
- Search for vehicles or fuel — later content
- Rescue or recruit survivors — **Phase 5** (recruit)
- Patrol or claim outposts — later / camp combat patrol

## Reuse / extend

| Existing piece | Role for CULL |
|----------------|---------------|
| [FACTION_MISSIONS.md](../JSON/FACTION_MISSIONS.md) | Mission JSON descriptions |
| `src/mission_companion.*` | Mission kinds, duration, `loot_building` |
| `src/faction_camp.*` | Camp mission start/return; `colony_loot_return` |
| `src/colony_expeditions.*` | Leadership expedition planner UI |
| `data/json/faction_missions.json` | Content-facing mission text/data |
| Local scouting still on loaded maps | May use NPC AI + jobs when the area is active; prefer missions when abstracting travel |

Off-map work stays on the companion-mission channel described in [Jobs.md](Jobs.md). Local foraging near camp stays on zones / multi-activities.

## Player decisions

- Destination / objective (scout path; loot target type / tile)
- Party composition (companion picker from camp workers)
- Rules of engagement ([Combat.md](Combat.md)) — standing orders apply to residents; expedition ROE deferred
- Cargo priorities — deferred; loot currently returns what `loot_building` scoops

Pathing and loot micro-choices during the mission belong to AI unless a future UI adds limited mid-mission orders.

## Outcomes

Expeditions feed:

- Real items into camp stocks via `place_results` ([Resources.md](Resources.md))
- Map knowledge along scout / loot destinations
- Casualties / time / food cost through existing companion-mission handling
- Optional new residents (recruit return)

## Related docs

- [Jobs.md](Jobs.md)
- [Combat.md](Combat.md)
- [ColonyManagement.md](ColonyManagement.md)
- [UI.md](UI.md)
