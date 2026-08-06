# Combat



## Principle



Combat **resolution** stays on CDDA mechanics (weapons, armor, skills, monsters, cover, etc.). The player does not click individual attacks as the primary interface. The player issues **tactical orders**; survivors fight (or avoid fighting) under those orders and their AI.



## Phase 6 — Colony Defense panel (shipped)



Open **Colony Defense** (`ACTION_COLONY_COMBAT`, also under Info in the action menu, or Factions → camp → **Colony defense**). Requires a `CULL_COLONY` world with a bootstrapped basecamp.



| Action | What it does |

|--------|----------------|

| **Standing tactical orders** | Apply hold / retreat / defend / avoid / escort to all assigned residents |

| **Assign guard post** | Pick a resident + map tile → `NPC_MISSION_GUARD_ALLY` + `guard_pos` + defend engagement |

| **Recall guards to duties** | `return_to_camp_duties` with defend stance |

| **Place retreat / rally zone** | Faction `NPC_RETREAT` zone for fall-back orders and careful-retreat AI |

| **Perimeter construction** | Opens Colony Construction for wall / barricade blueprints and camp upgrades |

| **Defense status** | Guards, inferred order, rally zones, blueprints, nearby hostiles |



### Standing orders → engine substrate



| Order | Engagement / rules | Mission / attitude |

|-------|--------------------|--------------------|

| **Hold position** | `NO_MOVE`, `hold_the_line` | Guard at current tile |

| **Retreat / fall back** | `NONE`, `follow_close` | Guard at nearest `NPC_RETREAT` |

| **Defend perimeter** | `ALL`, `hold_the_line` | Camp resident duties |

| **Avoid combat** | `NONE`, `forbid_engage` | Stay on camp duties |

| **Escort leader** | `CLOSE`, `follow_close` + distance 2 | `stop_guard` → follow |



Persistence is the existing NPC `rules` / `mission` / `guard_pos` JSON — no parallel order store.



### Bootstrap



`seed_colony_defense` (from `game::seed_colony_start`) places a central `NPC_RETREAT` rally zone and sets residents to defend (`engagement::ALL` + `hold_the_line`).



## Example tactical orders



- Hold position

- Retreat / fall back to rally point

- Defend area / perimeter

- Attack target or clear area (future refinement; defend + engagement covers local fights)

- Avoid combat / disengage

- Escort civilians or cargo (Phase 6: escort-target standing order)



## Scope



- **At camp:** Guard posts, rally zones, defense blueprints ([Jobs.md](Jobs.md), [Construction.md](Construction.md)).

- **On expeditions:** Rules of engagement attached to the mission ([Exploration.md](Exploration.md)) — Phase 5+.

- **Active local fights:** When the map is loaded and hostiles engage residents, AI + standing orders apply; player may adjust stance without taking direct control of a body.



## Reuse / extend



- NPC combat helpers: `src/npc_attack.*`, movement/combat logic in `src/npcmove.cpp`

- Ally rules / engagement: `npc_follower_rules`, `combat_engagement`, `ally_rule`

- Guard assignment: `talk_function::assign_guard` / `stop_guard` / `return_to_camp_duties`

- Retreat AI: `zone_type_NPC_RETREAT` via `npc::good_escape_direction`

- Colony panel: `src/colony_combat.*`, action `colony_combat`

- Existing monster/player combat simulation — do not replace with abstract “raid dice” as the only model

- Camp defense combines construction (walls, positions) with guard posts and standing orders



A dedicated tactical overlay UI is future work; see [UI.md](UI.md) and [Roadmap.md](Roadmap.md). This document locks the **orders-over-direct-control** rule.



## Failure and cost



Combat should produce real wounds, deaths, gear loss, and morale effects—same simulation fidelity as classic Cataclysm combat, filtered through colony consequences (labor shortage, medical demand).



## Related docs



- [SurvivorAI.md](SurvivorAI.md)

- [Exploration.md](Exploration.md)

- [Gameplay.md](Gameplay.md)

- [Construction.md](Construction.md)

- [UI.md](UI.md)


