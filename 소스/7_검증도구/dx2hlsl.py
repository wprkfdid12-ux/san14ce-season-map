"""Mechanical DXBC (SM5 pixel shader subset) -> HLSL translator.
Every register is a float4 holding raw bits; integer instructions go through asint/asuint.
Produces a body that behaves like the original instruction by instruction."""
import struct, sys
sys.path.insert(0, '/home/claude/sd')
from dxdis import OPN, DCL
from rdef import rdef
from sig2 import sig

def f32(u): return struct.unpack('<f', struct.pack('<I', u))[0]

def parse_operand(t, p):
    tok = t[p]; p += 1
    nc = [0, 1, 4, 0][tok & 3]; sel = (tok >> 2) & 3; comp = (tok >> 4) & 0xff
    typ = (tok >> 12) & 0xff; idim = (tok >> 20) & 3; reps = [(tok >> 22) & 7, (tok >> 25) & 7, (tok >> 28) & 7]
    mod = 0
    if tok >> 31:
        ext = t[p]; p += 1; mod = (ext >> 6) & 0xff
    o = dict(typ=typ, nc=nc, mod=mod, idx=[])
    if typ == 4:
        o['imm'] = list(t[p:p + nc]); p += nc
        o['sw'] = 'xyzw'[:nc] if nc == 4 else 'x'
        return o, p
    for k in range(idim):
        r = reps[k]
        if r == 0: o['idx'].append((t[p], None)); p += 1
        elif r == 2:
            s, p = parse_operand(t, p); o['idx'].append((0, s))
        elif r == 3:
            base = t[p]; p += 1; s, p = parse_operand(t, p); o['idx'].append((base, s))
        else: raise Exception('idx rep %d' % r)
    if nc == 4:
        if sel == 0: o['mask'] = ''.join('xyzw'[i] for i in range(4) if comp & (1 << i)); o['sw'] = 'xyzw'
        elif sel == 1: o['sw'] = ''.join('xyzw'[(comp >> (2 * i)) & 3] for i in range(4))
        else: o['sw'] = 'xyzw'[comp & 3] * 4
    elif nc == 1: o['sw'] = 'xxxx'
    else: o['sw'] = ''
    return o, p

