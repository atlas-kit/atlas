#!/usr/bin/env python3
"""
OTBM Map Converter - serverId -> clientId (structural, escape-safe).

Unlike a naive byte scan, this walks the real OTBM node tree exactly like the
server's OTB::parseTree (fileloader.cpp):

  ESCAPE = 0xFD  (next byte is literal)
  START  = 0xFE  (begins a child node)
  END    = 0xFF  (ends current node)

Node layout: <START> <type:1 raw> <props (escaped)> <children...> <END>.
Item IDs live in exactly two places:
  1. OTBM_ITEM node (type 6): first u16 of its prop stream.
  2. OTBM_ATTR_ITEM (9) inside an OTBM_TILE (5) / OTBM_HOUSETILE (14) prop
     stream: the u16 following the attribute byte (ground item).

Only those u16s are rewritten; every other byte is copied verbatim, so the
file structure is provably preserved. A --verify pass re-parses the output and
asserts the node tree (count/types) is unchanged and that every rewritten id
maps back correctly.
"""

import argparse
import json
import shutil
import sys
from datetime import datetime
from typing import Dict, List, Set, Tuple

ESCAPE = 0xFD
START = 0xFE
END = 0xFF

OTBM_TILE = 5
OTBM_ITEM = 6
OTBM_HOUSETILE = 14

OTBM_ATTR_TILE_FLAGS = 3
OTBM_ATTR_ITEM = 9


class OTBMError(Exception):
    pass


class PropReader:
    """Escape-aware reader over a prop byte range [pos, end) of `data`.

    Tracks, for each logical byte consumed, the original byte span so that a
    later in-place substitution can replace exactly the right raw bytes.
    """

    def __init__(self, data: bytes, pos: int, end: int):
        self.data = data
        self.pos = pos
        self.end = end

    def _read_raw_byte(self) -> Tuple[int, int, int]:
        """Return (value, orig_start, orig_end_excl) for one logical byte."""
        if self.pos >= self.end:
            raise OTBMError("prop underflow")
        start = self.pos
        b = self.data[self.pos]
        if b == ESCAPE:
            if self.pos + 1 >= self.end:
                raise OTBMError("escape at end of props")
            val = self.data[self.pos + 1]
            self.pos += 2
            return val, start, self.pos
        self.pos += 1
        return b, start, self.pos

    def read_u8(self) -> int:
        return self._read_raw_byte()[0]

    def read_u16_span(self) -> Tuple[int, int, int]:
        """Read a little-endian u16. Return (value, orig_start, orig_end)."""
        b0, s0, _ = self._read_raw_byte()
        b1, _, e1 = self._read_raw_byte()
        return b0 | (b1 << 8), s0, e1

    def skip(self, n: int) -> None:
        for _ in range(n):
            self._read_raw_byte()

    def at_end(self) -> bool:
        return self.pos >= self.end


def encode_u16(value: int) -> bytes:
    out = bytearray()
    for byte in (value & 0xFF, (value >> 8) & 0xFF):
        if byte in (ESCAPE, START, END):
            out.append(ESCAPE)
        out.append(byte)
    return bytes(out)


class OTBMConverter:
    def __init__(self, mapping: Dict[int, int]):
        self.mapping = mapping
        self.edits: List[Tuple[int, int, bytes]] = []  # (start, end, new_bytes)
        self.node_count = 0
        self.node_types: List[int] = []
        self.stats = {
            "items_found": 0,
            "items_converted": 0,
            "items_unchanged": 0,
            "items_unmapped": 0,
        }
        self.unmapped: Set[int] = set()

    def _map_id(self, sid: int) -> int:
        if sid in self.mapping:
            cid = self.mapping[sid]
            if cid != sid:
                self.stats["items_converted"] += 1
            else:
                self.stats["items_unchanged"] += 1
            return cid
        self.unmapped.add(sid)
        self.stats["items_unmapped"] += 1
        return sid

    def _queue_id_edit(self, reader: PropReader) -> None:
        sid, s, e = reader.read_u16_span()
        self.stats["items_found"] += 1
        cid = self._map_id(sid)
        if cid != sid:
            self.edits.append((s, e, encode_u16(cid)))

    def _convert_tile_props(self, data: bytes, pbegin: int, pend: int,
                            is_house: bool) -> None:
        r = PropReader(data, pbegin, pend)
        r.read_u8()  # x offset
        r.read_u8()  # y offset
        if is_house:
            r.skip(4)  # house_id u32
        while not r.at_end():
            attr = r.read_u8()
            if attr == OTBM_ATTR_TILE_FLAGS:
                r.skip(4)
            elif attr == OTBM_ATTR_ITEM:
                self._queue_id_edit(r)
            else:
                # Server only accepts TILE_FLAGS / ITEM in tile prop streams;
                # anything else means our structural assumption is wrong.
                raise OTBMError(f"unexpected tile attribute {attr}")

    def _parse_node(self, data: bytes, pos: int) -> int:
        """pos points at the node TYPE byte (just after its START).

        Returns the index just past this node's END byte.
        """
        ntype = data[pos]
        self.node_count += 1
        self.node_types.append(ntype)
        pos += 1
        props_begin = pos
        props_end = None
        first_child_seen = False

        i = pos
        while i < len(data):
            b = data[i]
            if b == ESCAPE:
                i += 2
                continue
            if b == START:
                if not first_child_seen:
                    props_end = i
                    first_child_seen = True
                    self._handle_node(data, ntype, props_begin, props_end)
                i = self._parse_node(data, i + 1)
                continue
            if b == END:
                if not first_child_seen:
                    props_end = i
                    self._handle_node(data, ntype, props_begin, props_end)
                return i + 1
            i += 1
        raise OTBMError("file underflow (missing END)")

    def _handle_node(self, data: bytes, ntype: int, pbegin: int,
                     pend: int) -> None:
        if ntype == OTBM_ITEM:
            self._queue_id_edit(PropReader(data, pbegin, pend))
        elif ntype in (OTBM_TILE, OTBM_HOUSETILE):
            self._convert_tile_props(data, pbegin, pend,
                                     ntype == OTBM_HOUSETILE)

    def convert(self, data: bytes) -> bytes:
        if len(data) < 5:
            raise OTBMError("file too small")
        if data[4] != START:
            raise OTBMError("invalid first byte (expected START at offset 4)")
        self._parse_node(data, 5)

        if not self.edits:
            return bytes(data)

        self.edits.sort()
        out = bytearray()
        cursor = 0
        for s, e, new in self.edits:
            if s < cursor:
                raise OTBMError("overlapping edits (parser bug)")
            out += data[cursor:s]
            out += new
            cursor = e
        out += data[cursor:]
        return bytes(out)


