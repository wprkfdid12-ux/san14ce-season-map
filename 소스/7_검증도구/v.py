# v26k38b2: the cloud over the rain cells only, fixed in the world. Old (k38h) vs new run in the emulator at 3-4 camera heights.
import sys, numpy as np
sys.path.insert(0, '/home/claude/sd')
from dxemu_cnt import Emu, f2b, b2f, F
from eqcheck import cbsizes, load
from PIL import Image
OLD = '/home/claude/season/v26/bincache/2ca197d6d44ecc0a1d047b2c8ddb837f328d73aa6c5915f13c77e4395ce6b703__SeasonRain.ps_5_0.8000.bin'
NEW = '/tmp/claude-0/shgG/ShaderFixes/SeasonRain.ps_5_0.8000.bin'
scene = np.asarray(Image.open('/mnt/user-data/attachments/8ca0dd50-97c9-5a30-87e3-16748ceb4d31/68022707-image.png').convert('RGB').resize((1920, 1080))).astype(np.float32) / 255 * 12.0
def persp(eye, at):
    f = at - eye; f /= np.linalg.norm(f); up = np.array([0, 1.0, 0]); s = np.cross(f, up); s /= np.linalg.norm(s); u = np.cross(s, f)
    V = np.eye(4); V[0, :3] = s; V[1, :3] = u; V[2, :3] = -f; V[:3, 3] = -V[:3, :3] @ eye
    n, fa = 100.0, 400000.0; t = 1 / np.tan(np.radians(25)); a = 16 / 9
    P = np.array([[t / a, 0, 0, 0], [0, t, 0, 0], [0, 0, n / (fa - n), n * fa / (fa - n)], [0, 0, -1, 0]])
    return P @ V
CW, R = 702.0, 468.0; CH = R * 1.7320508; C0 = np.array([37594.0, 13335.2998 + CH * 0.5])
def tier_at(x, z):
    d = np.hypot((x - 100000.0) / 6000.0, (z - 95000.0) / 4500.0)
    return 3 if d < 0.5 else 2 if d < 0.8 else 1 if d < 1.0 else 0
cells = np.zeros(48400, np.int64); cen = np.zeros((48400, 2))
for row in range(220):
    for col in range(220):
        c = C0 + np.array([col * CW, (row - 0.5 * (col % 2)) * CH]); cen[row * 220 + col] = c; cells[row * 220 + col] = tier_at(*c)
rainC = cen[cells > 0]
def dist_rain(p): return np.sqrt(((rainC - p) ** 2).sum(1)).min()
def cell_at(p):
    d = ((cen - p) ** 2).sum(1); return int(np.argmin(d))
def cov_at(p):
    o1 = np.array([CW, CH]) * 0.35; o2 = np.array([CW, -CH]) * 0.35
    t = [p, p, p + o1, p - o1, p + o2, p - o2]
    return np.mean([1.0 if cells[cell_at(q)] > 0 else 0.0 for q in t])
