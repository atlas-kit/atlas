# OTB → appearances.dat (client-ID) migration

The server now indexes items by **client ID** and loads visual/flag data from
`data/items/appearances.dat`. All server-side data had to be moved from the
legacy server-ID namespace to client IDs. This document records exactly what
was migrated, how to reproduce it, and what is still required of the operator.

`tools/server_to_client_map.json` (and its reverse) is the source of truth,
generated from `data/items/items.otb` by `tools/otb_extractor.py`
(38284 items, OTB 3.66.62). Example: gold coin `2148 → 3031`.

## Done in this repo (deterministic, verified)

| Target | Tool | Verification |
|---|---|---|
| `tools/server_to_client_map.json` | `otb_extractor.py` | gold/platinum/crystal coins spot-checked |
| `data/items/items.xml` (18080 ids) | `convert_items_xml.py` | line count preserved; coins/fluids spot-checked |
| `src/const.h` (65 `ITEM_*` constants) | inline script | all 65 mapped, 0 unmapped, comments preserved |
| `data/world/forgotten.otbm` (428993 items) | [Atlas Assets Editor](https://github.com/atlas-kit/atlas-assets-editor) — Map Converter | lossless node-tree round-trip; every id remapped per the `items.otb` mapping |

Map (`.otbm`) conversion is handled by the **Map Converter** tool of the
[Atlas Assets Editor](https://github.com/atlas-kit/atlas-assets-editor) — the
official atlas-kit asset tooling — not by a script in this repo. It parses the
OTBM node tree losslessly (`ESCAPE/START/END`-aware, matching
`src/fileloader.cpp`), rewrites only the byte spans that hold item ids (the
`OTBM_ITEM` node header u16 and the `OTBM_ATTR_ITEM` u16 in tile prop streams),
and converts in either direction (server ↔ client) using your `items.otb` as
the exact mapping. Do not use naive byte-scan scripts: they corrupt maps.

14 item ids `>= 100` referenced in `items.xml` are not present in `items.otb`
(custom/newer items, kept unchanged): 27543, 27587, 27840, 28411, 29888,
30367, 30368, 31575, 33794, 34721, 35293, 36375, 36408, 36422.

### appearances.dat — provided and validated

`data/items/appearances.dat` is the Tibia 15.24 client file
(`appearances-ee339aff…dat`, 3146389 bytes, 42099 objects, id range
100–54266). The mapping was validated end-to-end against it: every well-known
item resolves to an object that exists in this exact client build
(gold 2148→3031, platinum 2152→3035, crystal 2160→3043, depot 2594→3502,
bag 1987→2853, browsefield 460→470 — all present).

~6.4% of mapped client ids have no object in this `appearances.dat`. These are
server-side/deprecated `items.otb` entries whose sprite was removed across
client versions; they would not render in this client regardless of the
migration (pre-existing data hygiene, not a migration error). If you swap to a
different client build, regenerate the mapping and re-run every conversion.

## Required from the operator — NOT done here

These cannot be completed or verified without you, and the server will not be
correct until they are:

1. **Database (live data).** Back up first.
   - Column ids: run `tools/migrate_db_itemtype.sql` (covers `player_items`,
     `player_depotitems`, `player_inboxitems`, `player_storeinboxitems`,
     `market_offers`, `market_history` — the `itemtype` columns only).
   - **Serialized blobs are NOT handled**: `player_*.attributes` and
     `tile_store.data` (house items) embed item ids in a `PropStream` binary
     format (see `src/iomapserialize.cpp`). They need a binary migrator
     mirroring `loadItem`/`saveItem`, run against your dump. This is unwritten
     because it must be validated against real data.

2. **Lua scripts — manual review.** `tools/convert_lua_scripts.py` is
   heuristic (`100 ≤ n ≤ 50000`) and **was intentionally not run**: Lua has no
   structural signal to tell an item id from a storage key, count, level or
   price, so a blanket pass introduces silent gameplay bugs. Convert
   case-by-case.

## Reproduce

```sh
python tools/otb_extractor.py data/items/items.otb -o tools/server_to_client_map.json -f mapping
python tools/otb_extractor.py data/items/items.otb -o tools/client_to_server_map.json -f reverse
python tools/convert_items_xml.py -m tools/server_to_client_map.json -i data/items/items.xml -o data/items/items.xml
```

Map: use the [Atlas Assets Editor](https://github.com/atlas-kit/atlas-assets-editor)
(GUI, Windows/macOS/Linux):

1. Open the **Map Converter** tool from the launcher.
2. Load `data/items/items.otb` so the conversion uses your exact
   server↔client mapping (without it the built-in community table is used).
3. Select `data/world/forgotten.otbm`, direction **server → client**, and
   convert.
