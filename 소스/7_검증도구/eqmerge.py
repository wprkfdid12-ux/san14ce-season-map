# v26k27: the one-pass ground shader against the old two passes.
#   way 1: merged shader (season part off) == the game's original shader, every output
#   way 0: merged o0.rgb == season A + saturate(original o0.rgb) * season M; other outputs == original
import sys, numpy as np
sys.path.insert(0, '/home/claude/sd')
from dxemu import Emu, f2b, b2f, F
from eqcheck import load, cbsizes
def run(orig, merged, season, n, seed=3):
    O, Mg, Se = Emu(orig), Emu(merged), Emu(season)
    rng = np.random.default_rng(seed)
    sz = {}
    for p in (orig, merged, season):
        for k, v in cbsizes(p).items(): sz[k] = max(v, sz.get(k, 0))
    W = rng.normal(size=(200, 4, 4)).astype(F)
    def tex(slot, c, extra):
        c = np.nan_to_num(np.asarray(c, F))
        return (F(0.5) + F(0.5) * np.sin(W[slot % 200] @ (c * F(0.001) if slot >= 100 else c) * F(3.1) + F(extra) + F(slot))).astype(F)
    stats = dict(runs=0, disc=0, bad1=0, bad0=0, season_on=0, max0=0.0)
    for it in range(n):
        cb = {}
        for k, m in sz.items():
            a = rng.uniform(-1, 2, size=(m, 4)).astype(F)
            if k == 0 and m > 261: a[6:262, 0] = 2.0
            if k == 13: a = rng.normal(size=(m, 4)).astype(F) * F(0.01)
            cb[k] = f2b(a).reshape(m, 4)
        inp = {}
        for name, si, reg, mask, sv in O.sig['ISGN']:
            v = rng.uniform(-1, 1, 4).astype(F)
            if name == 'TEXCOORD' and si == 3 and 'TEXCOORD' in [x[0] for x in O.sig['ISGN']] and len(O.sig['ISGN']) > 6: v = np.array([rng.uniform(0, 204800), rng.uniform(0, 3000), rng.uniform(0, 204800), 1], F)
            if name == 'TEXCOORD' and ((si == 6) or (si == 3 and len(O.sig['ISGN']) <= 6)): v = np.array([rng.uniform(0, 1), rng.uniform(0, 1), 0, 0], F)
            if name == 'SV_Position': v = np.array([rng.uniform(0, 1920), rng.uniform(0, 1080), rng.uniform(0, 1), 1], F)
            inp[reg] = f2b(v)
        ip = {0: (5.0, 1.0, 1920.0, 1080.0), 100: (100.0, 3.0, 26.0, 1.0)}
        for e in (O, Mg, Se): e.ip = dict(ip)
        o_or = O.run(inp, cb, tex); d_or = O.disc
        Mg.ip[98] = (1.0, 0, 0, 0); o_m1 = Mg.run(inp, cb, tex); d_m1 = Mg.disc
        Mg.ip[98] = (0.0, 0, 0, 0); o_m0 = Mg.run(inp, cb, tex); d_m0 = Mg.disc
        o_se = Se.run(inp, cb, tex); d_se = Se.disc
        stats['runs'] += 1
        if d_or != d_m1 or d_or != d_m0: stats['bad1'] += 1; print('discard differs', it); continue
        if d_or: stats['disc'] += 1; continue
        for k in o_or:
            if not np.array_equal(o_or[k], o_m1.get(k)): stats['bad1'] += 1; print('way1 differs', it, k, b2f(o_or[k]), b2f(o_m1.get(k))); break
        g = b2f(o_or[0])
        if d_se: exp = g.copy()                                   # season pass discarded (sea floor): the game's colour stays
        else:
            stats['season_on'] += 1
            A, M = b2f(o_se[0]), b2f(o_se[1]); exp = g.copy(); exp[:3] = A[:3] + np.clip(g[:3], 0, 1) * M[:3]
        got = b2f(o_m0[0])
        d = float(np.max(np.abs(got - exp) / (1 + np.abs(exp))))
        stats['max0'] = max(stats['max0'], d)
        if d > 1e-4 or not np.array_equal(np.isnan(got), np.isnan(exp)): stats['bad0'] += 1; print('way0 differs', it, got, exp)
        for k in o_or:
            if k != 0 and not np.array_equal(o_or[k], o_m0.get(k)): stats['bad0'] += 1; print('way0 other output differs', it, k); break
    print(orig.split('/')[-1], stats)
if __name__ == '__main__':
    S = sys.argv[1]
    for h, n in (('9addb6cb884c7a7c', 'Far'), ('d3e1f71e809bac74', 'Near')):
        run('/home/claude/sd/GameShaders/%s.dxbc' % h, load(S + '%s-ps_replace.bin' % h), load(S + 'SeasonTerrain%s.ps_5_0.8000.bin' % n), int(sys.argv[2]))