ip = {0: (5.0, 1.0, 1920, 1080), 2: (0, 37.3, 0, 0), 60: (7171.0, 0, 0, 1.0), 100: (5.0, 3.0, 26.0, 1.0), 3420: (37594.0, 13335.2998, 702.0, 468.0)}
packed = np.zeros(48400 // 3 + 1)
for q in range(len(packed)):
    packed[q] = sum(int(cells[q * 3 + k]) << (8 * k) for k in range(3) if q * 3 + k < 48400)
for e in range(len(packed) // 4 + 1): ip[3424 + e] = tuple(float(packed[e * 4 + k]) if e * 4 + k < len(packed) else 0.0 for k in range(4))
grid = np.zeros(55 * 55)          # the rules' 4x4 grid: max tier of each block
for j in range(55):
    for i in range(55):
        m = 0
        for row in range(j * 4, j * 4 + 4):
            for col in range(i * 4, i * 4 + 4):
                if row < 220 and col < 220: m = max(m, cells[row * 220 + col])
        grid[j * 55 + i] = m * 1000 + 60
for q in range(757): ip[150 + q] = tuple(float(grid[q * 4 + k]) if q * 4 + k < 3025 else 0.0 for k in range(4))
progs = {'old': load(OLD), 'new': load(NEW)}
step = int(sys.argv[1]) if len(sys.argv) > 1 else 48
out = {}
for EY in (6428.0, 12374.0, 25981.0, 55000.0):
    CAMH = EY - 1250.0
    eye = np.array([100000.0, EY, 80000.0]); at = eye + np.array([0.0, -CAMH, CAMH * 0.75])
    M = persp(eye, at); Mrow = M.T; Minv = np.linalg.inv(Mrow)
    def ray(px, py):
        ndc = np.array([px / 1920 * 2 - 1, 1 - py / 1080 * 2])
        p0 = np.array([ndc[0], ndc[1], 1.0, 1.0]) @ Minv; p0 = p0[:3] / p0[3]
        p1 = np.array([ndc[0], ndc[1], 0.0001, 1.0]) @ Minv; p1 = p1[:3] / p1[3]
        d = p1 - p0; tt = (1250.0 - p0[1]) / d[1]; w = p0 + tt * d; cl = np.append(w, 1.0) @ Mrow
        return w, cl[2] / cl[3]
    def tex(slot, c, extra):
        x, y = int(c[0]), int(c[1])
        if slot == 4: return np.array([ray(x + 0.5, y + 0.5)[1], 0, 0, 0], F)
        if slot == 101: x = min(max(x, 0), 1919); y = min(max(y, 0), 1079); return np.array([*scene[y, x], 1.0], F)
        return np.zeros(4, F)
    for nm, P in progs.items():
        sz = cbsizes(P); cb0 = np.zeros((max(sz.get(0, 32), 32), 4), np.float32); cb0[13, :3] = eye
        for r in range(4): cb0[23 + r] = Minv[r]
        cb = {k: np.zeros((m, 4), np.uint32) for k, m in sz.items()}; cb[0] = f2b(cb0[:sz[0]]).reshape(-1, 4)
        E = Emu(P); E.ip = dict(ip); sig = E.sig['ISGN']
        pos_r = [r for n, s, r, m, sv in sig if n == 'SV_Position'][0]; tc_r = [r for n, s, r, m, sv in sig if n == 'TEXCOORD'][0]
        rows = []; steps = 0
        for py in range(step // 2, 1080, step):
            for px in range(step // 2, 1920, step):
                w, dep = ray(px + 0.5, py + 0.5)
                if not (0 < w[0] < 204800 and 0 < w[2] < 204800): continue
                inp = {pos_r: f2b(np.array([px + 0.5, py + 0.5, 0, 1], np.float32)), tc_r: f2b(np.array([px / 1920 * 2 - 1, 1 - py / 1080 * 2, 0, 1], np.float32))}
                o = E.run(inp, cb, tex); steps += E.steps
                a = 0.0 if E.disc else float(b2f(o[0])[3])
                # where the line of sight crosses the fixed sheet (5750), and the ground column
                Pg = w[[0, 2]]; toE = eye - w; t = (5750.0 - 1250.0) / toE[1]; Qc = (w + toE * t)[[0, 2]]
                colmin = min(dist_rain(Pg + (Qc - Pg) * f) for f in np.linspace(0, 1, 9))
                rows.append((px, py, a, dist_rain(Qc), cov_at(Qc), colmin, Qc[0], Qc[1]))
        out[(EY, nm)] = (np.array(rows), steps)
        r = np.array(rows)
        inn = r[r[:, 4] >= 0.999]; far = r[(r[:, 3] > 900) & (r[:, 5] > 900)]; colOnly = r[(r[:, 3] > 900) & (r[:, 5] <= 900)]
        f = lambda x: ('n=%d min %.2f med %.2f max %.2f' % (len(x), x[:, 2].min(), np.median(x[:, 2]), x[:, 2].max())) if len(x) else 'n=0'
        print('E.y %6d %s | sheet over rain cell: %s | clear (no rain under sheet nor column): %s | holes(<0.25) %d | clear but rain column below: %s | instr %d'
              % (EY, nm, f(inn), f(far), (inn[:, 2] < 0.25).sum() if len(inn) else 0, f(colOnly), steps), flush=True)
np.save('/tmp/claude-0/k38b2/v_out.npy', out, allow_pickle=True)
