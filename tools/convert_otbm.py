#!/usr/bin/env python3
"""
OTBM Map Converter - Converte item IDs de serverId para clientId
Usado para migração de OTB para Appearances/Protobuf
"""

import json
import struct
import sys
import os
import shutil
from pathlib import Path
from datetime import datetime
from typing import Dict, Set, Tuple
import argparse

# OTBM Node markers (same as OTB)
NODE_ESCAPE = 0xFD
NODE_START = 0xFE
NODE_END = 0xFF

# OTBM Node types
OTBM_ROOTV1 = 0
OTBM_MAP_DATA = 2
OTBM_TILE_AREA = 4
OTBM_TILE = 5
OTBM_ITEM = 6
OTBM_HOUSETILE = 14
OTBM_WAYPOINTS = 15
OTBM_WAYPOINT = 16
OTBM_TOWNS = 12
OTBM_TOWN = 13

# OTBM Attributes that contain item IDs
OTBM_ATTR_ITEM = 9


class OTBMConverter:
    def __init__(self, mapping: Dict[int, int]):
        self.mapping = mapping
        self.stats = {
            'items_found': 0,
            'items_converted': 0,
            'items_unchanged': 0,
            'items_unmapped': 0
        }
        self.unmapped_ids: Set[int] = set()

    def convert_id(self, server_id: int) -> int:
        """Convert a serverId to clientId"""
        if server_id in self.mapping:
            client_id = self.mapping[server_id]
            if server_id != client_id:
                self.stats['items_converted'] += 1
            else:
                self.stats['items_unchanged'] += 1
            return client_id
        else:
            self.unmapped_ids.add(server_id)
            self.stats['items_unmapped'] += 1
            return server_id  # Keep original if not mapped

    def convert_otbm(self, input_path: str, output_path: str) -> bool:
        """Convert an OTBM file, replacing serverIds with clientIds"""

        with open(input_path, 'rb') as f:
            data = bytearray(f.read())

        # OTBM starts with 4-byte identifier (usually 0x00000000 for OTBM)
        if len(data) < 4:
            print(f"Error: File too small")
            return False

        # Parse and convert the OTBM
        converted_data = self._process_otbm(data)

        # Write output
        with open(output_path, 'wb') as f:
            f.write(converted_data)

        return True

    def _process_otbm(self, data: bytearray) -> bytearray:
        """Process OTBM data and convert item IDs"""

        # We need to find all OTBM_ATTR_ITEM (0x09) attributes and convert the uint16 that follows
        # The challenge is handling escape sequences properly

        result = bytearray()
        i = 0
        length = len(data)

        while i < length:
            byte = data[i]

            if byte == NODE_ESCAPE and i + 1 < length:
                # Escape sequence - copy both bytes
                result.append(byte)
                result.append(data[i + 1])
                i += 2
            elif byte == OTBM_ATTR_ITEM:
                # Found OTBM_ATTR_ITEM - the next 2 bytes are the item ID
                result.append(byte)
                i += 1

                if i + 2 <= length:
                    # Read the item ID (handling escapes)
                    item_id, bytes_read = self._read_uint16_escaped(data, i)
                    self.stats['items_found'] += 1

                    # Convert the ID
                    new_id = self.convert_id(item_id)

                    # Write the new ID (with proper escaping)
                    escaped_bytes = self._write_uint16_escaped(new_id)
                    result.extend(escaped_bytes)

                    i += bytes_read
                else:
                    # Not enough bytes, just copy
                    result.append(data[i])
                    i += 1
            else:
                result.append(byte)
                i += 1

        return result

    def _read_uint16_escaped(self, data: bytearray, pos: int) -> Tuple[int, int]:
        """Read a uint16 from data, handling escape sequences. Returns (value, bytes_consumed)"""
        bytes_consumed = 0
        values = []

        for _ in range(2):
            if pos + bytes_consumed >= len(data):
                break

            byte = data[pos + bytes_consumed]
            bytes_consumed += 1

            if byte == NODE_ESCAPE and pos + bytes_consumed < len(data):
                byte = data[pos + bytes_consumed]
                bytes_consumed += 1

            values.append(byte)

        if len(values) == 2:
            return values[0] | (values[1] << 8), bytes_consumed
        return 0, bytes_consumed

    def _write_uint16_escaped(self, value: int) -> bytearray:
        """Write a uint16, escaping special bytes"""
        result = bytearray()

        low_byte = value & 0xFF
        high_byte = (value >> 8) & 0xFF

        for byte in [low_byte, high_byte]:
            if byte in (NODE_ESCAPE, NODE_START, NODE_END):
                result.append(NODE_ESCAPE)
            result.append(byte)

        return result


