// SAN14 season map v26: winter water. Used only in the months with river ice (9-2월) as a custom shader in place of the
// game's water shader 425be6c323067e73 (translated instruction by instruction) plus ice on northern rivers.
// 3-8월 the game draws its own water untouched.
SamplerState s0 : register(s0);
SamplerState s2 : register(s2);
SamplerState s3 : register(s3);
SamplerState s4 : register(s4);
Texture3D<float4> t0 : register(t0);
Texture2D<float4> t2 : register(t2);
Texture2D<float4> t3 : register(t3);
Texture2DArray<float4> t4 : register(t4);
cbuffer cb0 : register(b0) { float4 cb0[54]; }

// ---- SAN14 season map additions ----
Texture1D<float4> IniParams : register(t120);
// v26k30: the game's own hex grid (list 3420: x = first column's x, y = z origin, z = column step, w = hex radius - read from the
//         game by the rules module; the game's own numbers until then). Centre of cell (col,row): x = X0 + col*CW, z = Z0 + row*H +
//         (even column ? H/2 : 0), H = sqrt(3)*R; written below as C0 + (col*CW, (row - 0.5*odd)*CH) with C0 = (X0, Z0 + H/2).
//         (The grid used before - 700 x 808.29 from (37550, 13850) - drifted up to 0.7 of a cell off the game's toward the east and south.)
static float CW_ = 702.0, CH_ = 810.6002;
static float2 C0_ = float2(37594.0, 13740.6);
void gridInit()
{
    float4 g = IniParams.Load(int2(3420, 0));
    if (!(g.z > 500.0 && g.z < 900.0 && g.w > 300.0 && g.w < 600.0)) g = float4(37594.0, 13335.2998, 702.0, 468.0);
    CW_ = g.z; CH_ = g.w * 1.7320508; C0_ = float2(g.x, g.y + CH_ * 0.5);
}
float2 sanSH()                                       // v26k48: how far a map mod moved the old land (list 3420 against the game's own grid; 0 without one)
{
    float4 g = IniParams.Load(int2(3420, 0));
    float2 s = (g.z > 500.0 && g.z < 900.0 && g.w > 300.0 && g.w < 600.0) ? float2(g.x - 37594.0, g.y - 13335.2998) : float2(0.0, 0.0);
    return (abs(s.x) < 1.0 && abs(s.y) < 1.0) ? float2(0.0, 0.0) : s;
}
float hexIdxAt(float2 wp)                            // the cell under a world point (the game's nearest-centre rule), -1 off the map
{
    float c0 = floor((wp.x - C0_.x) / CW_); float bestD = 1e12, best = -1.0;
    [unroll] for (int kq = 0; kq < 2; kq++) {
        float col1 = c0 + kq; float codd = col1 - floor(col1 * 0.5) * 2.0;
        float row1 = round((wp.y - C0_.y) / CH_ + 0.5 * codd);
        float2 cand = C0_ + float2(col1 * CW_, (row1 - 0.5 * codd) * CH_);
        float dd = dot(wp - cand, wp - cand);
        if (dd < bestD) { bestD = dd; best = (col1 >= 0.0 && col1 < 220.0 && row1 >= 0.0 && row1 < 220.0) ? row1 * 220.0 + col1 : -1.0; }
    }
    return best;
}
// v26k30: the rules' own weather of a cell (list 3424 on, one byte a cell, three cells a float): bits 0-1 rain tier (1 drizzle / sleet,
//         2 rain / snow, 3 downpour / snowstorm), 2 snowing, 3-4 lying snow (1 thin, 2 deep - the value the speed and the tile info use),
//         5 river ice, 6 shaken ground, 7 mud. The map draws rain, snow and ice exactly on the cells that have them.
float cellWx(float idx)
{
    if (idx < 0.0) return 0.0;
    float q = floor((idx + 0.5) / 3.0); float k = idx - q * 3.0;
    float4 f4 = IniParams.Load(int2(3424 + (int)floor((q + 0.5) / 4.0), 0)); float f = f4[(int)(q - floor((q + 0.5) / 4.0) * 4.0)];
    float d = floor(f * exp2(-8.0 * k) + 1e-3); return d - floor(d / 256.0 + 1e-4) * 256.0;
}
float wxBits(float b, float lo, float n) { float d = floor(b * exp2(-lo) + 1e-3); float m = exp2(n); return d - floor(d / m + 1e-4) * m; }
Texture2D<float> FreezeMask : register(t113);       // rivers that can freeze (bound by d3dx.ini)
Texture2DArray<float4> TexClim : register(t109);     // v23: regional climate
Texture2D<float4> TexZone : register(t108);           // v24: r = north of the Yellow River
Texture2DArray<float4> TexGround : register(t117);    // v26: ground photos; slices 16 cracked ice, 17 clear ice
SamplerState sWrapW : register(s7);
Texture2D<float> LandHeightW : register(t100);       // v26k38: the land height map (the terrain's, kept by d3dx.ini once a frame)
float wnodeAt(int base, float2 uv)                    // v26: the year's temperature anomaly at the 9x9 weather nodes (from the rules module)
{
    float4 ctl = IniParams.Load(int2(100, 0)); if (ctl.w < 0.5 || abs(ctl.z - 26.0) > 0.5) return 0.0;
    float fx = saturate(uv.x) * 8.0, fy = saturate(uv.y) * 8.0; float x0 = min(floor(fx), 7.0), y0 = min(floor(fy), 7.0); float ax = fx - x0, ay = fy - y0;
    float k00 = y0 * 9.0 + x0, k10 = k00 + 1.0, k01 = k00 + 9.0, k11 = k01 + 1.0;
    float4 q00 = IniParams.Load(int2(base + (int)floor((k00 + 0.5) / 4.0), 0)); float v00 = q00[(int)(k00 - floor((k00 + 0.5) / 4.0) * 4.0)];
    float4 q10 = IniParams.Load(int2(base + (int)floor((k10 + 0.5) / 4.0), 0)); float v10 = q10[(int)(k10 - floor((k10 + 0.5) / 4.0) * 4.0)];
    float4 q01 = IniParams.Load(int2(base + (int)floor((k01 + 0.5) / 4.0), 0)); float v01 = q01[(int)(k01 - floor((k01 + 0.5) / 4.0) * 4.0)];
    float4 q11 = IniParams.Load(int2(base + (int)floor((k11 + 0.5) / 4.0), 0)); float v11 = q11[(int)(k11 - floor((k11 + 0.5) / 4.0) * 4.0)];
    return clamp(lerp(lerp(v00, v10, ax), lerp(v01, v11, ax), ay), -4.0, 4.0);
}
float zoneNorth(float2 uv)
{
    float2 p = saturate(uv) * 512.0 - 0.5;
    int2 b = int2(floor(p)); float2 f = p - b;
    int2 b0 = clamp(b, 0, 511), b1 = clamp(b + 1, 0, 511);
    float a = TexZone.Load(int3(b0.x, b0.y, 0)).r, c = TexZone.Load(int3(b1.x, b0.y, 0)).r;
    float d = TexZone.Load(int3(b0.x, b1.y, 0)).r, e = TexZone.Load(int3(b1.x, b1.y, 0)).r;
    return lerp(lerp(a, c, f.x), lerp(d, e, f.x), f.y);
}
float4 wclimAt(float2 uv, int sl)
{
    float2 p = saturate(uv) * 128.0 - 0.5;
    int2 b = int2(floor(p)); float2 f = p - b;
    int2 b0 = clamp(b, 0, 127), b1 = clamp(b + 1, 0, 127);
    float4 a = TexClim.Load(int4(b0.x, b0.y, sl, 0)), c = TexClim.Load(int4(b1.x, b0.y, sl, 0));
    float4 d = TexClim.Load(int4(b0.x, b1.y, sl, 0)), e = TexClim.Load(int4(b1.x, b1.y, sl, 0));
    return lerp(lerp(a, c, f.x), lerp(d, e, f.x), f.y);
}
float wh21(float2 p) { p = frac(p * float2(123.34, 456.21)); p += dot(p, p + 45.32); return frac(p.x * p.y); }
float wvn(float2 p)
{
    float2 i = floor(p), f = frac(p); float2 u = f * f * (3.0 - 2.0 * f);
    float a = wh21(i), b = wh21(i + float2(1, 0)), c = wh21(i + float2(0, 1)), d = wh21(i + float2(1, 1));
    return lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);
}
float2 woct(float3 r)
{
    float w = abs(r.z) + abs(r.y); w = w + abs(r.x);
    float3 n = r / w;
    float2 r4 = 1.0 - abs(n.zy);
    bool3 ge = n >= 0.0;
    float2 xw = float2(ge.y ? 1.0 : -1.0, ge.z ? 1.0 : -1.0) * r4;
    return (ge.x ? n.yz : xw) * 0.5 + 0.5;
}


