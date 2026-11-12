#!/usr/bin/env python3
# v26k48: 계절 셰이더가 큰 지도(맵 확장 137: 4097, 중원이 행 +1170 = 세계 z +117000)를 따라가게 고치는 도구.
#  - 칸 원점(목록 3420, 계절 규칙 DLL 이 게임에서 읽음)이 순정보다 옮겨진 만큼(SH)을 빼서 계절 그림은 순정 좌표로 계산
#  - 게임 땅 높이(LandHeight)·재질(LandControl)은 게임 지도 크기(SAN_MW)로 다시 맞춰 읽음
#  - 순정 지도 밖(새 땅)은 계절 그림 없이 게임 그림 그대로
#  - 맵 확장이 없으면(옮김 0, 지도 2049) 계산 경로가 예전과 똑같음 (SAN_ON = false)
#  사용: san_shift.py <ShaderFixes 폴더>
import re, sys, os

D = sys.argv[1]

def rd(n): return open(os.path.join(D, n), encoding='utf-8', newline='').read()
def wr(n, s): open(os.path.join(D, n), 'w', encoding='utf-8', newline='').write(s)
def nl(s): return '\r\n' if '\r\n' in s else '\n'

def once(s, old, new, name):
    c = s.count(old)
    if c != 1: raise SystemExit('%s: %d 개 (1 개여야 함): %r' % (name, c, old[:80]))
    return s.replace(old, new)

SH_CALC = ('(g.z > 500.0 && g.z < 900.0 && g.w > 300.0 && g.w < 600.0) ? float2(g.x - 37594.0, g.y - 13335.2998) : float2(0.0, 0.0)')

# ---------- 땅 셰이더 (같은 판 가까이/멀리, 1방식 가까이/멀리) ----------
TERR_BLOCK = '''
// v26k48: a map mod that makes the map bigger (맵 확장 137: 4097 samples, the old land moved down by 1170 samples = +117000 in z)
//         moves the game's hex grid by the same amount (list 3420 = the grid the rules module reads from the game). The season
//         picture is worked out in the old map's place (SAN_SH taken off the world point, the season maps are the old map's);
//         the game's own maps (land height, control map) are read at the game's size (SAN_MW). Land outside the old map keeps the
//         game's own picture. With no such mod the shift is 0 and the map 204800 wide: SAN_ON stays false, every path as before.
static float2 SAN_SH = float2(0.0, 0.0); static float SAN_MW = 204800.0; static bool SAN_ON = false; static bool SAN_SKIP = false; static int SAN_LC = 2048;
void sanInit()
{
    float4 g = IniParams.Load(int2(3420, 0));
    bool gok = g.z > 500.0 && g.z < 900.0 && g.w > 300.0 && g.w < 600.0;
    SAN_SH = %s;
    if (abs(SAN_SH.x) < 1.0 && abs(SAN_SH.y) < 1.0) SAN_SH = float2(0.0, 0.0);
    uint lw, lh; LandHeight.GetDimensions(lw, lh);
    SAN_MW = lw < 3000u ? 204800.0 : (float)(lw - 1u) * 100.0;
    SAN_ON = SAN_SH.x != 0.0 || SAN_SH.y != 0.0 || SAN_MW != MAPW;
    SAN_SKIP = !gok && lw >= 3000u;                             // a bigger map but the grid not known yet: draw nothing rather than misplace it
    uint cw0, ch0; LandControl.GetDimensions(cw0, ch0);
    SAN_LC = SAN_ON ? (int)cw0 - 1 : 2048;
}
float landS(float2 u) { return LandHeight.SampleLevel(sClamp, SAN_ON ? (u * MAPW + SAN_SH) / SAN_MW : u, 0); }   // old-map uv -> the game's height map
''' % SH_CALC

