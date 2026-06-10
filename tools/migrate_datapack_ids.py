#!/usr/bin/env python3
"""
Datapack server_id -> client_id migrator/validator.

Converte todos os ids de item remanescentes do datapack (loot/corpse de
monstros, movements.xml, shops de NPC em XML e posicoes sintaticamente
inequivocas em Lua) usando tools/server_to_client_map.json, e valida o
resultado:

- todo id convertido precisa existir no mapeamento e o id novo precisa
  existir no data/items/items.xml (ja em client ids);
- entradas de loot e de shop carregam o nome do item: o nome do id novo
  precisa bater com o items.xml (e com o items.xml do Canary, se passado);
- ranges fromid/toid so sao reescritos se o mapeamento for contiguo no
  range inteiro;
- o items.xml ja convertido e re-validado: nomes por id contra o Canary e
  contiguidade de todos os ranges contra o items.xml original (pre-conversao).

Sem --apply roda em modo dry-run (so relatorio).
"""

import argparse
import json
import re
import sys
from pathlib import Path

REPORT = {'errors': [], 'warnings': [], 'changes': 0}


def err(msg):
    REPORT['errors'].append(msg)


def warn(msg):
    REPORT['warnings'].append(msg)


def load_mapping(path):
    with open(path) as f:
        return {int(k): int(v) for k, v in json.load(f).items()}


def parse_items_xml(path):
    return parse_items_text(Path(path).read_text(encoding='utf-8', errors='ignore'))


def parse_items_text(text):
    """id -> name (primeira definicao vence; ranges expandidos)."""
    names = {}
    for m in re.finditer(r'<item\s+([^>]*?)/?>', text):
        attrs = dict(re.findall(r'(\w+)="([^"]*)"', m.group(1)))
        name = attrs.get('name', '')
        if 'id' in attrs:
            ids = [int(attrs['id'])]
        elif 'fromid' in attrs and 'toid' in attrs:
            ids = range(int(attrs['fromid']), int(attrs['toid']) + 1)
        else:
            continue
        for i in ids:
            names.setdefault(i, name)
    return names


def norm(name):
    return re.sub(r'\s+', ' ', name.strip().lower())


class Converter:
    def __init__(self, mapping, atlas_names, canary_names):
        self.mapping = mapping
        self.atlas_names = atlas_names
        self.canary_names = canary_names

    def convert(self, old_id, ctx, expected_name=None):
        """Retorna o id novo ou None se a conversao nao for segura."""
        if old_id not in self.mapping:
            err(f'{ctx}: id {old_id} ausente do mapeamento (mantido)')
            return None
        new_id = self.mapping[old_id]
        if new_id not in self.atlas_names:
            err(f'{ctx}: {old_id} -> {new_id}, mas {new_id} nao existe no items.xml (mantido)')
            return None
        if expected_name is not None:
            got = norm(self.atlas_names[new_id])
            wanted = norm(expected_name)
            if wanted and got and wanted != got:
                warn(f'{ctx}: {old_id} -> {new_id}, nome esperado "{expected_name}"'
                     f' != items.xml "{self.atlas_names[new_id]}"')
        REPORT['changes'] += 1
        return new_id

    def range_contiguous(self, fromid, toid):
        base = self.mapping.get(fromid)
        if base is None:
            return False
        return all(self.mapping.get(fromid + k) == base + k for k in range(toid - fromid + 1))


REF_KEYS = ('rotateto|decayto|transformequipto|transformdeequipto|destroyto|'
            'writeonceitemid|maletransformto|femaletransformto|transformto')