void main(float4 v0 : SV_ClipDistance1,
          float4 v1 : SV_ClipDistance0,
          float4 v2 : SV_Position,
          float4 v3 : TEXCOORD0,
          float4 v4 : TEXCOORD1,
          float4 v5 : TEXCOORD2,
          float4 v6 : TEXCOORD3,
          float4 v7 : TEXCOORD4,
          float v8 : TEXCOORD5,
          out float4 o0 : SV_Target0,
          out float4 o1 : SV_Target1,
          out float4 o2 : SV_Target2,
          out float4 o3 : SV_Target3,
          out float4 o4 : SV_Target4,
          out float4 o5 : SV_Target5)
{
    o0 = 0;
    o1 = 0;
    o2 = 0;
    o3 = 0;
    o4 = 0;
    o5 = 0;
    float4 r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0, r9 = 0;
    r0.x = (t3.Sample(s3, (v3.zw))).x;
    r0.z = (r0.x) + (cb0[6].z);
    r0.xy = (v7.xy);
    r0.xy = (t0.Sample(s0, (r0.xyz))).xy;
    r0.xy = (r0.xy) * (float2(2.0,2.0)) + (float2(-1.0,-1.0));
    r0.xz = (r0.xy) + (v6.xz);
    r0.y = (v6.y);
    r0.w = dot((r0.xyz), (r0.xyz));
    r0.w = rsqrt((r0.w));
    r1.xyz = (r0.www) * (r0.xyz);
    r2.xyz = (r1.zxy) * (float3(0.0,1.0,0.0));
    r2.xyz = (r1.yzx) * (float3(1.0,0.0,0.0)) + (-r2.xyz);
    r1.w = dot((r2.xy), (r2.xy));
    r1.w = rsqrt((r1.w));
    r2.xyz = (r1.www) * (r2.xyz);
    r3.xyz = (-v5.xyz) + (cb0[0].xyz);
    r1.w = dot((r3.xyz), (r3.xyz));
    r1.w = rsqrt((r1.w));
    r3.xyz = (r1.www) * (r3.xyz);
    r0.xyz = (-r0.xyz) * (r0.www) + (float3(0.0,1.0,0.0));
    r0.xyz = (r0.xyz) * (cb0[11].yyy) + (-r3.xyz);
    r0.w = dot((r0.xyz), (r0.xyz));
    r0.w = rsqrt((r0.w));
    r0.xyz = (r0.www) * (r0.xyz);
    r3.xzw = (v5.xyz) + (-cb0[13].xyz);
    r4.x = dot((v5.xyzw), (cb0[7].xyzw));
    r4.y = dot((v5.xyzw), (cb0[8].xyzw));
    r0.w = dot((v5.xyzw), (cb0[9].xyzw));
    r4.xy = (r4.xy) / (r0.ww);
    r0.w = (t2.Sample(s2, (r4.xy))).x;
    r0.w = saturate((cb0[12].y) * (r0.w) + (cb0[12].x));
    r0.w = (1.0) / (r0.w);
    r1.w = dot((r3.xzw), (cb0[14].xyz));
    r2.w = (r0.w) + (-r1.w);
    r2.w = min((r2.w), (cb0[11].z));
    r4.xyz = (r0.xyz) * (r2.www) + (v5.xyz);
    r4.w = (1.0);
    r0.x = dot((r4.xyzw), (cb0[7].xyzw));
    r0.y = dot((r4.xyzw), (cb0[8].xyzw));
    r0.z = dot((r4.xyzw), (cb0[9].xyzw));
    r0.xy = (r0.xy) / (r0.zz);
    r0.x = (t2.Sample(s2, (r0.xy))).x;
    r0.x = saturate((cb0[12].y) * (r0.x) + (cb0[12].x));
    r0.x = (1.0) / (r0.x);
    r0.y = asfloat((uint)((r0.x) < (r1.w)) * 0xffffffffu);
    r0.x = ((asuint(r0.y)) != 0) ? (r0.w) : (r0.x);
    r0.x = (-r1.w) + (r0.x);
    r0.y = (r0.x) * (cb0[11].w);
    r0.y = (abs(r3.y)) * (r0.y);
    r0.y = exp2((r0.y));
    r0.y = min((r0.y), (1.0));
    r0.y = (-r0.y) + (1.0);
    r0.zw = (v3.xy) * (cb0[48].zw);
    r0.zw = (v4.xy) * (cb0[48].xy) + (r0.zw);
    r3.xy = (r0.zw) + (cb0[51].xy);
    r0.zw = (v3.xy) * (cb0[49].zw);
    r0.zw = (v4.xy) * (cb0[49].xy) + (r0.zw);
    r4.xy = (r0.zw) + (cb0[51].xy);
    r0.zw = (v3.xy) * (cb0[50].zw);
    r0.zw = (v4.xy) * (cb0[50].xy) + (r0.zw);
    r5.xy = (r0.zw) + (cb0[51].xy);
    r0.z = saturate((v8.x));
    r3.z = (cb0[47].x);
    r3.xyzw = (t4.Sample(s4, (r3.xyz))).xyzw;
    r3.xyzw = (r3.xyzw) * (cb0[15].xyzw);
    r0.w = (1.0);
    r1.w = (r3.w) * (cb0[16].w);
    r1.w = (r0.z) * (r1.w);
    r4.z = (cb0[47].y);
    r4.xyzw = (t4.Sample(s4, (r4.xyz))).xyzw;
    r4.xyzw = (r4.xyzw) * (cb0[15].xyzw);
    r2.w = (1.0);
    r3.w = (r4.w) * (cb0[20].w);
    r3.w = (r0.z) * (r3.w);
    r5.z = (cb0[47].z);
    r5.xyzw = (t4.Sample(s4, (r5.xyz))).xyzw;
    r4.w = (1.0);
    r5.w = (r5.w) * (cb0[24].w);
    r5.w = (r0.z) * (r5.w);
    r6.x = asfloat((uint)((0.0) < (cb0[27].y)) * 0xffffffffu);
    r6.y = saturate((r1.y));
    r6.y = log2((r6.y));
    r6.y = (r6.y) * (cb0[27].y);
    r6.y = exp2((r6.y));
    r6.x = ((asuint(r6.x)) != 0) ? (r6.y) : (1.0);
    r6.y = asfloat((uint)((cb0[28].x) >= (v7.w)) * 0xffffffffu);
    r6.z = (v7.w) * (3.0);
    r6.z = (r6.z) / (cb0[28].x);
    r6.zw = (r6.zz) * (float2(1.4426950216293335,-1.4426950216293335));
    r6.zw = exp2((r6.zw));
    r6.z = (r6.w) + (r6.z);
    r6.z = (2.0) / (r6.z);
    r6.z = saturate((r6.z) + (-0.10000000149011612));
    r6.y = asfloat((asuint(r6.z)) & (asuint(r6.y)));
    r6.x = (r6.x) * (cb0[46].x);
    r5.w = (r5.w) * (r6.x);
    r6.x = (r6.y) * (r5.w);
    r6.x = asfloat((uint)((r6.x) < (cb0[27].w)) * 0xffffffffu);
    r6.z = asfloat((uint)((cb0[27].w) < (1.0)) * 0xffffffffu);
    r6.y = (r5.w) * (r6.y) + (-cb0[27].w);
    r6.w = (-cb0[27].w) + (1.0);
    r6.y = (r6.y) / (r6.w);
    r5.w = ((asuint(r6.z)) != 0) ? (r6.y) : (r5.w);
    r5.w = ((asuint(r6.x)) != 0) ? (0.0) : (r5.w);
    r6.xyz = asfloat((uint3)((r0.yyy) < (cb0[28].wyz)) * 0xffffffffu);
    r7.xyz = (r0.yyy) / (cb0[28].wyz);
    r7.xyz = (abs(r7.xyz)) * (abs(r7.xyz));
    r0.y = (r5.w) * (r7.x);
    r0.y = ((asuint(r6.x)) != 0) ? (r0.y) : (r5.w);
    r6.xw = (float2(0.0,0.0));
    [loop] while (true) {
        r5.w = asfloat((uint)((asuint(r6.w)) >= (16u)) * 0xffffffffu);
        if (((asuint(r5.w)) != 0)) break;
        r7.xw = (v5.xz) + (-cb0[30 + asint((r6.w))].xy);
        r5.w = dot((r7.xw), (r7.xw));
        r7.x = (cb0[30 + asint((r6.w))].z) * (cb0[30 + asint((r6.w))].z);
        r7.w = asfloat((uint)((r5.w) < (r7.x)) * 0xffffffffu);
        if (((asuint(r7.w)) != 0)) {
            r5.w = (r5.w) / (r7.x);
            r5.w = (-r5.w) + (1.0);
            r6.x = (cb0[30 + asint((r6.w))].w) * (r5.w) + (r6.x);
        }
        r6.w = asfloat((asint(r6.w)) + (1));
    }
    r5.w = min((r6.x), (3.0));
    r6.x = (v6.w) * (0.1591549515724182);
    r6.w = asfloat((uint)((r6.x) >= (-r6.x)) * 0xffffffffu);
    r6.x = frac((abs(r6.x)));
    r6.x = ((asuint(r6.w)) != 0) ? (r6.x) : (-r6.x);
    r6.w = (r6.x) * (6.283185005187988);
    r7.x = asfloat((uint)((0.49999991059303284) < (r6.x)) * 0xffffffffu);
    r7.w = asfloat((uint)((r6.x) < (-0.49999991059303284)) * 0xffffffffu);
    r8.xy = (r6.xx) * (float2(6.283185005187988,6.283185005187988)) + (float2(-6.283185005187988,6.283185005187988));
    r6.x = ((asuint(r7.w)) != 0) ? (r8.y) : (r6.w);
    r6.x = ((asuint(r7.x)) != 0) ? (r8.x) : (r6.x);
    r6.w = (cb0[19].x) + (1.570796012878418);
    r6.w = (-r6.w) + (r6.x);
    r7.x = asfloat((uint)((0.0) < (r6.w)) * 0xffffffffu);
    r7.w = (r6.w) + (cb0[19].y);
    r7.w = asfloat((uint)((r7.w) < (0.0)) * 0xffffffffu);
    r8.x = (cb0[19].y) * (-0.3333333432674408);
    r8.x = asfloat((uint)((r8.x) < (r6.w)) * 0xffffffffu);
    r8.y = (r6.w) * (-3.0);
    r8.y = (r8.y) / (cb0[19].y);
    r8.z = (1.5) / (cb0[19].y);
    r6.w = (r6.w) * (r8.z) + (1.5);
    r6.w = ((asuint(r8.x)) != 0) ? (r8.y) : (r6.w);
    r6.w = log2((r6.w));
    r6.w = (r6.w) * (cb0[19].z);
    r6.w = exp2((r6.w));
    r6.w = min((r6.w), (1.0));
    r8.x = saturate((v4.w));
    r6.w = (r6.w) * (r8.x);
    r7.x = asfloat((asuint(r7.x)) | (asuint(r7.w)));
    r7.w = (cb0[23].x) + (1.570796012878418);
    r6.x = (r6.x) + (-r7.w);
    r7.w = asfloat((uint)((0.0) < (r6.x)) * 0xffffffffu);
    r8.y = (r6.x) + (cb0[23].y);
    r8.y = asfloat((uint)((r8.y) < (0.0)) * 0xffffffffu);
    r8.z = (cb0[23].y) * (-0.125);
    r8.z = asfloat((uint)((r8.z) < (r6.x)) * 0xffffffffu);
    r8.w = (r6.x) * (-8.0);
    r8.w = (r8.w) / (cb0[23].y);
    r9.x = (1.1428571939468384) / (cb0[23].y);
    r6.x = (r6.x) * (r9.x) + (1.1428571939468384);
    r6.x = ((asuint(r8.z)) != 0) ? (r8.w) : (r6.x);
    r6.x = log2((r6.x));
    r6.x = (r6.x) * (cb0[23].z);
    r6.x = exp2((r6.x));
    r6.xw = min((r6.xw), (float2(1.0,0.8999999761581421)));
    r6.x = (r8.x) * (r6.x);
    r7.w = asfloat((asuint(r7.w)) | (asuint(r8.y)));
    r6.w = (-r6.w) + (1.0);
    r6.w = ((asuint(r7.x)) != 0) ? (1.0) : (r6.w);
    r6.w = asfloat((uint)((r1.w) < (r6.w)) * 0xffffffffu);
    r7.x = saturate((r5.w) * (cb0[46].x));
    r1.w = (r1.w) * (r7.x);
    r1.w = ((asuint(r6.w)) != 0) ? (0.0) : (r1.w);
    r6.x = (r6.x) * (cb0[46].x);
    r6.x = ((asuint(r7.w)) != 0) ? (0.0) : (r6.x);
    r5.w = saturate((r5.w) * (r6.x));
    r6.x = (r3.w) * (r5.w);
    r6.w = asfloat((uint)((r6.x) < (cb0[23].w)) * 0xffffffffu);
    r7.x = asfloat((uint)((cb0[23].w) < (1.0)) * 0xffffffffu);
    r3.w = (r3.w) * (r5.w) + (-cb0[23].w);
    r5.w = (-cb0[23].w) + (1.0);
    r3.w = (r3.w) / (r5.w);
    r3.w = ((asuint(r7.x)) != 0) ? (r3.w) : (r6.x);
    r3.w = ((asuint(r6.w)) != 0) ? (0.0) : (r3.w);
    r5.w = (r7.y) * (r1.w);
    r1.w = ((asuint(r6.y)) != 0) ? (r5.w) : (r1.w);
    r5.w = (r7.z) * (r3.w);
    r3.w = ((asuint(r6.z)) != 0) ? (r5.w) : (r3.w);
    r6.xyz = (r2.www) * (cb0[20].xyz) + (-cb0[52].xyz);
    r6.xyz = (r3.www) * (r6.xyz) + (cb0[52].xyz);
    r7.xyz = (r0.www) * (cb0[16].xyz) + (-r6.xyz);
    r6.xyz = (r1.www) * (r7.xyz) + (r6.xyz);
    r0.w = asfloat((uint)((0.0) < (v7.w)) * 0xffffffffu);
    r2.w = asfloat((uint)((0.0) < (cb0[29].w)) * 0xffffffffu);
    r0.w = asfloat((asuint(r0.w)) & (asuint(r2.w)));
    r7.xyz = (-cb0[29].xyz) + (float3(1.0,1.0,1.0));
    r7.xyz = (r6.xyz) * (r7.xyz);
    r2.w = (cb0[29].w) * (cb0[29].w);
    r2.w = (r2.w) * (v7.w);
    r2.w = (cb0[10].y) / (r2.w);
    r2.w = exp2((-r2.w));
    r7.xyz = (-r7.xyz) * (r2.www) + (r6.xyz);
    r6.xyz = ((asuint(r0.www)) != 0) ? (r7.xyz) : (r6.xyz);
    r7.xyz = (r4.www) * (cb0[24].xyz) + (-r6.xyz);
    o0.xyz = (r0.yyy) * (r7.xyz) + (r6.xyz);
    r6.xyzw = (cb0[21].xyzw) + (-cb0[53].xyzw);
    r6.xyzw = (r3.wwww) * (r6.xyzw) + (cb0[53].xyzw);
    r7.xyzw = (-r6.xyzw) + (cb0[17].xyzw);
    r6.xyzw = (r1.wwww) * (r7.xyzw) + (r6.xyzw);
    r7.xyzw = (-r6.xyzw) + (cb0[25].xyzw);
    r6.xyzw = (r0.yyyy) * (r7.wxyz) + (r6.wxyz);
    o2.xyz = saturate((r6.yzw));
    r6.x = saturate((r6.x));
    r0.w = max((v7.w), (0.0));
    r0.w = asfloat((uint)((r0.w) < (cb0[27].z)) * 0xffffffffu);
    r2.w = saturate((v7.w) / (cb0[27].z));
    r2.w = (-r2.w) + (1.0);
    r0.z = (r0.z) * (r2.w);
    r0.z = (r0.z) * (-r6.x) + (r6.x);
    r0.z = ((asuint(r0.w)) != 0) ? (r0.z) : (r6.x);
    r0.w = asfloat((uint)((cb0[22].y) < (r3.w)) * 0xffffffffu);
    r6.xyz = (r1.yzx) * (r2.zxy);
    r6.xyz = (r2.yzx) * (r1.zxy) + (-r6.xyz);
    r7.xyz = (r2.xyz) * (r4.yyy);
    r4.xyw = (r6.xyz) * (r4.xxx) + (r7.xyz);
    r4.xyz = (r1.xyz) * (r4.zzz) + (r4.xyw);
    r1.xyz = ((asuint(r0.www)) != 0) ? (r4.xyz) : (r1.xyz);
    r0.w = asfloat((uint)((cb0[26].y) < (r0.y)) * 0xffffffffu);
    r4.xyz = (r1.yzx) * (r2.zxy);
    r4.xyz = (r2.yzx) * (r1.zxy) + (-r4.xyz);
    r6.xyz = (r2.xyz) * (r5.yyy);
    r4.xyz = (r4.xyz) * (r5.xxx) + (r6.xyz);
    r4.xyz = (r1.xyz) * (r5.zzz) + (r4.xyz);
    r1.xyz = ((asuint(r0.www)) != 0) ? (r4.xyz) : (r1.xyz);
    r0.w = asfloat((uint)((cb0[18].y) < (r1.w)) * 0xffffffffu);
    r4.xyz = (r1.yzx) * (r2.zxy);
    r4.xyz = (r2.yzx) * (r1.zxy) + (-r4.xyz);
    r5.xyz = (r2.xyz) * (r3.yyy);
    r4.xyz = (r4.xyz) * (r3.xxx) + (r5.xyz);
    r3.xyz = (r1.xyz) * (r3.zzz) + (r4.xyz);
    r1.xyz = ((asuint(r0.www)) != 0) ? (r3.xyz) : (r1.xyz);
    r0.w = (-r1.w) + (1.0);
    r0.w = min((r0.w), (cb0[10].z));
    r1.w = (-r3.w) + (1.0);
    r0.w = min((r0.w), (r1.w));
    r0.y = (-r0.y) + (1.0);
    r0.y = min((r0.y), (r0.w));
    o2.w = (r0.z) * (0.5);
    r3.y = dot((cb0[1].xyz), (r1.xyz));
    r3.z = dot((cb0[2].xyz), (r1.xyz));
    r3.x = dot((cb0[3].xyz), (r1.xyz));
    r0.z = (abs(r3.z)) + (abs(r3.y));
    r0.z = (abs(r3.x)) + (r0.z);
    r1.xyz = (r3.xyz) / (r0.zzz);
    r0.zw = (-abs(r1.zy)) + (float2(1.0,1.0));
    r3.xyz = asfloat((uint3)((r1.xyz) >= (float3(0.0,0.0,0.0))) * 0xffffffffu);
    r1.xw = ((asuint(r3.yz)) != 0) ? (float2(1.0,1.0)) : (float2(-1.0,-1.0));
    r0.zw = (r0.zw) * (r1.xw);
    r0.zw = ((asuint(r3.xx)) != 0) ? (r1.yz) : (r0.zw);
    o1.xy = (r0.zw) * (float2(0.5,0.5)) + (float2(0.5,0.5));
    r0.z = (abs(v6.y)) + (abs(v6.x));
    r0.z = (r0.z) + (abs(v6.z));
    r1.xyz = (v6.zxy) / (r0.zzz);
    r0.zw = (-abs(r1.zy)) + (float2(1.0,1.0));
    r3.xyz = asfloat((uint3)((r1.xyz) >= (float3(0.0,0.0,0.0))) * 0xffffffffu);
    r1.xw = ((asuint(r3.yz)) != 0) ? (float2(1.0,1.0)) : (float2(-1.0,-1.0));
    r0.zw = (r0.zw) * (r1.xw);
    r0.zw = ((asuint(r3.xx)) != 0) ? (r1.yz) : (r0.zw);
    o4.xy = (r0.zw) * (float2(0.5,0.5)) + (float2(0.5,0.5));
    r0.z = (abs(r2.y)) + (abs(r2.x));
    r0.zw = (r2.xy) / (r0.zz);
    o4.zw = (r0.zw) * (float2(0.5,0.5)) + (float2(0.5,0.5));
    r0.y = (r0.y) * (255.0);
    r0.y = asfloat((uint)(r0.y));
    r0.y = asfloat((asint(r0.y)) << ((8u) & 31));
    r0.y = (float)(asuint(r0.y));
    o1.z = (r0.y) * (1.5259021893143654e-05);
    o1.w = (0.0);
    o3.xyzw = (float4(0.0,0.0,0.0,0.0));
    o5.xy = (cb0[10].xy);
    o5.w = (0.0);
    o5.z = (r0.x);
    o0.w = (1.0);
    // ---- season: winter river ice (northern rivers): once cold enough the whole river is one ice sheet ----
    {
        float4 ip = IniParams.Load(int2(0, 0));
        float3 wp = v5.xyz;
        float2 sanS = sanSH(); float2 muv = (wp.xz - sanS) / 204800.0;   // v26k48: the old map's place (the grid below stays the game's)
        bool sanIn = (sanS.x == 0.0 && sanS.y == 0.0) || (all(muv >= 0.0) && all(muv <= 1.0));   // new land: the game's water
        float4 pv = IniParams.Load(int2(99, 0));                                      // v26k16: the test build's snow and ice preview (z = 1)
        float prevOn = (pv.z > 0.5 && pv.y > 0.5 && pv.y < 36.5) ? 1.0 : 0.0;
        float4 wctlW = IniParams.Load(int2(100, 0)), dlkW = IniParams.Load(int2(60, 0));
        float rulesW = (wctlW.w > 0.5 && abs(wctlW.z - 26.0) < 0.5 && dlkW.w > 0.5 && prevOn < 0.5) ? 1.0 : 0.0;   // v26k30: the rules say which cells are frozen
        float iceW = 0.0;
        [branch] if (rulesW > 0.5) { gridInit(); iceW = wxBits(cellWx(hexIdxAt(wp.xz)), 5.0, 1.0); }
        float mk = (sanIn && ip.x > 0.5 && ip.y > 0.5 && (rulesW > 0.5 ? iceW > 0.5 : (ip.x < 2.5 || ip.x > 8.5 || prevOn > 0.5))) ? FreezeMask.SampleLevel(s3, muv, 0) : 0.0;   // v25.9i: 3-8월 the water is the game's own (v26k30: with the rules, the frozen cells)
        if (mk > 0.002) {
            float4 ip1 = IniParams.Load(int2(1, 0));
            float xun = (ip1.x >= 0.0 && ip1.x <= 2.0) ? round(ip1.x) : 1.0;
            float tpos = clamp(round(ip.x), 1.0, 12.0) + 1.0 + (xun - 1.0) / 3.0;   // v25.9i: KOEI seasons (game month m = real month m+2)
            [branch] if (prevOn > 0.5) { float tr = round(pv.y) - 1.0, mq = floor(tr / 3.0 + 0.01); tpos = mq + 2.0 + (tr - 3.0 * mq - 1.0) / 3.0 + saturate(pv.x) / 3.0; if (tpos >= 12.0) tpos -= 12.0; }
            float tfl = floor(tpos);
            float4 cl = lerp(wclimAt(saturate(muv), ((int)tfl + 12) % 12), wclimAt(saturate(muv), ((int)tfl + 13) % 12), tpos - tfl);   // v24: by turn
            cl.r -= 2.0 * zoneNorth(saturate(muv));                                   // v24: 2 C colder north of the Yellow River
            float T = cl.r - 0.0045 * max(wp.y - 750.0, 0.0) + wnodeAt(101, saturate(muv));   // v26: the year's cold or warmth
            T -= 2.0 * smoothstep(-1.5, 1.5, cl.g) * (1.0 - smoothstep(4.0, 8.0, T));   // ice lingers into early spring
            // v26: the Liaodong west coast (mask value under 0.5) is sea water: it freezes only in a hard frost
            float coastF = smoothstep(0.52, 0.40, mk) * smoothstep(0.08, 0.30, mk);
            float fr = saturate(mk * 3.0) * lerp(smoothstep(1.5, -5.0, T), smoothstep(-3.5, -10.0, T), coastF);
            if (rulesW > 0.5) fr = max(fr, saturate(mk * 3.0) * 0.42);   // v26k30: a cell the rules froze is one ice sheet (armies walk on it)
            float n = wvn(wp.xz / 900.0) * 0.55 + wvn(wp.xz / 230.0 + 3.1) * 0.30 + wvn(wp.xz / 60.0 + 7.7) * 0.15;
            float k = (fr - 0.12) / 0.18 * 1.2;                     // >= 1.2: solid sheet, no water left
            if (fr > 0.12 && n < k) {
                float s = wvn(wp.xz / 140.0 + 11.0) * 0.6 + wvn(wp.xz / 35.0 + 5.0) * 0.4;
                float3 iceC = lerp(float3(0.36, 0.46, 0.56), float3(0.74, 0.77, 0.81), smoothstep(0.40, 0.75, s));
                float crack = 1.0 - 0.35 * smoothstep(0.035, 0.0, abs(wvn(wp.xz / 85.0 + 19.0) - 0.5));
                // v26: real ice photos - cracked sheet ice where it is thick (cold), clearer ice near the edges
                float3 pc = pow(TexGround.SampleLevel(sWrapW, float3(wp.xz / 1900.0, 16), 0).rgb, 2.2);
                float3 pl = pow(TexGround.SampleLevel(sWrapW, float3(wp.xz / 1300.0 + 0.3, 17), 0).rgb, 2.2);
                float thick = smoothstep(0.9, 1.6, k);
                float3 photo = lerp(pl, pc, thick) * 1.15;
                iceC = lerp(iceC, photo, 0.45);
                // v26k38: something standing under the ice (the 2D mod's troop pictures sit at the river-bed height, below the water
                //         surface): the game's water let them show through; the opaque ice hid them. Where the picture behind this
                //         ice pixel stands clearly above the river bed (the land height map), the ice turns clear: the game's own water
                //         with a light ice tint, so the troops show as under clear ice. The bare bed and the banks stay solid ice.
                float under = 0.0;
                {
                    uint hw0, hh0; LandHeightW.GetDimensions(hw0, hh0);
                    [branch] if (hw0 > 0) {
                        float sw0 = dot(v5, cb0[9]);
                        float2 suv = float2(dot(v5, cb0[7]), dot(v5, cb0[8])) / sw0;
                        float lin = 1.0 / saturate(cb0[12].y * t2.SampleLevel(s2, suv, 0).x + cb0[12].x);
                        float sd = dot(v5.xyz - cb0[13].xyz, cb0[14].xyz);
                        float3 Pb = cb0[13].xyz + (v5.xyz - cb0[13].xyz) * (lin / max(sd, 1.0));
                        float bed = LandHeightW.SampleLevel(sWrapW, Pb.xz / (hw0 < 3000u ? 204800.0 : (float)(hw0 - 1u) * 100.0), 0) * 1.6;   // v26k48: the game's map size   // the map's height units x 1.6 = world height (as the ground shader)
                        under = smoothstep(60.0, 140.0, Pb.y - bed) * step(Pb.y, wp.y + 20.0);
                    }
                }
                float4 w0 = o0, w2 = o2, w5 = o5; float4 w1 = o1;
                o0 = float4(iceC * crack, 1.0);
                float3 rv;
                rv.y = cb0[1].y; rv.z = cb0[2].y; rv.x = cb0[3].y;          // world up in view space
                o1.xy = woct(rv);
                o1.z = 0; o1.w = 0;
                o2 = float4(0.03, 0.03, 0.03, 0.22);
                o5 = 0;
                [branch] if (under > 0.01) {
                    o0 = float4(lerp(w0.rgb, iceC * crack, 0.30), w0.w);
                    o2 = w2; o5 = w5; o1 = w1;
                    o0 = lerp(float4(iceC * crack, 1.0), o0, under);
                }
            }
        }
    }
    return;
}
