#!/usr/bin/env python3
"""
OTB Extractor - Extrai mapeamento serverId -> clientId do items.otb
Usado para migração de OTB para Appearances/Protobuf
"""

import struct
import json
import csv
import sys
from pathlib import Path
from dataclasses import dataclass, asdict
from typing import List, Dict, Optional

# OTB Node markers
NODE_ESCAPE = 0xFD
NODE_START = 0xFE
NODE_END = 0xFF

# OTB Attributes
ITEM_ATTR_SERVERID = 0x10
ITEM_ATTR_CLIENTID = 0x11
ITEM_ATTR_NAME = 0x12
ITEM_ATTR_DESCR = 0x13
ITEM_ATTR_SPEED = 0x14
ITEM_ATTR_SLOT = 0x15
ITEM_ATTR_MAXITEMS = 0x16
ITEM_ATTR_WEIGHT = 0x17
ITEM_ATTR_WEAPON = 0x18
ITEM_ATTR_AMU = 0x19
ITEM_ATTR_ARMOR = 0x1A
ITEM_ATTR_MAGLEVEL = 0x1B
ITEM_ATTR_MAGFIELDTYPE = 0x1C
ITEM_ATTR_WRITEABLE = 0x1D
ITEM_ATTR_ROTATETO = 0x1E
ITEM_ATTR_DECAY = 0x1F
ITEM_ATTR_SPRITEHASH = 0x20
ITEM_ATTR_MINIMAPCOLOR = 0x21
ITEM_ATTR_07 = 0x22
ITEM_ATTR_08 = 0x23
ITEM_ATTR_LIGHT = 0x24
ITEM_ATTR_DECAY2 = 0x25
ITEM_ATTR_WEAPON2 = 0x26
ITEM_ATTR_AMU2 = 0x27
ITEM_ATTR_ARMOR2 = 0x28
ITEM_ATTR_WRITEABLE2 = 0x29
ITEM_ATTR_LIGHT2 = 0x2A
ITEM_ATTR_TOPORDER = 0x2B
ITEM_ATTR_WRITEABLE3 = 0x2C
ITEM_ATTR_WAREID = 0x2D
ITEM_ATTR_CLASSIFICATION = 0x2E

# Item Groups
ITEM_GROUP_NONE = 0
ITEM_GROUP_GROUND = 1
ITEM_GROUP_CONTAINER = 2
ITEM_GROUP_WEAPON = 3
ITEM_GROUP_AMMUNITION = 4
ITEM_GROUP_ARMOR = 5
ITEM_GROUP_CHARGES = 6
ITEM_GROUP_TELEPORT = 7
ITEM_GROUP_MAGICFIELD = 8
ITEM_GROUP_WRITEABLE = 9
ITEM_GROUP_KEY = 10
ITEM_GROUP_SPLASH = 11
ITEM_GROUP_FLUID = 12
ITEM_GROUP_DOOR = 13
ITEM_GROUP_DEPRECATED = 14
ITEM_GROUP_PODIUM = 15

# Item Flags
FLAG_BLOCK_SOLID = 1 << 0
FLAG_BLOCK_PROJECTILE = 1 << 1
FLAG_BLOCK_PATHFIND = 1 << 2
FLAG_HAS_HEIGHT = 1 << 3
FLAG_USEABLE = 1 << 4
FLAG_PICKUPABLE = 1 << 5
FLAG_MOVEABLE = 1 << 6
FLAG_STACKABLE = 1 << 7
FLAG_ALWAYSONTOP = 1 << 13
FLAG_READABLE = 1 << 14
FLAG_ROTATABLE = 1 << 15
FLAG_HANGABLE = 1 << 16
FLAG_VERTICAL = 1 << 17
FLAG_HORIZONTAL = 1 << 18
FLAG_ALLOWDISTREAD = 1 << 20
FLAG_CLIENTDURATION = 1 << 21
FLAG_CLIENTCHARGES = 1 << 22
FLAG_LOOKTHROUGH = 1 << 23
FLAG_ANIMATION = 1 << 24
FLAG_FORCEUSE = 1 << 26

GROUP_NAMES = {
    0: "NONE", 1: "GROUND", 2: "CONTAINER", 3: "WEAPON",
    4: "AMMUNITION", 5: "ARMOR", 6: "CHARGES", 7: "TELEPORT",
    8: "MAGICFIELD", 9: "WRITEABLE", 10: "KEY", 11: "SPLASH",
    12: "FLUID", 13: "DOOR", 14: "DEPRECATED", 15: "PODIUM"
}


@dataclass
class ItemData:
    server_id: int
    client_id: int
    group: int
    group_name: str
    flags: int
    speed: int = 0
    light_level: int = 0
    light_color: int = 0
    ware_id: int = 0
    classification: int = 0
    top_order: int = 0
    # Parsed flags
    block_solid: bool = False
    block_projectile: bool = False
    block_pathfind: bool = False
    has_height: bool = False
    useable: bool = False
    pickupable: bool = False
    moveable: bool = False
    stackable: bool = False
    always_on_top: bool = False
    readable: bool = False
    rotatable: bool = False
    hangable: bool = False
    vertical: bool = False
    horizontal: bool = False
    allow_dist_read: bool = False
    look_through: bool = False
    is_animation: bool = False
    force_use: bool = False


