#include "tex.h"
#include "texfn.h"
#include <cstdio>
#include <cstring>
FnTab g_tab; TexFn g_fn = [](int s, float x, float y, float z) { return texfn(s, x, y, z); };
Tex1D IniParams; Tex2D4 t3; Tex2D1 LandHeight;
Tex2D4 TexSnow, TexVeg, TexFreeze, TexRegion, TexRock, TexLayer, TexSoil, TexForest, TexZone, TexFlood, TexHazard, TexRock2, TexRock3, TexRelief, TexTags;
Tex2DA TexGround, TexClim; float4 cvM[5];
namespace old_ns {
#include "season_old.inc"
}
static void setslots() {
    struct { TexBase *t; int s; } L[] = { {&t3, 3}, {&LandHeight, 110}, {&TexSnow, 111}, {&TexVeg, 112}, {&TexFreeze, 113}, {&TexRegion, 114}, {&TexRock, 115},
        {&TexLayer, 116}, {&TexSoil, 118}, {&TexForest, 119}, {&TexZone, 108}, {&TexFlood, 106}, {&TexHazard, 107}, {&TexRock2, 105}, {&TexRock3, 104},
        {&TexRelief, 102}, {&TexTags, 103}, {&TexGround, 117}, {&TexClim, 109} };
    for (auto &l : L) { l.t->slot = l.s; l.t->fn = true; }
}
int main(int argc, char **argv) {
    FILE *f = fopen(argv[1], "rb"); fread(&g_tab, sizeof g_tab, 1, f); fclose(f);
    setslots(); cvM[0] = float4(1, 0, 0, 0); cvM[1] = float4(0, 1, 0, 0); cvM[2] = float4(0, 0, 0, 1); cvM[3] = float4(0, 0, 1, 0);
    f = fopen(argv[2], "rb"); int n; fread(&n, 4, 1, f); IniParams.d.resize(8192);
    FILE *o = fopen(argv[3], "wb");
    for (int c = 0; c < n; c++) {
        fread(IniParams.d.data(), sizeof(float4), 8192, f);
        float in[16]; fread(in, 4, 16, f);   // v4.xyz, v6.xyz, v7.xy, dvx, dvy, g.rgb, far
        using namespace old_ns;
        float3 v4(in[0], in[1], in[2]), v6(in[3], in[4], in[5]); float2 v7(in[6], in[7]); float3 g(in[10], in[11], in[12]);
        float res[8] = { 0 };
        try {
            float4 pm98 = IniParams.Load(int2(98, 0));
            sanInit(); float2 uv = v7; float h = LandHeight.SampleLevel(sClamp, uv, 0);
            if (SAN_ON) uv = (uv * SAN_MW - SAN_SH) / MAPW;
            float2 wxzK = float2(v4.x, v4.z) - SAN_SH; float wyK = v4.y;
            if (in[13] > 0.5) { wxzK = uv * MAPW; wyK = h * 1.6; }   // the far ground: world point from the map uv
            bool sanIn = !SAN_ON || (!SAN_SKIP && uv.x >= 0 && uv.y >= 0 && uv.x <= 1 && uv.y <= 1);
            g_dx = float2(in[8]); g_dy = float2(in[9]); g_dhx = in[8]; g_dhy = in[9];
            float3 out = g; res[7] = 0;
            if (sanIn && (pm98.x < 0.5 || pm98.x > 2.5)) {
                int2 hpSea = clamp(int2(saturate(uv) * 512.0), 0, 511);
                float seaG = TexHazard.Load(int3(hpSea, 0)).y;
                if (!(h < 740.0 && seaG < 0.1)) { float3 A, M; float dynT; season(uv, v6, wxzK, h, wyK, A, M, dynT); out = A + saturate(g) * M; res[6] = dynT; res[7] = 1; }
            }
            res[0] = out.x; res[1] = out.y; res[2] = out.z;
        } catch (int) { res[7] = -1; }
        fwrite(res, 4, 8, o);
    }
    fclose(o); fclose(f);
}
