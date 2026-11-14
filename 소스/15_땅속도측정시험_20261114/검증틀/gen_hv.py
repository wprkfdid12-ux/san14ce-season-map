# minimal HLSL-like vector types for running the season shader code as C++ (float32, g++ -fsingle-precision-constant)
import itertools
L = 'xyzw'
out = ['#pragma once', '#include <cmath>', '#include <cstdint>', 'typedef unsigned int uint;',
       'struct float2; struct float3; struct float4; struct int2; struct int3; struct int4;']
def vec(name, n, T):
    c = L[:n]
    s = ['struct %s {' % name, '  %s %s;' % (T, ','.join(c)),
         '  %s() : %s {}' % (name, ','.join('%s(0)' % k for k in c)),
         '  %s(%s v) : %s {}' % (name, T, ','.join('%s(v)' % k for k in c)),
         '  %s(%s) : %s {}' % (name, ','.join('%s %s_' % (T, k) for k in c), ','.join('%s(%s_)' % (k, k) for k in c))]
    if n >= 3: s.append('  %s(%s2 a, %s b) : %s {}' % (name, 'float' if T == 'float' else 'int', T, ','.join(['x(a.x)', 'y(a.y)', 'z(b)'] + (['w(0)'] if n == 4 else []))))
    if n == 4:
        s.append('  %s(%s3 a, %s b) : x(a.x),y(a.y),z(a.z),w(b) {}' % (name, 'float' if T == 'float' else 'int', T))
        s.append('  %s(%s2 a, %s2 b) : x(a.x),y(a.y),z(b.x),w(b.y) {}' % (name, 'float' if T == 'float' else 'int', 'float' if T == 'float' else 'int'))
        s.append('  %s(%s2 a, %s b, %s d) : x(a.x),y(a.y),z(b),w(d) {}' % (name, 'float' if T == 'float' else 'int', T, T))
    s.append('  %s& operator[](int i) { return (&x)[i]; }  %s operator[](int i) const { return (&x)[i]; }' % (T, T))
    other = ('int%d' % n) if T == 'float' else ('float%d' % n)
    s.append('  %s%s(%s v);' % ('' if T == 'float' else 'explicit ', name, other))
    # swizzles (read only)
    vt = {1: T, 2: ('float2' if T == 'float' else 'int2'), 3: ('float3' if T == 'float' else 'int3'), 4: ('float4' if T == 'float' else 'int4')}
    for m in (2, 3, 4):
        for comb in itertools.product(c, repeat=m):
            s.append('  inline %s %s() const;' % (vt[m], ''.join(comb)))
    s.append('};')
    return s, vt
defs = []
for name, n, T in (('float2', 2, 'float'), ('float3', 3, 'float'), ('float4', 4, 'float'), ('int2', 2, 'int'), ('int3', 3, 'int'), ('int4', 4, 'int')):
    s, vt = vec(name, n, T); out += s
    for m in (2, 3, 4):
        for comb in itertools.product(L[:n], repeat=m):
            defs.append('inline %s %s::%s() const { return %s(%s); }' % (vt[m], name, ''.join(comb), vt[m], ','.join(comb)))
out += defs
for n in (2, 3, 4):
    c = L[:n]
    out.append('inline float%d::float%d(int%d v) : %s {}' % (n, n, n, ','.join('%s((float)v.%s)' % (k, k) for k in c)))
    out.append('inline int%d::int%d(float%d v) : %s {}' % (n, n, n, ','.join('%s((int)v.%s)' % (k, k) for k in c)))
# conversions between float and int vectors (explicit functions via constructors would need forward decl: use helper functions)
ops = []
for V, n, T in (('float2', 2, 'float'), ('float3', 3, 'float'), ('float4', 4, 'float'), ('int2', 2, 'int'), ('int3', 3, 'int'), ('int4', 4, 'int')):
    c = L[:n]
    for o in '+-*/':
        ops.append('inline %s operator%s(%s a, %s b) { return %s(%s); }' % (V, o, V, V, V, ','.join('a.%s%sb.%s' % (k, o, k) for k in c)))
        ops.append('inline %s operator%s(%s a, %s b) { return %s(%s); }' % (V, o, V, T, V, ','.join('a.%s%sb' % (k, o) for k in c)))
        ops.append('inline %s operator%s(%s a, %s b) { return %s(%s); }' % (V, o, T, V, V, ','.join('a%sb.%s' % (o, k) for k in c)))
        ops.append('inline %s& operator%s=(%s& a, %s b) { a = a %s b; return a; }' % (V, o, V, V, o))
        ops.append('inline %s& operator%s=(%s& a, %s b) { a = a %s b; return a; }' % (V, o, V, T, o))
    ops.append('inline %s operator-(%s a) { return %s(%s); }' % (V, V, V, ','.join('-a.%s' % k for k in c)))
    if T == 'int':
        for o in ('&', '|', '>>', '<<'):
            ops.append('inline %s operator%s(%s a, int b) { return %s(%s); }' % (V, o, V, V, ','.join('(a.%s %s b)' % (k, o) for k in c)))
out += ops
# int <-> float vector conversions
for n in (2, 3, 4):
    c = L[:n]
    out.append('inline float%d tof(int%d a) { return float%d(%s); }' % (n, n, n, ','.join('(float)a.%s' % k for k in c)))
    out.append('inline int%d toi(float%d a) { return int%d(%s); }' % (n, n, n, ','.join('(int)a.%s' % k for k in c)))
