# v26k27 (speed): the game's own ground and water shaders replaced by copies that carry the season picture inside them.
#   ground: the game's ground shader (translated instruction by instruction) + the season picture in the same pass, so the ground
#           is drawn once instead of twice (before: the game drew it, then the whole ground was drawn again for the season colours).
#   water:  the winter water (game water + river ice) as the game's own water shader: no extra pass per water piece.
# IniParams 98.x picks the way (for the speed comparison in the test build): 0 new (one pass), 1 old (two passes, custom shaders
# as before - the ini does that), 2 the game alone (no season picture on ground/water).
import re, sys
sys.path.insert(0, '/home/claude/sd')
import dx2hlsl
dx2hlsl.UTEMPS[0] = True   # temporaries hold raw bits (a float temp lets the compiler flush small integer bit patterns as denormals)
V = '/home/claude/season/v26/'
GS = '/home/claude/sd/GameShaders/'

def game_part(h):
    s = dx2hlsl.full_shader(GS + h + '.dxbc')
    decl, body = s.split('\nvoid main(', 1)
    params = body.split(')', 1)[0]
    return decl, 'void game_main(' + body, params

def season_part(src, game_decl):
    t = open(V + src, encoding='utf-8').read()
    t = t[:t.index('\nvoid main(')]
    # resources the game shader already declares at the same register are used under the game's name
    def alias(m):
        ty, nm, reg = m.group(1), m.group(2), m.group(3)
        gname = {'t': 't', 's': 's'}[reg[0]] + reg[1:]
        if re.search(r'\b%s : register\(%s\);' % (gname, reg), game_decl):
            return '#define %s %s' % (nm, gname)
        return m.group(0)
    t = re.sub(r'^(Texture2D<float4>|Texture2D<float>|SamplerState)\s+(\w+)\s*:\s*register\(([ts]\d+)\);', alias, t, flags=re.M)
    # the season picture now runs inside a branch (way 0, not on the sea floor): the screen-space rates of change are taken
    # before the branch (same values: the ground's own position, per pixel) and handed in
    rep = [('    float2 dx = ddx(wxz), dy = ddy(wxz);\n', '    float2 dx = g_dx, dy = g_dy;\n'),
           ('    float dhx = ddx(wy), dhy = ddy(wy);', '    float dhx = g_dhx, dhy = g_dhy;'),
           ('TexVeg.Sample(sWrap, wxz / 4200.0)', 'TexVeg.SampleGrad(sWrap, wxz / 4200.0, g_dx / 4200.0, g_dy / 4200.0)'),
           ('TexSnow.Sample(sWrap, wxz / 3200.0)', 'TexSnow.SampleGrad(sWrap, wxz / 3200.0, g_dx / 3200.0, g_dy / 3200.0)')]
    for a, b in rep:
        assert t.count(a) == 1, a
        t = t.replace(a, b)
    assert 'ddx(' not in t and 'ddy(' not in t and '.Sample(' not in t and '.SampleBias(' not in t
    # v26k29: season() also says whether the pixel's picture moves with time (fire, flood, falling rain on it) - those are never reused
    sig_old = 'void season(float2 uv, float3 nrm, float2 wxz, float h, float wy, out float3 A, out float3 M)\n{\n    A = 0; M = 1;\n'
    assert t.count(sig_old) == 1, 'season signature'
    t = t.replace(sig_old, 'void season(float2 uv, float3 nrm, float2 wxz, float h, float wy, out float3 A, out float3 M, out float dynT)\n{\n    A = 0; M = 1; dynT = 0;\n')
    calls = re.findall(r'(?<!void )\b(drawFire|drawFlood)\(A, M, wp', t)
    assert len(calls) == 4, calls                 # fire / flood from the cell state, and from the picture slots
    t = re.sub(r'(?<!void )\b(drawFire|drawFlood)\(A, M, wp', r'dynT = 1.0; \1(A, M, wp', t)
    rain_old = '    [branch] if (rLevel > 0.5 && inMap > 0.5) {\n        float4 R = R0;'
    assert t.count(rain_old) == 1, 'rain block'
    # with no rain tier (< 0.1) and a dry cell the rain picture adds exactly nothing whatever the time (its time wobble is at most 0.15,
    # the first visible step starts at 0.25 / 0.35); anywhere else the pixel is drawn afresh every frame
    t = t.replace(rain_old, '    if (!(R0.x < 0.1 && R0.y <= 0.0)) dynT = 1.0;          // v26k29: rain falling on this pixel moves with time\n' + rain_old)
    assert t.count('tm') and t.count('dynT = 1.0') == 5
    t = 'static float2 g_dx, g_dy; static float g_dhx, g_dhy;   // v26k27: set by main before the branch\n' + t
    return t

