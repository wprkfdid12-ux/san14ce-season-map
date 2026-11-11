# rollback EXE -> official EXE address mapper (function signatures with relative fields masked)
import struct, subprocess, re, numpy as np, functools
IB = 0x140000000
class Img:
    def __init__(self, path):
        self.D = open(path, 'rb').read(); D = self.D
        pe = struct.unpack_from('<I', D, 0x3c)[0]; n = struct.unpack_from('<H', D, pe + 6)[0]; opt = struct.unpack_from('<H', D, pe + 20)[0]
        self.S = []
        for i in range(n):
            nm, vs, va, rs, ro = struct.unpack_from('<8sIIII', D, pe + 24 + opt + 40 * i)
            self.S.append((nm.rstrip(b'\0').decode(), va, vs, ro, rs))
        # .pdata functions
        _, va, vs, ro, rs = self.sec('.pdata')
        a = np.frombuffer(D[ro:ro + vs // 12 * 12], dtype='<u4').reshape(-1, 3)
        self.fb = a[:, 0].astype(np.int64); self.fe = a[:, 1].astype(np.int64)
        _, self.tva, self.tvs, self.tro, _ = self.sec('.text')
        self.text = D[self.tro:self.tro + self.tvs]
    def sec(self, name):
        for s in self.S:
            if s[0] == name: return s
    def off(self, rva):
        for nm, va, vs, ro, rs in self.S:
            if va <= rva < va + rs: return ro + rva - va
    def rd(self, rva, n): o = self.off(rva); return self.D[o:o + n]
    def func(self, rva):
        i = np.searchsorted(self.fb, rva, side='right') - 1
        if i >= 0 and self.fb[i] <= rva < self.fe[i]: return int(self.fb[i]), int(self.fe[i])
        return None
    def dis(self, rva, n):
        open('/tmp/claude-0/xm.bin', 'wb').write(self.rd(rva, n))
        r = subprocess.run(['objdump', '-D', '-b', 'binary', '-m', 'i386:x86-64', '-M', 'intel', '--adjust-vma=0x%x' % (IB + rva), '/tmp/claude-0/xm.bin'], capture_output=True, text=True).stdout
        out = []
        for l in r.splitlines():
            m = re.match(r'\s+([0-9a-f]+):\t([0-9a-f ]+)\t(.*)$', l)
            if m:
                a = int(m.group(1), 16) - IB; b = bytes.fromhex(m.group(2).replace(' ', '')); out.append([a, b, m.group(3)])
            else:
                m = re.match(r'\s+([0-9a-f]+):\t([0-9a-f ]+)$', l)     # continuation line of a long instruction
                if m and out: out[-1][1] += bytes.fromhex(m.group(2).replace(' ', ''))
        return out
    def mask(self, rva, n):
        """bytes of [rva, rva+n) and a mask (True = must match); rel32 / rip-relative fields are wildcards. Returns (bytes, mask, refs) where refs = list of (offset_in_block, target_rva)"""
        ins = self.dis(rva, n + 16); B = bytearray(); M = []; refs = []
        for a, b, t in ins:
            if a >= rva + n: break
            mk = [True] * len(b); nxt = a + len(b)
            tgt = None
            m = re.search(r'#\s*0x([0-9a-f]+)', t)
            if m: tgt = int(m.group(1), 16) - IB
            else:
                m = re.match(r'(call|jmp|j[a-z]+)\s+0x([0-9a-f]+)', t)
                if m and len(b) >= 5: tgt = int(m.group(2), 16) - IB
            if tgt is not None:
                d = struct.pack('<i', tgt - nxt)
                k = b.rfind(d)
                if k < 0:   # rip-relative with an immediate after the displacement
                    for imm in (1, 2, 4):
                        d2 = struct.pack('<i', tgt - nxt)
                        k = b.find(d2)
                        if k >= 0: break
                if k >= 0:
                    for j in range(k, k + 4): mk[j] = False
                    refs.append((a - rva + k, tgt, a - rva, len(b)))
            B += b; M += mk
        return bytes(B[:n]), M[:n], [r for r in refs if r[0] + 4 <= n]
def pattern(b, m):
    return re.compile(b''.join(re.escape(bytes([x])) if k else b'.' for x, k in zip(b, m)), re.S)
@functools.lru_cache(None)
def maps():
    return Img('/home/claude/port/rb_KO.unpacked.exe'), Img('/home/claude/port/off_KO.unpacked.exe')
def map_code(rva, minlen=48, verbose=False):
    """rollback code rva -> official rva, through the masked bytes of the whole containing function"""
    R, O = maps(); f = R.func(rva)
    if not f: raise Exception('no function for %x' % rva)
    fb, fe = f; n = fe - fb
    for L in (n, min(n, 512), min(n, 256), min(n, 128), min(n, 64)):
        b, m, _ = R.mask(fb, L)
        hits = [x.start() for x in pattern(b, m).finditer(O.text)]
        if len(hits) == 1:
            nb = hits[0] + O.tva
            if verbose: print('func %x(%d) -> %x  [sig %d bytes]' % (fb, n, nb, L))
            return nb + (rva - fb)
        if verbose: print('  sig len %d: %d hits' % (L, len(hits)))
        if len(hits) > 1 and L == n: continue
    raise Exception('no unique match for %x' % rva)
def refs_to(img, target):
    """code locations whose rel32 / rip-relative displacement points at target (the displacement ends the instruction, or an imm8/16/32 follows)"""
    t = np.frombuffer(img.text[:len(img.text) // 1 * 1], dtype=np.uint8)
    n = len(img.text) - 4
    d = (t[0:n].astype(np.int64) | (t[1:n + 1].astype(np.int64) << 8) | (t[2:n + 2].astype(np.int64) << 16) | (t[3:n + 3].astype(np.int64) << 24))
    d = np.where(d >= 2**31, d - 2**32, d)
    pos = np.arange(n, dtype=np.int64) + img.tva
    out = []
    for extra in (0, 1, 2, 4):
        hit = np.nonzero(pos + 4 + extra + d == target)[0]
        out += [(int(h) + img.tva, extra) for h in hit]
    return sorted(out)
def map_data(rva, want=6, verbose=False):
    """rollback data rva -> official, through code that references it (all agreeing)"""
    R, O = maps(); res = {}
    rr = [(p, e) for p, e in refs_to(R, rva) if R.func(p)]
    step = max(1, len(rr) // 20)
    for pos, extra in rr[::step]:
        try: npos = map_code(pos)
        except Exception: continue
        dd = struct.unpack('<i', O.rd(npos, 4))[0]
        nt = npos + 4 + extra + dd
        res[nt] = res.get(nt, 0) + 1
        if sum(res.values()) >= want: break
    if verbose: print('data %x -> %s' % (rva, {hex(k): v for k, v in res.items()}))
    return res
