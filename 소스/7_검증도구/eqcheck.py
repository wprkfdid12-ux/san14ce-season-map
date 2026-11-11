# equivalence check: shader A vs shader B on random inputs (same textures / constants / inputs)
import sys, numpy as np
sys.path.insert(0, '/home/claude/sd')
from dxemu import Emu, f2b, b2f, F
from rdef import rdef
def load(p):
    b = open(p, 'rb').read(); i = b.find(b'DXBC'); q = '/tmp/claude-0/eq_' + str(abs(hash(p))) + '.dxbc'; open(q, 'wb').write(b[i:]); return q
def cbsizes(p):
    res, cbs = rdef(p); out = {}
    for r in res:
        if r['type'] == 'cbuffer':
            cb = [c for c in cbs if c['name'] == r['name']][0]; out[r['bind']] = (cb['size'] + 15) // 16
    return out
def check(pa, pb, n=300, seed=1, outs=None, setup=None):
    A, B = Emu(pa), Emu(pb)
    rng = np.random.default_rng(seed); sz = cbsizes(pa); sz.update({k: max(v, sz.get(k, 0)) for k, v in cbsizes(pb).items()})
    W = rng.normal(size=(200, 4, 4)).astype(F)
    def tex(slot, c, extra):
        c = np.nan_to_num(np.asarray(c, F))
        return (F(0.5) + F(0.5) * np.sin(W[slot % 200] @ c * F(3.1) + F(extra) + F(slot))).astype(F)
    bad = 0; maxd = 0; nd = 0
    for it in range(n):
        cb = {}
        for k, m in sz.items():
            a = rng.uniform(-1, 2, size=(m, 4)).astype(F)
            if rng.random() < 0.5: a = np.round(a * 4) / 4
            cb[k] = f2b(a).reshape(m, 4)
        if 0 in cb and len(cb[0]) > 261 and rng.random() < 0.8:
            a = b2f(cb[0].reshape(-1)).reshape(-1, 4); a[6:262, 0] = 2.0; cb[0] = f2b(a).reshape(-1, 4)   # ground: fewer discarded pixels
        if setup: setup(cb, rng)
        inp = {}
        for name, si, reg, mask, sv in A.sig['ISGN']:
            inp[reg] = f2b(rng.uniform(-2, 2, 4).astype(F) * (1000 if name == 'TEXCOORD' and si in (3,) else 1))
            if name == 'SV_Position': inp[reg] = f2b(np.array([rng.uniform(0, 1920), rng.uniform(0, 1080), rng.uniform(0, 1), 1], F))
        oa = A.run(inp, cb, tex); da = A.disc; ob = B.run(inp, cb, tex); db = B.disc
        if da != db: bad += 1; continue
        if da: nd += 1; continue
        for k in (outs or sorted(set(oa) | set(ob))):
            x = b2f(oa.get(k, np.zeros(4, np.uint32))); y = b2f(ob.get(k, np.zeros(4, np.uint32)))
            fin = np.isfinite(x) & np.isfinite(y)
            if not np.array_equal(np.isnan(x), np.isnan(y)): bad += 1; break
            d = np.max(np.abs(x[fin] - y[fin]) / (1 + np.abs(x[fin]))) if fin.any() else 0
            maxd = max(maxd, d)
            if d > 1e-4: bad += 1; print('  differs', it, k, x, y); break
    print('%s vs %s: %d runs, %d discarded both, %d differ, max rel diff %.2e' % (pa.split('/')[-1], pb.split('/')[-1], n, nd, bad, maxd))
    return bad
if __name__ == '__main__':
    check(sys.argv[1], load(sys.argv[2]), int(sys.argv[3]) if len(sys.argv) > 3 else 300)
