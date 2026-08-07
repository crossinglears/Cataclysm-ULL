# UI

## Main menu product rules

CULL is colony sim only. There is **no** singleplayer survivor start and **no** way to “play as one character” from the main menu.

| Menu area | Meaning |
|-----------|---------|
| **New Game** | Colony sim setup only (world + starting colony). No Custom / Preset / Random / Play Now character options. |
| **Play** / Continue | Load and resume **your colony game** (existing world saves). Not a solo campaign picker. |
| **Help** | Submenu: **General** (stock Cataclysm help) and **United Lifeline** (nested menu of colony special features). |

Implementation: `src/main_menu.cpp` — replace the old character-centric New Game submenu with colony options; Play/load paths only open colony worlds. Details in [TechnicalDesign.md](TechnicalDesign.md).

## New Game — colony sim options only

Examples of what New Game may offer (all colony-facing):

- Create / pick world settings
- Number of starting survivors
- Backgrounds / professions mix
- Starting equipment
- Starting location
- Colony difficulty

**Phase 1–7 ships:** **Start Colony** opens the colony setup UI (`colony_setup_ui`): survivor count (2–12), background mix, gear level, curated start locations, and difficulty presets. World create/pick via `worldfactory` follows. Worlds are marked with world option `CULL_COLONY`. Factions screen and Colony hub surface leadership tools.

**Removed from New Game (do not ship):**

- Custom Character
- Preset Character
- Random Character
- Play Now! (default scenario / full random avatar starts)

Those belonged to archived solo Cataclysm; see [Gameplay.md](Gameplay.md).

## Play — your colony game only

Play / load / continue means:

- Select an existing **colony** world save (`CULL_COLONY` world option)
- Resume leadership of that settlement

It does **not** offer a singleplayer avatar game, tutorial-as-solo-survivor, or any parallel “classic mode” play entry. Tutorial was removed from the main menu in Phase 1.

## In-game leadership UI (design goals)

Surfaces the player needs without avatar WASD control:

- Colony overview (people, morale, threats)
- Job priorities / board ([Jobs.md](Jobs.md)) — `#` factions → MANAGE_JOBS / colony job board
- Construction / blueprint placement ([Construction.md](Construction.md)) — action `colony_construction` (Colony Construction panel)
- Stock / logistics views ([Resources.md](Resources.md))
- Expedition planning ([Exploration.md](Exploration.md)) — action `colony_expeditions` (Colony Expeditions panel)
- Tactical standing orders ([Combat.md](Combat.md)) — action `colony_combat` (Colony Defense panel)
- Colony leadership hub — action `colony_hub` (jumps to overview / jobs / build / expeditions / defense)
- **Survivors roster** — action `colony_survivors` (default **ENTER**): all residents + followers and current activity; Confirm centers camera
- **Unified crafting** — action `craft`: camp skills/recipes + camp stock → companion craft mission
- **In-world ImGui toolbar** (`colony_toolbar`): Hub / Survivors / Overview / Build / Expeditions / Defense
- Camp expansion status ([ColonyManagement.md](ColonyManagement.md))

**Phase 4 ships:** Colony Construction panel for tile blueprint zones and camp upgrade queueing; materials remain real items in camp/haul zones.

**Phase 5 ships:** Colony Expeditions panel for scout / loot / hunt / recruit companion missions; loot returns real items to camp; recruit can add roster members.

**Phase 6 ships:** Colony Defense panel for standing orders (hold / retreat / defend / avoid / escort), guard posts, retreat zones, and a link into perimeter construction.

**Phase 7 ships:** Full colony setup UI; Colony hub; factions overview shows job busy/idle counts and caps long rosters; World menu labels colony vs legacy saves.
### Phase 2 — rough stock / expansion

- Reuse the factions screen (`ACTION_FACTIONS`, `faction_ui`): **Your faction** tab lists owned camps.
- For `CULL_COLONY` worlds, camp detail shows food stock/endurance, assigned residents, expansions, and next upgrade.

### Phase 3 — job board

- Factions camp tab: **`j`** (`MANAGE_JOBS`) opens `basecamp::colony_job_board_ui`.
- Confirm on a colony camp opens rename / job board / construction / defense / per-worker priorities / assign workers.
- Resident lines show top enabled job verb + priority.
- Configure farm seeds / construction blueprints via the zone manager (`Y`).

### Phase 6 — defense

- Action `colony_combat` / Factions camp → **Colony defense**: standing orders, guard posts, retreat zones, perimeter construction link.

## Avatar / camera (god mode)

**Locked:** the player is a **god-mode observer**, not a survivor.

- The engine still needs a save/turn owner avatar; it is an invisible immortal proxy (`ensure_colony_god_avatar` in `src/colony_camera.*`).
- **New Game → Start Colony** creates that proxy non-interactively (`character_type::NOW`) named “Colony Observer” — no character creator UI.
- Starting survivors are **all** camp-resident NPCs (`game::seed_colony_start`); the observer is not counted as one of them.
- **Movement keys** pan `view_offset` (camera only). They do **not** walk a body. Panning alone does **not** reveal unseen tiles.
- **Team vision:** the player sees the union of FOV from every on-map camp resident and player-ally follower (`colony_team_vision_sources` → `camera_cache` in `map::build_map_cache`), plus the parked observer body / map memory.
- **`<` / `>`** (`ACTION_MOVE_UP` / `DOWN`) change viewed Z freely — no stairs or ceiling checks.
- **Possess / control NPC** is blocked in colony worlds (“cannot see through a survivor's eyes” means **no body control**; shared team FOV is intentional).
- **Single-body DEFAULTMODE actions** (inventory, fire, personal craft variants, open/close, etc.) are refused in god mode; leadership surfaces remain.
- **Craft** (`&` / `ACTION_CRAFT`) opens **unified colony crafting**: camp assignee skills/recipes + camp stock → companion craft mission (`colony_open_unified_crafting`).
- **ENTER** (`ACTION_COLONY_SURVIVORS`) opens the ImGui survivors roster (`src/colony_survivors_ui.*`): who they are, what they are doing; Confirm centers the camera on them without granting control.
- Sidebar: ImGui `colony_sidebar` (place/weather, date/time, wind/temp, Log, overmap Map). Traditional `cull_colony_sidebar` layout still reserves terrain width. No HP/stamina/needs/weapon panels.
- Toolbar: Hub / Survivors / Overview / Build / Expeditions / Defense.

## Archived solo UI — reference only

Not reachable from New Game or Play. Kept so we remember the old loop while archiving code:

- Character creator: `src/newcharacter.cpp`, `src/character_creator_ui.h`
- Old New Game character submenu items
- Contrast: [Gameplay.md](Gameplay.md), [Vision.md](Vision.md)

Reuse camp mission UI patterns in `src/faction_camp.*` for colony panels.

In-world leadership toolbar: `src/colony_toolbar.*` (ImGui HUD; see above).
In-world colony sidebar: `src/colony_sidebar.*` (ImGui; see above).

## Related docs

- [Gameplay.md](Gameplay.md)
- [Vision.md](Vision.md)
- [Roadmap.md](Roadmap.md)
- [TODO.md](TODO.md)