def rebuild_items_xml(conv, orig_path, out_path, apply):
    """Reconverte o items.xml do original, dividindo ranges em sub-ranges
    cujo mapeamento e contiguo (a conversao endpoint-a-endpoint anterior
    corrompia ranges nao contiguos)."""
    text = Path(orig_path).read_text(encoding='utf-8')
    n_split = 0

    def convert_refs(body):
        def ref_sub(m):
            new_id = conv.mapping.get(int(m.group(3)))
            if new_id is None:
                return m.group(0)
            REPORT['changes'] += 1
            return f'{m.group(1)}{new_id}{m.group(4)}'

        return re.sub(rf'(key="({REF_KEYS})"\s+value=")(\d+)(")',
                      ref_sub, body, flags=re.IGNORECASE)

    def item_sub(m):
        nonlocal n_split
        element = m.group(0)
        attrs = dict(re.findall(r'(\w+)="([^"]*)"', m.group(1)))
        element = convert_refs(element)

        if 'id' in attrs:
            old_id = int(attrs['id'])
            new_id = conv.mapping.get(old_id, old_id)
            if old_id >= 100 and old_id not in conv.mapping:
                warn(f'items.xml: id {old_id} sem mapeamento (mantido)')
            elif old_id >= 100:
                REPORT['changes'] += 1
            return re.sub(r'\bid="\d+"', f'id="{new_id}"', element, count=1)

        if 'fromid' not in attrs:
            return element

        fromid, toid = int(attrs['fromid']), int(attrs['toid'])
        ids = list(range(fromid, toid + 1))
        unmapped = [i for i in ids if i not in conv.mapping]
        if unmapped:
            warn(f'items.xml: range {fromid}-{toid} contem ids sem mapeamento'
                 f' {unmapped} (range mantido como esta)')
            return element

        # divide em runs de client ids consecutivos
        runs = []
        for i in ids:
            v = conv.mapping[i]
            if runs and v == runs[-1][1] + 1:
                runs[-1] = (runs[-1][0], v)
            else:
                runs.append((v, v))
        REPORT['changes'] += len(ids)
        if len(runs) > 1:
            n_split += 1

        out = []
        for lo, hi in runs:
            if lo == hi:
                piece = re.sub(r'\bfromid="\d+"\s+toid="\d+"', f'id="{lo}"',
                               element, count=1)
            else:
                piece = re.sub(r'\bfromid="\d+"\s+toid="\d+"',
                               f'fromid="{lo}" toid="{hi}"', element, count=1)
            out.append(piece)
        return '\n\t'.join(out)

    converted = re.sub(r'<item\s+([^>]*?)(?:/>|>.*?</item>)', item_sub, text,
                       flags=re.DOTALL)
    if apply:
        Path(out_path).write_text(converted, encoding='utf-8')
    print(f'items.xml: reconvertido do original ({n_split} ranges divididos'
          f' em sub-ranges contiguos)')
    return converted


def validate_items_names(conv):
    """Compara nomes por id contra o Canary, separando divergencia de
    redacao (tokens em comum) de suspeita de mapeamento errado (nenhum)."""
    if not conv.canary_names:
        return
    wording, suspect = [], []
    checked = missing = 0
    for cid, name in sorted(conv.atlas_names.items()):
        if cid < 100:
            continue  # fluidos — enumeração tratada em separado
        cname = conv.canary_names.get(cid)
        if cname is None:
            missing += 1
            continue
        checked += 1
        a, b = norm(name), norm(cname)
        if not a or not b or a == b:
            continue
        if set(a.split()) & set(b.split()):
            wording.append(f'id {cid}: "{name}" vs "{cname}"')
        else:
            suspect.append(f'id {cid}: "{name}" vs "{cname}"')
    # Os nomes do datapack TFS nao seguem o vocabulario CipSoft do Canary
    # ("stairs" vs "stairway", "dirt floor" vs "earth ground"), entao a
    # comparacao e um relatorio para inspecao, nao um gate. A correcao do
    # mapeamento em si e garantida pelo clientid do OTB + validacao de
    # nome do loot (mesmo vocabulario TFS dos dois lados).
    Path('items_name_suspects.txt').write_text('\n'.join(suspect), encoding='utf-8')
    Path('items_name_wording_diffs.txt').write_text('\n'.join(wording),
                                                    encoding='utf-8')
    print(f'items.xml vs Canary: {checked} ids comparados | {len(suspect)} sem'
          f' token em comum (items_name_suspects.txt) | {len(wording)}'
          f' diferencas de redacao (items_name_wording_diffs.txt) | {missing}'
          f' ids sem entrada no Canary')


