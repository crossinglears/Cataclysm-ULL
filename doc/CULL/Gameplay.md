# Gameplay

## Core loop

1. Observe colony state (people, stocks, threats, unfinished work).
2. Set or adjust **priorities** and **orders** (build, harvest, craft, explore, defend, etc.).
3. Survivors claim and perform jobs according to skills, gear, distance, and priority.
4. Time advances; crises and opportunities appear (raids, weather, discoveries, recruits).
5. Expand capability (storage, farms, workshops, roads, defense) and push further out.

The player never takes direct control of a survivor’s movement or attack keys. Leadership UI is the interface; see [UI.md](UI.md). In world, the player is a **god-mode observer**: movement keys pan the camera without walking; `<`/`>` change Z freely. Visibility is the **union of what on-map team members can see** (camp residents + ally followers). ENTER opens the survivors roster; craft opens unified camp production.

## Player orders (examples)

- Build / place blueprint here
- Harvest forest; gather water; plant crops
- Craft tools; cook meals; expand storage
- Explore town; loot pharmacy; hunt; search for vehicles; rescue survivors
- Defend perimeter; hold position; retreat; escort civilians
- Research / improve facilities (as systems allow)

Colony systems decide *who* executes each job. See [Jobs.md](Jobs.md) and [Exploration.md](Exploration.md).

## What the player does not do

- Walk a single avatar tile-by-tile as the primary play pattern
- Manually open every door, pick every berry, or queue every swing in combat as the default
- Replace Cataclysm item/combat simulation with board-game abstractions

Survivors still use real paths, tools, hunger/thirst/fatigue, and combat rules underneath AI and orders.

## Archived original (solo) gameplay — reference

The classic loop created one avatar via character creation (`src/newcharacter.cpp`, `src/character_creator_ui.h`) after world pick/create (`src/main_menu.cpp`, `src/worldfactory.*`). The player then micromanaged that body on the map.

**CULL replaces that product path entirely:**

- **New Game** → colony sim options only (setup a settlement / world).
- **Play** → load and continue **your colony game** only.

There is no menu path to start or play a singleplayer survivor game. Solo New Game items (Custom / Preset / Random / Play Now) are **removed**. Keep old code and upstream design docs as archive/reference only. See [Vision.md](Vision.md) and [UI.md](UI.md).

## Related systems

| Concern | Doc |
|---------|-----|
| Settlement focus | [ColonyManagement.md](ColonyManagement.md) |
| Agent behavior | [SurvivorAI.md](SurvivorAI.md) |
| Task assignment | [Jobs.md](Jobs.md) |
| Build pipeline | [Construction.md](Construction.md) |
| Off-map missions | [Exploration.md](Exploration.md) |
| Tactical stance | [Combat.md](Combat.md) |
