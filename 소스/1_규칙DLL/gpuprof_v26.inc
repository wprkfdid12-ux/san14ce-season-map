/* ==== v26k54 MEASURE build only: the graphics card's own clock per picture part ("GPU 시계").
   SeasonSync.dll (below the graphics tool) creates the game's device right after it started us: we take its pointer to the creating
   function (found by value in its data) and so see the device and context the graphics tool gets. We put our functions in the
   context's own table for the draw calls and in the device's table for pixel-shader creation; nothing else changes.
   - every pixel shader made is recognised by its compiled code: our ShaderFixes .bin files (DXBC checksum) = ground near / far,
     water, territory colours, foliage, tree shadows, the weather layer; a few of the game's own by the graphics tool's hash
   - one frame every 2 s: the graphics card writes its clock right before and right after every draw that uses one of those
     shaders (the game's own draws are only counted); a frame bracket says whether the clock stayed steady
   - the clock values are read back over the next frames a slice at a time (never waiting for the card)
   Nothing is drawn, nothing is moved, the picture is the game's. Results go to SeasonMeasure.txt with the CPU sampler's. ==== */
typedef long (WINAPI *GP_CreatePS)(void *dev, const void *bc, SIZE_T len, void *link, void **out);
typedef void (WINAPI *GP_DrawIndexed)(void *ctx, u32 n, u32 start, i32 base);
typedef void (WINAPI *GP_Draw)(void *ctx, u32 n, u32 start);
typedef void (WINAPI *GP_DrawIndexedInst)(void *ctx, u32 n, u32 inst, u32 start, i32 base, u32 sinst);
typedef void (WINAPI *GP_DrawInst)(void *ctx, u32 n, u32 inst, u32 start, u32 sinst);
typedef void (WINAPI *GP_Async)(void *ctx, void *q);
typedef long (WINAPI *GP_GetData)(void *ctx, void *q, void *data, u32 size, u32 flags);
typedef void (WINAPI *GP_PSGetShader)(void *ctx, void **ps, void **ci, u32 *nci);
typedef long (WINAPI *GP_CreateQuery)(void *dev, const u32 *desc, void **q);
typedef u32 (WINAPI *GP_Release)(void *obj);
typedef void (WINAPI *GP_GetImm)(void *dev, void **ctx);
/* the methods' places in the tables (d3d11.h order) */
#define GPV_DEV_CREATEPS 15
#define GPV_DEV_CREATEQUERY 24
#define GPV_DEV_GETIMM 40
#define GPV_CTX_DRAWINDEXED 12
#define GPV_CTX_DRAW 13
#define GPV_CTX_DRAWINDEXEDINST 20
#define GPV_CTX_DRAWINST 21
#define GPV_CTX_BEGIN 27
#define GPV_CTX_END 28
#define GPV_CTX_GETDATA 29
#define GPV_CTX_PSGETSHADER 74
static GP_CreatePS gpo_CreatePS; static GP_DrawIndexed gpo_DrawIndexed; static GP_Draw gpo_Draw; static GP_DrawIndexedInst gpo_DrawIndexedInst; static GP_DrawInst gpo_DrawInst;
static GP_Async gpo_Begin, gpo_End; static GP_GetData gpo_GetData; static GP_PSGetShader gpo_PSGetShader;
static void **gp_dev_vt, **gp_ctx_vt; static void *volatile gp_ctx; static int gp_state;   /* 0 not yet, 1 on, -1 off */
static void gp_rel(void *o) { if (o) ((GP_Release)(*(void ***)o)[2])(o); }
/* ---- categories ---- */
#define GP_NCAT 11
static const char *GP_CATN[GP_NCAT] = { "game's own", "ground near", "ground far", "water", "territory colours", "foliage/objects", "tree shadows", "weather layer", "lighting pass", "big unknown (100KB+)", "-" };
typedef struct { const char *file; u8 cat; } GPF;
static const GPF GP_FILES[] = {
    { "d3e1f71e809bac74-ps_replace.bin", 1 }, { "SeasonTerrainNear.ps_5_0.8000.bin", 1 }, { "SeasonGameNear.ps_5_0.8000.bin", 1 }, { "SeasonTestNear0.ps_5_0.8000.bin", 1 }, { "SeasonTestNearA.ps_5_0.8000.bin", 1 },
    { "9addb6cb884c7a7c-ps_replace.bin", 2 }, { "SeasonTerrainFar.ps_5_0.8000.bin", 2 }, { "SeasonGameFar.ps_5_0.8000.bin", 2 }, { "SeasonTestFar0.ps_5_0.8000.bin", 2 }, { "SeasonTestFarA.ps_5_0.8000.bin", 2 },
    { "425be6c323067e73-ps_replace.bin", 3 }, { "SeasonWaterWinter.ps_5_0.8000.bin", 3 },
    { "48bed74946919969-ps_replace.bin", 4 }, { "8f8964766ab0977e-ps_replace.bin", 5 }, { "d28d1d5e4d8d3a3c-ps_replace.bin", 6 },
    { "SeasonRain.ps_5_0.8000.bin", 7 }, { "SeasonRainB.ps_5_0.8000.bin", 7 }, { "SeasonRainDrops.ps_5_0.8000.bin", 7 }, { "SeasonRainDropsB.ps_5_0.8000.bin", 7 },
};
#define GP_NFILES ((int)(sizeof(GP_FILES) / sizeof(GP_FILES[0])))
static u8 gp_sum[GP_NFILES][16]; static u8 gp_sum_ok[GP_NFILES]; static int gp_files_found;
static const u64 GP_GAME_HASH[2] = { 0x70b6fa39daa0880eULL, 0x6642ae48461b1425ULL };   /* the game's lighting pass (the weather layer hangs on it): the graphics tool's hash */
static int gp_eq16(const u8 *a, const u8 *b) { for (int i = 0; i < 16; i++) if (a[i] != b[i]) return 0; return 1; }
static u64 gp_fnv(const u8 *b, SIZE_T n) { u64 h = 0xcbf29ce484222325ULL; for (SIZE_T i = 0; i < n; i++) { h ^= b[i]; h *= 0x100000001b3ULL; } return h; }
static void gp_load_sums(void)
{
    u16 dir[600]; DWORD n = GetModuleFileNameW(self_module, dir, 560); if (!n) return;
    while (n && dir[n - 1] != '\\' && dir[n - 1] != '/') n--; dir[n] = 0; wcat(dir, L"ShaderFixes\\");
    for (int i = 0; i < GP_NFILES; i++) {
        u16 fp[700]; fp[0] = 0; wcat(fp, dir); int k = wlen(fp); for (const char *c = GP_FILES[i].file; *c && k < 690; c++) fp[k++] = (u16)*c; fp[k] = 0;
        HANDLE h = CreateFileW(fp, 0x80000000, 1, 0, 3, 0x80, 0); if (h == (HANDLE)(i64)-1) continue;
        u8 hd[20]; DWORD r = 0; BOOL ok = ReadFile(h, hd, 20, &r, 0); CloseHandle(h);
        if (!ok || r != 20 || hd[0] != 'D' || hd[1] != 'X' || hd[2] != 'B' || hd[3] != 'C') continue;
        memcpy(gp_sum[i], hd + 4, 16); gp_sum_ok[i] = 1; gp_files_found++; }
}
/* shader object -> category (open addressing; a new shader at a freed address overwrites the old entry) */
#define GP_PS_HT 8192
static void *volatile gp_ps_key[GP_PS_HT]; static volatile u8 gp_ps_cat[GP_PS_HT]; static int gp_ps_n; static u32 gp_seen[GP_NCAT];
static u32 gp_slot(void *p) { return (u32)(((u64)p >> 4) * 2654435761u) & (GP_PS_HT - 1); }
static void gp_ps_put(void *p, u8 cat)
{
    if (!p) return; u32 i = gp_slot(p);
    for (int k = 0; k < GP_PS_HT; k++, i = (i + 1) & (GP_PS_HT - 1)) {
        if (gp_ps_key[i] == p) { gp_ps_cat[i] = cat; return; }
        if (!gp_ps_key[i]) { if (gp_ps_n > GP_PS_HT * 3 / 4) return; gp_ps_cat[i] = cat; __sync_synchronize(); gp_ps_key[i] = p; gp_ps_n++; return; } }
}
static u8 gp_ps_get(void *p)
{
    u32 i = gp_slot(p);
    for (int k = 0; k < 64; k++, i = (i + 1) & (GP_PS_HT - 1)) { void *q = gp_ps_key[i]; if (q == p) return gp_ps_cat[i]; if (!q) return 0; }
    return 0;
}
static long WINAPI gp_CreatePS(void *dev, const void *bc, SIZE_T len, void *link, void **out)
{
    long hr = gpo_CreatePS(dev, bc, len, link, out);
    if (hr >= 0 && out && *out && bc && len >= 20) {
        const u8 *b = (const u8 *)bc; u8 cat = 0;
        if (b[0] == 'D' && b[1] == 'X' && b[2] == 'B' && b[3] == 'C')
            for (int i = 0; i < GP_NFILES && !cat; i++) if (gp_sum_ok[i] && gp_eq16(gp_sum[i], b + 4)) cat = GP_FILES[i].cat;
        if (!cat) { u64 h = gp_fnv(b, len); if (h == GP_GAME_HASH[0] || h == GP_GAME_HASH[1]) cat = 8; }
        if (!cat && len >= 100000) cat = 9;
        gp_ps_put(*out, cat); if (cat < GP_NCAT) gp_seen[cat]++; }
    return hr;
}
/* ---- the clock ---- */
#define GP_MAXQ 8192                           /* clock marks per sampled frame: two per measured draw */
#define GP_EVERY 2000
#define GP_READ_SLICE 1024                     /* marks read back per frame */
static void *gp_q[GP_MAXQ], *gp_dj;
static u8 gp_qcat[GP_MAXQ / 2];
static int gp_samp, gp_nq, gp_pend, gp_rd, gp_pend_nq; static u32 gp_draws, gp_pend_draws, gp_frame_over;
static u64 gp_next, gp_pend_t, gp_freq; static float gp_fc = -1.0f; static volatile float *volatile gp_fcp;
static double gp_fms[GP_NCAT]; static u32 gp_fdr[GP_NCAT];
typedef struct { u64 frames, draws, over; double ms[GP_NCAT]; u64 dr[GP_NCAT]; } GPACC;
static GPACC gp_acc[3];                        /* 0 frames without the ground (menus, other screens), 1 with the ground, 2 with the ground and the weather layer */
static u64 gp_lost, gp_disjoint, gp_partial, gp_frames_tried;
static void gp_frame_done(void)
{
    int g = (gp_fdr[1] + gp_fdr[2]) ? (gp_fdr[7] ? 2 : 1) : 0; GPACC *a = &gp_acc[g];
    a->frames++; a->draws += gp_pend_draws; a->over += gp_frame_over;
    for (int c = 0; c < GP_NCAT; c++) { a->ms[c] += gp_fms[c]; a->dr[c] += gp_fdr[c]; }
}
static void gp_read_more(void *c)
{
    if (gp_rd == 0) {                          /* the bracket first: the clock's rate, and whether it stayed steady */
        struct { u64 freq; int disjoint; int pad; } dj = { 0, 0, 0 };
        if (gpo_GetData(c, gp_dj, &dj, 16, 1) != 0) { if (GetTickCount64() - gp_pend_t > 3000) { gp_pend = 0; gp_lost++; } return; }
        if (dj.disjoint || !dj.freq) { gp_pend = 0; gp_disjoint++; return; }
        gp_freq = dj.freq; for (int k = 0; k < GP_NCAT; k++) { gp_fms[k] = 0; gp_fdr[k] = 0; } }
    int end = gp_rd + GP_READ_SLICE; if (end > gp_pend_nq) end = gp_pend_nq;
    for (; gp_rd < end; gp_rd += 2) {
        u64 t0 = 0, t1 = 0;
        if (gpo_GetData(c, gp_q[gp_rd], &t0, 8, 1) != 0 || gpo_GetData(c, gp_q[gp_rd + 1], &t1, 8, 1) != 0) {
            if (GetTickCount64() - gp_pend_t > 3000) { gp_pend = 0; gp_partial++; } return; }
        u8 cat = gp_qcat[gp_rd >> 1]; if (cat >= GP_NCAT) cat = 0;
        if (t1 >= t0) gp_fms[cat] += (double)(t1 - t0) * 1000.0 / (double)gp_freq; gp_fdr[cat]++; }
    if (gp_rd >= gp_pend_nq) { gp_frame_done(); gp_pend = 0; }
}
static void gp_frame(void *c)                  /* the first draw of a new frame (the graphics tool's frame counter moved) */
{
    u64 now = GetTickCount64();
    if (gp_samp) { gpo_End(c, gp_dj); gp_samp = 0; gp_pend = 1; gp_rd = 0; gp_pend_nq = gp_nq; gp_pend_draws = gp_draws; gp_pend_t = now; return; }
    if (gp_pend) { gp_read_more(c); return; }
    if (now >= gp_next) { gp_next = now + GP_EVERY; gpo_Begin(c, gp_dj); gp_samp = 1; gp_nq = 0; gp_draws = 0; gp_frame_over = 0; gp_frames_tried++; }
}
static int gp_pre(void *c)
{
    volatile float *fp = gp_fcp; float fc = fp ? *fp : 0.0f;
    if (fc != gp_fc) { gp_fc = fc; gp_frame(c); }
    if (!gp_samp) return -1;
    gp_draws++;
    void *ps = 0; gpo_PSGetShader(c, &ps, 0, 0); u8 cat = 0; if (ps) { cat = gp_ps_get(ps); gp_rel(ps); }
    if (!cat) return -1;
    if (gp_nq + 2 > GP_MAXQ) { gp_frame_over++; return -1; }
    int k = gp_nq; gp_qcat[k >> 1] = cat; gpo_End(c, gp_q[k]); gp_nq += 2; return k;
}
static void gp_post(void *c, int k) { gpo_End(c, gp_q[k + 1]); }
static void WINAPI gp_DrawIndexed(void *c, u32 n, u32 s, i32 b) { int k = (c == gp_ctx && gp_state == 1) ? gp_pre(c) : -1; gpo_DrawIndexed(c, n, s, b); if (k >= 0) gp_post(c, k); }
static void WINAPI gp_Draw(void *c, u32 n, u32 s) { int k = (c == gp_ctx && gp_state == 1) ? gp_pre(c) : -1; gpo_Draw(c, n, s); if (k >= 0) gp_post(c, k); }
static void WINAPI gp_DrawIndexedInst(void *c, u32 n, u32 i, u32 s, i32 b, u32 si) { int k = (c == gp_ctx && gp_state == 1) ? gp_pre(c) : -1; gpo_DrawIndexedInst(c, n, i, s, b, si); if (k >= 0) gp_post(c, k); }
static void WINAPI gp_DrawInst(void *c, u32 n, u32 i, u32 s, u32 si) { int k = (c == gp_ctx && gp_state == 1) ? gp_pre(c) : -1; gpo_DrawInst(c, n, i, s, si); if (k >= 0) gp_post(c, k); }
static int gp_patch(void **vt, int idx, void *fn, void **orig)
{
    DWORD old = 0; if (!VirtualProtect(&vt[idx], 8, 0x04, &old)) return 0;
    *orig = vt[idx]; vt[idx] = fn; VirtualProtect(&vt[idx], 8, old, &old); return 1;
}
/* called by SeasonSync right after it created a device for the graphics tool (any thread; once per device) */
static void gp_device(void *dev, void *ctx)
{
    if (!dev || gp_state < 0) return;
    void **dvt = *(void ***)dev;
    if (!ctx) { ((GP_GetImm)dvt[GPV_DEV_GETIMM])(dev, &ctx); if (!ctx) return; gp_rel(ctx); }   /* the device keeps its context alive */
    void **cvt = *(void ***)ctx;
    if (gp_state == 1) {                       /* a later device: the same tables = already in; another kind = left alone */
        if (dvt == gp_dev_vt && cvt == gp_ctx_vt) { gp_ctx = ctx; logs("gpu clock: a new device of the same kind - followed"); }
        else logs("gpu clock: a new device of another kind - not followed");
        return; }
    gp_load_sums();
    u32 dq[2] = { 2 /* D3D11_QUERY_TIMESTAMP */, 0 }, dd[2] = { 3 /* D3D11_QUERY_TIMESTAMP_DISJOINT */, 0 };
    GP_CreateQuery cq = (GP_CreateQuery)dvt[GPV_DEV_CREATEQUERY];
    if (cq(dev, dd, &gp_dj) < 0 || !gp_dj) { gp_state = -1; logs("gpu clock: refused (no clock bracket) - off"); return; }
    for (int i = 0; i < GP_MAXQ; i++) if (cq(dev, dq, &gp_q[i]) < 0 || !gp_q[i]) { gp_state = -1; logs("gpu clock: refused (clock marks) - off"); return; }
    gpo_Begin = (GP_Async)cvt[GPV_CTX_BEGIN]; gpo_End = (GP_Async)cvt[GPV_CTX_END]; gpo_GetData = (GP_GetData)cvt[GPV_CTX_GETDATA]; gpo_PSGetShader = (GP_PSGetShader)cvt[GPV_CTX_PSGETSHADER];
    gp_dev_vt = dvt; gp_ctx_vt = cvt; gp_ctx = ctx;
    int ok = gp_patch(dvt, GPV_DEV_CREATEPS, (void *)gp_CreatePS, (void **)&gpo_CreatePS)
          && gp_patch(cvt, GPV_CTX_DRAWINDEXED, (void *)gp_DrawIndexed, (void **)&gpo_DrawIndexed)
          && gp_patch(cvt, GPV_CTX_DRAW, (void *)gp_Draw, (void **)&gpo_Draw)
          && gp_patch(cvt, GPV_CTX_DRAWINDEXEDINST, (void *)gp_DrawIndexedInst, (void **)&gpo_DrawIndexedInst)
          && gp_patch(cvt, GPV_CTX_DRAWINST, (void *)gp_DrawInst, (void **)&gpo_DrawInst);
    if (!ok) { gp_state = -1; logs("gpu clock: could not join the device's tables - off"); return; }
    gp_next = GetTickCount64() + 20000;        /* after the title screen */
    gp_state = 1;
    char b[160], *p = b; p = pstr(p, "gpu clock: on (one frame every 2 s, a mark before and after each season-picture draw); shader files recognised "); p = pint(p, gp_files_found); *p = 0; logs(b);
}
/* SeasonSync's pointers to the function that creates the device (its pick: the 2D mod's d3d11_chain.dll if loaded, else the system's) */
typedef long (WINAPI *GP_CreateDev)(void *, int, void *, unsigned, const void *, unsigned, unsigned, void **, void *, void **);
typedef long (WINAPI *GP_CreateDevSC)(void *, int, void *, unsigned, const void *, unsigned, unsigned, const void *, void **, void **, void *, void **);
static GP_CreateDev gpo_cd; static GP_CreateDevSC gpo_cdsc; static int gp_devices_seen;
static long WINAPI gp_cd(void *a, int b, void *c, unsigned d, const void *e, unsigned f, unsigned g, void **h, void *i, void **j)
{
    long hr = gpo_cd(a, b, c, d, e, f, g, h, i, j);
    if (hr >= 0 && h && *h) { gp_devices_seen++; gp_device(*h, j ? *j : 0); }
    return hr;
}
static long WINAPI gp_cdsc(void *a, int b, void *c, unsigned d, const void *e, unsigned f, unsigned g, const void *sd, void **sc, void **dev, void *fl, void **ctx)
{
    long hr = gpo_cdsc(a, b, c, d, e, f, g, sd, sc, dev, fl, ctx);
    if (hr >= 0 && dev && *dev) { gp_devices_seen++; gp_device(*dev, ctx ? *ctx : 0); }
    return hr;
}
IMP u32 WINAPI GetSystemDirectoryW(u16 *, u32);
static int gp_swap_ptr(u8 *img, void *from, void *to)    /* the one 8-byte value 'from' in the module's writable sections -> 'to' */
{
    if (!img || !from || !readable(img, 0x400)) return 0; u32 pe = *(u32 *)(img + 0x3c); if (pe > 0x1000 || !readable(img + pe, 0x200) || *(u32 *)(img + pe) != 0x4550) return 0;
    int ns = *(u16 *)(img + pe + 6), opt = *(u16 *)(img + pe + 20); u64 *hit = 0; int nh = 0;
    for (int i = 0; i < ns; i++) { u8 *sh = img + pe + 24 + opt + 40 * i; u32 vs = *(u32 *)(sh + 8), va = *(u32 *)(sh + 12), ch = *(u32 *)(sh + 36);
        if (!(ch & 0x80000000u) || !vs || !readable(img + va, vs)) continue;
        for (u32 o = 0; o + 8 <= vs; o += 8) { u64 *q = (u64 *)(img + va + o); if (*q == (u64)from) { hit = q; nh++; } } }
    if (nh != 1) return nh ? -nh : 0;
    *hit = (u64)to; return 1;
}
static void gp_hook_seasonsync(void)
{
    u8 *ss = (u8 *)GetModuleHandleW(L"SeasonSync.dll"); if (!ss) { logs("gpu clock: SeasonSync.dll not loaded - off"); gp_state = -1; return; }
    u16 sp[600]; u32 n = GetSystemDirectoryW(sp, 520); const char *t = "\\d3d11.dll"; for (int i = 0; t[i]; i++) sp[n++] = (u16)t[i]; sp[n] = 0;
    void *sys = GetModuleHandleW(sp), *chain = GetModuleHandleW(L"d3d11_chain.dll");
    void *f1 = chain ? GetProcAddress(chain, "D3D11CreateDevice") : 0; if (!f1 && sys) f1 = GetProcAddress(sys, "D3D11CreateDevice");
    void *f2 = chain ? GetProcAddress(chain, "D3D11CreateDeviceAndSwapChain") : 0; if (!f2 && sys) f2 = GetProcAddress(sys, "D3D11CreateDeviceAndSwapChain");
    gpo_cd = (GP_CreateDev)f1; gpo_cdsc = (GP_CreateDevSC)f2;
    int r1 = f1 ? gp_swap_ptr(ss, f1, (void *)gp_cd) : 0, r2 = f2 ? gp_swap_ptr(ss, f2, (void *)gp_cdsc) : 0;
    char b[200], *p = b; p = pstr(p, "gpu clock: SeasonSync's device creation "); p = pstr(p, chain ? "(through d3d11_chain.dll) " : "(system d3d11) ");
    p = pstr(p, "taken "); p = pint(p, r1); *p++ = '/'; p = pint(p, r2); p = pstr(p, r1 == 1 || r2 == 1 ? " - waiting for the device" : " - not found, off"); *p = 0; logs(b);
    if (r1 != 1 && r2 != 1) gp_state = -1;
}
/* the sampler thread, every 2 s: the graphics tool's frame counter (the direct list), checked there rather than in the draw calls */
static void gp_tick(void)
{
    if (gp_state != 1) return;
    float *L = dl_state == 1 ? dl_list(IP_END) : 0;
    gp_fcp = L ? &L[IP_CTRL * 4] : 0;
}
static void gp_summary(void *hh)
{
    HANDLE h = (HANDLE)hh; static char b[2400]; char *p = b;
    p = pstr(p, "\r\n== GPU CLOCK (graphics card's own clock around each season-picture draw, one frame every 2 s) ");
    p = pstr(p, gp_state == 1 ? "on" : gp_state < 0 ? "OFF (refused)" : "NOT STARTED (no device seen)");
    p = pstr(p, " | frames sampled "); p = pint(p, (i64)gp_frames_tried); p = pstr(p, ", lost "); p = pint(p, (i64)gp_lost); p = pstr(p, ", clock unsteady "); p = pint(p, (i64)gp_disjoint); p = pstr(p, ", half read "); p = pint(p, (i64)gp_partial);
    p = pstr(p, "\r\n  shaders made and recognised:"); for (int c = 0; c < GP_NCAT - 1; c++) { *p++ = ' '; p = pstr(p, GP_CATN[c]); *p++ = '='; p = pint(p, gp_seen[c]); }
    p = pstr(p, " | shader files read "); p = pint(p, gp_files_found); p = pstr(p, "\r\n"); ms_wr(h, b, p);
    static const char *G[3] = { "frames without the ground (menus, other screens)", "frames with the ground", "frames with the ground and the weather layer" };
    for (int g = 0; g < 3; g++) { GPACC *a = &gp_acc[g]; if (!a->frames) continue; p = b;
        double tot = 0; for (int c = 1; c < GP_NCAT; c++) tot += a->ms[c];
        p = pstr(p, "  "); p = pstr(p, G[g]); p = pstr(p, ": frames "); p = pint(p, (i64)a->frames); p = pstr(p, ", draws per frame "); p = pint(p, (i64)(a->draws / a->frames));
        if (a->over) { p = pstr(p, " (marks ran out on "); p = pint(p, (i64)a->over); p = pstr(p, " draws)"); }
        p = pstr(p, " | ms per frame:");
        for (int c = 1; c < GP_NCAT - 1; c++) { if (!a->dr[c]) continue; *p++ = ' '; p = pstr(p, GP_CATN[c]); *p++ = '='; p = ms_dec(p, a->ms[c] / (double)a->frames, 1000); p = pstr(p, " ("); p = pint(p, (i64)(a->dr[c] / a->frames)); p = pstr(p, " draws)"); }
        p = pstr(p, " | all measured "); p = ms_dec(p, tot / (double)a->frames, 1000); p = pstr(p, "\r\n"); ms_wr(h, b, p); }
}