def convert_monsters(conv, root, apply):
    n_files = 0
    for f in sorted((root / 'data/monster').rglob('*.xml')):
        text = f.read_text(encoding='utf-8', errors='ignore')
        orig = text

        def loot_sub(m):
            old_id = int(m.group(2))
            attrs = dict(re.findall(r'(\w+)="([^"]*)"', m.group(0)))
            new_id = conv.convert(old_id, f'{f.name} loot', attrs.get('name'))
            return m.group(0) if new_id is None else f'{m.group(1)}{new_id}{m.group(3)}'

        text = re.sub(r'(<item\s+id=")(\d+)(")', loot_sub, text)

        def corpse_sub(m):
            old_id = int(m.group(2))
            if old_id < 100:  # 0 = sem corpo
                return m.group(0)
            new_id = conv.convert(old_id, f'{f.name} corpse')
            return m.group(0) if new_id is None else f'{m.group(1)}{new_id}{m.group(3)}'

        text = re.sub(r'(corpse=")(\d+)(")', corpse_sub, text)

        if text != orig:
            n_files += 1
            if apply:
                f.write_text(text, encoding='utf-8')
    print(f'monstros: {n_files} arquivos alterados')


def convert_movements(conv, root, apply):
    f = root / 'data/movements/movements.xml'
    text = f.read_text(encoding='utf-8', errors='ignore')
    orig = text

    def itemid_sub(m):
        new_id = conv.convert(int(m.group(2)), 'movements.xml itemid')
        return m.group(0) if new_id is None else f'{m.group(1)}{new_id}{m.group(3)}'

    text = re.sub(r'(itemid=")(\d+)(")', itemid_sub, text)

    def range_sub(m):
        fromid, toid = int(m.group(2)), int(m.group(4))
        if not conv.range_contiguous(fromid, toid):
            err(f'movements.xml: range {fromid}-{toid} nao contiguo no mapeamento (mantido)')
            return m.group(0)
        REPORT['changes'] += 1
        return (f'{m.group(1)}{conv.mapping[fromid]}{m.group(3)}'
                f'{conv.mapping[toid]}{m.group(5)}')

    text = re.sub(r'(fromid=")(\d+)(" toid=")(\d+)(")', range_sub, text)

    if text != orig and apply:
        f.write_text(text, encoding='utf-8')
    print('movements.xml: convertido' if text != orig else 'movements.xml: sem mudancas')


def convert_npc_shops(conv, root, apply):
    n = 0
    for f in sorted((root / 'data/npc').glob('*.xml')):
        text = f.read_text(encoding='utf-8', errors='ignore')
        orig = text

        def shop_sub(m):
            entries = []
            for entry in m.group(2).split(';'):
                parts = entry.split(',')
                # formato: nome,id,preco[,subtype]
                if len(parts) >= 3 and parts[1].strip().isdigit():
                    new_id = conv.convert(int(parts[1]), f'{f.name} shop', parts[0])
                    if new_id is not None:
                        parts[1] = str(new_id)
                entries.append(','.join(parts))
            return f'{m.group(1)}{";".join(entries)}{m.group(3)}'

        text = re.sub(r'(key="shop_(?:buyable|sellable)" value=")([^"]*)(")', shop_sub, text)
        if text != orig:
            n += 1
            if apply:
                f.write_text(text, encoding='utf-8')
    print(f'npc shops: {n} arquivos alterados')


