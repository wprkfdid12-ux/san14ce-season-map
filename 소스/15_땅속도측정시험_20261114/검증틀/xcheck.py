# cross-check: the C++ season code (xval) against the compiled shader run instruction by instruction (emu_t), same inputs and textures
import sys, struct, subprocess, numpy as np, warnings, os
warnings.filterwarnings('ignore')
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from emu_t import Emu, f2b, b2f, F
import texfn
SH = sys.argv[1]; N = int(sys.argv[2]); seed = int(sys.argv[3]); out = sys.argv[4]
T = texfn.table(); T['K3'][:] = 0; T['K3'][117] = 0.37; T['K3'][109] = 0.21
texfn.write(T, out + '.tab')
rng = np.random.default_rng(seed)
def mkip():
    ip = np.zeros((8192, 4), F)
    evn = int(rng.choice([0, 0, 0, 0, 1, 3]))
    ip[0] = (rng.integers(1, 13), 1, 0, 0); ip[1] = (rng.integers(0, 3), 0, 0, 0); ip[2] = (0, rng.uniform(0, 500), 0, 0)
    ip[60] = (0, 0, evn, 1 if rng.random() < 0.9 else 0); ip[97] = (5, rng.integers(0, 2), 0, 0); ip[99] = (rng.random(), rng.integers(1, 37), 0, 0)
    ip[100] = (100, rng.integers(0, 4), 26, 1 if rng.random() < 0.92 else 0)
    ip[101:122] = rng.uniform(-3, 3, (21, 4)); ip[122:143] = rng.uniform(0.6, 1.5, (21, 4))
    g = np.zeros(55 * 55)
    m = rng.random(55 * 55) < 0.35; g[m] = rng.integers(0, 101, m.sum()) + 1000 * rng.integers(0, 4, m.sum()) + 10000 * (rng.random(m.sum()) < 0.3) + 100000 * (rng.random(m.sum()) < 0.1)
    g = np.concatenate([g, np.zeros(-len(g) % 4)]); ip[150:150 + len(g) // 4] = g.reshape(-1, 4)
    cs = np.zeros(48400); m = rng.random(48400) < 0.08; cs[m] = rng.choice([1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14], m.sum())
    cs = np.concatenate([cs, np.zeros(-len(cs) % 5)]).reshape(-1, 5); fv = (cs * (16.0 ** np.arange(5))).sum(1)
    fv = np.concatenate([fv, np.zeros(-len(fv) % 4)]); ip[1000:1000 + len(fv) // 4] = fv.reshape(-1, 4)
    wx = np.zeros(48400); m = rng.random(48400) < 0.3; wx[m] = rng.integers(0, 256, m.sum())
    wx = np.concatenate([wx, np.zeros(-len(wx) % 3)]).reshape(-1, 3); wv = (wx * (256.0 ** np.arange(3))).sum(1)
    wv = np.concatenate([wv, np.zeros(-len(wv) % 4)]); ip[3424:3424 + len(wv) // 4] = wv.reshape(-1, 4)
    ip[3420] = (37594.0, 13335.2998, 702.0, 468.0)
    for k in range(evn):
        cell = rng.integers(0, 48400); ty = rng.integers(1, 6); nb = rng.integers(0, 36)
        ip[64 + k // 4, k % 4] = (ty * 36 + nb) * 48400 + cell
    return ip
e = Emu(SH)
e.CW = set(); e.fine_in = set(); e.fine_cb = set(); e.fine_ip = {}; e.fine_pcs = set(); e.noctl = set(); e.cat = lambda pc, f: 'X'
def tex(slot, c, extra): return texfn.texfn(T, slot, c[0], c[1], c[2])
cb = {0: np.zeros((1027, 4), np.uint32), 13: f2b(np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 1], [0, 0, 1, 0], [0, 0, 0, 0]], F)).reshape(5, 4)}
a0 = np.zeros((1027, 4), F); a0[517:1027, 0] = 2.0; a0[6:262, 0] = 2.0; cb[0] = f2b(a0).reshape(1027, 4)
for k in range(14): cb.setdefault(k, np.zeros((64, 4), np.uint32))
far = '9add' in SH
cases = open(out + '.in', 'wb'); cases.write(struct.pack('<i', N)); ref = []
for it in range(N):
    ip = mkip(); e.ip = {i: tuple(ip[i]) for i in range(8192) if ip[i].any()}
    wx, wz = rng.uniform(40000, 190000, 2).astype(F); nrm = np.array([rng.uniform(-0.6, 0.6), rng.uniform(0.5, 1), rng.uniform(-0.6, 0.6)], F); nrm /= np.linalg.norm(nrm)
    foot = F(rng.choice([3, 10, 25, 60, 150, 400, 1000])); e.dvx = foot; e.dvy = F(-foot * 0.7)
    uv = np.array([wx / F(204800), wz / F(204800)], F); wy = F(rng.uniform(400, 2500))
    if far: inp = {0: f2b(np.array([960.5, 540.5, 0.5, 1], F)), 1: f2b(np.array([0.5, 0.5, 1, 1], F)), 2: f2b(np.array([0, 1, 0, 0], F)), 3: f2b(np.array(list(nrm) + [0], F)), 4: f2b(np.array([uv[0], uv[1], 0, 0], F))}
    else: inp = {0: f2b(np.array([960.5, 540.5, 0.5, 1], F)), 1: f2b(np.array([0.5, 0.5, 1, 1], F)), 2: f2b(np.array([0, 1, 0, 0], F)), 3: f2b(np.array([1, 1, 1, 1], F)),
                 4: f2b(np.array([wx, wy, wz, 1], F)), 5: f2b(np.array([0, 1, 0, 0], F)), 6: f2b(np.array(list(nrm) + [0], F)), 7: f2b(np.array([uv[0], uv[1], 0, 0], F))}
    e.ip[98] = (2, 0, 0, 0); e.uav = {}; o2 = e.run(inp, cb, tex); g = b2f(o2.get(0, np.zeros(4, np.uint32)))[:3]
    e.ip[98] = (0, 0, 0, 0); e.uav = {}; o0 = e.run(inp, cb, tex); r = b2f(o0.get(0, np.zeros(4, np.uint32)))[:3]
    ref.append((r, e.disc))
    for i in range(8192): cases.write(struct.pack('<4f', *ip[i])) if i != 98 else cases.write(struct.pack('<4f', 0, 0, 0, 0))
    if far:   # far: world point = uv * 204800, height from the map * 1.6 (the shader's own main)
        cases.write(struct.pack('<16f', uv[0] * 204800, 0, uv[1] * 204800, *nrm, *uv, foot, -foot * 0.7, *g, 1, 0, 0))
    else: cases.write(struct.pack('<16f', wx, wy, wz, *nrm, *uv, foot, -foot * 0.7, *g, 0, 0, 0))
cases.close()
subprocess.run([os.path.dirname(os.path.abspath(__file__)) + '/xval', out + '.tab', out + '.in', out + '.out'], check=True)
res = np.frombuffer(open(out + '.out', 'rb').read(), '<f4').reshape(N, 8)
bad = 0; md = []; nd = 0
for i, (r, d) in enumerate(ref):
    h = res[i]
    if d or h[7] < 0:
        if not (d and h[7] < 0): bad += 1; print('discard differs', i, d, h[7])
        nd += 1; continue
    dd = np.max(np.abs(h[:3] - r)); md.append(dd)
    if dd > 2e-3: bad += 1; print('case', i, 'emu', r, 'cpp', h[:3], 'season' if h[7] > 0 else 'game')
md = np.array(md)
print('cases %d discarded %d | differ > 2e-3: %d | median diff %.2e  99%% %.2e  max %.2e' % (N, nd, bad, np.median(md), np.quantile(md, 0.99), md.max()))
