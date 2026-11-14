import sys, warnings; warnings.filterwarnings('ignore')
sys.path.insert(0, sys.argv[1])
import numpy as np
from run_gs import mk, ranges, scen, run
snowWx = {k: (1052688.0,) * 4 for k in range(3424, 3424 + 4100)}
rainWx = {k: (131586.0,) * 4 for k in range(3424, 3424 + 4100)}
rainG = {k: (2060.0,) * 4 for k in range(150, 150 + 760)}
S = {
 '여름 들판(밭)':      dict(),
 '봄 꽃밭(풀밭)':      dict(over_ip={0: (2, 1, 0, 0)}, over_tex={3: (0, 0, 0, 1), 109: (12, 2.0, 0.2, 0)}),
 '겨울 눈 쌓인 들판':  dict(over_ip={0: (11, 1, 0, 0), **snowWx}, over_tex={3: (0, 0, 0, 1), 109: (-8, -1.0, 0.3, -10), 108: (1, 0, 0, 0)}),
 '바위산(수목한계 위)': dict(h=2800.0, slope=60000.0, nrm=(0.45, 0.89, 0.0), over_tex={113: (0, 0, 1, 0), 3: (0, 0, 0, 1)}),
 '비 오는 들판':       dict(over_ip={100: (0, 2, 26, 1), 97: (5, 1, 0, 0), **rainWx, **rainG}),
 '숲':                 dict(over_tex={119: (1, 0, 0, 0), 3: (7/255, 7/255, 0, 1)}),
}
R = []
for shader in sys.argv[2:]:
    e = mk(shader); e.cat = ranges(e)
    print('==', shader, '(가까이)' if shader.startswith('d3e1') else '(멀리)')
    for foot in (25.0, 150.0)[:1]:
        for name, kw in S.items():
            c, d = run(e, scen(foot=foot, **kw))
            g = lambda cat, k: c.get((cat, k), 0)
            van = sum(g('VAN', k) for k in ('alu', 'tex', 'cf'))
            pre = sum(g('PRE', k) for k in ('alu', 'tex', 'cf'))
            cac = sum(g('CACHE', k) for k in ('alu', 'tex', 'cf'))
            stt = sum(g('STATIC', k) for k in ('alu', 'tex', 'cf')); dy = sum(g('DAY', k) for k in ('alu', 'tex', 'cf')); fi = sum(g('FINE', k) for k in ('alu', 'tex', 'cf'))
            tot = van + pre + cac + stt + dy + fi
            print('  foot %4.0f %-11s 합계 %5d | 순정 %4d 앞 %3d 재사용 %3d(t%2d) | 고정 %4d(t%2d) 날마다 %4d(t%2d) 매프레임 %4d(t%2d)' % (
                foot, name, tot, van, pre, cac, g('CACHE','tex'), stt, g('STATIC','tex'), dy, g('DAY','tex'), fi, g('FINE','tex')))
            R.append((name, foot, tot, van, pre, cac, stt, dy, fi))

import json; json.dump(R, open('res.json', 'w'), ensure_ascii=False)
