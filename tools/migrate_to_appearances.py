#!/usr/bin/env python3
"""
Migration Script - Complete migration from OTB to Appearances/Protobuf
This script orchestrates the entire migration process
"""

import subprocess
import sys
import os
import shutil
from pathlib import Path
from datetime import datetime
import argparse


def run_command(cmd: list, description: str) -> bool:
    """Run a command and return success status"""
    print(f"\n{'=' * 60}")
    print(f"[STEP] {description}")
    print(f"{'=' * 60}")
    print(f"Command: {' '.join(cmd)}")
    print()

    result = subprocess.run(cmd, capture_output=False)
    return result.returncode == 0


def main():
    parser = argparse.ArgumentParser(
        description='Complete migration from OTB to Appearances/Protobuf',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Full migration with all steps
  python3 migrate_to_appearances.py --all

  # Only convert items.xml
  python3 migrate_to_appearances.py --items-xml

  # Only convert maps
  python3 migrate_to_appearances.py --maps

  # Only convert Lua scripts
  python3 migrate_to_appearances.py --lua

  # Dry run to see what would be changed
  python3 migrate_to_appearances.py --all --dry-run
"""
    )

    parser.add_argument('--all', action='store_true',
                       help='Run complete migration (recommended)')
    parser.add_argument('--extract-mapping', action='store_true',
                       help='Extract serverId->clientId mapping from items.otb')
    parser.add_argument('--items-xml', action='store_true',
                       help='Convert items.xml')
    parser.add_argument('--maps', action='store_true',
                       help='Convert .otbm map files')
    parser.add_argument('--lua', action='store_true',
                       help='Convert Lua scripts')
    parser.add_argument('--dry-run', action='store_true',
                       help='Show what would be done without making changes')
    parser.add_argument('--no-backup', action='store_true',
                       help='Skip creating backups (not recommended)')

    args = parser.parse_args()

    # If no specific step selected, show help
    if not any([args.all, args.extract_mapping, args.items_xml, args.maps, args.lua]):
        parser.print_help()
        sys.exit(0)

    tools_dir = Path(__file__).parent
    project_dir = tools_dir.parent

    # Change to project directory
    os.chdir(project_dir)

    print("=" * 60)
    print("OTB to APPEARANCES MIGRATION TOOL")
    print("=" * 60)
    print(f"Project directory: {project_dir}")
    print(f"Dry run: {args.dry_run}")
    print()

    if args.dry_run:
        print("⚠️  DRY RUN MODE - No files will be modified")
        print()

    backup_suffix = datetime.now().strftime('%Y%m%d_%H%M%S')

    # Step 1: Extract mapping from items.otb
    if args.all or args.extract_mapping:
        if not args.dry_run:
            success = run_command([
                'python3', 'tools/otb_extractor.py',
                'data/items/items.otb',
                '-o', 'tools/server_to_client_map.json',
                '-f', 'mapping',
                '--stats'
            ], "Extracting serverId->clientId mapping from items.otb")

            if not success:
                print("❌ Failed to extract mapping")
                sys.exit(1)
        else:
            print("[DRY RUN] Would extract mapping from items.otb")

    # Verify mapping exists
    mapping_file = project_dir / 'tools' / 'server_to_client_map.json'
    if not mapping_file.exists() and not args.dry_run:
        print(f"❌ Mapping file not found: {mapping_file}")
        print("   Run with --extract-mapping first")
        sys.exit(1)

    # Step 2: Convert items.xml
    if args.all or args.items_xml:
        if not args.dry_run:
            # Backup
            if not args.no_backup:
                backup_path = f"data/items/items.xml.backup.{backup_suffix}"
                shutil.copy2('data/items/items.xml', backup_path)
                print(f"Backup created: {backup_path}")

            success = run_command([
                'python3', 'tools/convert_items_xml.py',
                '--input', 'data/items/items.xml',
                '--output', 'data/items/items.xml',
                '--mapping', 'tools/server_to_client_map.json',
                '--replace'
            ], "Converting items.xml")

            if not success:
                print("❌ Failed to convert items.xml")
                sys.exit(1)
        else:
            print("[DRY RUN] Would convert items.xml")

    # Step 3: Convert maps
    if args.all or args.maps:
        map_dir = project_dir / 'data' / 'world'
        otbm_files = list(map_dir.glob('*.otbm'))

        if not otbm_files:
            print("No .otbm files found in data/world/")
        else:
            for otbm_file in otbm_files:
                if not args.dry_run:
                    # Backup
                    if not args.no_backup:
                        backup_path = f"{otbm_file}.backup.{backup_suffix}"
                        shutil.copy2(otbm_file, backup_path)
                        print(f"Backup created: {backup_path}")

                    success = run_command([
                        'python3', 'tools/convert_otbm.py',
                        str(otbm_file),
                        '--replace',
                        '--mapping', 'tools/server_to_client_map.json'
                    ], f"Converting map: {otbm_file.name}")

                    if not success:
                        print(f"❌ Failed to convert {otbm_file}")
                        sys.exit(1)
                else:
                    print(f"[DRY RUN] Would convert {otbm_file}")

    # Step 4: Convert Lua scripts
    if args.all or args.lua:
        lua_dirs = ['data/scripts', 'data/lib', 'data/actions', 'data/movements',
                   'data/creaturescripts', 'data/globalevents', 'data/spells',
                   'data/talkactions', 'data/npc', 'data/monster']

        for lua_dir in lua_dirs:
            lua_path = project_dir / lua_dir
            if lua_path.exists():
                if not args.dry_run:
                    # Backup
                    if not args.no_backup:
                        backup_path = f"{lua_dir}.backup.{backup_suffix}"
                        if (project_dir / backup_path).exists():
                            shutil.rmtree(project_dir / backup_path)
                        shutil.copytree(lua_path, project_dir / backup_path)
                        print(f"Backup created: {backup_path}")

                    success = run_command([
                        'python3', 'tools/convert_lua_scripts.py',
                        lua_dir,
                        '--replace',
                        '--mapping', 'tools/server_to_client_map.json'
                    ], f"Converting Lua scripts: {lua_dir}")

                    if not success:
                        print(f"⚠️  Warning: Some issues converting {lua_dir}")
                else:
                    print(f"[DRY RUN] Would convert Lua scripts in {lua_dir}")

    print("\n" + "=" * 60)
    print("MIGRATION COMPLETE")
    print("=" * 60)
    print()
    print("Next steps:")
    print("1. Review the changes and test thoroughly")
    print("2. Obtain appearances.dat file from Tibia Client 12+")
    print("3. Place it in data/items/appearances.dat")
    print("4. Enable in config.lua: useAppearances = true")
    print("5. Rebuild the server with protobuf support")
    print()
    print("⚠️  Important notes:")
    print("   - Database player items may need migration")
    print("   - Custom scripts should be reviewed manually")
    print("   - Test on a copy of your server first!")


if __name__ == '__main__':
    main()
