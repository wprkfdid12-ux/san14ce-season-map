"""Tiny SM5 pixel-shader emulator (one pixel at a time) for equivalence checks: two shaders that should compute the same thing
are run on the same random inputs; textures are smooth functions of (slot, coordinates, gradients / level), derivatives a fixed
function of their argument. Float maths in float32 (numpy)."""
import numpy as np, struct, sys
sys.path.insert(0, '/home/claude/sd')
import dx2hlsl
from rdef import rdef
from sig2 import sig
F = np.float32
def b2f(u): return np.frombuffer(np.array(u, dtype=np.uint32).tobytes(), dtype=np.float32).copy()
def f2b(f): return np.frombuffer(np.array(f, dtype=np.float32).tobytes(), dtype=np.uint32).copy()
SW = 'xyzw'

class Emu:
    def __init__(self, path):
        self.ins, self.nt = dx2hlsl.instructions(path)
        self.sig = sig(path)
        d = open(path, 'rb').read(); n = struct.unpack_from('<I', d, 28)[0]; self.icb = np.zeros((0, 4), np.uint32)
        for i in range(n):
            o = struct.unpack_from('<I', d, 32 + 4 * i)[0]
            if d[o:o + 4] in (b'SHEX', b'SHDR'):
                sz = struct.unpack_from('<I', d, o + 4)[0]; t = struct.unpack_from('<%dI' % (sz // 4), d, o + 8)
                p = 2
                while p < len(t):
                    tok = t[p]; op = tok & 0x7ff; ln = (tok >> 24) & 0x7f
                    if op == 53:
                        ln = t[p + 1]
                        if (tok >> 11) == 3: self.icb = np.array(t[p + 2:p + ln], np.uint32).reshape(-1, 4)
                    p += ln
        # map if/else/endif
        st = []; self.jump = {}; ls = []; self.lend = {}; self.lbeg = {}
        for k, I in enumerate(self.ins):
            if I['op'] == 'loop': ls.append(k)
            elif I['op'] == 'endloop': b = ls.pop(); self.lend[b] = k; self.lbeg[k] = b
            elif I['op'] in ('break', 'breakc', 'continue', 'continuec'): self.lbeg.setdefault(('in', k), ls[-1])
            if I['op'] == 'if': st.append([k, None])
            elif I['op'] == 'else': st[-1][1] = k
            elif I['op'] == 'endif':
                a, e = st.pop(); self.jump[a] = (e, k)
                if e is not None: self.jump[e] = (k, k)
    ip = {}
    def run(self, inputs, cb, tex):
        r = [np.zeros(4, np.uint32) for _ in range(max(self.nt, 1))]
        o = {}; xt = {}
        self.disc = False
        if not hasattr(self, 'uav'): self.uav = {}
        self.stores = 0
        def xidx(op):
            reg = op['idx'][0][0]; base, rel = op['idx'][1]
            ix = base + (int(np.int32(get(rel)[0].view(np.int32))) if rel is not None else 0)
            arr = xt.setdefault(reg, {}); return arr, ix
        INTOPS = {'bfrev', 'ubfe', 'bfi', 'countbits', 'udiv', 'iadd', 'ineg', 'imul', 'imad', 'ishl', 'ushr', 'ishr', 'imax', 'imin', 'ilt', 'ige', 'ieq', 'ine', 'and', 'or', 'xor', 'not', 'umax', 'umin', 'ult', 'uge', 'itof', 'utof', 'movc_i'}
        cur = [None]
        def get(op):
            t = op['typ']
            if t == 4:
                imm = op['imm']; v = np.array((imm * 4)[:4] if len(imm) == 1 else imm, np.uint32)
            elif t == 0: v = r[op['idx'][0][0]].copy()
            elif t == 1: v = inputs[op['idx'][0][0]].copy()
            elif t == 8:
                cbi = op['idx'][0][0]; base, rel = op['idx'][1]
                ix = base + (int(np.int32(get(rel)[0].view(np.int32))) if rel is not None else 0)
                arr = cb[cbi]; v = arr[ix].copy() if 0 <= ix < len(arr) else np.zeros(4, np.uint32)
            elif t == 3:
                arr, ix = xidx(op); v = arr.get(ix, np.zeros(4, np.uint32)).copy()
            elif t == 9:
                base, rel = op['idx'][0]
                ix = base + (int(np.int32(get(rel)[0].view(np.int32))) if rel is not None else 0)
                v = self.icb[ix].copy() if 0 <= ix < len(self.icb) else np.zeros(4, np.uint32)
            else: raise Exception('operand type %d' % t)
            sw = op.get('sw', 'xyzw')
            if len(sw) < 4: sw = (sw + sw[-1] * 4)[:4]
            v = np.array([v[SW.index(c)] for c in sw], np.uint32)
            m = op.get('mod', 0)
            if m and cur[0] in INTOPS:
                if m & 1: v = ((~v).astype(np.uint64) + 1 & 0xffffffff).astype(np.uint32)
            elif m:
                f = b2f(v)
                if m & 2: f = np.abs(f)
                if m & 1: f = -f
                v = f2b(f)
            return v
        def geti(op):
            v = get(op)
            m = op.get('mod', 0)
            return v
        def put(op, val, sat=False):
            val = np.array(val, np.uint32)
            if sat: val = f2b(np.clip(b2f(val), 0, 1))
            t = op['typ']; mask = op.get('mask', 'xyzw')
            if t == 0: dst = r[op['idx'][0][0]]
            elif t == 2:
                k = op['idx'][0][0]; dst = o.setdefault(k, np.zeros(4, np.uint32))
            elif t == 13: return
            elif t == 3:
                arr, ix = xidx(op); dst = arr.setdefault(ix, np.zeros(4, np.uint32))
            else: raise Exception('dst type %d' % t)
            for c in mask: dst[SW.index(c)] = val[SW.index(c)]
        pc = 0; n = len(self.ins); self.steps = 0; self.lds = 0
        while pc < n:
            I = self.ins[pc]; op = I['op']; O = I['ops']; sat = I['sat']; self.steps += 1
            if op in ('ld', 'sample', 'sample_l'): self.lds += 1
            if op == 'bfrev' and len(O) == 5: op = 'bfi'
            cur[0] = op
            def f(i): return b2f(get(O[i]))
            def u(i): return get(O[i])
            def s(i): return get(O[i]).view(np.int32)
            if op == 'mov': put(O[0], u(1) if not sat else f2b(f(1)), sat)
            elif op == 'add': put(O[0], f2b(f(1) + f(2)), sat)
            elif op == 'mul': put(O[0], f2b(f(1) * f(2)), sat)
            elif op == 'div':
                with np.errstate(all='ignore'): put(O[0], f2b(f(1) / f(2)), sat)
            elif op == 'mad': put(O[0], f2b(f(1) * f(2) + f(3)), sat)
            elif op == 'min': put(O[0], f2b(np.fmin(f(1), f(2))), sat)
            elif op == 'max': put(O[0], f2b(np.fmax(f(1), f(2))), sat)
            elif op in ('dp2', 'dp3', 'dp4'):
                k = int(op[2]); a, b = f(1), f(2); acc = F(0)
                for i in range(k): acc = F(acc + a[i] * b[i])
                put(O[0], f2b(np.full(4, acc, F)), sat)
            elif op == 'rsq':
                with np.errstate(all='ignore'): put(O[0], f2b(F(1) / np.sqrt(f(1))), sat)
            elif op == 'sqrt':
                with np.errstate(all='ignore'): put(O[0], f2b(np.sqrt(f(1))), sat)
            elif op == 'exp':
                with np.errstate(all='ignore'): put(O[0], f2b(np.exp2(f(1))), sat)
            elif op == 'log':
                with np.errstate(all='ignore'): put(O[0], f2b(np.log2(f(1))), sat)
            elif op == 'frc': put(O[0], f2b(f(1) - np.floor(f(1))), sat)
            elif op == 'round_ni': put(O[0], f2b(np.floor(f(1))), sat)
            elif op == 'round_pi': put(O[0], f2b(np.ceil(f(1))), sat)
            elif op == 'round_z': put(O[0], f2b(np.trunc(f(1))), sat)
            elif op == 'round_ne': put(O[0], f2b(np.rint(f(1))), sat)
            elif op in ('lt', 'ge', 'eq', 'ne'):
                a, b = f(1), f(2)
                with np.errstate(all='ignore'): c = {'lt': a < b, 'ge': a >= b, 'eq': a == b, 'ne': a != b}[op]
                put(O[0], np.where(c, 0xffffffff, 0).astype(np.uint32))
            elif op in ('ilt', 'ige', 'ieq', 'ine'):
                a, b = s(1), s(2); c = {'ilt': a < b, 'ige': a >= b, 'ieq': a == b, 'ine': a != b}[op]
                put(O[0], np.where(c, 0xffffffff, 0).astype(np.uint32))
            elif op in ('ult', 'uge'):
                a, b = u(1), u(2); c = a < b if op == 'ult' else a >= b
                put(O[0], np.where(c, 0xffffffff, 0).astype(np.uint32))
            elif op == 'udiv':
                a, b = u(2), u(3); q = np.where(b != 0, a // np.maximum(b, 1), 0xffffffff).astype(np.uint32); r_ = np.where(b != 0, a % np.maximum(b, 1), 0xffffffff).astype(np.uint32)
                if O[0]['typ'] != 13: put(O[0], q)
                if O[1]['typ'] != 13: put(O[1], r_)
            elif op == 'sincos':
                x = b2f(get(O[2]))
                if O[0]['typ'] != 13: put(O[0], f2b(np.sin(x).astype(F)), sat)
                if O[1]['typ'] != 13: put(O[1], f2b(np.cos(x).astype(F)), sat)
            elif op == 'and': put(O[0], u(1) & u(2))
            elif op == 'or': put(O[0], u(1) | u(2))
            elif op == 'xor': put(O[0], u(1) ^ u(2))
            elif op == 'not': put(O[0], ~u(1))
            elif op == 'iadd': put(O[0], (s(1).astype(np.int64) + s(2)).astype(np.int64).astype(np.uint32) if False else ((u(1).astype(np.uint64) + u(2)) & 0xffffffff).astype(np.uint32))
            elif op == 'ineg': put(O[0], ((~u(1)).astype(np.uint64) + 1 & 0xffffffff).astype(np.uint32))
            elif op == 'imul':
                put(O[1], ((s(2).astype(np.int64) * s(3)) & 0xffffffff).astype(np.uint32))
            elif op == 'imad': put(O[0], ((s(1).astype(np.int64) * s(2) + s(3)) & 0xffffffff).astype(np.uint32))
            elif op == 'ishl': put(O[0], ((u(1).astype(np.uint64) << (u(2) & 31)) & 0xffffffff).astype(np.uint32))
            elif op == 'ushr': put(O[0], u(1) >> (u(2) & 31))
            elif op == 'ishr': put(O[0], (s(1) >> (u(2) & 31).astype(np.int32)).view(np.uint32))
            elif op == 'imax': put(O[0], np.maximum(s(1), s(2)).view(np.uint32))
            elif op == 'imin': put(O[0], np.minimum(s(1), s(2)).view(np.uint32))
            elif op == 'umax': put(O[0], np.maximum(u(1), u(2)))
            elif op == 'umin': put(O[0], np.minimum(u(1), u(2)))
            elif op == 'movc': put(O[0], np.where(u(1) != 0, u(2), u(3)).astype(np.uint32) if not sat else f2b(b2f(np.where(u(1) != 0, u(2), u(3)).astype(np.uint32))), sat)
            elif op == 'ftoi':
                x = f(1); x = np.nan_to_num(np.trunc(x), nan=0, posinf=2**31 - 1, neginf=-2**31); put(O[0], np.clip(x, -2**31, 2**31 - 1).astype(np.int64).astype(np.int32).view(np.uint32))
            elif op == 'ftou':
                x = f(1); x = np.nan_to_num(np.trunc(x), nan=0, posinf=2**32 - 1, neginf=0); put(O[0], np.clip(x, 0, 2**32 - 1).astype(np.uint64).astype(np.uint32))
            elif op == 'itof': put(O[0], f2b(s(1).astype(F)), sat)
            elif op == 'utof': put(O[0], f2b(u(1).astype(F)), sat)
            elif op in ('deriv_rtx', 'deriv_rtx_coarse', 'deriv_rtx_fine'): put(O[0], f2b(f(1) * F(0.37)), sat)
            elif op in ('deriv_rty', 'deriv_rty_coarse', 'deriv_rty_fine'): put(O[0], f2b(f(1) * F(-0.23)), sat)
            elif op in ('sample', 'sample_l', 'sample_b', 'sample_d'):
                c = f(1); tr = O[2]; slot = tr['idx'][0][0]
                extra = 0.0
                if op == 'sample_l' or op == 'sample_b': extra = float(f(4)[0]) * 0.13
                if op == 'sample_d': g1, g2 = f(4), f(5)
                if op == 'sample': g1, g2 = c * F(0.37), c * F(-0.23)
                if op in ('sample', 'sample_d'): extra = float(g1[0] * F(0.07) + g1[1] * F(0.05) + g2[0] * F(0.03) + g2[1] * F(0.11)) * 1e-3
                v = tex(slot, c, extra)
                sw = tr['sw']; v4 = np.array([v[SW.index(ch)] for ch in sw], F)
                put(O[0], f2b(v4), sat)
            elif op == 'ld':
                c = u(1).view(np.int32).astype(F); tr = O[2]; slot = tr['idx'][0][0]
                v = np.array(self.ip.get(int(c[0]), (0, 0, 0, 0)), F) if slot == 120 else tex(slot, c, 0.0)
                sw = tr['sw']; v4 = np.array([v[SW.index(ch)] for ch in sw], F)
                put(O[0], f2b(v4), sat)
            elif op == 'bfrev':                       # (the disassembler's table names opcode 0x8c 'bfrev'; with 4 sources it is bfi - see above)
                v = u(1); put(O[0], np.array([int('{:032b}'.format(int(x))[::-1], 2) for x in v], np.uint32))
            elif op == 'ubfe':                        # width, offset, value
                w_, o_, v_ = u(1), u(2), u(3); out = []
                for i in range(4):
                    W_ = int(w_[i]) & 31; Of = int(o_[i]) & 31
                    out.append(0 if W_ == 0 else ((int(v_[i]) >> Of) & ((1 << W_) - 1) if W_ + Of < 32 else int(v_[i]) >> Of))
                put(O[0], np.array(out, np.uint32))
            elif op == 'bfi':                         # width, offset, insert, base
                w_, o_, s_, b_ = u(1), u(2), u(3), u(4); out = []
                for i in range(4):
                    W_ = int(w_[i]) & 31; Of = int(o_[i]) & 31; mask = (((1 << W_) - 1) << Of) & 0xffffffff
                    out.append(((int(s_[i]) << Of) & mask) | (int(b_[i]) & ~mask & 0xffffffff))
                put(O[0], np.array(out, np.uint32))
            elif op == 'resinfo':                     # v26k29: GetDimensions (uint form): a 1920 x 1080 screen-sized texture
                sw = O[2].get('sw', 'xyzw'); base = np.array([1920, 1080, 1, 1], np.uint32)
                put(O[0], np.array([base[SW.index(c)] for c in (sw + sw[-1] * 4)[:4]], np.uint32))
            elif op == 'countbits': put(O[0], np.array([bin(int(x)).count('1') for x in u(1)], np.uint32))
            elif op == 'op163':                       # v26k29: ld_uav_typed (R32_UINT array texture): dst, address, u#
                a = u(1); slot = O[2]['idx'][0][0]; val = self.uav.get((slot, int(a[0]), int(a[1]), int(a[2])), 0)
                vv = np.array([val, val, val, val], np.uint32); put(O[0], vv)
            elif op == 'op164':                       # v26k29: store_uav_typed: u#, address, value
                a = u(1); slot = O[0]['idx'][0][0]; vv = u(2); self.uav[(slot, int(a[0]), int(a[1]), int(a[2]))] = int(vv[0]); self.stores += 1
            elif op == 'discard':
                c = u(0)[0] if O[0]['typ'] != 4 else u(0)[0]
                if (c != 0) == (I['nz'] == 1): self.disc = True; return o
            elif op == 'if':
                c = u(0)[0]; take = (c != 0) == (I['nz'] == 1)
                if not take:
                    e, end = self.jump[pc]; pc = (e if e is not None else end) + 1; continue
            elif op == 'else':
                e, end = self.jump[pc]; pc = end + 1; continue
            elif op == 'endif': pass
            elif op == 'ret': return o
            elif op == 'loop': pass
            elif op == 'endloop': pc = self.lbeg[pc] + 1; continue
            elif op in ('break', 'breakc'):
                b = self.lbeg[('in', pc)]
                if op == 'break' or ((u(0)[0] != 0) == (I['nz'] == 1)): pc = self.lend[b] + 1; continue
            elif op in ('continue', 'continuec'):
                b = self.lbeg[('in', pc)]
                if op == 'continue' or ((u(0)[0] != 0) == (I['nz'] == 1)): pc = b + 1; continue
            else: raise Exception('op ' + op)
            pc += 1
        return o
