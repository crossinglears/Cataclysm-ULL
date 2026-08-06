# Cataclysm: United Lifeline (CULL) — Design Docs

CULL is the primary experience for this fork: instead of controlling a single survivor, the player leads a colony. Survivors act as autonomous agents; the player sets priorities, assignments, expansion, and strategy.

**New Game** offers colony sim setup only. **Play** continues your colony world. There is no singleplayer survivor option on the main menu. The archived solo loop is documented for reference in [Gameplay.md](Gameplay.md) and [Vision.md](Vision.md).

These documents are the design source of truth. Update them whenever systems or decisions change.

## Document index

| Document | Topic |
|----------|--------|
| [Vision.md](Vision.md) | What CULL is and is not |
| [Gameplay.md](Gameplay.md) | Macro-order loop; no direct control |
| [ColonyManagement.md](ColonyManagement.md) | Settlement focus; extend basecamp |
| [SurvivorAI.md](SurvivorAI.md) | Autonomous survivors; extend NPC AI |
| [Jobs.md](Jobs.md) | Job board; zones and multi-activities |
| [Construction.md](Construction.md) | Blueprints → materials → build |
| [Exploration.md](Exploration.md) | Expeditions; companion missions |
| [Combat.md](Combat.md) | Tactical orders; reuse CDDA combat |
| [Resources.md](Resources.md) | Real items; avoid generic abstractions |
| [UI.md](UI.md) | Main menu, colony setup, leadership UI |
| [TechnicalDesign.md](TechnicalDesign.md) | Architecture, extension points, non-goals |
| [SaveSystem.md](SaveSystem.md) | Persistence on existing savegame |
| [Roadmap.md](Roadmap.md) | Phased milestones |
| [TODO.md](TODO.md) | Implementation checklist |
| [Ideas.md](Ideas.md) | Speculative parking lot |

## Documentation rules

1. **Keep CULL docs here.** All colony-mode design notes live under `doc/CULL/`.
2. **Do not overwrite upstream docs.** Preserve and extend references to existing CDDA documentation under `doc/` (especially `doc/JSON/` and `doc/design-balance-lore/`). Cross-link instead of copying.
3. **Update continuously.** When a design decision or system lands in code, update the relevant CULL doc in the same change when practical.
4. **Prefer reuse.** Document how to extend existing systems before proposing new parallel ones.

## Upstream references

JSON and design references (do not duplicate; link from CULL docs as needed):

- [BASECAMP.md](../JSON/BASECAMP.md) — faction camp blueprints and expansions
- [FACTION_MISSIONS.md](../JSON/FACTION_MISSIONS.md) — companion / camp missions
- [FACTIONS.md](../JSON/FACTIONS.md) — faction definitions
- [NPCs.md](../JSON/NPCs.md) — NPC JSON and behavior
- [ITEM.md](../JSON/ITEM.md) — item system
- [PLAYER_ACTIVITY.md](../PLAYER_ACTIVITY.md) — activities
- [EFFECT_ON_CONDITION.md](../JSON/EFFECT_ON_CONDITION.md) — EOCs / scripting
- [MODDING.md](../MODDING.md) — scenarios, mods, content packaging

Key code entry points:

- `src/main_menu.cpp` — New Game (CULL colony flow; remove solo options)
- `src/newcharacter.cpp` — archived solo character creation (reference only; see [Gameplay.md](Gameplay.md))
- `src/basecamp.*`, `src/faction_camp.*` — camp substrate
- `src/mission_companion.*` — off-map companion missions
- `src/npc.*`, `src/npcmove.cpp` — NPC AI and `job_data`
- `src/activity_item_handling.cpp` — `ACT_MULTIPLE_*` zone work
- `src/construction.*` — tile construction
- `src/colony_construction.*` — Colony Construction panel (Phase 4)
- `src/colony_expeditions.*` — Colony Expeditions planner (Phase 5)
- `src/colony_combat.*` — Colony Defense / standing orders (Phase 6)
- `src/colony_setup.*` — Colony setup UI + leadership hub (Phase 7)
- `data/json/cull/` — CULL starter content packs
- `src/savegame_json.cpp` — serialization (camps, NPCs, factions)
- `src/gamemode.*` — `special_game` hooks (tutorial today)
