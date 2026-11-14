#pragma once
#include "hv.h"
#include <vector>
#include <functional>
// texture stand-ins: either a function of the raw coordinates (to compare with the DXBC emulator) or real texel data with
// D3D-style bilinear filtering (8-bit fraction weights), clamp (s3) or wrap (s7) addressing, level 0 only
typedef std::function<float4(int slot, float x, float y, float z)> TexFn;
extern TexFn g_fn;
const int s3 = 3, s7 = 7;
struct TexBase {
    int slot = 0, w = 1, h = 1, n = 1; std::vector<float4> d; bool fn = false; uint dw = 1920, dh = 1080;
    float4 texel(int x, int y, int k) const {
        if (x < 0 || y < 0 || x >= w || y >= h || k < 0 || k >= n) return float4(0.0f);
        return d[((size_t)k * h + y) * w + x];
    }
    float4 bil(int s, float u, float v, int k) const {
        float tx = u * w - 0.5f, ty = v * h - 0.5f; float fx0 = std::floor(tx), fy0 = std::floor(ty);
        float fx = tx - fx0, fy = ty - fy0; fx = std::floor(fx * 256.0f) / 256.0f; fy = std::floor(fy * 256.0f) / 256.0f;
        int x0 = (int)fx0, y0 = (int)fy0, x1 = x0 + 1, y1 = y0 + 1;
        auto ad = [&](int &x, int m) { if (s == s7) { x %= m; if (x < 0) x += m; } else x = x < 0 ? 0 : (x >= m ? m - 1 : x); };
        ad(x0, w); ad(x1, w); ad(y0, h); ad(y1, h);
        float4 a = texel(x0, y0, k), b = texel(x1, y0, k), c = texel(x0, y1, k), e = texel(x1, y1, k);
        return lerp(lerp(a, b, fx), lerp(c, e, fx), fy);
    }
};
struct Tex2D4 : TexBase {
    float4 Load(int3 p) const { return fn ? g_fn(slot, (float)p.x, (float)p.y, (float)p.z) : texel(p.x, p.y, 0); }
    float4 SampleLevel(int s, float2 uv, float) const { return fn ? g_fn(slot, uv.x, uv.y, uv.x) : bil(s, uv.x, uv.y, 0); }
    float4 SampleGrad(int s, float2 uv, float2, float2) const { return SampleLevel(s, uv, 0.f); }
    void GetDimensions(uint &a, uint &b) const { a = dw; b = dh; }
};
struct Tex2D1 : TexBase {
    float Load(int3 p) const { return fn ? g_fn(slot, (float)p.x, (float)p.y, (float)p.z).x : texel(p.x, p.y, 0).x; }
    float SampleLevel(int s, float2 uv, float) const { return fn ? g_fn(slot, uv.x, uv.y, uv.x).x : bil(s, uv.x, uv.y, 0).x; }
    void GetDimensions(uint &a, uint &b) const { a = dw; b = dh; }
};
struct Tex2DA : TexBase {
    float4 Load(int4 p) const { return fn ? g_fn(slot, (float)p.x, (float)p.y, (float)p.z) : texel(p.x, p.y, p.z); }
    float4 SampleLevel(int s, float3 c, float) const {
        if (fn) return g_fn(slot, c.x, c.y, c.z);
        int k = (int)std::nearbyint(c.z); k = k < 0 ? 0 : (k >= n ? n - 1 : k); return bil(s, c.x, c.y, k); }
};
struct Tex1D { std::vector<float4> d; float4 Load(int2 p) const { return (p.x >= 0 && p.x < (int)d.size()) ? d[p.x] : float4(0.0f); } };
extern Tex1D IniParams; extern Tex2D4 t3; extern Tex2D1 LandHeight;
extern Tex2D4 TexSnow, TexVeg, TexFreeze, TexRegion, TexRock, TexLayer, TexSoil, TexForest, TexZone, TexFlood, TexHazard, TexRock2, TexRock3, TexRelief, TexTags;
extern Tex2DA TexGround, TexClim;
extern float4 cvM[5];
#define LandControl t3
#define sClamp s3
#define sWrap s7
