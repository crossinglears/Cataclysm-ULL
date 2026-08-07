# CULL Implementation TODO

Working checklist. Design detail: other files in this folder. Update [`Roadmap.md`](Roadmap.md) when a phase completes.

Legend: `[ ]` todo · `[x]` done · `[-]` cancelled / deferred

## Locked product menu (do not regress)

| Menu | Ships as |
|------|----------|
| **New Game** | Colony sim options only |
| **Play** | Continue / load **your colony** save only |
| Singleplayer | **None** — no Custom / Preset / Random / Play Now; no solo play path |

Archived solo loop (reference only): [Gameplay.md](Gameplay.md), [Vision.md](Vision.md), [UI.md](UI.md)

---

## Phase 0 — Documentation

- [x] `doc/CULL/` design docs hub + system docs
- [x] Root README pointer
- [x] This TODO
- [x] Menu stance locked: New Game = colony; Play = colony; no singleplayer

---

## Phase 1 — Main menu + colony start stub

### New Game (colony options only)

- [x] Replace character New Game submenu in `src/main_menu.cpp` with **colony sim** flow
- [x] **Delete from live menu:** Custom Character, Preset Character, Random Character, Play Now! (all solo avatar starts)
- [x] New Game options are colony-facing only, e.g.:
  - [x] World create / pick (`src/worldfactory.*`)
  - [x] Starting survivor count (hardcoded OK for stub)
  - [x] Starting location / difficulty placeholders as needed
- [x] Do not call solo character creator as a live path (`src/newcharacter.cpp` — archive/reference)

### Play (your colony only)

- [x] Play / load / continue opens **colony world saves** only
- [x] No menu entry that starts or resumes a singleplayer avatar game
- [x] Ensure Special / Tutorial (if still present) cannot reintroduce solo play without a doc change

### Boot into colony

- [x] Minimal New Game → in-world colony stub (hardcoded starter group OK)
- [x] Persist colony / world state as needed (`src/savegame_json.cpp`; [SaveSystem.md](SaveSystem.md))
- [x] Decide avatar/camera approach; write it in [UI.md](UI.md) + [TechnicalDesign.md](TechnicalDesign.md)
- [x] Smoke test: main menu has **zero** singleplayer play options; New Game and Play only touch colony
- [x] Plan archive pass for solo character-creation code (keep [Gameplay.md](Gameplay.md) reference)
- [x] Sync docs after Phase 1 lands

---

## Phase 2 — Camp as colony

- [x] On start, create/attach settlement as extended faction basecamp
- [x] Spawn starting survivors as camp residents
- [x] Rough leadership UI: camp stock / expansion status
- [x] Save/load restores camp + residents
- [x] Update [ColonyManagement.md](ColonyManagement.md) / [SaveSystem.md](SaveSystem.md)

---

## Phase 3 — Jobs and local AI

- [x] Job priorities → `npc::job_data`
- [x] Zones + `ACT_MULTIPLE_*` (haul / farm / construction support)
- [x] Eat/rest under autonomy ([SurvivorAI.md](SurvivorAI.md))
- [x] Basic job board UI ([Jobs.md](Jobs.md), [UI.md](UI.md))
- [x] Update Jobs / SurvivorAI docs

---

## Phase 4 — Construction pipeline

- [x] Place / queue blueprint
- [x] Gather + haul real materials ([Resources.md](Resources.md))
- [x] Complete via basecamp blueprints and/or tile construction ([Construction.md](Construction.md))

---

## Phase 5 — Expeditions

- [x] Expedition planner on companion missions
- [x] Scout / loot / hunt / recruit hooks
- [x] Outcomes → real items + roster ([Exploration.md](Exploration.md))

---

## Phase 6 — Combat orders and defense

- [x] Standing tactical orders (hold, retreat, defend, avoid, escort)
- [x] Camp defense + guard jobs ([Combat.md](Combat.md))
- [x] Tie to perimeter construction

---

## Phase 7 — Polish and content

- [x] Full colony setup UI (count, backgrounds, gear, location, difficulty)
- [x] Leadership UI UX pass
- [x] Facilities / missions content
- [x] Legacy solo save policy + migrations ([SaveSystem.md](SaveSystem.md))
- [x] Performance with many residents / jobs
- [x] Finish solo-code archive (out of live paths; keep documented reference)

---

## Standing rules

- [ ] Extend basecamp / NPC jobs / companion missions — no parallel stock/job systems without [TechnicalDesign.md](TechnicalDesign.md)
- [ ] Real Cataclysm items only as simulation truth ([Resources.md](Resources.md))
- [ ] Update `doc/CULL/` for product design; do not overwrite upstream `doc/` system docs
- [ ] Never reintroduce singleplayer New Game or Play without an explicit doc change
- [ ] Keep archived-solo reference accurate when code is archived

---

## Next up

Polish backlog / Ideas parking lot — see [Ideas.md](Ideas.md). Core phases 0–7 complete.

## Post–Phase 7 — God-mode leadership polish

- [x] Shared team FOV (on-map residents + ally followers → `camera_cache`)
- [x] Strip single-body DEFAULTMODE actions in `CULL_COLONY`
- [x] Unified colony crafting via `basecamp::start_crafting`
- [x] Survivors roster ImGui UI on ENTER (`colony_survivors`)
