# Vision

## What CULL is

Cataclysm: United Lifeline (CULL) is the game’s primary experience: **colony leadership**. The player does not micromanage one body. The player sets priorities, assignments, expansion plans, and strategy while survivors carry out work as autonomous agents.

The fantasy is leadership under apocalypse pressure: keep people fed and housed, grow capability, send expeditions, and hold ground—using the same world, items, and threats as the underlying Cataclysm systems.

## What CULL is not

- **Not a classic single-survivor game.** New Game is colony sim options only; Play continues your colony. No Custom / Preset / Random / Play Now avatar starts. Archived solo loop: [Gameplay.md](Gameplay.md).
- **Not a total conversion that throws away Cataclysm systems.** Prefer extending basecamp, NPC AI, activities, construction, missions, and items.
- **Not an RTS with abstracted generic resources** as the primary model. Food is food items; wood is lumber and related items; see [Resources.md](Resources.md).
- **Not direct control of individuals.** Pathing, tool choice, rest, and eating are survivor/AI concerns once priorities and jobs exist; see [SurvivorAI.md](SurvivorAI.md) and [Gameplay.md](Gameplay.md).

## Archived original gameplay vs CULL

Reference only — the solo avatar loop is not a supported New Game option going forward. Keep this table so implementers remember what we branched from:

| Archived solo Cataclysm | CULL |
|-------------------------|------|
| Control one survivor | Control nobody directly |
| Every action is player-issued on the avatar | Player issues colony-level orders and priorities |
| Survive as an individual | Lead a settlement over the long term |
| Character creation for one person | Colony setup for a starting group |

Code and design references for the archived path: `src/newcharacter.cpp`, `src/character_creator_ui.h`, and upstream design notes under `doc/design-balance-lore/` (especially gameplay/UX docs). Do not delete that reference material blindly; archive deliberately when the code pass lands.

## Design pillars

1. **Macro over micro** — Decisions are where to build, what to prioritize, whom to send, what to risk—not which tile to step on.
2. **Reuse Cataclysm** — Camps, NPCs, items, combat resolution, mapgen, and saves are the substrate.
3. **Colony is the only game** — New Game = colony setup; Play = your colony save; no singleplayer play option.
4. **Settlement as protagonist** — Housing, production, logistics, defense, and knowledge growth are the arc; see [ColonyManagement.md](ColonyManagement.md).

## Related docs

- [Gameplay.md](Gameplay.md) — player loop and archived solo contrast
- [TechnicalDesign.md](TechnicalDesign.md) — how mode entry and extensions are expected to land
- [Roadmap.md](Roadmap.md) — delivery phases
- [TODO.md](TODO.md) — implementation checklist
