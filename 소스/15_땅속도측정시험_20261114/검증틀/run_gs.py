import sys, numpy as np
sys.path.insert(0, sys.argv[1] if len(sys.argv) > 1 else '.')
from emu_t import Emu, f2b, b2f, F
SH = '/home/claude/k52/pkg/game/ShaderFixes/'
def mk(name):
    e = Emu(SH + name + '-ps_replace.bin'); e.far = name.startswith('9add')
    e.CW = {'mov','add','mul','mad','min','max','frc','round_ni','round_pi','round_z','round_ne','lt','ge','eq','ne','ilt','ige','ieq','ine','ult','uge',
            'and','or','xor','not','iadd','ineg','ishl','ushr','ishr','imax','imin','umax','umin','movc','ftoi','ftou','itof','utof','rsq','sqrt','exp','log','div','imad'}
    e.fine_in = {0, 4}          # screen position, world position
    e.fine_cb = {13}            # camera matrix
    e.fine_ip = {2: (0, 1, 0, 0), 100: (1, 0, 0, 0)}   # time (tm), frame counter
    e.fine_pcs = set()
    return e
def ranges(e):
    ins = e.ins
    p98 = next(i for i, I in enumerate(ins) if I['op'] == 'ld' and I['ops'][1]['typ'] == 4 and I['ops'][1]['imm'][0] == 98 and I['ops'][2]['idx'][0][0] == 120)
    u = [i for i, I in enumerate(ins) if I['op'] == 'op163']; st = [i for i, I in enumerate(ins) if I['op'] == 'op164']
    # cache: from the ld of list 0 after the derivatives (key inputs) to the hit test; the stores block
    k0 = next(i for i in range(p98, u[0]) if ins[i]['op'] == 'ld' and ins[i]['ops'][1]['typ'] == 4 and ins[i]['ops'][1]['imm'][0] == 0 and ins[i]['ops'][2]['idx'][0][0] == 120)
    k1 = next(i for i in range(u[-1], len(ins)) if ins[i]['op'] == 'discard')
    s0 = st[0] - 4; s1 = st[-1] + 1
    e.noctl = set(i for i in range(k0, k1) if ins[i]['op'] == 'if')
    if e.far:   # far terrain: uv is the input, the world point is uv * 204800 (marked as the fine source)
        e.fine_in = {0}
        for i in range(p98, k0 + 40):
            I = ins[i]
            if I['op'] in ('mul', 'mad') and any(o['typ'] == 4 and 0x48480000 in o['imm'] for o in I['ops'][1:]): e.fine_pcs.add(i)
        print('  far: world point at', sorted(e.fine_pcs))
    def cat(pc, fine):
        if pc < p98: return 'VAN'
        if k0 + 4 <= pc < k1 - 2 or s0 <= pc <= s1: return 'CACHE'
        if pc < k0 + 4: return 'PRE'
        return 'FINE' if fine & 1 else ('DAY' if fine & 2 else 'STATIC')
    return cat
def scen(over_ip={}, over_tex={}, foot=25.0, h=900.0, slope=0.0, nrm=(0, 1, 0)):
    ip = {0: (7, 1, 0, 0), 1: (1, 0, 0, 0), 2: (0, 100.0, 0, 0), 60: (0, 0, 0, 1), 97: (5, 0, 0, 0), 98: (0, 0, 0, 0), 99: (0.5, 19, 0, 0),
          100: (0, 0, 26, 1), 3420: (37594.0, 13335.2998, 702.0, 468.0)}
    for k in range(122, 143): ip[k] = (1, 1, 1, 1)
    ip.update(over_ip)
    tx = {3: (10/255, 10/255, 0, 1), 113: (0, 0, 0, 0), 109: (25, 0, 0.3, 2), 108: (0, 0, 0, 0), 119: (0, 0, 0, 0), 118: (0.25, 0.25, 0.25, 0.25),
          117: (0.3, 0.27, 0.2, 1), 112: (0.35, 0.35, 0.3, 1), 111: (0.8, 0.8, 0.85, 1), 114: (0.4, 0.38, 0.35, 0.5), 115: (0.5, 0.5, 0.5, 0.5),
          116: (1, 0, 0, 0), 103: (0, 0, 0, 0), 102: (0.5, 0.5, 0.5, 0.5), 107: (1, 0, 0, 0), 106: (0.5, 0.5, 0.5, 0.5), 105: (0.5,) * 4, 104: (0.5,) * 4}
    tx.update(over_tex)
    return ip, tx, foot, h, slope, nrm
def run(e, sc):
    ip, tx, foot, h, slope, nrm = sc
    e.ip = ip; e.dvx = F(foot / 1.4142); e.dvy = F(-foot / 1.4142 * 0.7)
    def tex(slot, c, extra):
        if slot == 110: return np.array([h + slope * float(c[0]), 0, 0, 0], F)
        return np.array(tx.get(slot, (0.5, 0.5, 0.5, 0.5)), F)
    cb = {0: np.zeros((1027, 4), np.uint32), 13: f2b(np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 1], [0, 0, 1, 0], [0, 0, 0, 0]], F)).reshape(5, 4)}
    a0 = np.zeros((1027, 4), F); a0[517:1027, 0] = 2.0; a0[6:262, 0] = 2.0; cb[0] = f2b(a0).reshape(1027, 4)
    for k in range(14): cb.setdefault(k, np.zeros((64, 4), np.uint32))
    wx = 102400.0 + 37.0
    inp = {0: f2b(np.array([960.5, 540.5, 0.5, 1], F)), 1: f2b(np.array([0.5, 0.5, 1, 1], F)), 2: f2b(np.array([0, 1, 0, 0], F)),
           3: f2b(np.array([1, 1, 1, 1], F)), 4: f2b(np.array([wx, h, wx, 1], F)), 5: f2b(np.array([0, 1, 0, 0], F)),
           6: f2b(np.array(list(nrm) + [0], F)), 7: f2b(np.array([wx / 204800, wx / 204800, 0, 0], F))}
    e.uav = {}
    o = e.run(inp, cb, tex)
    return e.cnt, e.disc
if __name__ == '__main__':
    e = mk('d3e1f71e809bac74'); e.cat = ranges(e)
    c, d = run(e, scen())
    print('disc', d); 
    for k in sorted(c): print(k, c[k])
