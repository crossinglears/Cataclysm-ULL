# Resources

## Principle

Use **real CDDA items** and stocks whenever possible. Food is comestibles; wood is lumber and related items; medicine is medical items; ammunition is ammo. Do **not** invent parallel generic resource pools (`Food: 120`) as the simulation source of truth.

UI may later show **aggregates or categories** derived from real inventories (e.g. “calories in stock,” “boards available”) as long as the underlying storage remains items.

## Resource domains (examples)

- Food and water
- Wood, metal, components
- Fuel
- Medicine
- Clothing
- Ammunition
- Tools and machinery parts

All of these map to existing item definitions and containers/stockpiles.

## Reuse / extend

| Existing piece | Role for CULL |
|----------------|---------------|
| [ITEM.md](../JSON/ITEM.md) | Item data model |
| Basecamp inventory (`src/basecamp.*`) | Settlement stock access for camp workflows |
| Zones (loot / stockpile / farm) | Where items live and how hauling jobs find them |
| Crafting / recipes | Production jobs consume and produce real items |

Avoid a shadow economy that crafts “+10 Metal” without creating or consuming catalog items.

## Colony UI expectations

- Shortage warnings tied to job/blueprint requirements ([Jobs.md](Jobs.md), [Construction.md](Construction.md))
- Hauling and storage expansion as first-class logistics play
- Expedition loot merges into real camp/map items ([Exploration.md](Exploration.md))

## Phase 7 — starter gear content

`data/json/cull/colony_starter_gear.json` defines real-item crates (`cull_colony_gear_sparse` / `standard` / `well_supplied`) seeded into camp storage on Start Colony. Food stock remains faction `fac_food_supply` nutrients scaled by gear/difficulty.

## Related docs

- [ColonyManagement.md](ColonyManagement.md)
- [Construction.md](Construction.md)
- [TechnicalDesign.md](TechnicalDesign.md) (locked: no generic meters as simulation)