def verify(original: bytes, converted: bytes, mapping: Dict[int, int]) -> None:
    """Re-parse both; assert identical node tree and correct id remap."""
    a = OTBMConverter({})
    a._parse_node(original, 5)
    b = OTBMConverter({})
    b._parse_node(converted, 5)
    if a.node_count != b.node_count:
        raise OTBMError(
            f"node count changed: {a.node_count} -> {b.node_count}")
    if a.node_types != b.node_types:
        raise OTBMError("node type sequence changed")

    # Collect every item id (in tree order) from both files and check the
    # transformation matches the mapping exactly.
    def collect(buf: bytes) -> List[int]:
        c = OTBMConverter({})
        ids: List[int] = []
        c._queue_id_edit = lambda r: ids.append(r.read_u16_span()[0])  # type: ignore
        c._parse_node(buf, 5)
        return ids

    before = collect(original)
    after = collect(converted)
    if len(before) != len(after):
        raise OTBMError(
            f"item count changed: {len(before)} -> {len(after)}")
    for i, (sid, got) in enumerate(zip(before, after)):
        want = mapping.get(sid, sid)
        if got != want:
            raise OTBMError(
                f"item #{i}: {sid} -> {got}, expected {want}")


def load_mapping(path: str) -> Dict[int, int]:
    with open(path, "r", encoding="utf-8") as f:
        return {int(k): v for k, v in json.load(f).items()}


def main() -> None:
    p = argparse.ArgumentParser(
        description="Convert OTBM item IDs serverId -> clientId (safe).")
    p.add_argument("input", help="Input .otbm file")
    p.add_argument("-o", "--output", help="Output file (default: <input>.converted)")
    p.add_argument("-m", "--mapping", default="tools/server_to_client_map.json")
    p.add_argument("-r", "--replace", action="store_true",
                   help="Replace input in place (timestamped backup created)")
    p.add_argument("--no-verify", action="store_true",
                   help="Skip the re-parse verification pass")
    args = p.parse_args()

    try:
        mapping = load_mapping(args.mapping)
        print(f"Loaded {len(mapping)} ID mappings")

        with open(args.input, "rb") as f:
            data = f.read()

        conv = OTBMConverter(mapping)
        out = conv.convert(data)

        if not args.no_verify:
            verify(data, out, mapping)
            print("Verification OK: node tree preserved, ids remapped correctly")

        s = conv.stats
        print(f"Items found:     {s['items_found']}")
        print(f"Items converted: {s['items_converted']}")
        print(f"Items unchanged: {s['items_unchanged']}")
        print(f"Items unmapped:  {s['items_unmapped']}")
        if conv.unmapped:
            u = sorted(conv.unmapped)
            print(f"Unmapped IDs ({len(u)}): {u[:20]}"
                  + (" ..." if len(u) > 20 else ""))

        if args.replace:
            backup = f"{args.input}.backup.{datetime.now():%Y%m%d_%H%M%S}"
            shutil.copy2(args.input, backup)
            print(f"Backup: {backup}")
            target = args.input
        else:
            target = args.output or f"{args.input}.converted"
        with open(target, "wb") as f:
            f.write(out)
        print(f"Written: {target}")

    except (OTBMError, FileNotFoundError) as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