def merged(h, src, uvsem, nrmsem, wpos_expr, wxzE, wyE):
    gdecl, gmain, params = game_part(h)
    sea = season_part(src, gdecl)
    KID = '1' if 'Near' in src else '2'
    POS = [p.strip().split()[-3] for p in params.split(',') if 'SV_Position' in p][0]
    cache = ('// v26k29: last frame\'s season colours per screen pixel (8 numbers: A.rgb, key half 1, M.rgb, key half 2), bound by d3dx.ini\n'
             'RWTexture2DArray<uint> SeasonCache : register(u5);\n'
             'uint2 ck(uint2 k, uint v) { uint a = v * 0xCC9E2D51u; a = (a << 15) | (a >> 17); a *= 0x1B873593u; k.x ^= a; k.x = (k.x << 13) | (k.x >> 19); k.x = k.x * 5u + 0xE6546B64u;\n'
             '                            uint b = (v ^ 0xA5A5A5A5u) * 0x85EBCA77u; b = (b << 17) | (b >> 15); b *= 0xC2B2AE3Du; k.y ^= b; k.y = (k.y << 11) | (k.y >> 21); k.y = k.y * 9u + 0x27D4EB2Fu; return k; }\n'
             'uint fm32(uint h) { h ^= h >> 16; h *= 0x85EBCA6Bu; h ^= h >> 13; h *= 0xC2B2AE35u; h ^= h >> 16; return h; }\n'
             'uint2 cfin(uint2 k) { uint2 r = uint2(fm32(k.x ^ (k.y * 0x9E3779B1u)), fm32(k.y + k.x)); return r | uint2(1u, 1u); }   // never 0 (an empty slot)\n')
    sea = cache + sea
    names = {}
    for p in params.split(','):
        p = p.strip(); m = re.match(r'(out\s+)?(\w+)\s+(\w+)\s*:\s*(\w+)', p)
        if m and not m.group(1): names[m.group(4)] = m.group(3)
    uv = names[uvsem]; nrm = names[nrmsem]
    wp = wpos_expr(names)
    args = ', '.join(re.match(r'(?:out\s+)?\w+\s+(\w+)', p.strip()).group(1) for p in params.split(','))
    main = ('void main(' + params + ')\n{\n'
            '    game_main(' + args + ');\n'
            '    // ---- the season picture, in the same pass (v26k27). Way 0 only; ways 1 and 2 leave the game\'s colour as it is ----\n'
            '    float4 pm98 = IniParams.Load(int2(98, 0));\n'
            '    float2 uv = ' + uv + '.xy;\n'
            '    float h = LandHeight.SampleLevel(sClamp, uv, 0);\n'
            '    float2 wxzK = ' + wxzE(names) + '; float wyK = ' + wyE(names) + ';\n'
            '    g_dx = ddx(wxzK); g_dy = ddy(wxzK); g_dhx = ddx(wyK); g_dhy = ddy(wyK);\n'
            '    [branch] if (pm98.x < 0.5 || pm98.x > 2.5) {   // way 0 (and the speed test\'s way 3: new, rain picture off)\n'
            '        int2 hpSea = clamp(int2(saturate(uv) * 512.0), 0, 511);\n'
            '        float seaG = TexHazard.Load(int3(hpSea, 0)).g;\n'
            '        [branch] if (!(h < 740.0 && seaG < 0.1)) {   // nothing is painted on the sea floor (as before: the game\'s colour stays)\n'
            '            float3 A, M;\n'
            '            // v26k29: the season colours of this pixel are kept from the frame before when every input is the same (the place on\n'
            '            //         the ground, its height, slope and screen size, the month, the turn, the day data from the rules module):\n'
            '            //         then they are exactly what the calculation would give again. Pixels with fire, flood or falling rain are\n'
            '            //         always drawn afresh (they move with time).\n'
            '            float4 ipK0 = IniParams.Load(int2(0, 0)), ipK1 = IniParams.Load(int2(1, 0)), ipK97 = IniParams.Load(int2(97, 0)), ipK60 = IniParams.Load(int2(60, 0));\n'
            '            uint2 key = uint2(0x9747B28Cu, 0x2F6C1A3Du);\n'
            '            key = ck(key, asuint(uv.x)); key = ck(key, asuint(uv.y)); key = ck(key, asuint(' + nrm + '.x)); key = ck(key, asuint(' + nrm + '.y)); key = ck(key, asuint(' + nrm + '.z));\n'
            '            key = ck(key, asuint(wxzK.x)); key = ck(key, asuint(wxzK.y)); key = ck(key, asuint(h)); key = ck(key, asuint(wyK));\n'
            '            key = ck(key, asuint(g_dx.x)); key = ck(key, asuint(g_dx.y)); key = ck(key, asuint(g_dy.x)); key = ck(key, asuint(g_dy.y)); key = ck(key, asuint(g_dhx)); key = ck(key, asuint(g_dhy));\n'
            '            key = ck(key, asuint(ipK0.x)); key = ck(key, asuint(ipK0.y)); key = ck(key, asuint(ipK1.x)); key = ck(key, asuint(ipK97.x)); key = ck(key, asuint(pm98.x)); key = ck(key, ' + KID + 'u);\n'
            '            key = cfin(key);\n'
            '            bool reuseOK = ipK60.w > 0.5;   // only with the direct link (otherwise the disaster slots come from key presses the day count does not see)\n'
            '            uint2 px = uint2(' + POS + '.xy);\n'
            '            bool hit = false;\n'
            '            [branch] if (reuseOK) {\n'
            '                uint a0 = SeasonCache[uint3(px, 0)], a1 = SeasonCache[uint3(px, 1)], a2 = SeasonCache[uint3(px, 2)], a3 = SeasonCache[uint3(px, 3)];\n'
            '                uint m0 = SeasonCache[uint3(px, 4)], m1 = SeasonCache[uint3(px, 5)], m2 = SeasonCache[uint3(px, 6)], m3 = SeasonCache[uint3(px, 7)];\n'
            '                [branch] if (a3 == key.x && m3 == key.y) { A = asfloat(uint3(a0, a1, a2)); M = asfloat(uint3(m0, m1, m2)); hit = true; }\n'
            '            }\n'
            '            [branch] if (!hit) {\n'
            '                float dynT;\n'
            '                season(uv, ' + nrm + '.xyz, ' + wp + ', A, M, dynT);\n'
            '                [branch] if (reuseOK && dynT < 0.5) {\n'
            '                    uint3 ua = asuint(A), um = asuint(M);\n'
            '                    SeasonCache[uint3(px, 0)] = ua.x; SeasonCache[uint3(px, 1)] = ua.y; SeasonCache[uint3(px, 2)] = ua.z; SeasonCache[uint3(px, 3)] = key.x;\n'
            '                    SeasonCache[uint3(px, 4)] = um.x; SeasonCache[uint3(px, 5)] = um.y; SeasonCache[uint3(px, 6)] = um.z; SeasonCache[uint3(px, 7)] = key.y;\n'
            '                }\n'
            '            }\n'
            '            o0.rgb = A + saturate(o0.rgb) * M;   // the same sum the blend unit made before: season + game colour x season factor\n'
            '        }\n'
            '    }\n'
            '}\n')
    head = ('// SAN14 season map v26k27: the game\'s ground shader ' + h + ' (translated instruction by instruction)\n'
            '// with the season picture (' + src + ') in the same pass - the ground is drawn once, not twice.\n')
    return head + gdecl + '\n// ======== season picture ========\n' + sea + '\n// ======== the game\'s shader ========\n' + gmain + '\n' + main