def convert_map_file(input_path: str, output_path: str, mapping: Dict[int, int]) -> dict:
    """Convert a single OTBM file"""
    converter = OTBMConverter(mapping)

    if not converter.convert_otbm(input_path, output_path):
        return {'error': 'Conversion failed'}

    return {
        'stats': converter.stats,
        'unmapped': sorted(converter.unmapped_ids)
    }


def load_mapping(mapping_file: str) -> Dict[int, int]:
    """Load the serverId -> clientId mapping"""
    with open(mapping_file, 'r') as f:
        mapping = json.load(f)
    # Convert string keys to int
    return {int(k): v for k, v in mapping.items()}


def main():
    parser = argparse.ArgumentParser(description='Convert OTBM map from serverId to clientId')
    parser.add_argument('input', help='Input OTBM file or directory')
    parser.add_argument('-o', '--output', help='Output file or directory')
    parser.add_argument('-m', '--mapping', default='tools/server_to_client_map.json',
                       help='Mapping file (JSON)')
    parser.add_argument('-b', '--backup', action='store_true',
                       help='Create backup of original files')
    parser.add_argument('-r', '--replace', action='store_true',
                       help='Replace original files (creates backup automatically)')
    parser.add_argument('--recursive', action='store_true',
                       help='Process directories recursively')

    args = parser.parse_args()

    try:
        print(f"Loading mapping from {args.mapping}...")
        mapping = load_mapping(args.mapping)
        print(f"  Loaded {len(mapping)} ID mappings")

        input_path = Path(args.input)

        if input_path.is_file():
            # Single file
            if args.replace:
                backup_path = f"{args.input}.backup.{datetime.now().strftime('%Y%m%d_%H%M%S')}"
                shutil.copy2(args.input, backup_path)
                print(f"Backup created: {backup_path}")
                output_path = args.input
            else:
                output_path = args.output or f"{args.input}.converted"

            print(f"\nConverting {args.input}...")
            result = convert_map_file(args.input, output_path, mapping)

            if 'error' in result:
                print(f"Error: {result['error']}")
                sys.exit(1)

            print_stats(result)
            print(f"\nOutput saved to: {output_path}")

        elif input_path.is_dir():
            # Directory
            pattern = '**/*.otbm' if args.recursive else '*.otbm'
            files = list(input_path.glob(pattern))

            if not files:
                print(f"No .otbm files found in {args.input}")
                sys.exit(1)

            print(f"Found {len(files)} OTBM files to convert")

            if args.output:
                output_dir = Path(args.output)
                output_dir.mkdir(parents=True, exist_ok=True)
            else:
                output_dir = input_path

            total_stats = {
                'items_found': 0,
                'items_converted': 0,
                'items_unchanged': 0,
                'items_unmapped': 0
            }
            all_unmapped: Set[int] = set()

            for otbm_file in files:
                rel_path = otbm_file.relative_to(input_path)

                if args.replace:
                    backup_path = f"{otbm_file}.backup.{datetime.now().strftime('%Y%m%d_%H%M%S')}"
                    shutil.copy2(otbm_file, backup_path)
                    out_file = otbm_file
                else:
                    out_file = output_dir / rel_path
                    out_file.parent.mkdir(parents=True, exist_ok=True)

                print(f"  Converting {rel_path}...")
                result = convert_map_file(str(otbm_file), str(out_file), mapping)

                if 'error' not in result:
                    for key in total_stats:
                        total_stats[key] += result['stats'][key]
                    all_unmapped.update(result['unmapped'])

            print("\n" + "=" * 60)
            print("TOTAL STATISTICS")
            print("=" * 60)
            print_stats({'stats': total_stats, 'unmapped': sorted(all_unmapped)})

        else:
            print(f"Error: {args.input} not found")
            sys.exit(1)

    except FileNotFoundError as e:
        print(f"Error: File not found: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


def print_stats(result: dict):
    stats = result['stats']
    unmapped = result.get('unmapped', [])

    print(f"\nItems found: {stats['items_found']}")
    print(f"Items converted: {stats['items_converted']}")
    print(f"Items unchanged (same ID): {stats['items_unchanged']}")
    print(f"Items unmapped (kept original): {stats['items_unmapped']}")

    if unmapped:
        print(f"\n⚠️  Unmapped IDs ({len(unmapped)}):")
        if len(unmapped) <= 20:
            print(f"   {unmapped}")
        else:
            print(f"   First 20: {unmapped[:20]}")


if __name__ == '__main__':
    main()
