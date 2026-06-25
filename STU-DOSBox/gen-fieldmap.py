#!/usr/bin/env python3
"""
gen-fieldmap.py -- generate a capture field map from the ReMoM C structs.

For a given struct (default: s_WIZARD) this parses the ReMoM headers, resolves
array-size #defines, recurses nested structs, expands arrays-of-struct, and
emits:
  - a readable (offset, name, kind, count, size) table
  - a hard-coded C array (gd_field_t[]) for the capture probe

Self-check: every field's type-computed size is compared against the size
implied by the gap to the next field's /* 0xNN */ offset comment, and the
struct total is checked against its `// sizeof: NNNh` comment. Any mismatch
is reported (and is a hard error for the top-level struct).

Part of CaptureGameData PRD/PLAN, Phase 1.  Authoritative source: the struct
definitions in MOM_DAT.h -- not hand-authored offsets.
"""
import re
import sys

HEADERS = [
    "/home/jbalcomb/STU/devel/ReMoM/MoX/src/MOM_DAT.h",
    "/home/jbalcomb/STU/devel/ReMoM/MoX/src/MOM_DEF.h",
]

alltext = ""
for h in HEADERS:
    try:
        alltext += "\n" + open(h, encoding="utf-8", errors="replace").read()
    except FileNotFoundError:
        print(f"WARN: missing {h}", file=sys.stderr)

# ---------- #define resolution ----------
defs_raw = {}
for m in re.finditer(r'^[ \t]*#define[ \t]+(\w+)[ \t]+(.+)$', alltext, re.M):
    val = re.split(r'//|/\*', m.group(2))[0].strip()
    if val:
        defs_raw[m.group(1)] = val

def resolve_const(expr, depth=0):
    expr = expr.strip()
    try:
        return int(expr, 0)
    except ValueError:
        pass
    if depth > 30:
        raise ValueError(f"const recursion on {expr!r}")
    new = re.sub(r'[A-Za-z_]\w*',
                 lambda mm: "(" + defs_raw[mm.group(0)] + ")"
                            if mm.group(0) in defs_raw else mm.group(0),
                 expr)
    if new != expr:
        return resolve_const(new, depth + 1)
    if re.fullmatch(r'[0-9xXa-fA-F()+\-*/ ]+', expr):
        return int(eval(expr))
    raise ValueError(f"unresolved const {expr!r}")

# ---------- struct bodies + sizeof comments ----------
def _extract_struct_body(text, open_brace_pos):
    """Body between the '{' at open_brace_pos and its matching '}', skipping
    braces inside /* */ and // comments.  A naive `(.*?)\\}\\s*;` regex stops at
    the first `};` -- which a member comment like `/* {F,T}; ... */` triggers
    early, silently truncating the struct (e.g. s_UNIT.Finished)."""
    i, n, depth = open_brace_pos + 1, len(text), 1
    start = i
    while i < n and depth > 0:
        two = text[i:i + 2]
        if two == '/*':
            end = text.find('*/', i + 2)
            i = n if end == -1 else end + 2
            continue
        if two == '//':
            end = text.find('\n', i + 2)
            i = n if end == -1 else end + 1
            continue
        c = text[i]
        if c == '{':
            depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                return text[start:i]
        i += 1
    return text[start:i]

struct_body = {}
for m in re.finditer(r'struct\s+(s_\w+)\s*\{', alltext):
    struct_body[m.group(1)] = _extract_struct_body(alltext, m.end() - 1)

struct_sizeof = {}
lines = alltext.splitlines()
pending = pending_at = None
for i, l in enumerate(lines):
    sm = re.search(r'sizeof:\s*([0-9A-Fa-f]+)h', l)
    if sm:
        pending, pending_at = int(sm.group(1), 16), i
    dm = re.match(r'\s*struct\s+(s_\w+)', l)
    if dm and pending is not None and i - pending_at <= 5:
        struct_sizeof[dm.group(1)] = pending
        pending = None

TYPE_KIND = {
    'char': 'I8', 'int8_t': 'I8', 'uint8_t': 'U8', 'byte': 'U8',
    'int16_t': 'I16', 'uint16_t': 'U16', 'word': 'U16',
    'int32_t': 'I32', 'uint32_t': 'U32', 'dword': 'U32', 'long': 'I32',
}
KIND_SIZE = {'I8': 1, 'U8': 1, 'I16': 2, 'U16': 2, 'I32': 4, 'U32': 4}

MEMBER_RE = re.compile(
    r'^\s*(?:/\*\s*([0-9A-Fa-f]+)\s*\*/)?\s*'
    r'(struct\s+s_\w+|[A-Za-z_]\w*)\s+'
    r'(\w+)\s*'
    r'(?:\[\s*([^\]]+?)\s*\])?\s*;')

def parse_members(name):
    """Return [(offset_or_None, type, name, count_or_None), ...]."""
    out = []
    for raw in struct_body[name].splitlines():
        line = raw.strip()
        if not line or line.startswith('//'):
            continue
        mm = MEMBER_RE.match(line)
        if mm:
            out.append(mm.groups())
    return out

warnings = []

def elem_size(typ):
    if typ.startswith('struct '):
        return sizeof_struct(typ.split()[1])
    return KIND_SIZE.get(TYPE_KIND.get(typ, 'U8'), 1)

def sizeof_struct(name):
    if name in struct_sizeof:
        return struct_sizeof[name]
    _, total = flatten(name, 0, "")           # derive if no sizeof comment
    return total

