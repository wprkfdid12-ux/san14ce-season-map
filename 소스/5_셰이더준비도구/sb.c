/* SeasonShaderPrep: builds 3DMigoto's own shader cache for the season shaders (terrain near/far, winter water, rain, rain drops) at install time,
   so the game does not have to compile them on its first start. Same compiler (d3dcompiler_46.dll from the
   game folder), same options and the same cache file name and time stamp 3DMigoto uses itself. */
typedef unsigned long long u64; typedef unsigned int u32; typedef unsigned short u16; typedef long HRESULT;
typedef int BOOL; typedef void *HANDLE; typedef unsigned long DWORD; typedef u64 SIZE_T;
typedef struct { DWORD lo, hi; } FILETIME;
#define IMP __declspec(dllimport)
#define WINAPI __stdcall
IMP u16 *WINAPI GetCommandLineW(void);
IMP void *WINAPI LoadLibraryW(const u16 *);
IMP void *WINAPI GetProcAddress(void *, const char *);
IMP HANDLE WINAPI CreateFileW(const u16 *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
IMP BOOL WINAPI ReadFile(HANDLE, void *, DWORD, DWORD *, void *);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
IMP BOOL WINAPI CloseHandle(HANDLE);
IMP DWORD WINAPI GetFileSize(HANDLE, DWORD *);
IMP BOOL WINAPI GetFileTime(HANDLE, FILETIME *, FILETIME *, FILETIME *);
IMP BOOL WINAPI SetFileTime(HANDLE, const FILETIME *, const FILETIME *, const FILETIME *);
IMP void *WINAPI VirtualAlloc(void *, SIZE_T, DWORD, DWORD);
IMP int WINAPI WideCharToMultiByte(u32, DWORD, const u16 *, int, char *, int, const char *, BOOL *);
IMP void WINAPI ExitProcess(u32);
IMP BOOL WINAPI DeleteFileW(const u16 *);
IMP BOOL WINAPI CopyFileW(const u16 *, const u16 *, BOOL);
typedef struct Blob Blob;
typedef struct { void *qi, *addref; u32 (WINAPI *Release)(Blob *); void *(WINAPI *Ptr)(Blob *); SIZE_T (WINAPI *Size)(Blob *); } BlobVt;
struct Blob { BlobVt *vt; };
typedef struct { const char *name, *def; } Macro;
typedef HRESULT (WINAPI *Compile)(const void *, SIZE_T, const char *, const Macro *, void *, const char *, const char *, u32, u32, Blob **, Blob **);
static void wcat(u16 *d, const u16 *s) { while (*d) d++; while (*s) *d++ = *s++; *d = 0; }
/* v25.8: a short log next to the shaders: which ones were built, and the compiler's message for any that failed */
static HANDLE lg = (HANDLE)(long long)-1;
static void lput(const char *t, int n) { DWORD w; if (lg != (HANDLE)(long long)-1) { if (n < 0) { n = 0; while (t[n]) n++; } WriteFile(lg, t, n, &w, 0); } }
static void lputw(const u16 *t) { char b[400]; int n = WideCharToMultiByte(65001, 0, t, -1, b, sizeof(b), 0, 0); if (n > 1) lput(b, n - 1); }
static int build(Compile cc, const u16 *game, const u16 *src_name, const u16 *bin_name, const char *target, int custom)
{
    static u16 src[700], bin[700]; src[0] = 0; bin[0] = 0;
    wcat(src, game); wcat(src, L"\\ShaderFixes\\"); wcat(src, src_name);
    wcat(bin, game); wcat(bin, L"\\ShaderFixes\\"); wcat(bin, bin_name);
    HANDLE f = CreateFileW(src, 0x80000000, 1, 0, 3, 0x80, 0); if (f == (HANDLE)(long long)-1) return 0;
    FILETIME ft; DWORD n = GetFileSize(f, 0), rd = 0; char *buf = (char *)VirtualAlloc(0, n + 16, 0x3000, 4);
    int ok = buf && GetFileTime(f, 0, 0, &ft) && ReadFile(f, buf, n, &rd, 0) && rd == n; CloseHandle(f); if (!ok) return 0;
    static char apath[1400]; if (!WideCharToMultiByte(0, 0, src, -1, apath, sizeof(apath), 0, 0)) return 0;
    static const Macro ps[2] = { { "PIXEL_SHADER", "" }, { 0, 0 } }, vs[2] = { { "VERTEX_SHADER", "" }, { 0, 0 } };
    Blob *code = 0, *err = 0;
    HRESULT hr = cc(buf, n, apath, custom == 2 ? vs : (custom ? ps : 0), (void *)1, "main", target, 0x8000, 0, &code, &err);
    lputw(src_name); lput(hr < 0 || !code ? ": FAILED\r\n" : ": ok\r\n", -1);
    if (err) { if (hr < 0 || !code) { SIZE_T en = err->vt->Size(err); if (en > 6000) en = 6000; lput((const char *)err->vt->Ptr(err), (int)en); lput("\r\n", 2); } err->vt->Release(err); }
    if (hr < 0 || !code) { DeleteFileW(bin); return 0; }
    HANDLE o = CreateFileW(bin, 0x40000000, 0, 0, 2, 0x80, 0); if (o == (HANDLE)(long long)-1) { code->vt->Release(code); return 0; }
    DWORD wr = 0; ok = WriteFile(o, code->vt->Ptr(code), (DWORD)code->vt->Size(code), &wr, 0) && SetFileTime(o, 0, 0, &ft);
    CloseHandle(o); code->vt->Release(code);
    if (!ok) DeleteFileW(bin);
    return ok;
}
void start(void)
{
    static u16 game[600]; u16 *c = GetCommandLineW(); int i = 0;
    if (*c == '"') { c++; while (*c && *c != '"') c++; if (*c) c++; } else { while (*c && *c != ' ') c++; }
    while (*c == ' ') c++;
    if (*c == '"') c++;
    while (*c && *c != '"' && i < 590) game[i++] = *c++;
    while (i && (game[i - 1] == ' ' || game[i - 1] == '\\')) i--;
    game[i] = 0; if (!i) ExitProcess(2);
    static u16 dll[700]; dll[0] = 0; wcat(dll, game); wcat(dll, L"\\d3dcompiler_46.dll");
    static u16 lp[700]; lp[0] = 0; wcat(lp, game); wcat(lp, L"\\ShaderFixes\\SeasonShaderPrep_log.txt");
    lg = CreateFileW(lp, 0x40000000, 1, 0, 2, 0x80, 0);
    void *m = LoadLibraryW(dll); if (!m) { lput("d3dcompiler_46.dll not loaded\r\n", -1); ExitProcess(3); }
    Compile cc = (Compile)GetProcAddress(m, "D3DCompile"); if (!cc) ExitProcess(4);
    /* the two big season shaders (3DMigoto's custom-shader cache), then the replaced game shaders (its replace cache) */
    int a = build(cc, game, L"SeasonTerrainNear.hlsl", L"SeasonTerrainNear.ps_5_0.8000.bin", "ps_5_0", 1);
    int b = build(cc, game, L"SeasonTerrainFar.hlsl", L"SeasonTerrainFar.ps_5_0.8000.bin", "ps_5_0", 1);
    /* v25.9f: winter water (river ice) is a custom shader used only in winter months; the rest of the year the game's own water */
    int w = build(cc, game, L"SeasonWaterWinter.hlsl", L"SeasonWaterWinter.ps_5_0.8000.bin", "ps_5_0", 1);
    /* v26d: the rain layer over the whole picture (custom shader after the lighting) */
    int r = build(cc, game, L"SeasonRain.hlsl", L"SeasonRain.ps_5_0.8000.bin", "ps_5_0", 1);
    /* v26j: the drops, flakes and splashes as geometry: a vertex shader and a pixel shader from one file */
    int gv = build(cc, game, L"SeasonRainDrops.hlsl", L"SeasonRainDrops.vs_5_0.8000.bin", "vs_5_0", 2);
    int gp = build(cc, game, L"SeasonRainDrops.hlsl", L"SeasonRainDrops.ps_5_0.8000.bin", "ps_5_0", 1);
    /* v26k27: the game's own ground shaders as custom shaders - only in the test build (speed test); skipped quietly when absent */
    { static u16 tp[700]; tp[0] = 0; wcat(tp, game); wcat(tp, L"\\ShaderFixes\\SeasonGameNear.hlsl");
      HANDLE t = CreateFileW(tp, 0x80000000, 1, 0, 3, 0x80, 0);
      if (t != (HANDLE)(long long)-1) { CloseHandle(t);
          build(cc, game, L"SeasonGameNear.hlsl", L"SeasonGameNear.ps_5_0.8000.bin", "ps_5_0", 1);
          build(cc, game, L"SeasonGameFar.hlsl", L"SeasonGameFar.ps_5_0.8000.bin", "ps_5_0", 1); } }
    /* v26k27: + the game's ground (near, far) and water shaders carrying the season picture in the same pass */
    static const u16 *rp[6] = { L"8f8964766ab0977e-ps_replace", L"d28d1d5e4d8d3a3c-ps_replace", L"48bed74946919969-ps_replace",
                                L"d3e1f71e809bac74-ps_replace", L"9addb6cb884c7a7c-ps_replace", L"425be6c323067e73-ps_replace" };
    static const u16 *rv[2] = { L"c9a4e0386624d4d2-vs_replace", L"fbe6dbf68635f4f4-vs_replace" };
    int okr = 1;
    for (int k = 0; k < 8; k++) {
        static u16 s1[80], s2[80]; s1[0] = 0; s2[0] = 0; const u16 *nm = k < 6 ? rp[k] : rv[k - 6];
        wcat(s1, nm); wcat(s1, L".txt"); wcat(s2, nm); wcat(s2, L".bin");
        int okk = build(cc, game, s1, s2, k < 6 ? "ps_5_0" : "vs_5_0", 0);
        /* v26k29: the ground shaders with the kept-colours reuse: should the game's compiler refuse them, the previous ground
           shaders (same picture, without the reuse) take their place, so the season ground never goes missing */
        if (!okk && (k == 3 || k == 4)) {
            static u16 fb[700], dst[700]; fb[0] = 0; dst[0] = 0;
            wcat(fb, game); wcat(fb, L"\\ShaderFixes\\"); wcat(fb, nm); wcat(fb, L".k27");
            wcat(dst, game); wcat(dst, L"\\ShaderFixes\\"); wcat(dst, s1);
            if (CopyFileW(fb, dst, 0)) { lputw(s1); lput(": the previous ground shader is used instead (no reuse)\r\n", -1); okk = build(cc, game, s1, s2, "ps_5_0", 0); }
        }
        if (!okk) okr = 0;
    }
    /* city appearance test (TEST package only): the new-building shader and three replaced object shaders. Missing files are skipped
       quietly and never change the result (the release package has none of them). */
    { static u16 tp[700]; tp[0] = 0; wcat(tp, game); wcat(tp, L"\\ShaderFixes\\CityTestBuild.hlsl");
      HANDLE t = CreateFileW(tp, 0x80000000, 1, 0, 3, 0x80, 0);
      if (t != (HANDLE)(long long)-1) {
          CloseHandle(t);
          build(cc, game, L"CityTestBuild.hlsl", L"CityTestBuild.vs_5_0.8000.bin", "vs_5_0", 2);
          build(cc, game, L"CityTestBuild.hlsl", L"CityTestBuild.ps_5_0.8000.bin", "ps_5_0", 1);
          build(cc, game, L"21b20d4947ce454f-ps_replace.txt", L"21b20d4947ce454f-ps_replace.bin", "ps_5_0", 0);
          build(cc, game, L"8fd83211313bbef4-vs_replace.txt", L"8fd83211313bbef4-vs_replace.bin", "vs_5_0", 0);
          build(cc, game, L"ce87496a46cd5d91-vs_replace.txt", L"ce87496a46cd5d91-vs_replace.bin", "vs_5_0", 0);
          build(cc, game, L"90609d9cc86a8527-vs_replace.txt", L"90609d9cc86a8527-vs_replace.bin", "vs_5_0", 0);
      } }
    if (lg != (HANDLE)(long long)-1) CloseHandle(lg);
    ExitProcess(a && b && w && r && gv && gp ? (okr ? 0 : 6) : 5);
}
int _fltused = 0;
