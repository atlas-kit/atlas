#!/usr/bin/env python3
"""
Conversor de items.xml - Muda server_id para client_id
Usado para migração de OTB para Appearances/Protobuf
"""

import json
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Dict, Set
import argparse
import shutil
from datetime import datetime


def load_mapping(mapping_file: str) -> Dict[int, int]:
    """Carrega o mapeamento server_id -> client_id"""
    with open(mapping_file, 'r') as f:
        mapping = json.load(f)
    # Converter chaves de string para int
    return {int(k): v for k, v in mapping.items()}


def convert_id(server_id: int, mapping: Dict[int, int], unmapped: Set[int]) -> int:
    """Converte um server_id para client_id"""
    if server_id in mapping:
        return mapping[server_id]
    else:
        unmapped.add(server_id)
        return server_id  # Mantém o original se não encontrar


def convert_items_xml(input_file: str, output_file: str, mapping: Dict[int, int]) -> dict:
    """Converte o items.xml trocando server_id por client_id"""

    # Atributos que contêm IDs de items que precisam ser convertidos
    ID_ATTRIBUTES = {
        'id', 'fromid', 'toid',  # Definição principal
        'rotateto', 'decayto', 'transformequipto', 'transformdeequipto',
        'destroyto', 'writeonceitemid', 'maletransformto', 'femaletransformto',
        'transformto'
    }

    unmapped: Set[int] = set()
    stats = {
        'items_converted': 0,
        'ids_changed': 0,
        'ranges_converted': 0,
        'attribute_refs_converted': 0,
    }

    # Ler arquivo original
    with open(input_file, 'r', encoding='utf-8') as f:
        content = f.read()

    # Usar regex para processar o XML preservando formatação
    # Isso é melhor que ElementTree para preservar comentários e formatação

    def replace_id_in_attr(match):
        """Substitui IDs em atributos"""
        attr_name = match.group(1)
        old_id = int(match.group(2))
        new_id = convert_id(old_id, mapping, unmapped)

        if old_id != new_id:
            stats['ids_changed'] += 1

        return f'{attr_name}="{new_id}"'

    def replace_value_attr(match):
        """Substitui IDs em atributos key/value que referenciam items"""
        key = match.group(1)
        old_id = int(match.group(2))
        new_id = convert_id(old_id, mapping, unmapped)

        if old_id != new_id:
            stats['attribute_refs_converted'] += 1

        return f'key="{key}" value="{new_id}"'

    # Padrões para substituição

    # 1. Substituir id="X", fromid="X", toid="X"
    for attr in ['id', 'fromid', 'toid']:
        pattern = rf'({attr})="(\d+)"'
        content = re.sub(pattern, replace_id_in_attr, content)

    # 2. Substituir atributos que referenciam outros items
    ref_keys = [
        'rotateto', 'decayto', 'transformequipto', 'transformdeequipto',
        'destroyto', 'writeonceitemid', 'maletransformto', 'femaletransformto',
        'transformto'
    ]

    for key in ref_keys:
        pattern = rf'key="({key})"\s+value="(\d+)"'
        content = re.sub(pattern, replace_value_attr, content, flags=re.IGNORECASE)

    # Contar items convertidos
    stats['items_converted'] = len(re.findall(r'<item\s', content))

    # Escrever arquivo convertido
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(content)

    return {
        'stats': stats,
        'unmapped': sorted(unmapped)
    }