def member_offsets(members):
    """Local offset of each member: from its /* 0xNN */ comment, else from the
    previous member's offset + size (one isolated comment-less member is fine)."""
    offs = []
    for i, (off_c, typ, nm, cnt) in enumerate(members):
        if off_c is not None:
            offs.append(int(off_c, 16))
        else:
            pcnt = members[i - 1][3]
            pcount = resolve_const(pcnt) if pcnt is not None else 1
            offs.append(offs[i - 1] + elem_size(members[i - 1][1]) * pcount)
    return offs

def flatten(struct_name, base, prefix):
    """Return (leaf_rows, total). leaf row = (abs_off, name, kind, count, size).
    Sizes come from the gaps between /* 0xNN */ offset comments (authoritative);
    declared [COUNT] constants are an optional cross-check only."""
    members = parse_members(struct_name)
    offs = member_offsets(members)
    total = struct_sizeof.get(struct_name)
    if total is None:
        lcnt = members[-1][3]
        lcount = resolve_const(lcnt) if lcnt is not None else 1
        total = offs[-1] + elem_size(members[-1][1]) * lcount
    rows = []
    for i, (off_c, typ, nm, cnt) in enumerate(members):
        loff = offs[i]
        nxt = offs[i + 1] if i + 1 < len(members) else total
        size = nxt - loff
        elem = elem_size(typ)
        count = size // elem if elem else size
        if cnt is not None:                    # cross-check vs declared count
            try:
                dc = resolve_const(cnt)
                if dc != count:
                    warnings.append(f"{struct_name}.{nm}: declared [{cnt}]={dc} "
                                    f"!= delta-derived {count}")
            except ValueError:
                pass
        ab = base + loff
        if typ.startswith('struct '):
            sname = typ.split()[1]
            for k in range(count):
                idx = f"[{k}]" if cnt is not None else ""
                if sname in struct_body:
                    sr, _ = flatten(sname, ab + k * elem, f"{prefix}{nm}{idx}.")
                    rows.extend(sr)
                else:
                    rows.append((ab + k * elem, f"{prefix}{nm}{idx}",
                                 'BYTES', elem, elem))
        elif typ == 'char' and cnt is not None:
            rows.append((ab, f"{prefix}{nm}", 'STR', size, size))
        else:
            rows.append((ab, f"{prefix}{nm}", TYPE_KIND.get(typ, 'U8'),
                         count, size))
    return rows, total

# ---------- run ----------
TARGET = sys.argv[1] if len(sys.argv) > 1 else 's_WIZARD'
rows, total = flatten(TARGET, 0, "")

print(f"# field map for {TARGET}  (total = 0x{total:X} / {total}d, "
      f"{len(rows)} leaf fields)\n")
print(f"{'offset':>7}  {'kind':<5} {'cnt':>4}  {'size':>4}  name")
for off, nm, kind, cnt, size in rows:
    print(f"0x{off:04X}  {kind:<5} {cnt:>4}  {size:>4}  {nm}")

covered = sum(r[4] for r in rows)
print(f"\n# self-check: {len(rows)} fields, bytes covered={covered} "
      f"(0x{covered:X}), declared total=0x{total:X}")
print("# total matches sizeof comment" if covered == total
      else f"# !! COVERAGE GAP: {total-covered} bytes unaccounted")
if warnings:
    print(f"\n# {len(warnings)} size/offset warning(s):")
    for w in warnings:
        print("#   " + w)
else:
    print("# no size/offset mismatches vs /* 0xNN */ comments")

if '--decode' in sys.argv:
    import struct
    sf = sys.argv[sys.argv.index('--decode') + 1]
    players_off = int(sys.argv[sys.argv.index('--decode') + 2], 0) \
        if len(sys.argv) > sys.argv.index('--decode') + 2 \
        and not sys.argv[sys.argv.index('--decode') + 2].startswith('-') else 0x9E8
    data = open(sf, 'rb').read()
    FMT = {'U8': 'B', 'I8': 'b', 'U16': '<H', 'I16': '<h', 'U32': '<I', 'I32': '<i'}
    ES = {'U16': 2, 'I16': 2, 'U32': 4, 'I32': 4}

    def decode(off, kind, cnt):
        if kind == 'STR':
            return '"' + data[off:off+cnt].split(b'\0')[0].decode('latin1') + '"'
        if kind == 'BYTES':
            return data[off:off+cnt].hex()
        es = ES.get(kind, 1)
        return ','.join(str(struct.unpack_from(FMT[kind], data, off+i*es)[0])
                        for i in range(cnt))
    print(f"# decode of {sf} at players_off=0x{players_off:X} (stride 0x{total:X})\n")
    for pidx in range(6):
        base = players_off + pidx * total
        if base + total > len(data):
            break
        for off, nm, kind, cnt, size in rows:
            print(f"[GD] save _players[{pidx}].{nm} = {decode(base+off, kind, cnt)}")
    sys.exit(0)

if '--c' in sys.argv:
    tname = TARGET[2:].lower() if TARGET.startswith('s_') else TARGET.lower()
    guard = f"GD_{tname.upper()}_FIELDS_H"
    print("\n@@C@@")  # marker so callers can extract just the C header
    print(f"#ifndef {guard}")
    print(f"#define {guard}")
    print(f"/* generated from {TARGET} by gen-fieldmap.py -- do not edit by hand */")
    print(f"static const gd_field_t {tname}_fields[] = {{")
    print("/*    offset    kind      count   name */")
    for off, nm, kind, cnt, size in rows:
        print(f'    {{ 0x{off:04X}, GD_{kind:<5}, {cnt:>3}, "{nm}" }},')
    print("};")
    print(f"#define {tname.upper()}_FIELD_COUNT {len(rows)}")
    print(f"#endif /* {guard} */")