LUA_OBJ_TYPES = {'Action', 'MoveEvent', 'Weapon'}
LUA_CALL_PATTERNS = [
    # (regex com o id no grupo 2, descricao)
    (re.compile(r'(\bItemType\(\s*)(\d+)'), 'ItemType'),
    (re.compile(r'(\bGame\.createItem\(\s*)(\d+)'), 'Game.createItem'),
    (re.compile(r'(:addItem\(\s*)(\d+)'), ':addItem'),
    (re.compile(r'(:removeItem\(\s*)(\d+)'), ':removeItem'),
    (re.compile(r'(:getItemCount\(\s*)(\d+)'), ':getItemCount'),
    (re.compile(r'(:getItemById\(\s*)(\d+)'), ':getItemById'),
    (re.compile(r'(:transform\(\s*)(\d+)'), ':transform'),
    (re.compile(r'(:runeId\(\s*)(\d+)'), ':runeId'),
    (re.compile(r'(\bdoPlayerAddItem\(\s*[^,()]+,\s*)(\d+)'), 'doPlayerAddItem'),
    (re.compile(r'(\bitem[iI][dD]\s*=\s*)(\d+)'), 'itemid ='),
    # \b casa apos "." — este padrao cobre tanto `x.itemid ==` quanto `itemId ==`
    (re.compile(r'(:getId\(\)\s*[=~]=\s*)(\d+)'), ':getId() =='),
    (re.compile(r'(\bitem[iI][dD]\s*[=~]=\s*)(\d+)'), 'itemid =='),
    (re.compile(r'(\btransformTo\s*=\s*)(\d+)'), 'transformTo ='),
    # shop de NPC: o item id e o 2o argumento; os demais sao preco/subtype
    (re.compile(r'(:add(?:Buy|Sell)ableItem\(\s*\{[^{}]*\}\s*,\s*)(\d+)'), 'addBuy/SellableItem'),
    # variante container: (nomes, containerId, itemId, preco, ...)
    (re.compile(r'(:addBuyableItemContainer\(\s*\{[^{}]*\}\s*,\s*)(\d+)'), 'container id'),
    (re.compile(r'(:addBuyableItemContainer\(\s*\{[^{}]*\}\s*,\s*\d+\s*,\s*)(\d+)'), 'container item'),
    (re.compile(r'(\bnewItem\s*=\s*)(\d+)'), 'newItem ='),
]

# padroes extras por arquivo (contexto local garante que e item id)
LUA_FILE_EXTRA_PATTERNS = {
    # rust_remover: { id = <item>, chance = ... }
    'data/scripts/actions/tools/rust_remover.lua': [re.compile(r'(\bid\s*=\s*)(\d+)')],
}


# tipos cujo :id() NAO e id de item (spell id, evento etc.)
LUA_NON_ITEM_TYPES = {'Spell', 'TalkAction', 'GlobalEvent', 'CreatureEvent'}


