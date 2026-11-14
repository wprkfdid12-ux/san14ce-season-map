#pragma once
// the shared stand-in texture function (same formula as texfn.py): v_i = B + A sin(K1 x + K2 y + K3 z + P)
#include <cmath>
#include <cstdio>
struct FnTab { float B[200][4], A[200][4], K1[200][4], K2[200][4], K3[200][4], P[200][4], Q[200][4]; };
extern FnTab g_tab;
inline float4 texfn(int slot, float x, float y, float z) {
    int s = slot % 200; float o[4];
    for (int i = 0; i < 4; i++) {
        float a = (float)g_tab.K1[s][i] * x + (float)g_tab.K2[s][i] * y + (float)g_tab.K3[s][i] * z + g_tab.P[s][i];
        float v = g_tab.B[s][i] + g_tab.A[s][i] * std::sin(a);
        if (g_tab.Q[s][i] > 0) v = std::floor(v) / g_tab.Q[s][i];      // quantised (material / site ids)
        o[i] = v;
    }
    return float4(o[0], o[1], o[2], o[3]);
}