def instructions(path):
    d = open(path, 'rb').read(); n = struct.unpack_from('<I', d, 28)[0]
    for i in range(n):
        o = struct.unpack_from('<I', d, 32 + 4 * i)[0]
        if d[o:o + 4] in (b'SHEX', b'SHDR'):
            sz = struct.unpack_from('<I', d, o + 4)[0]; t = struct.unpack_from('<%dI' % (sz // 4), d, o + 8)
    p = 2; out = []; ntemps = 0
    while p < len(t):
        tok = t[p]; op = tok & 0x7ff; ln = (tok >> 24) & 0x7f
        if op == 53: ln = t[p + 1]
        if op == 104: ntemps = t[p + 1]
        if op in DCL or op == 53: p += ln; continue
        q = p + 1; ext = tok >> 31
        while ext: e = t[q]; q += 1; ext = e >> 31
        ops = []
        while q < p + ln:
            s, q = parse_operand(t, q); ops.append(s)
        out.append(dict(op=OPN.get(op, 'op%d' % op), sat=(tok >> 13) & 1, nz=(tok >> 18) & 1, ops=ops))
        p += ln
    return out, ntemps

REG = {0: 'r', 1: 'v', 2: 'o', 8: 'cb', 7: 't', 6: 's'}

class Gen:
    def __init__(s, inputs): s.inputs = inputs   # v reg -> hlsl expression of float4
    def regname(s, o):
        typ = o['typ']
        if typ == 0: return 'r%d' % o['idx'][0][0]
        if typ == 1: return s.inputs[o['idx'][0][0]]
        if typ == 2: return 'o%d' % o['idx'][0][0]
        if typ == 8:
            cbn, _ = o['idx'][0]; base, rel = o['idx'][1]
            if rel is None: return 'cb%d[%d]' % (cbn, base)
            return 'cb%d[%d + asint(%s)]' % (cbn, base, s.src(rel, 'x', 'f'))
        raise Exception('reg type %d' % typ)
    def src(s, o, mask, kind):
        """kind: f (float), i (int), u (uint). mask: dest components to pick"""
        n = len(mask)
        if o['typ'] == 4:
            vals = o['imm']
            pick = [vals[0]] * n if len(vals) == 1 else [vals['xyzw'.index(c)] for c in mask]
            if kind == 'f':
                def fl(v):
                    e = v & 0x7f800000
                    if v == 0: return '0.0'
                    if e == 0x7f800000 or e == 0 or v == 0x80000000: return 'asfloat(%du)' % v
                    return '%r' % f32(v)
                lit = [fl(v) for v in pick]
                e = ('float%d(%s)' % (n, ','.join(lit))) if n > 1 else lit[0]
            elif kind == 'i':
                lit = [str(struct.unpack('<i', struct.pack('<I', v))[0]) for v in pick]
                e = ('int%d(%s)' % (n, ','.join(lit))) if n > 1 else lit[0]
            else:
                lit = ['%du' % v for v in pick]
                e = ('uint%d(%s)' % (n, ','.join(lit))) if n > 1 else lit[0]
        else:
            sw = o['sw']; comp = ''.join(sw['xyzw'.index(c)] for c in mask)
            base = '%s.%s' % (s.regname(o), comp)
            if UTEMPS[0] and o['typ'] == 0:      # temps hold raw bits (uint4): nothing ever passes through float moves
                e = 'asfloat(%s)' % base if kind == 'f' else ('asint(%s)' % base if kind == 'i' else base)
            else:
                e = base if kind == 'f' else ('asint(%s)' % base if kind == 'i' else 'asuint(%s)' % base)
        if o['mod'] & 2 and o['mod'] & 1: e = '-abs(%s)' % e
        elif o['mod'] & 2: e = 'abs(%s)' % e
        elif o['mod'] & 1: e = '-%s' % e
        return '(' + e + ')'
    def dst(s, o): return '%s.%s' % (s.regname(o), o.get('mask', 'xyzw'))

def translate(path, inputs, resdims, extra_before_ret='', restype={}):
    ins, ntemps = instructions(path)
    g = Gen(inputs)
    L = []; ind = 1
    def emit(x): L.append('    ' * ind + x)
    for k, I in enumerate(ins):
        op = I['op']; O = I['ops']; sat = I['sat']
        def assign(expr, kind='f'):
            d = O[0]; m = d.get('mask', 'xyzw'); n = len(m)
            if UTEMPS[0] and d['typ'] == 0:
                if sat: expr = 'saturate(%s)' % expr if kind == 'f' else expr
                e = 'asuint(%s)' % expr if kind in ('f', 'i') else expr
                emit('%s = %s;' % (g.dst(d), e)); return
            if kind == 'f': e = expr
            elif kind == 'i': e = 'asfloat(%s)' % expr
            else: e = 'asfloat(%s)' % expr
            if sat: e = 'saturate(%s)' % e
            emit('%s = %s;' % (g.dst(d), e))
        def m(): return O[0].get('mask', 'xyzw')
        def S(i, kind='f', mask=None): return g.src(O[i], mask or m(), kind)
        def bv(n): return 'uint%d' % n if n > 1 else 'uint'
        n = len(m()) if O else 0
        def cmp(c, kind):
            a, b = S(1, kind), S(2, kind)
            return '(%s)(%s %s %s) * 0xffffffffu' % (bv(n), a, c, b)
        if op == 'mov':
            if UTEMPS[0] and not sat and not (O[1].get('mod', 0)): assign(S(1, 'u'), 'u')
            else: assign(S(1))
        elif op == 'add': assign('%s + %s' % (S(1), S(2)))
        elif op == 'mul': assign('%s * %s' % (S(1), S(2)))
        elif op == 'div': assign('%s / %s' % (S(1), S(2)))
        elif op == 'mad': assign('%s * %s + %s' % (S(1), S(2), S(3)))
        elif op == 'min': assign('min(%s, %s)' % (S(1), S(2)))
        elif op == 'max': assign('max(%s, %s)' % (S(1), S(2)))
        elif op == 'rsq': assign('rsqrt(%s)' % S(1))
        elif op == 'sqrt': assign('sqrt(%s)' % S(1))
        elif op == 'exp': assign('exp2(%s)' % S(1))
        elif op == 'log': assign('log2(%s)' % S(1))
        elif op == 'frc': assign('frac(%s)' % S(1))
        elif op == 'rcp': assign('rcp(%s)' % S(1))
        elif op == 'round_ne': assign('round(%s)' % S(1))
        elif op == 'round_ni': assign('floor(%s)' % S(1))
        elif op == 'round_pi': assign('ceil(%s)' % S(1))
        elif op == 'round_z': assign('trunc(%s)' % S(1))
        elif op in ('dp2', 'dp3', 'dp4'):
            k2 = 'xyzw'[:int(op[2])]
            e = 'dot(%s, %s)' % (g.src(O[1], k2, 'f'), g.src(O[2], k2, 'f'))
            if n > 1: e = '(float%d)%s' % (n, e)
            assign(e)
        elif op == 'lt': assign(cmp('<', 'f'), 'u')
        elif op == 'ge': assign(cmp('>=', 'f'), 'u')
        elif op == 'eq': assign(cmp('==', 'f'), 'u')
        elif op == 'ne': assign(cmp('!=', 'f'), 'u')
        elif op == 'ilt': assign(cmp('<', 'i'), 'u')
        elif op == 'ige': assign(cmp('>=', 'i'), 'u')
        elif op == 'ieq': assign(cmp('==', 'i'), 'u')
        elif op == 'ine': assign(cmp('!=', 'i'), 'u')
        elif op == 'ult': assign(cmp('<', 'u'), 'u')
        elif op == 'uge': assign(cmp('>=', 'u'), 'u')
        elif op == 'and': assign('%s & %s' % (S(1, 'u'), S(2, 'u')), 'u')
        elif op == 'or': assign('%s | %s' % (S(1, 'u'), S(2, 'u')), 'u')
        elif op == 'xor': assign('%s ^ %s' % (S(1, 'u'), S(2, 'u')), 'u')
        elif op == 'not': assign('~%s' % S(1, 'u'), 'u')
        elif op == 'iadd': assign('%s + %s' % (S(1, 'i'), S(2, 'i')), 'i')
        elif op == 'imul': assign('%s * %s' % (S(2, 'i'), S(3, 'i')), 'i')
        elif op == 'ishl': assign('%s << (%s & 31)' % (S(1, 'i'), S(2, 'u')), 'i')
        elif op == 'ishr': assign('%s >> (%s & 31)' % (S(1, 'i'), S(2, 'u')), 'i')
        elif op == 'ushr': assign('%s >> (%s & 31)' % (S(1, 'u'), S(2, 'u')), 'u')
        elif op == 'ineg': assign('-%s' % S(1, 'i'), 'i')
        elif op == 'imax': assign('max(%s, %s)' % (S(1, 'i'), S(2, 'i')), 'i')
        elif op == 'imin': assign('min(%s, %s)' % (S(1, 'i'), S(2, 'i')), 'i')
        elif op == 'umax': assign('max(%s, %s)' % (S(1, 'u'), S(2, 'u')), 'u')
        elif op == 'umin': assign('min(%s, %s)' % (S(1, 'u'), S(2, 'u')), 'u')
        elif op == 'itof': assign('(float%s)%s' % (n if n > 1 else '', S(1, 'i')))
        elif op == 'utof': assign('(float%s)%s' % (n if n > 1 else '', S(1, 'u')))
        elif op == 'ftoi': assign('(int%s)%s' % (n if n > 1 else '', S(1)), 'i')
        elif op == 'ftou': assign('(uint%s)%s' % (n if n > 1 else '', S(1)), 'u')
        elif op == 'movc':
            if UTEMPS[0] and not sat and not O[2].get('mod', 0) and not O[3].get('mod', 0):
                assign('(%s != 0) ? %s : %s' % (S(1, 'u'), S(2, 'u'), S(3, 'u')), 'u')
            else: assign('(%s != 0) ? %s : %s' % (S(1, 'u'), S(2), S(3)))
        elif op in ('deriv_rtx', 'deriv_rtx_coarse', 'deriv_rtx_fine'): assign('ddx(%s)' % S(1))
        elif op in ('deriv_rty', 'deriv_rty_coarse', 'deriv_rty_fine'): assign('ddy(%s)' % S(1))
        elif op in ('sample', 'sample_l', 'sample_b', 'sample_d'):
            tr = O[2]; tnum = tr['idx'][0][0]; snum = O[3]['idx'][0][0]
            dim = resdims[tnum]; cd = {4: 'xy', 5: 'xyz', 8: 'xyz', 9: 'xyz'}[dim]
            coord = g.src(O[1], cd, 'f')
            if op == 'sample': call = 't%d.Sample(s%d, %s)' % (tnum, snum, coord)
            elif op == 'sample_l': call = 't%d.SampleLevel(s%d, %s, %s)' % (tnum, snum, coord, g.src(O[4], 'x', 'f'))
            elif op == 'sample_d':
                gd = {4: 'xy', 5: 'xy', 8: 'xyz', 9: 'xyz'}[dim]
                call = 't%d.SampleGrad(s%d, %s, %s, %s)' % (tnum, snum, coord, g.src(O[4], gd, 'f'), g.src(O[5], gd, 'f'))
            else: call = 't%d.SampleBias(s%d, %s, %s)' % (tnum, snum, coord, g.src(O[4], 'x', 'f'))
            sw = tr['sw']; comp = ''.join(sw['xyzw'.index(c)] for c in m())
            assign('(%s).%s' % (call, comp))
        elif op == 'udiv':
            for di, ex in ((0, '/'), (1, '%')):
                d = O[di]
                if d['typ'] == 13: continue
                mm = d.get('mask', 'xyzw')
                e = '(%s %s %s)' % (g.src(O[2], mm, 'u'), ex, g.src(O[3], mm, 'u'))
                if not (UTEMPS[0] and d['typ'] == 0): e = 'asfloat%s' % e
                emit('%s = %s;' % (g.dst(d), e))
        elif op == 'sincos':
            for di, fn in ((0, 'sin'), (1, 'cos')):
                d = O[di]
                if d['typ'] == 13: continue
                mm = d.get('mask', 'xyzw')
                e = '%s(%s)' % (fn, g.src(O[2], mm, 'f'))
                if sat: e = 'saturate(%s)' % e
                if UTEMPS[0] and d['typ'] == 0: e = 'asuint(%s)' % e
                emit('%s = %s;' % (g.dst(d), e))
        elif op == 'ld':
            tr = O[2]; tnum = tr['idx'][0][0]; dim = resdims[tnum]
            if dim == 1: call = 't%d.Load(%s)' % (tnum, g.src(O[1], 'x', 'i'))
            elif dim == 4: call = 't%d.Load(int3(%s, %s))' % (tnum, g.src(O[1], 'xy', 'i'), g.src(O[1], 'w', 'i'))
            else: raise Exception('ld dim %d' % dim)
            sw = tr['sw']; comp = ''.join(sw['xyzw'.index(c)] for c in m())
            e = '(%s).%s' % (call, comp)
            if restype.get(tnum) == 'u':
                if UTEMPS[0] and O[0]['typ'] == 0: assign(e, 'u'); continue
                e = 'asfloat(%s)' % e
            assign(e)
        elif op == 'loop': emit('[loop] while (true) {'); ind += 1
        elif op == 'endloop': ind -= 1; emit('}')
        elif op == 'breakc':
            emit('if (%s(%s != 0)) break;' % ('' if I['nz'] else '!', g.src(O[0], 'x', 'u')))
        elif op == 'break': emit('break;')
        elif op == 'if':
            emit('if (%s(%s != 0)) {' % ('' if I['nz'] else '!', g.src(O[0], 'x', 'u'))); ind += 1
        elif op == 'else': ind -= 1; emit('} else {'); ind += 1
        elif op == 'endif': ind -= 1; emit('}')
        elif op == 'discard':
            emit('if (%s(%s != 0)) discard;' % ('' if I['nz'] else '!', g.src(O[0], 'x', 'u')))
        elif op == 'ret':
            if extra_before_ret: L.append(extra_before_ret)
            emit('return;')
        else: raise Exception('unsupported ' + op)
    return L, ntemps

if __name__ == '__main__':
    pass

UTEMPS = [False]
TEXT = {1: 'Buffer<float4>', 4: 'Texture2D<float4>', 5: 'Texture2DArray<float4>', 8: 'Texture3D<float4>', 9: 'TextureCube<float4>'}
def full_shader(path, header='', inject_top='', inject_end='', extra_decl=''):
    res, cbs = rdef(path); sg = sig(path)
    resdims = {r['bind']: r['dim'] for r in res if r['type'] == 'texture'}
    restype = {r['bind']: ('u' if r['ret'] == 4 else 'f') for r in res if r['type'] == 'texture'}
    D = [header]
    for r in res:
        if r['type'] == 'cbuffer':
            cb = [c for c in cbs if c['name'] == r['name']][0]
            D.append('cbuffer cb%d : register(b%d) { float4 cb%d[%d]; }' % (r['bind'], r['bind'], r['bind'], (cb['size'] + 15) // 16))
        elif r['type'] == 'texture':
            tx = TEXT[r['dim']]
            if r['ret'] == 4: tx = tx.replace('float4', 'uint4')
            D.append('%s t%d : register(t%d);' % (tx, r['bind'], r['bind']))
        elif r['type'] == 'sampler':
            D.append('SamplerState s%d : register(s%d);' % (r['bind'], r['bind']))
    D.append(extra_decl)
    params = []; inputs = {}
    for name, si, reg, mask, sv in sg['ISGN']:
        nm = 'v%d' % reg
        sem = name + (str(si) if (si or name.startswith('SV_ClipDistance') or name == 'TEXCOORD') else '')
        w = bin(mask).count('1'); ty = 'float' + (str(w) if w > 1 else '')
        params.append('%s %s : %s' % (ty, nm, sem)); inputs[reg] = nm
    for name, si, reg, mask, sv in sg.get('OSGN', []):
        params.append('out float4 o%d : SV_Target%d' % (reg, si))
    gen_body, nt = translate(path, inputs, resdims, inject_end, restype)
    body = ['void main(' + ',\n          '.join(params) + ')', '{']
    for o in sg.get('OSGN', []): body.append('    o%d = 0;' % o[2])
    if nt: body.append(('    uint4 ' if UTEMPS[0] else '    float4 ') + ', '.join('r%d = 0' % i for i in range(nt)) + ';')
    if inject_top: body.append(inject_top)
    body += gen_body
    body.append('}')
    return '\n'.join(D) + '\n\n' + '\n'.join(body) + '\n'
