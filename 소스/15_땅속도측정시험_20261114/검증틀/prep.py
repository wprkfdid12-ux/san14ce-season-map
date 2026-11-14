# HLSL season code -> C++ (with hv.h / tex.h). usage: prep.py in.hlsl out.inc
import re, sys
s = open(sys.argv[1]).read()
s = re.sub(r'//[^\n]*', '', s)                                  # comments
s = s.replace('\\\n', ' ')                                        # macro continuations
lines = []
for ln in s.split('\n'):
    if re.match(r'\s*(Texture1D|Texture2D|Texture2DArray|SamplerState|RWTexture2DArray|cbuffer)\b', ln): continue
    if re.match(r'\s*#define\s+(LandControl|sClamp|sWrap)\b', ln): continue
    lines.append(ln)
s = '\n'.join(lines)
s = re.sub(r'\[(branch|unroll|flatten|loop)\]', '', s)
s = re.sub(r'\b(out|inout)\s+(float[234]?|int[234]?|uint|bool)\s+(\w+)', r'\2 &\3', s)
s = re.sub(r'\bdiscard\s*;', 'throw 1;', s)
s = s.replace('g.xy -= SAN_SH;', 'g.x -= SAN_SH.x; g.y -= SAN_SH.y;')
s = re.sub(r'dot\(A, float3\(0\.3, 0\.59, 0\.11\)\)\.xxx', 'float3(dot(A, float3(0.3, 0.59, 0.11)))', s)
s = re.sub(r'(?<![\w.])(\d+\.\d*)\.(x{2,4})\b', lambda m: 'float%d(%s)' % (len(m.group(2)), m.group(1)), s)
MAP = str.maketrans('rgba', 'xyzw')
def sw(m):
    t = m.group(1).translate(MAP)
    return '.' + t if len(t) == 1 else '.' + t + '()'
s = re.sub(r'(?<=[\w\)\]])\.([xyzwrgba]{1,4})\b(?!\s*\()', sw, s)
open(sys.argv[2], 'w').write(s)