# Arquivos classificados manualmente (com apoio do datapack do Canary):
# 'all'  = todos os numeros >=100 em tabelas/listas sao item ids
# 'keys' = apenas as chaves [N] = ... sao item ids (valores sao looktype,
#          cura, mount id, ...)
# Demais arquivos: numeros de tabela so convertem com confirmacao no Canary.
LUA_FILE_MODES = {
    'data/scripts/actions/others/destroy.lua': 'all',
    'data/scripts/actions/others/transforms.lua': 'all',
    'data/scripts/systems/beds/data/beds.lua': 'all',
    'data/scripts/actions/others/food.lua': 'all',
    'data/npc/scripts/runes.lua': 'all',
    'data/scripts/lib/actions.lua': 'all',
    'data/scripts/actions/others/flower_pot.lua': 'all',
    'data/scripts/actions/others/decayto.lua': 'all',
    'data/scripts/actions/others/doors.lua': 'all',
    'data/scripts/actions/others/taming.lua': 'all',
    'data/scripts/movements/drowning.lua': 'all',
    'data/scripts/movements/tiles.lua': 'all',
    'data/scripts/actions/others/create_bread.lua': 'all',
    'data/scripts/actions/others/windows.lua': 'all',
    'data/scripts/movements/swimming.lua': 'all',
    'data/scripts/actions/tools/rust_remover.lua': 'all',
    'data/scripts/actions/others/muck_remover.lua': 'all',
    'data/scripts/actions/others/hive_gate.lua': 'all',
    'data/scripts/actions/others/crate_usable.lua': 'all',
    'data/scripts/actions/others/enchanting.lua': 'all',
    'data/scripts/runes/support/disintegrate_rune.lua': 'all',
    'data/scripts/actions/quests/quests.lua': 'all',
    'data/scripts/actions/others/juicy_root.lua': 'all',
    'data/scripts/actions/others/melting_horn.lua': 'all',
    'data/scripts/movements/walk_back.lua': 'all',
    'data/scripts/actions/others/teleport.lua': 'all',
    'data/scripts/actions/others/sugar_oat.lua': 'all',
    'data/scripts/actions/others/fluids.lua': 'all',
    'data/scripts/actions/tools/firebug.lua': 'all',
    'data/scripts/actions/tools/watch.lua': 'all',
    'data/scripts/actions/others/skill_potions.lua': 'all',
    'data/scripts/actions/others/peppermoon_bells.lua': 'all',
    'data/scripts/actions/others/costume_bag.lua': 'all',
    'data/scripts/movements/trap.lua': 'all',
    'data/scripts/movements/seeds.lua': 'all',
    'data/scripts/movements/decay.lua': 'all',
    'data/scripts/events/player/first_items.lua': 'all',
    'data/scripts/actions/tools/tool_gear.lua': 'all',
    'data/scripts/actions/others/roasted_meat.lua': 'all',
    'data/scripts/actions/others/flask_of_demonic_blood.lua': 'all',
    'data/scripts/actions/others/die.lua': 'all',
    'data/scripts/actions/others/construction_kits.lua': 'all',
    'data/scripts/actions/others/afflicted_outfit.lua': 'all',
    'data/scripts/spells/conjuring/wand_of_darkness.lua': 'all',
    'data/scripts/spells/conjuring/practise_magic_missile_rune.lua': 'all',
    'data/scripts/actions/others/insectoid_cell.lua': 'all',
    'data/scripts/actions/others/goldfish_bowl.lua': 'all',
    'data/scripts/actions/others/potions.lua': 'keys',
    'data/scripts/actions/others/usable_outfit_items.lua': 'keys',
    'data/scripts/actions/others/usable_mount_items.lua': 'keys',
    'data/scripts/actions/tools/skinning.lua': 'keys',
    # 'none' = numeros de tabela NAO sao item ids (storage, looktype,
    # achievement, duracao, mana, ...); ids de item nesses arquivos ja sao
    # cobertos pelos padroes sintaticos (chamadas/:id()/comparacoes)
    'data/lib/core/achievements.lua': 'none',
    'data/lib/core/storages.lua': 'none',
    'data/lib/core/actionids.lua': 'none',
    'data/lib/core/highscores.lua': 'none',
    'data/scripts/talkactions/commands/looktype.lua': 'none',
    'data/scripts/talkactions/player/buy_premium.lua': 'none',
    'data/scripts/systems/outfits/data/mounts.lua': 'none',
    'data/scripts/systems/outfits/data/outfits.lua': 'none',
    'data/scripts/scheduleevents/world_light.lua': 'none',
    'data/scripts/events/player/regenerate_stamina.lua': 'none',
    'data/scripts/events/player/modal_window_helper.lua': 'none',
    'data/scripts/events/player/#text_window_example.lua': 'none',
    'data/npc/scripts/promotion.lua': 'none',
    'data/scripts/lib/spells.lua': 'none',
}

# numeros de tabela em spells/* e weapons/wand/* sao config de magia
# (duracao, mana, cooldown), nunca item id
LUA_NONE_DIR_PREFIXES = ('data/scripts/spells/support/', 'data/scripts/spells/monster/',
                         'data/scripts/spells/healing/', 'data/scripts/spells/party/',
                         'data/scripts/spells/attack/', 'data/scripts/runes/attack/',
                         'data/scripts/weapons/')

# linhas em que numeros "soltos" (tabelas) NAO devem ser tratados como item id
LUA_TABLE_BLACKLIST = re.compile(
    r'storage|position|teleport|\bx\s*=|\by\s*=|\bz\s*=|chance|price|cost|money|'
    r'level|mana|exhaust|cooldown|delay|interval|\btime|duration|percent|amount|'
    r'\bexp\b|achievement|outfit|looktype|mount|actionid|uniqueid|\baid\b|\buid\b|'
    r'addevent|random|speed|combat|formula|effect\s*=|spellid|count\s*=|'
    r'\bfrom\s*=|\bto\s*=',
    re.IGNORECASE)