class OTBReader:
    def __init__(self, data: bytes):
        self.data = data
        self.pos = 0

    def read_byte(self) -> int:
        if self.pos >= len(self.data):
            raise EOFError("End of data")
        b = self.data[self.pos]
        self.pos += 1
        return b

    def read_escaped_byte(self) -> int:
        b = self.read_byte()
        if b == NODE_ESCAPE:
            return self.read_byte()
        return b

    def read_u16(self) -> int:
        low = self.read_escaped_byte()
        high = self.read_escaped_byte()
        return low | (high << 8)

    def read_u32(self) -> int:
        b0 = self.read_escaped_byte()
        b1 = self.read_escaped_byte()
        b2 = self.read_escaped_byte()
        b3 = self.read_escaped_byte()
        return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)

    def read_bytes(self, count: int) -> bytes:
        result = bytearray()
        for _ in range(count):
            result.append(self.read_escaped_byte())
        return bytes(result)

    def skip(self, count: int):
        for _ in range(count):
            self.read_escaped_byte()

    def peek(self) -> int:
        if self.pos >= len(self.data):
            return -1
        return self.data[self.pos]

    def at_end(self) -> bool:
        return self.pos >= len(self.data)


def parse_otb(filepath: str) -> tuple:
    """Parse OTB file and return (version_info, items)"""

    with open(filepath, 'rb') as f:
        data = f.read()

    # Check identifier (first 4 bytes should be 0x00000000 for OTBI)
    if data[:4] != b'\x00\x00\x00\x00':
        raise ValueError("Invalid OTB file - wrong identifier")

    # Skip identifier
    reader = OTBReader(data[4:])

    # Read root node start
    if reader.read_byte() != NODE_START:
        raise ValueError("Expected NODE_START")

    # Root node type (should be 0)
    root_type = reader.read_byte()

    # Read root node flags (4 bytes, unused)
    reader.skip(4)

    # Read root attribute version
    attr_version = reader.read_byte()
    if attr_version != 0x01:
        raise ValueError(f"Unknown root attribute version: {attr_version}")

    # Read version info length (should be 140)
    version_len = reader.read_u16()
    if version_len != 140:
        raise ValueError(f"Invalid version info length: {version_len}")

    # Read version info
    major_version = reader.read_u32()
    minor_version = reader.read_u32()
    build_number = reader.read_u32()

    # Skip rest of version info (140 - 12 = 128 bytes)
    reader.skip(128)

    version_info = {
        'major': major_version,
        'minor': minor_version,
        'build': build_number
    }

    print(f"OTB Version: {major_version}.{minor_version}.{build_number}")

    items: List[ItemData] = []

    # Parse item nodes
    while not reader.at_end():
        b = reader.peek()

        if b == NODE_START:
            reader.read_byte()  # consume NODE_START

            # Read item group (node type)
            group = reader.read_byte()

            # Read flags
            flags = reader.read_u32()

            server_id = 0
            client_id = 0
            speed = 0
            light_level = 0
            light_color = 0
            ware_id = 0
            classification = 0
            top_order = 0

            # Read attributes until NODE_END
            while reader.peek() not in (NODE_END, NODE_START, -1):
                attr = reader.read_byte()

                # Handle escaped attribute byte
                if attr == NODE_ESCAPE:
                    attr = reader.read_byte()

                attr_len = reader.read_u16()

                if attr == ITEM_ATTR_SERVERID:
                    server_id = reader.read_u16()
                elif attr == ITEM_ATTR_CLIENTID:
                    client_id = reader.read_u16()
                elif attr == ITEM_ATTR_SPEED:
                    speed = reader.read_u16()
                elif attr == ITEM_ATTR_LIGHT2:
                    light_level = reader.read_u16()
                    light_color = reader.read_u16()
                elif attr == ITEM_ATTR_TOPORDER:
                    top_order = reader.read_byte()
                elif attr == ITEM_ATTR_WAREID:
                    ware_id = reader.read_u16()
                elif attr == ITEM_ATTR_CLASSIFICATION:
                    classification = reader.read_byte()
                else:
                    # Skip unknown attribute
                    reader.skip(attr_len)

            if server_id > 0:
                item = ItemData(
                    server_id=server_id,
                    client_id=client_id,
                    group=group,
                    group_name=GROUP_NAMES.get(group, f"UNKNOWN_{group}"),
                    flags=flags,
                    speed=speed,
                    light_level=light_level,
                    light_color=light_color,
                    ware_id=ware_id,
                    classification=classification,
                    top_order=top_order,
                    block_solid=bool(flags & FLAG_BLOCK_SOLID),
                    block_projectile=bool(flags & FLAG_BLOCK_PROJECTILE),
                    block_pathfind=bool(flags & FLAG_BLOCK_PATHFIND),
                    has_height=bool(flags & FLAG_HAS_HEIGHT),
                    useable=bool(flags & FLAG_USEABLE),
                    pickupable=bool(flags & FLAG_PICKUPABLE),
                    moveable=bool(flags & FLAG_MOVEABLE),
                    stackable=bool(flags & FLAG_STACKABLE),
                    always_on_top=bool(flags & FLAG_ALWAYSONTOP),
                    readable=bool(flags & FLAG_READABLE),
                    rotatable=bool(flags & FLAG_ROTATABLE),
                    hangable=bool(flags & FLAG_HANGABLE),
                    vertical=bool(flags & FLAG_VERTICAL),
                    horizontal=bool(flags & FLAG_HORIZONTAL),
                    allow_dist_read=bool(flags & FLAG_ALLOWDISTREAD),
                    look_through=bool(flags & FLAG_LOOKTHROUGH),
                    is_animation=bool(flags & FLAG_ANIMATION),
                    force_use=bool(flags & FLAG_FORCEUSE),
                )
                items.append(item)

        elif b == NODE_END:
            reader.read_byte()  # consume NODE_END
        else:
            reader.read_byte()  # skip unknown byte

    return version_info, items