def terr_common(s, name):
    n = nl(s)
    s = once(s, 'static const float MAPW = 204800.0;' + n,
             'static const float MAPW = 204800.0;' + n + TERR_BLOCK.replace('\n', n), name)
    # 칸 원점: 계절 그림은 순정 자리에서 계산
    s = once(s, 'if (!(g.z > 500.0 && g.z < 900.0 && g.w > 300.0 && g.w < 600.0)) g = float4(37594.0, 13335.2998, 702.0, 468.0);' + n,
             'if (!(g.z > 500.0 && g.z < 900.0 && g.w > 300.0 && g.w < 600.0)) g = float4(37594.0, 13335.2998, 702.0, 468.0);' + n +
             '    g.xy -= SAN_SH;                                   // v26k48: in the old map\'s place (0 without a map mod)' + n, name)
    # 재질 지도(LandControl): 게임 지도 크기로
    s = once(s, '    float2 t = uv * 2048.0;' + n,
             '    float2 t = SAN_ON ? (uv * MAPW + SAN_SH) * 0.01 : uv * 2048.0;   // v26k48: the game\'s control map (100 world units a texel)' + n, name)
    s = once(s, 'LandControl.Load(int3(clamp(b + o, 0, 2048), 0))', 'LandControl.Load(int3(clamp(b + o, 0, SAN_LC), 0))', name)
    # 땅 높이: 계절 코드 안의 것만 landS 로 (main 의 첫 읽기는 게임 uv 그대로)
    cut = s.find('void game_main(')
    m = s.find('void main(')
    if cut < 0 or (m >= 0 and m < cut): cut = m
    head, tail = s[:cut], s[cut:]
    out = []; i = 0; k = 0
    key = 'LandHeight.SampleLevel(sClamp, '
    defpos = head.find('float landS(float2 u)')
    while True:
        j = head.find(key, i)
        if j < 0: out.append(head[i:]); break
        if defpos <= j < defpos + 200:   # landS 자신
            out.append(head[i:j + len(key)]); i = j + len(key); continue
        # 괄호 짝 찾기: LandHeight.SampleLevel( ... )
        p = j + len('LandHeight.SampleLevel('); depth = 1; q = p
        while depth:
            ch = head[q]
            if ch == '(': depth += 1
            elif ch == ')': depth -= 1
            q += 1
        inner = head[j + len(key):q - 1]
        if not inner.endswith(', 0'): raise SystemExit('%s: 예상과 다른 높이 읽기: %r' % (name, inner))
        out.append(head[i:j]); out.append('landS(' + inner[:-3] + ')'); i = q; k += 1
    head = ''.join(out)
    if 'LandHeight.SampleLevel' in head.replace('float landS(float2 u) { return LandHeight.SampleLevel', ''):
        raise SystemExit(name + ': 남은 높이 읽기')
    return head + tail, k