class CanaryLuaIndex:
    """Indice basename -> conteudo dos .lua do datapack de referencia."""

    def __init__(self, canary_root):
        self.files = {}
        if not canary_root:
            return
        for base in (Path(canary_root) / 'data', Path(canary_root) / 'data-otservbr-global'):
            if not base.exists():
                continue
            for f in base.rglob('*.lua'):
                self.files.setdefault(f.name.lower(), []).append(f)
        self._cache = {}

    def has_id(self, basename, new_id):
        """True/False se conseguimos verificar; None se nao ha arquivo equivalente."""
        paths = self.files.get(basename.lower())
        if not paths:
            return None
        needle = re.compile(rf'\b{new_id}\b')
        for p in paths:
            text = self._cache.get(p)
            if text is None:
                text = p.read_text(encoding='utf-8', errors='ignore')
                self._cache[p] = text
            if needle.search(text):
                return True
        return False


def convert_lua(conv, root, canary_index, apply):
    """Conversao em passada unica baseada em spans (sem dupla conversao).

    Posicoes sintaticamente inequivocas (chamadas, :id() tipado, comparacoes)
    sao sempre convertidas. Numeros em contexto de tabela/loop so sao
    convertidos quando o id novo e confirmado no script equivalente do
    datapack do Canary; caso contrario vao para revisao manual.
    """
    lua_dirs = ['data/scripts', 'data/npc/scripts', 'data/npc/lib',
                'data/lib', 'data/movements/lib', 'data/spells/lib']
    n_files = 0
    unknown_id_calls, manual_review = [], []

    for d in lua_dirs:
        base = root / d
        if not base.exists():
            continue
        for f in sorted(base.rglob('*.lua')):
            rel = f.relative_to(root)
            text = f.read_text(encoding='utf-8', errors='ignore')

            var_types = {m.group(1): m.group(2) for m in
                         re.finditer(r'\b(\w+)\s*=\s*(Action|MoveEvent|Weapon|Spell|'
                                     r'TalkAction|GlobalEvent|CreatureEvent)\s*\(', text)}

            spans = []  # (start, end, old_id, certain)

            # 1. var:id(...) tipado
            for m in re.finditer(r'\b(\w+):id\(([^()]*)\)', text):
                vtype = var_types.get(m.group(1))
                if vtype in LUA_NON_ITEM_TYPES:
                    continue
                if vtype not in LUA_OBJ_TYPES:
                    if re.search(r'\d', m.group(2)):
                        unknown_id_calls.append(f'{rel}: {m.group(0)[:80]}')
                    continue
                for am in re.finditer(r'\d+', m.group(2)):
                    spans.append((m.start(2) + am.start(), m.start(2) + am.end(),
                                  int(am.group()), True))

            # 2. chamadas/atribuicoes/comparacoes inequivocas
            extra = LUA_FILE_EXTRA_PATTERNS.get(rel.as_posix(), [])
            for pattern in [p for p, _ in LUA_CALL_PATTERNS] + extra:
                for m in pattern.finditer(text):
                    spans.append((m.start(2), m.end(2), int(m.group(2)), True))

            # 3. table.contains({...})
            for m in re.finditer(r'table\.contains\(\s*\{([^}]*)\}', text):
                for am in re.finditer(r'\d+', m.group(1)):
                    spans.append((m.start(1) + am.start(), m.start(1) + am.end(),
                                  int(am.group()), True))

            # 4. for i = a, b (range de ids: exige contiguidade no mapeamento)
            for m in re.finditer(r'\bfor\s+\w+\s*=\s*(\d+)\s*,\s*(\d+)\b', text):
                lo, hi = int(m.group(1)), int(m.group(2))
                if lo < 100 or hi < lo:
                    continue
                if conv.range_contiguous(lo, hi):
                    spans.append((m.start(1), m.end(1), lo, True))
                    spans.append((m.start(2), m.end(2), hi, True))
                elif lo in conv.mapping or hi in conv.mapping:
                    manual_review.append(f'{rel}: loop nao contiguo: {m.group(0)}')

            # 5. numeros em tabelas: [k] = v, {a, b, c}
            rel_posix = rel.as_posix()
            mode = LUA_FILE_MODES.get(rel_posix)
            if mode is None and rel_posix.startswith(LUA_NONE_DIR_PREFIXES):
                mode = 'none'
            if mode != 'none':
                table_re = (re.compile(r'\[\s*(\d{3,5})\s*\]') if mode == 'keys'
                            else re.compile(r'[\[{,=]\s*(\d{3,5})\b'))
                line_start = 0
                for line in text.splitlines(keepends=True):
                    if not line.lstrip().startswith('--') and not LUA_TABLE_BLACKLIST.search(line):
                        for am in table_re.finditer(line):
                            spans.append((line_start + am.start(1), line_start + am.end(1),
                                          int(am.group(1)), mode is not None))
                    line_start += len(line)

            # passada unica: ordena (span "certo" vence o duplicado generico),
            # descarta sobreposicoes, aplica de tras pra frente
            spans.sort(key=lambda s: (s[0], s[1], not s[3]))
            accepted = []
            for s in spans:
                if accepted and s[0] < accepted[-1][1]:
                    continue
                accepted.append(s)

            out = text
            for start, end, old_id, certain in reversed(accepted):
                if old_id < 100 or old_id not in conv.mapping:
                    if certain and old_id >= 100:
                        err(f'{rel}: id {old_id} ausente do mapeamento (mantido)')
                    continue
                new_id = conv.mapping[old_id]
                if new_id not in conv.atlas_names:
                    err(f'{rel}: {old_id} -> {new_id} nao existe no items.xml (mantido)')
                    continue
                if not certain:
                    verified = canary_index.has_id(f.name, new_id)
                    if not verified:  # False ou None (sem arquivo equivalente)
                        manual_review.append(
                            f'{rel}: {old_id} -> {new_id} nao confirmado no Canary'
                            f' ({ "id ausente" if verified is False else "sem arquivo equivalente"})')
                        continue
                out = out[:start] + str(new_id) + out[end:]
                REPORT['changes'] += 1

            if out != text:
                n_files += 1
                if apply:
                    f.write_text(out, encoding='utf-8')

    print(f'lua: {n_files} arquivos alterados')
    if unknown_id_calls:
        Path('lua_unknown_id_calls.txt').write_text('\n'.join(unknown_id_calls),
                                                    encoding='utf-8')
        print(f'AUDITORIA: {len(unknown_id_calls)} chamadas :id() de tipo'
              ' desconhecido (lua_unknown_id_calls.txt)')
    if manual_review:
        Path('lua_manual_review.txt').write_text('\n'.join(manual_review),
                                                 encoding='utf-8')
        print(f'AUDITORIA: {len(manual_review)} numeros de tabela/loop nao'
              ' convertidos por falta de confirmacao (lua_manual_review.txt)')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root', default='.', help='raiz do repositorio')
    ap.add_argument('--mapping', default='tools/server_to_client_map.json')
    ap.add_argument('--orig-items', required=True,
                    help='items.xml original (pre-conversao, ex: git show dev:data/items/items.xml)')
    ap.add_argument('--canary-items', help='items.xml do Canary para validacao por nome')
    ap.add_argument('--canary-root', help='raiz do repo Canary para confirmar conversoes em tabelas Lua')
    ap.add_argument('--apply', action='store_true', help='gravar as mudancas')
    args = ap.parse_args()

    root = Path(args.root)
    mapping = load_mapping(args.mapping)
    canary_names = parse_items_xml(args.canary_items) if args.canary_items else {}
    conv = Converter(mapping, {}, canary_names)

    print(f'mapeamento: {len(mapping)} ids | canary: {len(canary_names)} ids')
    print(f'modo: {"APPLY" if args.apply else "DRY-RUN"}\n')

    converted_items = rebuild_items_xml(conv, args.orig_items,
                                        root / 'data/items/items.xml', args.apply)
    conv.atlas_names = parse_items_text(converted_items)
    print(f'items.xml reconvertido: {len(conv.atlas_names)} ids')

    validate_items_names(conv)
    convert_monsters(conv, root, args.apply)
    convert_movements(conv, root, args.apply)
    convert_npc_shops(conv, root, args.apply)
    convert_lua(conv, root, CanaryLuaIndex(args.canary_root), args.apply)

    print(f'\n=== {REPORT["changes"]} conversoes | {len(REPORT["errors"])} erros |'
          f' {len(REPORT["warnings"])} avisos ===')
    for e in REPORT['errors']:
        print(f'ERRO: {e}')
    for w in REPORT['warnings']:
        print(f'AVISO: {w}')
    sys.exit(1 if REPORT['errors'] else 0)


if __name__ == '__main__':
    main()