def convert_lua_scripts(input_dir: str, output_dir: str, mapping: Dict[int, int]) -> dict:
    """Converte IDs de items em scripts Lua"""

    stats = {
        'files_processed': 0,
        'files_modified': 0,
        'ids_changed': 0
    }

    input_path = Path(input_dir)
    output_path = Path(output_dir)

    # Padrões comuns de uso de item IDs em Lua
    patterns = [
        # Item(id), doCreateItem(id), etc
        r'\b(Item|doCreateItem|doPlayerAddItem|doCreateItemEx|getTileItemById|getItemById)\s*\(\s*(\d+)',
        # itemid = X, ItemId = X
        r'([iI]tem[iI]d)\s*=\s*(\d+)',
        # ITEM_XXX = X (constantes)
        r'(ITEM_\w+)\s*=\s*(\d+)',
    ]

    unmapped: Set[int] = set()

    for lua_file in input_path.rglob('*.lua'):
        rel_path = lua_file.relative_to(input_path)
        out_file = output_path / rel_path
        out_file.parent.mkdir(parents=True, exist_ok=True)

        with open(lua_file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        original_content = content

        def replace_lua_id(match):
            prefix = match.group(1)
            old_id = int(match.group(2))
            new_id = convert_id(old_id, mapping, unmapped)

            if old_id != new_id:
                stats['ids_changed'] += 1

            return f'{prefix}({new_id}' if '(' in match.group(0) else f'{prefix} = {new_id}'

        for pattern in patterns:
            content = re.sub(pattern, replace_lua_id, content)

        with open(out_file, 'w', encoding='utf-8') as f:
            f.write(content)

        stats['files_processed'] += 1
        if content != original_content:
            stats['files_modified'] += 1

    return {
        'stats': stats,
        'unmapped': sorted(unmapped)
    }


def main():
    parser = argparse.ArgumentParser(description='Converte items.xml de server_id para client_id')
    parser.add_argument('--mapping', '-m', default='tools/server_to_client_map.json',
                       help='Arquivo de mapeamento JSON')
    parser.add_argument('--input', '-i', default='data/items/items.xml',
                       help='Arquivo items.xml de entrada')
    parser.add_argument('--output', '-o', default='data/items/items_converted.xml',
                       help='Arquivo items.xml de saída')
    parser.add_argument('--backup', '-b', action='store_true',
                       help='Criar backup do arquivo original')
    parser.add_argument('--replace', '-r', action='store_true',
                       help='Substituir arquivo original (cria backup automaticamente)')
    parser.add_argument('--lua-dir', help='Diretório de scripts Lua para converter')
    parser.add_argument('--lua-out', help='Diretório de saída para Lua convertido')

    args = parser.parse_args()

    try:
        print(f"Carregando mapeamento de {args.mapping}...")
        mapping = load_mapping(args.mapping)
        print(f"  {len(mapping)} mapeamentos carregados")

        # Backup se necessário
        if args.backup or args.replace:
            backup_name = f"{args.input}.backup.{datetime.now().strftime('%Y%m%d_%H%M%S')}"
            shutil.copy2(args.input, backup_name)
            print(f"Backup criado: {backup_name}")

        output_file = args.input if args.replace else args.output

        print(f"\nConvertendo {args.input}...")
        result = convert_items_xml(args.input, output_file, mapping)

        print(f"\n{'='*60}")
        print("RESULTADO DA CONVERSÃO")
        print(f"{'='*60}")
        print(f"Items processados: {result['stats']['items_converted']}")
        print(f"IDs alterados: {result['stats']['ids_changed']}")
        print(f"Referências de atributos convertidas: {result['stats']['attribute_refs_converted']}")

        if result['unmapped']:
            print(f"\n⚠️  IDs não mapeados (mantidos originais): {len(result['unmapped'])}")
            if len(result['unmapped']) <= 20:
                print(f"   {result['unmapped']}")
            else:
                print(f"   Primeiros 20: {result['unmapped'][:20]}")

        print(f"\nArquivo salvo em: {output_file}")

        # Converter scripts Lua se especificado
        if args.lua_dir and args.lua_out:
            print(f"\nConvertendo scripts Lua em {args.lua_dir}...")
            lua_result = convert_lua_scripts(args.lua_dir, args.lua_out, mapping)
            print(f"  Arquivos processados: {lua_result['stats']['files_processed']}")
            print(f"  Arquivos modificados: {lua_result['stats']['files_modified']}")
            print(f"  IDs alterados: {lua_result['stats']['ids_changed']}")

    except FileNotFoundError as e:
        print(f"Erro: Arquivo não encontrado: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Erro: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
