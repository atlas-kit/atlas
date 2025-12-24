#!/usr/bin/env python3
"""
Lua Scripts Converter - Converte item IDs de serverId para clientId
Usado para migração de OTB para Appearances/Protobuf

MELHORADO: Agora converte:
- Arrays simples: {1499, 11099}
- Tabelas key-value: [3901] = 1666 (converte AMBOS key e value)
- Valores em tabelas: = 1666,
- Comentários com IDs para referência
"""

import json
import re
import sys
import os
import shutil
from pathlib import Path
from datetime import datetime
from typing import Dict, Set, List, Tuple
import argparse


class LuaConverter:
    def __init__(self, mapping: Dict[int, int], add_comments: bool = False):
        self.mapping = mapping
        self.add_comments = add_comments
        self.stats = {
            'files_processed': 0,
            'files_modified': 0,
            'ids_converted': 0,
        }
        self.unmapped_ids: Set[int] = set()
        self.converted_files: List[str] = []

    def convert_id(self, server_id: int) -> Tuple[int, bool]:
        """Convert a serverId to clientId. Returns (new_id, was_changed)"""
        if server_id in self.mapping:
            client_id = self.mapping[server_id]
            return client_id, server_id != client_id
        else:
            self.unmapped_ids.add(server_id)
            return server_id, False

    def _is_likely_item_id(self, value: int) -> bool:
        """Heuristic to determine if a number is likely an item ID"""
        # Item IDs in Tibia are typically in certain ranges
        # Avoid converting coordinates, percentages, small numbers, etc.
        return 100 <= value <= 50000

    def convert_file(self, input_path: str, output_path: str) -> bool:
        """Convert a single Lua file"""
        try:
            with open(input_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"  Error reading {input_path}: {e}")
            return False

        original_content = content

        # 1. Converter tabelas key-value: [key] = value ou [key] = {value}
        # Captura: [3901] = 1666 ou [3901] = {1666}
        def replace_table_keyvalue(match):
            key_id = int(match.group(1))
            equals_part = match.group(2)  # " = " ou " = {"
            value_id = int(match.group(3))
            suffix = match.group(4)  # "}" ou "," ou resto

            new_key = key_id
            new_value = value_id
            changed = False

            if self._is_likely_item_id(key_id):
                new_key, key_changed = self.convert_id(key_id)
                if key_changed:
                    self.stats['ids_converted'] += 1
                    changed = True

            if self._is_likely_item_id(value_id):
                new_value, val_changed = self.convert_id(value_id)
                if val_changed:
                    self.stats['ids_converted'] += 1
                    changed = True

            return f"[{new_key}]{equals_part}{new_value}{suffix}"

        # Padrão: [1234] = 5678, ou [1234] = {5678}
        content = re.sub(
            r'\[(\d+)\](\s*=\s*\{?\s*)(\d+)(\s*\}?,?)',
            replace_table_keyvalue,
            content
        )

        # 2. Converter arrays simples: {1499, 11099, 2782}
        # Detecta linhas que parecem ser arrays de IDs de items
        def replace_simple_array(match):
            full_match = match.group(0)
            var_name = match.group(1)
            array_content = match.group(2)

            # Extrair todos os números do array
            numbers = re.findall(r'\d+', array_content)

            # Verificar se a maioria são IDs de items válidos
            valid_ids = [int(n) for n in numbers if self._is_likely_item_id(int(n))]

            if len(valid_ids) < len(numbers) * 0.5:
                # Menos da metade são IDs válidos, provavelmente não é array de items
                return full_match

            # Converter cada número
            def convert_number(m):
                old_id = int(m.group(0))
                if self._is_likely_item_id(old_id):
                    new_id, changed = self.convert_id(old_id)
                    if changed:
                        self.stats['ids_converted'] += 1
                    return str(new_id)
                return m.group(0)

            new_content = re.sub(r'\b(\d+)\b', convert_number, array_content)
            return f"{var_name}{new_content}"

        # Padrão para arrays simples: local varName = {números}
        # Nomes comuns: Ids, Items, Ground, Sand, Hole, Fruits, Growth, etc.
        array_var_patterns = [
            r'(local\s+\w*[Ii]ds?\s*=\s*\{)([^}]+)(\})',
            r'(local\s+\w*[Ii]tems?\s*=\s*\{)([^}]+)(\})',
            r'(local\s+wildGrowth\s*=\s*\{)([^}]+)(\})',
            r'(local\s+jungleGrass\s*=\s*\{)([^}]+)(\})',
            r'(local\s+groundIds\s*=\s*\{)([^}]+)(\})',
            r'(local\s+sandIds\s*=\s*\{)([^}]+)(\})',
            r'(local\s+holeId\s*=\s*\{)([^}]+)(\})',
            r'(local\s+holes\s*=\s*\{)([^}]+)(\})',
            r'(local\s+fruits\s*=\s*\{)([^}]+)(\})',
            r'(local\s+ropeSpots\s*=\s*\{)([^}]+)(\})',
        ]

        for pattern in array_var_patterns:
            def replace_array(match):
                prefix = match.group(1)
                array_content = match.group(2)
                suffix = match.group(3)

                def convert_number(m):
                    old_id = int(m.group(0))
                    if self._is_likely_item_id(old_id):
                        new_id, changed = self.convert_id(old_id)
                        if changed:
                            self.stats['ids_converted'] += 1
                        return str(new_id)
                    return m.group(0)

                new_content = re.sub(r'\b(\d+)\b', convert_number, array_content)
                return f"{prefix}{new_content}{suffix}"

            content = re.sub(pattern, replace_array, content, flags=re.DOTALL)

        # 3. Converter valores após = em tabelas (que não foram pegos antes)
        # Padrão: = 1666, ou = 1666} ou = 1666 --
        def replace_table_value(match):
            prefix = match.group(1)  # "= " ou "= {"
            old_id = int(match.group(2))
            suffix = match.group(3)  # "," ou "}" ou " --"

            if not self._is_likely_item_id(old_id):
                return match.group(0)

            new_id, changed = self.convert_id(old_id)
            if changed:
                self.stats['ids_converted'] += 1
            return f"{prefix}{new_id}{suffix}"

        # Captura valores após =
        content = re.sub(
            r'(=\s*\{?\s*)(\d+)(\s*[,}\-])',
            replace_table_value,
            content
        )

        # 4. Converter chamadas de função (já existente, mas melhorado)
        func_patterns = [
            r'\b(Item)\s*\(\s*(\d+)',
            r'\b(doCreateItem)\s*\(\s*(\d+)',
            r'\b(doCreateItemEx)\s*\(\s*(\d+)',
            r'\b(getItemById)\s*\(\s*(\d+)',
            r'\b(player:addItem)\s*\(\s*(\d+)',
            r'\b(tile:getItemById)\s*\(\s*(\d+)',
            r'\b(container:addItem)\s*\(\s*(\d+)',
            r'\b(Game\.createItem)\s*\(\s*(\d+)',
            r'\b(ItemType)\s*\(\s*(\d+)',
            r'\b(item:transform)\s*\(\s*(\d+)',
            r'\b(target:transform)\s*\(\s*(\d+)',
            r'\b(ground:transform)\s*\(\s*(\d+)',
        ]

        for pattern in func_patterns:
            def replace_func(match):
                func_name = match.group(1)
                old_id = int(match.group(2))

                if not self._is_likely_item_id(old_id):
                    return match.group(0)

                new_id, changed = self.convert_id(old_id)
                if changed:
                    self.stats['ids_converted'] += 1
                return f"{func_name}({new_id}"

            content = re.sub(pattern, replace_func, content)

        # 5. Converter segundo argumento de funções
        second_arg_patterns = [
            r'\b(doPlayerAddItem)\s*\([^,]+,\s*(\d+)',
            r'\b(doPlayerAddItemEx)\s*\([^,]+,\s*(\d+)',
            r'\b(getTileItemById)\s*\([^,]+,\s*(\d+)',
        ]

        for pattern in second_arg_patterns:
            def replace_second_arg(match):
                prefix = match.group(1)
                old_id = int(match.group(2))

                if not self._is_likely_item_id(old_id):
                    return match.group(0)

                new_id, changed = self.convert_id(old_id)
                if changed:
                    self.stats['ids_converted'] += 1

                full_match = match.group(0)
                return full_match.replace(str(old_id), str(new_id), 1)

            content = re.sub(pattern, replace_second_arg, content)

        # 6. Converter comparações
        comparison_patterns = [
            r'(item:getId\(\)\s*==\s*)(\d+)',
            r'(item\.itemid\s*==\s*)(\d+)',
            r'(target\.itemid\s*==\s*)(\d+)',
            r'(ground\.itemid\s*==\s*)(\d+)',
            r'(itemId\s*==\s*)(\d+)',
            r'(\.itemid\s*==\s*)(\d+)',
        ]

        for pattern in comparison_patterns:
            def replace_comparison(match):
                prefix = match.group(1)
                old_id = int(match.group(2))

                if not self._is_likely_item_id(old_id):
                    return match.group(0)

                new_id, changed = self.convert_id(old_id)
                if changed:
                    self.stats['ids_converted'] += 1
                return f"{prefix}{new_id}"

            content = re.sub(pattern, replace_comparison, content)

        # 7. Converter atribuições de variáveis
        assignment_patterns = [
            r'([iI]tem[iI]d)\s*=\s*(\d+)',
            r'([iI]tem_[iI]d)\s*=\s*(\d+)',
            r'(itemid)\s*=\s*(\d+)',
            r'(transformTo)\s*=\s*(\d+)',
        ]

        for pattern in assignment_patterns:
            def replace_assignment(match):
                var_name = match.group(1)
                old_id = int(match.group(2))

                if not self._is_likely_item_id(old_id):
                    return match.group(0)

                new_id, changed = self.convert_id(old_id)
                if changed:
                    self.stats['ids_converted'] += 1
                return f"{var_name} = {new_id}"

            content = re.sub(pattern, replace_assignment, content)

        # 8. Converter table.contains e similares
        contains_patterns = [
            r'(table\.contains\s*\([^,]+,\s*)(\d+)(\s*\))',
        ]

        for pattern in contains_patterns:
            def replace_contains(match):
                prefix = match.group(1)
                old_id = int(match.group(2))
                suffix = match.group(3)

                if not self._is_likely_item_id(old_id):
                    return match.group(0)

                new_id, changed = self.convert_id(old_id)
                if changed:
                    self.stats['ids_converted'] += 1
                return f"{prefix}{new_id}{suffix}"

            content = re.sub(pattern, replace_contains, content)

        self.stats['files_processed'] += 1

        if content != original_content:
            self.stats['files_modified'] += 1
            self.converted_files.append(input_path)

        try:
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(content)
        except Exception as e:
            print(f"  Error writing {output_path}: {e}")
            return False

        return True


