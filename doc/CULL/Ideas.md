# Ideas

Parking lot for speculative or deferred ideas. Nothing here is committed design until promoted into the relevant system doc and [TechnicalDesign.md](TechnicalDesign.md) if it affects architecture.

## Setup and meta

- [-] Multiple starting colony archetypes — partially shipped as Phase 7 background mixes (farmstead / scavengers / militia / medical); richer scenario packs still open
- Scenario packs that bias biome, threats, or starting tech
- [-] Import or convert archived solo saves into colony seeds — Phase 7 locked **refuse-with-message** (no auto-migrate); see [SaveSystem.md](SaveSystem.md)

## Leadership and social

- Council / specialist advisor NPCs that surface suggestions
- Morale policies (rations, recreation, memorials)
- Internal conflict, crime, or desertion events
- Named roles (foreman, quartermaster, doctor) as UI filters over jobs

## Logistics and production

- Supply routes between outposts
- Vehicle fleets as colony assets with scheduled runs
- Power grid visualization over real batteries/wiring
- Seasonal planning calendar for farms and scavenging pushes

## Exploration and diplomacy

- Trade caravans with existing factions ([FACTIONS.md](../JSON/FACTIONS.md))
- Claiming and upgrading satellite outposts
- Radio / quest board that generates expedition templates
- Rescue chains and refugee waves as mid-game pressure

## Combat and defense

- Layered alert states (green / yellow / red) changing job permissions
- Trap and turret doctrine as colony policies
- Contested loot: expeditions that can be interrupted mid-mission with limited orders

## UI / accessibility

- Overmap-first leadership screen with drill-down to local map
- Spectator camera following a selected survivor without granting control
- Accessibility summaries (threat, hunger risk, unfinished blueprints)

## Technical experiments (validate before adopting)

- Heavier EOC-driven mode logic vs C++ `special_game` hooks
- Behavior-tree-first job claiming vs priority maps only
- Read-only aggregates for stocks without caching divergent totals

## Promoted elsewhere

When an idea is accepted, move the decision into the proper doc (Vision, Jobs, UI, etc.), delete or mark it promoted here, and update [Roadmap.md](Roadmap.md) if it affects sequencing.