def patch_combined(name, near):
    s = rd(name); n = nl(s)
    s, k = terr_common(s, name)
    if near:
        s = once(s, '    float2 uv = v7.xy;' + n + '    float h = LandHeight.SampleLevel(sClamp, uv, 0);' + n + '    float2 wxzK = v4.xz; float wyK = v4.y;' + n,
                 '    sanInit();                                        // v26k48: a bigger map / the old land moved (0 and the old size without a map mod)' + n +
                 '    float2 uv = v7.xy;' + n + '    float h = LandHeight.SampleLevel(sClamp, uv, 0);' + n +
                 '    [flatten] if (SAN_ON) uv = (uv * SAN_MW - SAN_SH) / MAPW;   // the old map\'s uv (the season maps are the old map\'s)' + n +
                 '    float2 wxzK = v4.xz - SAN_SH; float wyK = v4.y;' + n +
                 '    bool sanIn = !SAN_ON || (!SAN_SKIP && all(uv >= 0.0) && all(uv <= 1.0));   // new land: the game\'s own picture' + n, name)
        s = once(s, 'season(uv, v6.xyz, v4.xz, h, v4.y, A, M, dynT);', 'season(uv, v6.xyz, wxzK, h, v4.y, A, M, dynT);', name)
    else:
        s = once(s, '    float2 uv = v4.xy;' + n + '    float h = LandHeight.SampleLevel(sClamp, uv, 0);' + n + '    float2 wxzK = uv * MAPW; float wyK = h * 1.6;' + n,
                 '    sanInit();                                        // v26k48: a bigger map / the old land moved (0 and the old size without a map mod)' + n +
                 '    float2 uv = v4.xy;' + n + '    float h = LandHeight.SampleLevel(sClamp, uv, 0);' + n +
                 '    [flatten] if (SAN_ON) uv = (uv * SAN_MW - SAN_SH) / MAPW;   // the old map\'s uv (the season maps are the old map\'s)' + n +
                 '    float2 wxzK = uv * MAPW; float wyK = h * 1.6;' + n +
                 '    bool sanIn = !SAN_ON || (!SAN_SKIP && all(uv >= 0.0) && all(uv <= 1.0));   // new land: the game\'s own picture' + n, name)
    s = once(s, '    [branch] if (pm98.x < 0.5 || pm98.x > 2.5) {   // way 0', '    [branch] if (sanIn && (pm98.x < 0.5 || pm98.x > 2.5)) {   // way 0', name)
    wr(name, s); return k

def patch_way1(name, near):
    s = rd(name); n = nl(s)
    s, k = terr_common(s, name)
    s = once(s, '    float h = LandHeight.SampleLevel(sClamp, uv, 0);' + n + '    // v26: nothing is painted on the sea floor',
             '    float h = LandHeight.SampleLevel(sClamp, uv, 0);' + n +
             '    sanInit();                                        // v26k48: a bigger map / the old land moved' + n +
             '    [flatten] if (SAN_ON) uv = (uv * SAN_MW - SAN_SH) / MAPW;' + n +
             '    if (SAN_ON && (SAN_SKIP || any(uv < 0.0) || any(uv > 1.0))) discard;   // new land: the game\'s own picture' + n +
             '    // v26: nothing is painted on the sea floor', name)
    if near: s = once(s, 'season(uv, nrm, wpos.xz, h, wpos.y, A, M);', 'season(uv, nrm, wpos.xz - SAN_SH, h, wpos.y, A, M);', name)
    else: s = once(s, 'season(uv, nrm, uv * MAPW, h, h * 1.6, A, M);', 'season(uv, nrm, uv * MAPW, h, h * 1.6, A, M);', name)
    wr(name, s); return k

# ---------- 옮김만 필요한 셰이더: 도우미 ----------
SHF = '''float2 sanSH()                                       // v26k48: how far a map mod moved the old land (list 3420 against the game's own grid; 0 without one)
{
    float4 g = IniParams.Load(int2(3420, 0));
    float2 s = %s;
    return (abs(s.x) < 1.0 && abs(s.y) < 1.0) ? float2(0.0, 0.0) : s;
}
''' % SH_CALC

def patch_rain(name):
    s = rd(name); n = nl(s)
    s = once(s, '    CW_ = g.z; CH_ = g.w * 1.7320508; C0_ = float2(g.x, g.y + CH_ * 0.5);' + n + '}' + n,
             '    CW_ = g.z; CH_ = g.w * 1.7320508; C0_ = float2(g.x, g.y + CH_ * 0.5);' + n + '}' + n + SHF.replace('\n', n), name)
    if 'if (wp.x < 0.0 || wp.x > MAPW || wp.y < 0.0 || wp.y > MAPW) discard;' in s:
        s = once(s, 'if (wp.x < 0.0 || wp.x > MAPW || wp.y < 0.0 || wp.y > MAPW) discard;',
                 '{ float2 wpV = wp - sanSH(); if (wpV.x < 0.0 || wpV.x > MAPW || wpV.y < 0.0 || wpV.y > MAPW) discard; }   // v26k48: the old map\'s place', name)
    else:
        s = once(s, 'if (PT.x < 0.0 || PT.x > MAPW || PT.z < 0.0 || PT.z > MAPW) {',
                 'float2 PTV = PT.xz - sanSH(); if (PTV.x < 0.0 || PTV.x > MAPW || PTV.y < 0.0 || PTV.y > MAPW) {   // v26k48: the old map\'s place' + n + '   ', name)
    wr(name, s)

