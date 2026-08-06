# Survivor AI

## Role

Each survivor is an **autonomous agent**. The player sets priorities and orders; survivors choose paths, tools, rest, meals, and nearby resource picks within those constraints.

## Agent decisions (player does not micromanage)

- Which path to take
- Which tool or item to use when several are valid
- When to rest, eat, drink, or treat wounds (within colony policy)
- Which nearby job tile or resource to claim next
- How to react to immediate danger unless overridden by tactical orders ([Combat.md](Combat.md))

## Assignment inputs

Survivors weigh (at minimum):

- Skills and proficiencies
- Available equipment
- Job priority from the colony board ([Jobs.md](Jobs.md))
- Job availability and distance
- Personal needs (hunger, thirst, fatigue, morale, injury)

## Reuse / extend

| Existing piece | Role for CULL |
|----------------|---------------|
| [NPCs.md](../JSON/NPCs.md) | NPC definitions, dialogue, missions hooks |
| `src/npc.*`, `src/npcmove.cpp` | Movement, attitudes, mission state, AI cache |
| `npc::job_data` | Per-NPC priority map for camp multi-activities |
| Behavior trees (`src/behavior.*`, character/behavior oracles) | Structured needs / goals; prefer extending over one-off hardcoding |
| `src/npc_attack.*` | Combat decision helpers under tactical stance |
| `NPC_MISSION_CAMP_RESIDENT` and camp mission types | Mark residents vs expedition members |

CULL should deepen **autonomy under orders**, not replace the NPC entity with a new agent type.

## Phase 3 — eat / rest autonomy

Camp residents use the existing NPC behavior tree (`data/json/npcs/npc_behavior.json`):

| Need | Path | Camp support |
|------|------|--------------|
| Hunger | `npc_needs` → eat → `consume_food_from_camp` then inventory/ground | Starter faction `food_supply` from `seed_colony_local_work` (~12k kcal × starters) |
| Thirst | drink water from camp well / inventory / terrain | Needs `water_well` provide or other water source |
| Sleep | `npc_needs_sleep_badly` → `go_to_sleep` | `ally_rule::allow_sleep` reaffirmed on bootstrap |

Needs outrank `camp_work` urgency when severe, so residents pause jobs to eat/rest. No parallel “colony needs” system — extend oracles/BT if policies deepen later.

## Phase 6 — combat under standing orders

Colony Defense (`src/colony_combat.*`) sets colony-wide `npc_follower_rules.engagement` and ally flags, plus optional `NPC_MISSION_GUARD_ALLY` posts. Local fight resolution stays in `npcmove` / `npc_attack`; the player changes stance rather than issuing per-attack commands. See [Combat.md](Combat.md).

## Policies vs micro

Colony-level policies (e.g. “eat stored food before foraging,” “avoid combat unless defending”) belong in leadership UI. Per-step movement does not.

Exact avatar/camera representation for “no direct control” is deferred in [TechnicalDesign.md](TechnicalDesign.md) and [UI.md](UI.md); it must not imply micromanaging one body.

## Related docs

- [Jobs.md](Jobs.md)
- [Combat.md](Combat.md)
- [Gameplay.md](Gameplay.md)