def export_mapping(items: List[ItemData], output_path: str, format: str = 'json'):
    """Export server_id -> client_id mapping"""

    if format == 'json':
        # Full data export
        data = {
            'total_items': len(items),
            'items': [asdict(item) for item in items]
        }
        with open(output_path, 'w') as f:
            json.dump(data, f, indent=2)

    elif format == 'csv':
        with open(output_path, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['server_id', 'client_id', 'group', 'flags', 'speed',
                           'light_level', 'light_color', 'ware_id', 'classification'])
            for item in items:
                writer.writerow([
                    item.server_id, item.client_id, item.group_name, item.flags,
                    item.speed, item.light_level, item.light_color,
                    item.ware_id, item.classification
                ])

    elif format == 'mapping':
        # Simple server_id -> client_id mapping
        mapping = {item.server_id: item.client_id for item in items}
        with open(output_path, 'w') as f:
            json.dump(mapping, f, indent=2)

    elif format == 'reverse':
        # client_id -> server_id mapping (for appearances migration)
        mapping = {}
        for item in items:
            if item.client_id not in mapping:
                mapping[item.client_id] = item.server_id
        with open(output_path, 'w') as f:
            json.dump(mapping, f, indent=2)


def print_stats(items: List[ItemData]):
    """Print statistics about the items"""

    print(f"\n{'='*60}")
    print(f"ESTATÍSTICAS DO items.otb")
    print(f"{'='*60}")
    print(f"Total de items: {len(items)}")

    # Group statistics
    groups = {}
    for item in items:
        groups[item.group_name] = groups.get(item.group_name, 0) + 1

    print(f"\nPor grupo:")
    for group, count in sorted(groups.items(), key=lambda x: -x[1]):
        print(f"  {group}: {count}")

    # ID ranges
    server_ids = [item.server_id for item in items]
    client_ids = [item.client_id for item in items if item.client_id > 0]

    print(f"\nServer IDs: {min(server_ids)} - {max(server_ids)}")
    print(f"Client IDs: {min(client_ids)} - {max(client_ids)}")

    # Items where server_id != client_id
    different = [item for item in items if item.server_id != item.client_id]
    print(f"\nItems com server_id ≠ client_id: {len(different)}")

    # Sample of differences
    if different:
        print(f"\nExemplos de diferenças (primeiros 10):")
        for item in different[:10]:
            print(f"  server_id={item.server_id} -> client_id={item.client_id} ({item.group_name})")


def main():
    import argparse

    parser = argparse.ArgumentParser(description='Extrai dados do items.otb')
    parser.add_argument('input', nargs='?', default='data/items/items.otb',
                       help='Caminho para items.otb')
    parser.add_argument('-o', '--output', default='tools/otb_mapping.json',
                       help='Arquivo de saída')
    parser.add_argument('-f', '--format', choices=['json', 'csv', 'mapping', 'reverse'],
                       default='json', help='Formato de saída')
    parser.add_argument('--stats', action='store_true',
                       help='Mostrar estatísticas')

    args = parser.parse_args()

    try:
        print(f"Lendo {args.input}...")
        version_info, items = parse_otb(args.input)

        if args.stats:
            print_stats(items)

        print(f"\nExportando para {args.output} (formato: {args.format})...")
        export_mapping(items, args.output, args.format)
        print(f"Exportado {len(items)} items com sucesso!")

    except FileNotFoundError:
        print(f"Erro: Arquivo não encontrado: {args.input}")
        sys.exit(1)
    except Exception as e:
        print(f"Erro: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