def patch_water(name):
    s = rd(name); n = nl(s)
    s = once(s, '    CW_ = g.z; CH_ = g.w * 1.7320508; C0_ = float2(g.x, g.y + CH_ * 0.5);' + n + '}' + n,
             '    CW_ = g.z; CH_ = g.w * 1.7320508; C0_ = float2(g.x, g.y + CH_ * 0.5);' + n + '}' + n + SHF.replace('\n', n), name)
    s = once(s, '        float2 muv = wp.xz / 204800.0;' + n,
             '        float2 sanS = sanSH(); float2 muv = (wp.xz - sanS) / 204800.0;   // v26k48: the old map\'s place (the grid below stays the game\'s)' + n +
             '        bool sanIn = (sanS.x == 0.0 && sanS.y == 0.0) || (all(muv >= 0.0) && all(muv <= 1.0));   // new land: the game\'s water' + n, name)
    if 'float mk = ((wayW < 1.5' in s: s = once(s, 'float mk = ((wayW < 1.5 || wayW > 2.5) && ip.x > 0.5', 'float mk = (sanIn && (wayW < 1.5 || wayW > 2.5) && ip.x > 0.5', name)
    else: s = once(s, 'float mk = (ip.x > 0.5 && ip.y > 0.5 && (rulesW', 'float mk = (sanIn && ip.x > 0.5 && ip.y > 0.5 && (rulesW', name)
    s = once(s, 'float bed = LandHeightW.SampleLevel(sWrapW, Pb.xz / 204800.0, 0) * 1.6;',
             'float bed = LandHeightW.SampleLevel(sWrapW, Pb.xz / (hw0 < 3000u ? 204800.0 : (float)(hw0 - 1u) * 100.0), 0) * 1.6;   // v26k48: the game\'s map size', name)
    wr(name, s)

def patch_tree(name, colour):
    s = rd(name); n = nl(s)
    anchor = '// world position of a pixel: ndc = (x, y, depth) ; clip = [world,1] * M'
    s = once(s, anchor, SHF.replace('\n', n) + anchor, name)
    s = once(s, '    float2 uv = saturate(w.xz / 204800.0);', '    float2 uv = saturate((w.xz - sanSH()) / 204800.0);   // v26k48: the old map\'s place', name)
    if colour:
        old = ('    if (tree_active() && ip.x > 0.5 && ip.y > 0.5 && ip.z > 0.5 && ip.w > 0.5) {' + n +
               '        float3 ndc = float3(pos.x / ip.z * 2.0 - 1.0, 1.0 - pos.y / ip.w * 2.0, pos.z);' + n +
               '        float3 w = tree_world(ndc);' + n)
        new = ('    bool sanTr = tree_active() && ip.x > 0.5 && ip.y > 0.5 && ip.z > 0.5 && ip.w > 0.5; float3 sanW = float3(0.0, 0.0, 0.0);' + n +
               '    [branch] if (sanTr) {                       // v26k48: trees on land outside the old map keep the game\'s look' + n +
               '        sanW = tree_world(float3(pos.x / ip.z * 2.0 - 1.0, 1.0 - pos.y / ip.w * 2.0, pos.z)); float2 sanS = sanSH(), sanQ = (sanW.xz - sanS) / 204800.0;' + n +
               '        sanTr = (sanS.x == 0.0 && sanS.y == 0.0) || (all(sanQ >= 0.0) && all(sanQ <= 1.0)); }' + n +
               '    if (sanTr) {' + n +
               '        float3 ndc = float3(pos.x / ip.z * 2.0 - 1.0, 1.0 - pos.y / ip.w * 2.0, pos.z);' + n +
               '        float3 w = sanW;' + n)
    else:
        old = ('    if (tree_active() && ip.x > 0.5 && ip.y > 0.5 && 0 < cb0[1].x) {' + n +
               '        float3 w = tree_world(float3(0.0, 0.0, 0.5));' + n)
        new = ('    bool sanTr = tree_active() && ip.x > 0.5 && ip.y > 0.5 && 0 < cb0[1].x; float3 sanW = float3(0.0, 0.0, 0.0);' + n +
               '    [branch] if (sanTr) {                       // v26k48: trees on land outside the old map keep the game\'s look' + n +
               '        sanW = tree_world(float3(0.0, 0.0, 0.5)); float2 sanS = sanSH(), sanQ = (sanW.xz - sanS) / 204800.0;' + n +
               '        sanTr = (sanS.x == 0.0 && sanS.y == 0.0) || (all(sanQ >= 0.0) && all(sanQ <= 1.0)); }' + n +
               '    if (sanTr) {' + n +
               '        float3 w = sanW;' + n)
    s = once(s, old, new, name)
    wr(name, s)

