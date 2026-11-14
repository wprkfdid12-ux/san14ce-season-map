#include "tex.h"
#include "texfn.h"
#include <cstdio>
FnTab g_tab; TexFn g_fn = [](int s, float x, float y, float z) { return texfn(s, x, y, z); };
Tex1D IniParams; Tex2D4 t3; Tex2D1 LandHeight;
Tex2D4 TexSnow, TexVeg, TexFreeze, TexRegion, TexRock, TexLayer, TexSoil, TexForest, TexZone, TexFlood, TexHazard, TexRock2, TexRock3, TexRelief, TexTags;
Tex2DA TexGround, TexClim; float4 cvM[5];
namespace nb {
#include "season_NearB.inc"
}
namespace na {
#include "season_NearA.inc"
}
int main(int argc, char **argv) {
    FILE *f = fopen(argv[1], "rb"); fread(&g_tab, sizeof g_tab, 1, f); fclose(f);
    struct { TexBase *t; int s; } L[] = { {&t3, 3}, {&LandHeight, 110}, {&TexSnow, 111}, {&TexVeg, 112}, {&TexFreeze, 113}, {&TexRegion, 114}, {&TexRock, 115},
        {&TexLayer, 116}, {&TexSoil, 118}, {&TexForest, 119}, {&TexZone, 108}, {&TexFlood, 106}, {&TexHazard, 107}, {&TexRock2, 105}, {&TexRock3, 104},
        {&TexRelief, 102}, {&TexTags, 103}, {&TexGround, 117}, {&TexClim, 109} };
    for (auto &l : L) { l.t->slot = l.s; l.t->fn = true; }
    f = fopen(argv[2], "rb"); int n; fread(&n, 4, 1, f); IniParams.d.resize(8192); int bad = 0, same = 0, cnt = 0;
    for (int c = 0; c < n; c++) {
        fread(IniParams.d.data(), sizeof(float4), 8192, f); float in[16]; fread(in, 4, 16, f);
        float3 r[2]; int ok[2] = {0, 0};
        for (int v = 0; v < 2; v++) {
            try { float3 A, M; float dT; float2 uv(in[6], in[7]); float h = LandHeight.SampleLevel(sClamp, uv, 0);
                if (v == 0) { nb::sanInit(); nb::g_dx = float2(in[8]); nb::g_dy = float2(in[9]); nb::g_dhx = in[8]; nb::g_dhy = in[9]; nb::season(uv, float3(in[3], in[4], in[5]), float2(in[0], in[2]), h, in[1], A, M, dT); }
                else { na::sanInit(); na::g_dx = float2(in[8]); na::g_dy = float2(in[9]); na::g_dhx = in[8]; na::g_dhy = in[9]; na::season(uv, float3(in[3], in[4], in[5]), float2(in[0], in[2]), h, in[1], A, M, dT); }
                r[v] = A + M * 0.5; ok[v] = 1; } catch (int) {}
        }
        if (!ok[0] || !ok[1]) continue; cnt++;
        for (int k = 0; k < 3; k++) if (!std::isfinite(r[0][k])) bad++;
        if (std::fabs(r[0].x - r[1].x) + std::fabs(r[0].y - r[1].y) + std::fabs(r[0].z - r[1].z) < 1e-6) same++;
    }
    printf("light variant: %d pixels, non-finite %d, same as A %d (the rest differ where disasters / weather marks / northwest winter would show)\n", cnt, bad, same);
}