near = merged('d3e1f71e809bac74', 'SeasonTerrainNear.hlsl', 'TEXCOORD6', 'TEXCOORD5',
              lambda n: n['TEXCOORD3'] + '.xz, h, ' + n['TEXCOORD3'] + '.y', lambda n: n['TEXCOORD3'] + '.xz', lambda n: n['TEXCOORD3'] + '.y')
far = merged('9addb6cb884c7a7c', 'SeasonTerrainFar.hlsl', 'TEXCOORD3', 'TEXCOORD2',
             lambda n: 'uv * MAPW, h, h * 1.6', lambda n: 'uv * MAPW', lambda n: 'h * 1.6')
open(V + 'd3e1f71e809bac74-ps_replace.txt', 'w', encoding='utf-8').write(near)
open(V + '9addb6cb884c7a7c-ps_replace.txt', 'w', encoding='utf-8').write(far)

# ---- water: the winter water shader already is the game's water (instruction by instruction) + river ice that only appears in
#      the ice months; as the game's own water shader it needs no extra pass. Way 2 (the game alone) leaves the ice out.
w = open(V + 'SeasonWaterWinter.hlsl', encoding='utf-8').read()
old = "        float mk = (ip.x > 0.5 && ip.y > 0.5 && (rulesW > 0.5 ? iceW > 0.5 : (ip.x < 2.5 || ip.x > 8.5 || prevOn > 0.5))) ? FreezeMask.SampleLevel(s3, muv, 0) : 0.0;"
assert w.count(old) == 1
w = w.replace(old, "        float wayW = IniParams.Load(int2(98, 0)).x;                                   // v26k27: way 2 (the game alone) = no ice\n"
              "        float mk = ((wayW < 1.5 || wayW > 2.5) && ip.x > 0.5 && ip.y > 0.5 && (rulesW > 0.5 ? iceW > 0.5 : (ip.x < 2.5 || ip.x > 8.5 || prevOn > 0.5))) ? FreezeMask.SampleLevel(s3, muv, 0) : 0.0;")
w = w.replace("// SAN14 season map v26: winter water.", "// SAN14 season map v26k27: the game's water shader 425be6c323067e73 replaced by the winter water (was a custom shader per\n// water piece). Outside the ice months the ice part is skipped and this is the game's own water.\n// (from) SAN14 season map v26: winter water.", 1)
open(V + '425be6c323067e73-ps_replace.txt', 'w', encoding='utf-8').write(w)
print('ok', len(near.splitlines()), len(far.splitlines()), len(w.splitlines()))