# float+int vector mixes (HLSL promotes to float)
for n in (2, 3, 4):
    for o in '+-*/':
        out.append('inline float%d operator%s(float%d a, int%d b) { return a %s tof(b); }' % (n, o, n, n, o))
        out.append('inline float%d operator%s(int%d a, float%d b) { return tof(a) %s b; }' % (n, o, n, n, o))
        out.append('inline float%d operator%s(int%d a, float b) { return tof(a) %s b; }' % (n, o, n, o))
        out.append('inline float%d operator%s(float a, int%d b) { return a %s tof(b); }' % (n, o, n, o))
# intrinsics
F1 = {'abs': 'std::fabs(a)', 'floor': 'std::floor(a)', 'ceil': 'std::ceil(a)', 'frac': '(a - std::floor(a))', 'saturate': '(a < 0.f ? 0.f : (a > 1.f ? 1.f : a))',
      'sqrt': 'std::sqrt(a)', 'exp': 'std::exp2(a * 1.44269504f)', 'exp2': 'std::exp2(a)', 'log2': 'std::log2(a)', 'sin': 'std::sin(a)', 'cos': 'std::cos(a)',
      'round': 'std::nearbyint(a)', 'trunc': 'std::trunc(a)', 'rsqrt': '(1.f / std::sqrt(a))', 'sign': '(a > 0.f ? 1.f : (a < 0.f ? -1.f : 0.f))'}
out.append('inline float pow(float a, float b) { return std::exp2(b * std::log2(a)); }')
for k, e in F1.items():
    out.append('inline float %s(float a) { return %s; }' % (k, e))
    for n in (2, 3, 4):
        c = L[:n]
        out.append('inline float%d %s(float%d v) { return float%d(%s); }' % (n, k, n, n, ','.join('%s(v.%s)' % (k, q) for q in c)))
out.append('inline int abs(int a) { return a < 0 ? -a : a; }')
out.append('inline float lerp(float a, float b, float t) { return a + (b - a) * t; }')
out.append('inline float smoothstep(float a, float b, float x) { float t = saturate((x - a) / (b - a)); return t * t * (3.f - 2.f * t); }')
out.append('inline float step(float a, float x) { return x >= a ? 1.f : 0.f; }')
out.append('inline float clamp(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }')
out.append('inline int clamp(int x, int a, int b) { return x < a ? a : (x > b ? b : x); }')
out.append('inline float min(float a, float b) { return a < b ? a : b; }  inline float max(float a, float b) { return a > b ? a : b; }')
out.append('inline int min(int a, int b) { return a < b ? a : b; }  inline int max(int a, int b) { return a > b ? a : b; }')
out.append('inline float atan2(float y, float x) { return std::atan2(y, x); }')
for n in (2, 3, 4):
    c = L[:n]; V = 'float%d' % n; I = 'int%d' % n
    out.append('inline %s lerp(%s a, %s b, %s t) { return %s(%s); }' % (V, V, V, V, V, ','.join('lerp(a.%s,b.%s,t.%s)' % (q, q, q) for q in c)))
    out.append('inline %s lerp(%s a, %s b, float t) { return %s(%s); }' % (V, V, V, V, ','.join('lerp(a.%s,b.%s,t)' % (q, q) for q in c)))
    out.append('inline %s smoothstep(%s a, %s b, %s x) { return %s(%s); }' % (V, V, V, V, V, ','.join('smoothstep(a.%s,b.%s,x.%s)' % (q, q, q) for q in c)))
    out.append('inline %s clamp(%s x, float a, float b) { return %s(%s); }' % (V, V, V, ','.join('clamp(x.%s,a,b)' % q for q in c)))
    out.append('inline %s clamp(%s x, %s a, %s b) { return %s(%s); }' % (V, V, V, V, V, ','.join('clamp(x.%s,a.%s,b.%s)' % (q, q, q) for q in c)))
    out.append('inline %s clamp(%s x, int a, int b) { return %s(%s); }' % (I, I, I, ','.join('clamp(x.%s,a,b)' % q for q in c)))
    out.append('inline %s min(%s a, %s b) { return %s(%s); }  inline %s max(%s a, %s b) { return %s(%s); }' % (V, V, V, V, ','.join('min(a.%s,b.%s)' % (q, q) for q in c), V, V, V, V, ','.join('max(a.%s,b.%s)' % (q, q) for q in c)))
    out.append('inline %s min(%s a, float b) { return min(a, %s(b)); }  inline %s max(%s a, float b) { return max(a, %s(b)); }' % (V, V, V, V, V, V))
    out.append('inline %s pow(%s a, float b) { return %s(%s); }' % (V, V, V, ','.join('pow(a.%s,b)' % q for q in c)))
    out.append('inline %s pow(%s a, %s b) { return %s(%s); }' % (V, V, V, V, ','.join('pow(a.%s,b.%s)' % (q, q) for q in c)))
    out.append('inline float dot(%s a, %s b) { return %s; }' % (V, V, '+'.join('a.%s*b.%s' % (q, q) for q in c)))
    out.append('inline float length(%s a) { return std::sqrt(dot(a, a)); }' % V)
    out.append('inline %s normalize(%s a) { return a * (1.f / std::sqrt(dot(a, a))); }' % (V, V))
    out.append('inline %s saturate(%s a, int) { return a; }' % (V, V))
    out.append('inline %s step(%s a, %s x) { return %s(%s); }' % (V, V, V, V, ','.join('step(a.%s,x.%s)' % (q, q) for q in c)))
    out.append('inline bool all_ge(%s a, float b) { return %s; }' % (V, ' && '.join('a.%s >= b' % q for q in c)))
out.append('inline float3 cross(float3 a, float3 b) { return float3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x); }')
open('hv.h', 'w').write('\n'.join(out) + '\n')