def load_mapping(mapping_file: str) -> Dict[int, int]:
    """Load the serverId -> clientId mapping"""
    with open(mapping_file, 'r') as f:
        mapping = json.load(f)
    return {int(k): v for k, v in mapping.items()}


def main():
    parser = argparse.ArgumentParser(description='Convert Lua scripts from serverId to clientId')
    parser.add_argument('input', help='Input directory or file')
    parser.add_argument('-o', '--output', help='Output directory (default: same as input with .converted suffix)')
    parser.add_argument('-m', '--mapping', default='tools/server_to_client_map.json',
                       help='Mapping file (JSON)')
    parser.add_argument('-r', '--replace', action='store_true',
                       help='Replace files in-place (creates backups)')
    parser.add_argument('--recursive', action='store_true', default=True,
                       help='Process directories recursively (default: True)')
    parser.add_argument('--dry-run', action='store_true',
                       help='Show what would be done without making changes')
    parser.add_argument('-v', '--verbose', action='store_true',
                       help='Show each file being processed')
    parser.add_argument('--add-comments', action='store_true',
                       help='Add comments showing old IDs (for reference)')

    args = parser.parse_args()

    try:
        print(f"Loading mapping from {args.mapping}...")
        mapping = load_mapping(args.mapping)
        print(f"  Loaded {len(mapping)} ID mappings")

        converter = LuaConverter(mapping, add_comments=args.add_comments)
        input_path = Path(args.input)

        if input_path.is_file():
            # Single file
            if args.replace:
                backup_path = f"{args.input}.backup.{datetime.now().strftime('%Y%m%d_%H%M%S')}"
                shutil.copy2(args.input, backup_path)
                output_path = args.input
            else:
                output_path = args.output or f"{args.input}.converted"

            print(f"\nConverting {args.input}...")
            if not args.dry_run:
                converter.convert_file(args.input, output_path)
            print(f"Output: {output_path}")

        elif input_path.is_dir():
            # Directory
            lua_files = list(input_path.rglob('*.lua') if args.recursive else input_path.glob('*.lua'))

            if not lua_files:
                print(f"No .lua files found in {args.input}")
                sys.exit(0)

            print(f"\nFound {len(lua_files)} Lua files")

            if args.replace:
                output_dir = input_path
            elif args.output:
                output_dir = Path(args.output)
            else:
                output_dir = Path(str(input_path) + '_converted')

            if not args.dry_run and not args.replace:
                output_dir.mkdir(parents=True, exist_ok=True)

            for lua_file in lua_files:
                rel_path = lua_file.relative_to(input_path)

                if args.replace:
                    out_file = lua_file
                else:
                    out_file = output_dir / rel_path
                    if not args.dry_run:
                        out_file.parent.mkdir(parents=True, exist_ok=True)

                if args.verbose:
                    print(f"  Processing {rel_path}...")

                if not args.dry_run:
                    converter.convert_file(str(lua_file), str(out_file))

            print(f"\n{'=' * 60}")
            print("CONVERSION STATISTICS")
            print(f"{'=' * 60}")
            print(f"Files processed: {converter.stats['files_processed']}")
            print(f"Files modified: {converter.stats['files_modified']}")
            print(f"IDs converted: {converter.stats['ids_converted']}")

            if converter.converted_files and args.verbose:
                print(f"\nModified files:")
                for f in converter.converted_files[:20]:
                    print(f"  - {f}")
                if len(converter.converted_files) > 20:
                    print(f"  ... and {len(converter.converted_files) - 20} more")

            if converter.unmapped_ids:
                unmapped = sorted(converter.unmapped_ids)
                print(f"\n⚠️  Unmapped IDs ({len(unmapped)}):")
                if len(unmapped) <= 20:
                    print(f"   {unmapped}")
                else:
                    print(f"   First 20: {unmapped[:20]}")

            if not args.replace:
                print(f"\nOutput directory: {output_dir}")
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


if __name__ == '__main__':
    main()