def patch_vs(name):
    s = rd(name); n = nl(s)
    anchor = 'float sr_hash21(float2 p)'
    s = once(s, anchor, SHF.replace('\n', n) + anchor, name)
    old = ('        b0 = sr_bump(wpos.xz);' + n)
    new = ('        float2 sanP = wpos.xz - sanSH();              // v26k48: the disasters are on the old map\'s cells' + n + '        b0 = sr_bump(sanP);' + n)
    s = once(s, old, new, name)
    s = once(s, 'bx0 = sr_bump(wpos.xz - float2(ex, 0)); bx1 = sr_bump(wpos.xz + float2(ex, 0));', 'bx0 = sr_bump(sanP - float2(ex, 0)); bx1 = sr_bump(sanP + float2(ex, 0));', name)
    s = once(s, 'bz0 = sr_bump(wpos.xz - float2(0, ez)); bz1 = sr_bump(wpos.xz + float2(0, ez));', 'bz0 = sr_bump(sanP - float2(0, ez)); bz1 = sr_bump(sanP + float2(0, ez));', name)
    wr(name, s)

def patch_territory(name):
    s = rd(name); n = nl(s)
    anchor = 'Texture2D<float4> TexForest : register(t119);'
    s = once(s, anchor, 'Texture1D<float4> IniParams : register(t120);              // v26k48: the grid the rules module reads from the game' + n + SHF.replace('\n', n) + anchor, name)
    s = once(s, 'float sanSk = swampAt(sanWorld);', 'float sanSk = swampAt(sanWorld - sanSH());', name)
    wr(name, s)

r = {}
r['d3e1'] = patch_combined('d3e1f71e809bac74-ps_replace.txt', True)
r['9add'] = patch_combined('9addb6cb884c7a7c-ps_replace.txt', False)
r['TNear'] = patch_way1('SeasonTerrainNear.hlsl', True)
r['TFar'] = patch_way1('SeasonTerrainFar.hlsl', False)
for f in ('SeasonRain.hlsl', 'SeasonRainB.hlsl', 'SeasonRainDrops.hlsl', 'SeasonRainDropsB.hlsl'): patch_rain(f)
for f in ('425be6c323067e73-ps_replace.txt', 'SeasonWaterWinter.hlsl'): patch_water(f)
patch_tree('8f8964766ab0977e-ps_replace.txt', True)
patch_tree('d28d1d5e4d8d3a3c-ps_replace.txt', False)
patch_vs('c9a4e0386624d4d2-vs_replace.txt'); patch_vs('fbe6dbf68635f4f4-vs_replace.txt')
patch_territory('48bed74946919969-ps_replace.txt')
print('높이 읽기 바꾼 수', r)
