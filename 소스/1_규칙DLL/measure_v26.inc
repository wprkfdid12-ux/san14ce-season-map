/* ==== v26k35 MEASURE build only: where the frames go while the turn runs.
   1) Every frame: the graphics tool's frame counter (list entry 100.x) is watched ~1000x a second, so every frame's length is
      known. Frames are split into "progress" (within 2.5 s of a day tick) and "idle".
   2) Sampling: about every millisecond the game's window thread and the up-to-three busiest other threads are paused for a few
      microseconds, their instruction address and the return addresses on their stack are read, and they are let go. Nothing is
      locked or allocated while a thread is paused. Addresses are folded to functions with each module's own unwind table (.pdata),
      so the game's functions, the 통합패치 modules, the season rules, the graphics tool and the driver are told apart.
      exclusive = the function the thread was IN; inclusive = every function on the way there (who called it).
   3) Output: SAN14Mods\SeasonRules\SeasonMeasure.txt rewritten every 60 s (whole totals), and SeasonMeasure_stalls.txt one line per
      second that had a frame of 50 ms or more (what the window thread was doing in that second). ==== */
typedef struct { DWORD dwSize, cntUsage, th32ThreadID, th32OwnerProcessID; long tpBasePri, tpDeltaPri; DWORD dwFlags; } THREADENTRY32;
typedef struct { DWORD dwSize, th32ModuleID, th32ProcessID, GlblcntUsage, ProccntUsage, pad0; u8 *modBaseAddr; DWORD modBaseSize, pad1; void *hModule; u16 szModule[256]; u16 szExePath[260]; } MODULEENTRY32W;
typedef struct { u64 ct, et, kt, ut; } THTIMES;
IMP HANDLE WINAPI CreateToolhelp32Snapshot(DWORD, DWORD);
IMP BOOL WINAPI Thread32First(HANDLE, THREADENTRY32 *); IMP BOOL WINAPI Thread32Next(HANDLE, THREADENTRY32 *);
IMP BOOL WINAPI Module32FirstW(HANDLE, MODULEENTRY32W *); IMP BOOL WINAPI Module32NextW(HANDLE, MODULEENTRY32W *);
IMP HANDLE WINAPI OpenThread(DWORD, BOOL, DWORD);
IMP DWORD WINAPI SuspendThread(HANDLE); IMP DWORD WINAPI ResumeThread(HANDLE);
IMP BOOL WINAPI GetThreadContext(HANDLE, void *);
IMP BOOL WINAPI GetThreadTimes(HANDLE, u64 *, u64 *, u64 *, u64 *);
IMP void *WINAPI LoadLibraryW(const u16 *); IMP void *WINAPI GetProcAddress(void *, const char *);
IMP BOOL WINAPI SetThreadPriority(HANDLE, int); IMP HANDLE WINAPI GetCurrentThread(void);

#define MS_MODS 256
typedef struct { u64 base, end; char name[32]; const u32 *pd; u32 npd; int nr; u64 rlo[24], rhi[24]; } MSMOD;
static MSMOD *ms_mod; static int ms_nmod; static u64 ms_exe_base;
static int ms_readable_in(const MSMOD *m, u64 a, u64 n) { for (int i = 0; i < m->nr; i++) if (a >= m->rlo[i] && a + n <= m->rhi[i]) return 1; return 0; }
static int ms_find_mod(u64 a) { int lo = 0, hi = ms_nmod - 1; while (lo <= hi) { int md = (lo + hi) / 2; if (a < ms_mod[md].base) hi = md - 1; else if (a >= ms_mod[md].end) lo = md + 1; else return md; } return -1; }
static void ms_mods_refresh(void)
{
    static MSMOD tmp[MS_MODS]; int n = 0;
    HANDLE s = CreateToolhelp32Snapshot(0x18, GetCurrentProcessId());   /* TH32CS_SNAPMODULE | SNAPMODULE32 */
    if (s == (HANDLE)(i64)-1) return;
    static MODULEENTRY32W me; me.dwSize = sizeof(me);
    for (BOOL ok = Module32FirstW(s, &me); ok && n < MS_MODS; ok = Module32NextW(s, &me)) {
        MSMOD *m = &tmp[n]; m->base = (u64)me.modBaseAddr; m->end = m->base + me.modBaseSize; m->pd = 0; m->npd = 0; m->nr = 0;
        int k = 0; for (; k < 31 && me.szModule[k]; k++) { u16 c = me.szModule[k]; m->name[k] = (char)(c < 0x80 ? (c >= 'A' && c <= 'Z' ? c + 32 : c) : '_'); } m->name[k] = 0;
        /* readable ranges inside the image (some sections may be no-access) */
        for (u64 a = m->base; a < m->end && m->nr < 24; ) { MBI mb; if (VirtualQuery((void *)a, &mb, sizeof(mb)) != sizeof(mb)) break;
            u64 lo = (u64)mb.Base, hi = lo + mb.Size; if (hi > m->end) hi = m->end;
            int rd = mb.State == 0x1000 && !(mb.Protect & 0x101) && (mb.Protect & 0xEE);
            if (rd) { if (m->nr && m->rhi[m->nr - 1] == lo) m->rhi[m->nr - 1] = hi; else { m->rlo[m->nr] = lo; m->rhi[m->nr] = hi; m->nr++; } }
            if (hi <= a) break; a = hi; }
        /* its unwind table */
        if (ms_readable_in(m, m->base, 0x400)) { u32 pe = *(u32 *)(m->base + 0x3c);
            if (pe < 0x1000 && ms_readable_in(m, m->base + pe, 0x108) && *(u32 *)(m->base + pe) == 0x4550 && *(u16 *)(m->base + pe + 24) == 0x20b) {
                u32 rva = *(u32 *)(m->base + pe + 24 + 112 + 3 * 8), sz = *(u32 *)(m->base + pe + 24 + 112 + 3 * 8 + 4);
                if (rva && sz >= 12 && ms_readable_in(m, m->base + rva, sz)) { m->pd = (const u32 *)(m->base + rva); m->npd = sz / 12; } } }
        if (m->base == (u64)game) ms_exe_base = m->base;
        n++; }
    CloseHandle(s);
    for (int i = 1; i < n; i++) { MSMOD t = tmp[i]; int j = i - 1; while (j >= 0 && tmp[j].base > t.base) { tmp[j + 1] = tmp[j]; j--; } tmp[j + 1] = t; }
    for (int i = 0; i < n; i++) ms_mod[i] = tmp[i];
    ms_nmod = n;
}
/* the function an address belongs to: its primary unwind entry's start (chained fragments followed); a leaf function without an
   unwind entry is kept as its 64-byte block, flagged with bit 0 */
static u64 ms_func(u64 a, int *mi)
{
    int k = ms_find_mod(a); *mi = k; if (k < 0) return a & ~0xFFFULL;
    const MSMOD *m = &ms_mod[k]; u32 r = (u32)(a - m->base);
    if (m->pd && m->npd) { int lo = 0, hi = (int)m->npd - 1, f = -1;
        while (lo <= hi) { int md = (lo + hi) / 2; u32 b = m->pd[md * 3], e = m->pd[md * 3 + 1]; if (r < b) hi = md - 1; else if (r >= e) lo = md + 1; else { f = md; break; } }
        if (f >= 0) { u32 b = m->pd[f * 3], u = m->pd[f * 3 + 2];
            for (int c = 0; c < 4; c++) { if (!ms_readable_in(m, m->base + u, 4)) break; const u8 *ui = (const u8 *)(m->base + u); if (!((ui[0] >> 3) & 4)) break;
                u32 off = 4 + 2 * ((ui[2] + 1) & ~1u); if (!ms_readable_in(m, m->base + u + off, 12)) break; const u32 *ch = (const u32 *)(m->base + u + off); b = ch[0]; u = ch[2]; }
            return m->base + b; } }
    return (a & ~0x3FULL) | 1;
}
/* a value on the stack is a return address when it points into a module's readable image right after a call instruction */
static int ms_is_ret(u64 v)
{
    int k = ms_find_mod(v); if (k < 0) return 0; const MSMOD *m = &ms_mod[k]; if (!ms_readable_in(m, v - 7, 7)) return 0;
    const u8 *p = (const u8 *)v;
    if (p[-5] == 0xE8) return 1;                                         /* call rel32 */
    if (p[-6] == 0xFF && p[-5] == 0x15) return 1;                        /* call [rip+x] */
    if (p[-2] == 0xFF && (p[-1] & 0xF8) == 0xD0) return 1;               /* call reg */
    if (p[-3] == 0xFF && (p[-2] & 0xF8) == 0x50) return 1;               /* call [reg+d8] */
    if (p[-4] == 0xFF && (p[-3] & 0xF8) == 0x54) return 1;               /* call [reg+reg*s+d8] */
    if (p[-6] == 0xFF && (p[-5] & 0xF8) == 0x90) return 1;               /* call [reg+d32] */
    if (p[-7] == 0xFF && (p[-6] & 0xF8) == 0x94) return 1;               /* call [reg+reg*s+d32] */
    if (p[-2] == 0xFF && (p[-1] & 0xF8) == 0x10) return 1;               /* call [reg] */
    return 0;
}
/* ---- counters: function -> 16 counts (phase 2 x thread slot 4 x exclusive/inclusive) ---- */
#define MS_HT 32768
typedef struct { u64 key; u32 c[16]; } MSENT;
static MSENT *ms_ht; static int ms_full;
static MSENT *ms_get(u64 key) { u32 i = (u32)((key >> 4) * 2654435761u) & (MS_HT - 1);
    for (int k = 0; k < MS_HT; k++, i = (i + 1) & (MS_HT - 1)) { if (ms_ht[i].key == key) return &ms_ht[i]; if (!ms_ht[i].key) { if (ms_full > MS_HT * 7 / 8) return 0; ms_full++; ms_ht[i].key = key; return &ms_ht[i]; } } return 0; }
#define SEC_HT 1024
typedef struct { u64 key; u32 ex, in; } SECENT;
static SECENT ms_secs[2][SEC_HT]; static int ms_secs_n[2];
static void sec_add(int w, u64 key, int ex) { SECENT *T = ms_secs[w]; u32 i = (u32)((key >> 4) * 2654435761u) & (SEC_HT - 1);
    for (int k = 0; k < SEC_HT; k++, i = (i + 1) & (SEC_HT - 1)) { if (T[i].key == key) { if (ex) T[i].ex++; else T[i].in++; return; } if (!T[i].key) { if (ms_secs_n[w] > SEC_HT * 3 / 4) return; ms_secs_n[w]++; T[i].key = key; T[i].ex = ex ? 1 : 0; T[i].in = ex ? 0 : 1; return; } } }
/* ---- threads ---- */
#define MS_TH 160
typedef struct { DWORD tid; HANDLE h; u64 cpu, dcpu; u64 sbase; int seen; } MSTH;
static MSTH ms_th[MS_TH]; static int ms_nth;
static int ms_slot_th[4] = { -1, -1, -1, -1 }; static DWORD ms_slot_tid[4];
static DWORD ms_self_tid, ms_win_tid;
typedef long (WINAPI *NTQIT)(HANDLE, int, void *, u32, u32 *);
static NTQIT ms_ntqit;
static u64 ms_slot_cpu[2][4]; static u64 ms_slot_samples[2][4];
static void ms_threads_refresh(int phase_progress)
{
    for (int i = 0; i < ms_nth; i++) ms_th[i].seen = 0;
    HANDLE s = CreateToolhelp32Snapshot(4, 0); if (s == (HANDLE)(i64)-1) return;   /* TH32CS_SNAPTHREAD */
    THREADENTRY32 te; te.dwSize = sizeof(te); DWORD pid = GetCurrentProcessId();
    for (BOOL ok = Thread32First(s, &te); ok; ok = Thread32Next(s, &te)) {
        if (te.th32OwnerProcessID != pid || te.th32ThreadID == ms_self_tid) continue;
        int k = 0; for (; k < ms_nth; k++) if (ms_th[k].tid == te.th32ThreadID) break;
        if (k == ms_nth) { if (ms_nth >= MS_TH) continue; HANDLE h = OpenThread(0x2 | 0x8 | 0x40, 0, te.th32ThreadID); if (!h) continue;
            ms_th[k].tid = te.th32ThreadID; ms_th[k].h = h; ms_th[k].cpu = 0; ms_th[k].sbase = 0;
            if (ms_ntqit) { u64 tbi[6]; u32 rl = 0; if (ms_ntqit(h, 0, tbi, 48, &rl) >= 0 && tbi[1]) { u64 teb = tbi[1]; if (readable((void *)(teb + 8), 8)) ms_th[k].sbase = *(u64 *)(teb + 8); } }
            ms_nth++; }
        ms_th[k].seen = 1; }
    CloseHandle(s);
    { int j = 0; for (int i = 0; i < ms_nth; i++) { if (!ms_th[i].seen) { CloseHandle(ms_th[i].h); continue; } if (j != i) ms_th[j] = ms_th[i]; j++; } ms_nth = j; }   /* threads that ended */
    for (int i = 0; i < ms_nth; i++) { u64 ct, et, kt, ut; if (!ms_th[i].seen || !GetThreadTimes(ms_th[i].h, &ct, &et, &kt, &ut)) { ms_th[i].dcpu = 0; continue; }
        u64 c = kt + ut; ms_th[i].dcpu = ms_th[i].cpu ? c - ms_th[i].cpu : 0; ms_th[i].cpu = c; }
    /* slot 0 = the game window's thread; slots 1-3 = the busiest others */
    DWORD pidw = 0; void *w = GetForegroundWindow(); if (w) { DWORD t = GetWindowThreadProcessId(w, &pidw); if (pidw == pid) ms_win_tid = t; }
    int used[MS_TH] = { 0 }; for (int s2 = 0; s2 < 4; s2++) ms_slot_th[s2] = -1;
    for (int i = 0; i < ms_nth; i++) if (ms_th[i].seen && ms_th[i].tid == ms_win_tid) { ms_slot_th[0] = i; used[i] = 1; }
    for (int s2 = 1; s2 < 4; s2++) { int b = -1; for (int i = 0; i < ms_nth; i++) if (ms_th[i].seen && !used[i] && ms_th[i].dcpu > 0 && (b < 0 || ms_th[i].dcpu > ms_th[b].dcpu)) b = i;
        if (b >= 0 && ms_th[b].dcpu > 400000) { ms_slot_th[s2] = b; used[b] = 1; } }   /* > 2% of a core over the 2 s window */
    for (int s2 = 0; s2 < 4; s2++) { ms_slot_tid[s2] = ms_slot_th[s2] >= 0 ? ms_th[ms_slot_th[s2]].tid : 0; if (ms_slot_th[s2] >= 0) ms_slot_cpu[phase_progress][s2] += ms_th[ms_slot_th[s2]].dcpu; }
}
/* ---- frames ---- */
static u32 ms_fh[2][9]; static u64 ms_frames[2]; static double ms_ftime[2];
static const int MS_FB[8] = { 10, 17, 25, 34, 50, 100, 250, 1000 };
typedef struct { float ms; int prog; SYSTEMTIME t; } MSLONG;
static MSLONG ms_long[24]; static int ms_nlong;
static void ms_frame(double ms, int prog)
{
    int b = 0; while (b < 8 && ms >= MS_FB[b]) b++; ms_fh[prog][b]++; ms_frames[prog]++; ms_ftime[prog] += ms;
    if (ms < 50) return;
    int k = ms_nlong < 24 ? ms_nlong++ : -1; if (k < 0) { k = 0; for (int i = 1; i < 24; i++) if (ms_long[i].ms < ms_long[k].ms) k = i; if (ms_long[k].ms >= ms) return; }
    ms_long[k].ms = (float)ms; ms_long[k].prog = prog; GetLocalTime(&ms_long[k].t);
}
static __attribute__((aligned(16))) u8 ms_ctx[1232 + 64];
static void ct_summary(void *h);
/* ---- output ---- */
static u16 ms_path[600], ms_spath[600];
static void ms_name(char **pp, u64 key)
{
    char *p = *pp; int mi = ms_find_mod(key & ~1ULL);
    if (mi < 0) { p = pstr(p, "?"); p = phex(p, key & ~1ULL); }
    else { p = pstr(p, ms_mod[mi].name); *p++ = '+'; p = phex(p, (key & ~1ULL) - ms_mod[mi].base); if (key & 1) *p++ = '*'; }
    *pp = p;
}
static void ms_wr(HANDLE h, const char *b, char *e) { DWORD w; WriteFile(h, b, (DWORD)(e - b), &w, 0); }
static u64 ms_samples_total, ms_ticks; static u64 ms_t0;
static void ms_write_summary(void)
{
    HANDLE h = CreateFileW(ms_path, 0x40000000, 1, 0, 2, 0x80, 0); if (h == (HANDLE)(i64)-1) return;
    static char b[4096]; char *p = b; SYSTEMTIME t; GetLocalTime(&t);
    p = pstr(p, "SeasonMeasure v26k36 - written "); p = pint(p, t.h); *p++ = ':'; p = pint(p, t.mi); *p++ = ':'; p = pint(p, t.s);
    p = pstr(p, "  sampler ticks "); p = pint(p, (i64)ms_ticks); p = pstr(p, " samples "); p = pint(p, (i64)ms_samples_total);
    p = pstr(p, " over s "); p = pint(p, (i64)((GetTickCount64() - ms_t0) / 1000)); p = pstr(p, "\r\n"); ms_wr(h, b, p);
    static const char *PH[2] = { "IDLE", "PROGRESS" };
    for (int ph = 0; ph < 2; ph++) {
        p = b; p = pstr(p, "\r\n== frames "); p = pstr(p, PH[ph]); p = pstr(p, ": "); p = pint(p, (i64)ms_frames[ph]);
        p = pstr(p, " frames, avg ms x10 "); p = pint(p, ms_frames[ph] ? (i64)(ms_ftime[ph] * 10 / ms_frames[ph]) : 0); p = pstr(p, " | <10 ");
        for (int i = 0; i < 9; i++) { if (i) { p = pstr(p, " | "); p = pint(p, MS_FB[i - 1]); p = pstr(p, "+ "); } p = pint(p, ms_fh[ph][i]); }
        p = pstr(p, "\r\n"); ms_wr(h, b, p); }
    p = b; p = pstr(p, "longest frames:"); for (int i = 0; i < ms_nlong; i++) { p = pstr(p, " "); p = pint(p, (i64)ms_long[i].ms); p = pstr(p, ms_long[i].prog ? "P@" : "I@"); p = pint(p, ms_long[i].t.h); *p++ = ':'; p = pint(p, ms_long[i].t.mi); *p++ = ':'; p = pint(p, ms_long[i].t.s); }
    p = pstr(p, "\r\n"); ms_wr(h, b, p);
    static const char *SL[4] = { "window thread", "busy thread A", "busy thread B", "busy thread C" };
    for (int ph = 0; ph < 2; ph++) for (int sl = 0; sl < 4; sl++) {
        if (!ms_slot_samples[ph][sl]) continue;
        p = b; p = pstr(p, "\r\n== "); p = pstr(p, PH[ph]); p = pstr(p, " / "); p = pstr(p, SL[sl]); p = pstr(p, " (tid now "); p = pint(p, ms_slot_tid[sl]);
        p = pstr(p, "): samples "); p = pint(p, (i64)ms_slot_samples[ph][sl]); p = pstr(p, ", cpu ms "); p = pint(p, (i64)(ms_slot_cpu[ph][sl] / 10000)); p = pstr(p, "\r\n"); ms_wr(h, b, p);
        for (int ex = 1; ex >= 0; ex--) { int ci = ph * 8 + sl * 2 + (ex ? 0 : 1);
            p = b; p = pstr(p, ex ? "  IN (exclusive, permille):" : "  UNDER (inclusive, permille):");
            static u32 picked[24]; int np = 0;
            for (int r = 0; r < 24; r++) { u32 best = 0; int bi = -1; for (int i = 0; i < MS_HT; i++) if (ms_ht[i].key && ms_ht[i].c[ci] > best) { int dup = 0; for (int q = 0; q < np; q++) if (picked[q] == (u32)i) dup = 1; if (!dup) { best = ms_ht[i].c[ci]; bi = i; } }
                if (bi < 0) break; picked[np++] = (u32)bi; p = pstr(p, " "); ms_name(&p, ms_ht[bi].key); *p++ = '='; p = pint(p, (i64)((u64)best * 1000 / ms_slot_samples[ph][sl])); }
            p = pstr(p, "\r\n"); ms_wr(h, b, p); } }
    ct_summary(h);
    p = b; p = pstr(p, "\r\nmodules:"); for (int i = 0; i < ms_nmod; i++) { p = pstr(p, " "); p = pstr(p, ms_mod[i].name); *p++ = '@'; p = phex(p, ms_mod[i].base); if (p - b > 3900) { ms_wr(h, b, p); p = b; } }
    p = pstr(p, "\r\n"); ms_wr(h, b, p);
    CloseHandle(h);
}
static int ms_stall_lines;
static void ms_stall_line(int frames, float maxms, int prog, u32 *samples)
{
    if (ms_stall_lines > 3000) return; ms_stall_lines++;
    HANDLE h = CreateFileW(ms_spath, 4, 3, 0, 4, 0x80, 0); if (h == (HANDLE)(i64)-1) return; SetFilePointer(h, 0, 0, 2);
    static char b[2048]; char *p = b; SYSTEMTIME t; GetLocalTime(&t);
    *p++ = '['; p = pint(p, t.h); *p++ = ':'; p = pint(p, t.mi); *p++ = ':'; p = pint(p, t.s); p = pstr(p, "] ");
    p = pstr(p, prog ? "PROGRESS" : "idle"); p = pstr(p, " frames "); p = pint(p, frames); p = pstr(p, " longest ms "); p = pint(p, (i64)maxms);
    for (int w = 0; w < 2; w++) { SECENT *T = ms_secs[w]; p = pstr(p, w ? " || busy thread A samples " : " | window thread samples "); p = pint(p, samples[w]);
    for (int ex = 1; ex >= 0; ex--) { p = pstr(p, ex ? " | IN" : " | UNDER"); static int pk[10]; int np = 0;
        for (int r = 0; r < (ex ? 6 : 10); r++) { u32 best = 0; int bi = -1; for (int i = 0; i < SEC_HT; i++) { u32 v = ex ? T[i].ex : T[i].in; if (T[i].key && v > best) { int d = 0; for (int q = 0; q < np; q++) if (pk[q] == i) d = 1; if (!d) { best = v; bi = i; } } }
            if (bi < 0) break; pk[np++] = bi; p = pstr(p, " "); ms_name(&p, T[bi].key); *p++ = '='; p = pint(p, best); } } }
    p = pstr(p, "\r\n"); ms_wr(h, b, p); CloseHandle(h);
}
/* ---- v26k36 long-frame catcher ("천라지망"): the moment a frame runs past 40 ms, EVERY thread of the game is sampled until the frame
   ends, with each thread's CPU time over that frame, the process's disk reads and page faults. A thread whose instruction address
   sits right after a syscall is waiting in the kernel; the rest are running. One record per frame of 80 ms+ goes to
   SeasonMeasure_catch.txt, and SeasonMeasure.txt gets the running-only totals over all caught frames. ---- */
#define CT_HT 8192
typedef struct { u64 key; u32 ex, in; } CTENT;
static CTENT *ct_ht; static int ct_n;
#define AG_HT 16384
typedef struct { u64 key; u32 c[4]; } AGENT;
static AGENT *ag_ht; static int ag_n; static u64 ag_run[2], ag_all[2], ag_catches[2]; static double ag_ms[2], ag_cpu[2], ag_rd[2], ag_pf[2];
static int ct_on, ct_prog; static u64 ct_cpu0[MS_TH], ct_dcpu[MS_TH]; static u32 ct_run[MS_TH], ct_all[MS_TH]; static u64 ct_ticks, ct_t0;
static u64 ct_io0[6]; static u32 ct_pf0; static u16 ct_path[600]; static u64 ct_bytes; static int ct_records;
typedef BOOL (WINAPI *GPIOC)(HANDLE, u64 *); typedef BOOL (WINAPI *GPMI)(HANDLE, u32 *, u32);
static GPIOC ct_gpioc; static GPMI ct_gpmi;
static void ct_add(u64 key, int ex) { u32 i = (u32)((key >> 4) * 2654435761u) & (CT_HT - 1);
    for (int k = 0; k < CT_HT; k++, i = (i + 1) & (CT_HT - 1)) { if (ct_ht[i].key == key) { if (ex) ct_ht[i].ex++; else ct_ht[i].in++; return; }
        if (!ct_ht[i].key) { if (ct_n > CT_HT * 3 / 4) return; ct_n++; ct_ht[i].key = key; ct_ht[i].ex = ex ? 1 : 0; ct_ht[i].in = ex ? 0 : 1; return; } } }
static void ag_add(u64 key, int ci) { u32 i = (u32)((key >> 4) * 2654435761u) & (AG_HT - 1);
    for (int k = 0; k < AG_HT; k++, i = (i + 1) & (AG_HT - 1)) { if (ag_ht[i].key == key) { ag_ht[i].c[ci]++; return; }
        if (!ag_ht[i].key) { if (ag_n > AG_HT * 3 / 4) return; ag_n++; ag_ht[i].key = key; ag_ht[i].c[ci] = 1; return; } } }
static int ct_io(u64 *io, u32 *pf)
{ int ok = 0; HANDLE p = GetCurrentProcess(); for (int i = 0; i < 6; i++) io[i] = 0; *pf = 0;
  if (ct_gpioc && ct_gpioc(p, io)) ok = 1; if (ct_gpmi) { static u32 pmc[18]; pmc[0] = 72; if (ct_gpmi(p, pmc, 72)) *pf = pmc[1]; } return ok; }
static void ct_begin(int prog)
{
    ct_on = 1; ct_prog = prog; ct_ticks = 0; ct_t0 = GetTickCount64(); for (int i = 0; i < CT_HT; i++) ct_ht[i].key = 0; ct_n = 0;
    for (int i = 0; i < ms_nth; i++) { u64 a, b, kt, ut; ct_run[i] = ct_all[i] = 0; ct_cpu0[i] = GetThreadTimes(ms_th[i].h, &a, &b, &kt, &ut) ? kt + ut : 0; }
    ct_io(ct_io0, &ct_pf0);
}
static void ct_sample(void)
{
    if (ct_ticks++ > 4000) return;                                   /* a loading screen: 4 s of samples is plenty */
    for (int i = 0; i < ms_nth; i++) { MSTH *th = &ms_th[i];
        u64 *ctx = (u64 *)(((u64)ms_ctx + 15) & ~15ULL); *(u32 *)((u8 *)ctx + 0x30) = 0x100001;
        static u64 stk[96]; int ns = 0; u64 rip = 0;
        if (SuspendThread(th->h) == (DWORD)-1) continue;
        if (GetThreadContext(th->h, ctx)) { rip = *(u64 *)((u8 *)ctx + 0xF8); u64 rsp = *(u64 *)((u8 *)ctx + 0x98);
            if (th->sbase && rsp && rsp < th->sbase) { u64 lim = (th->sbase - rsp) / 8; if (lim > 96) lim = 96; for (u64 q = 0; q < lim; q++) stk[q] = ((u64 *)rsp)[q]; ns = (int)lim; } }
        ResumeThread(th->h);
        if (!rip) continue;
        int mi; u64 f0 = ms_func(rip, &mi);
        int wait = mi >= 0 && ms_readable_in(&ms_mod[mi], rip - 2, 2) && ((const u8 *)rip)[-2] == 0x0F && ((const u8 *)rip)[-1] == 0x05;
        ct_all[i]++; if (!wait) ct_run[i]++;
        u64 chain[12]; int nc = 0; chain[nc++] = f0;
        for (int q = 0; q < ns && nc < 12; q++) { u64 v = stk[q]; if (v < 0x10000 || !ms_is_ret(v)) continue; int m2; u64 f = ms_func(v - 1, &m2); int d = 0; for (int z = 0; z < nc; z++) if (chain[z] == f) d = 1; if (!d) chain[nc++] = f; }
        u64 tk = (u64)(i + 1) << 52;
        ct_add(tk | f0, 1); for (int q = 0; q < nc; q++) ct_add(tk | chain[q], 0);
        ag_all[ct_prog]++;
        if (!wait) { ag_run[ct_prog]++; ag_add(f0, ct_prog * 2); for (int q = 0; q < nc; q++) ag_add(chain[q], ct_prog * 2 + 1); } }
}
static void ct_top(char **pp, u64 tk, int ex, int nmax, u32 tot)
{
    char *p = *pp; static int pk[16]; int np = 0;
    for (int r = 0; r < nmax && r < 16; r++) { u32 best = 0; int bi = -1;
        for (int i = 0; i < CT_HT; i++) { if (!ct_ht[i].key || (ct_ht[i].key & ~((1ULL << 52) - 1)) != tk) continue; u32 v = ex ? ct_ht[i].ex : ct_ht[i].in;
            if (v > best) { int d = 0; for (int q = 0; q < np; q++) if (pk[q] == i) d = 1; if (!d) { best = v; bi = i; } } }
        if (bi < 0) break; pk[np++] = bi; *p++ = ' '; ms_name(&p, ct_ht[bi].key & ((1ULL << 52) - 1)); *p++ = '='; p = pint(p, tot ? (i64)((u64)best * 100 / tot) : 0); *p++ = '%'; }
    *pp = p;
}
static void ct_end(double ms)
{
    ct_on = 0; if (ms < 80.0 || ct_ticks < 5) return;
    u64 io[6]; u32 pf; ct_io(io, &pf);
    double tcpu = 0; for (int i = 0; i < ms_nth; i++) { u64 a, b, kt, ut; u64 c = GetThreadTimes(ms_th[i].h, &a, &b, &kt, &ut) ? kt + ut : 0; ct_dcpu[i] = c > ct_cpu0[i] && ct_cpu0[i] ? c - ct_cpu0[i] : 0; tcpu += (double)ct_dcpu[i] / 10000.0; }
    double rd = (double)(io[3] - ct_io0[3]) / 1024.0; double pfd = (double)(pf - ct_pf0);
    ag_catches[ct_prog]++; ag_ms[ct_prog] += ms; ag_cpu[ct_prog] += tcpu; ag_rd[ct_prog] += rd; ag_pf[ct_prog] += pfd;
    if (ct_bytes > 6000000 || ct_records > 4000) return; ct_records++;
    HANDLE h = CreateFileW(ct_path, 4, 3, 0, 4, 0x80, 0); if (h == (HANDLE)(i64)-1) return; SetFilePointer(h, 0, 0, 2);
    static char b[8192]; char *p = b; SYSTEMTIME t; GetLocalTime(&t);
    *p++ = '['; p = pint(p, t.h); *p++ = ':'; p = pint(p, t.mi); *p++ = ':'; p = pint(p, t.s); p = pstr(p, "] ");
    p = pstr(p, ct_prog ? "PROGRESS" : "idle"); p = pstr(p, " frame ms "); p = pint(p, (i64)ms);
    if (day_tick_at) { p = pstr(p, " | since day tick ms "); p = pint(p, (i64)(ct_t0 - day_tick_at)); }
    p = pstr(p, " | sample rounds "); p = pint(p, (i64)ct_ticks); p = pstr(p, " threads "); p = pint(p, ms_nth);
    p = pstr(p, " | cpu ms all threads "); p = pint(p, (i64)tcpu); p = pstr(p, " | disk read KB "); p = pint(p, (i64)rd); p = pstr(p, " ops "); p = pint(p, (i64)(io[0] - ct_io0[0]));
    p = pstr(p, " | page faults "); p = pint(p, (i64)pfd); p = pstr(p, "\r\n");
    int pick[5]; int np = 0; int wi = -1; for (int i = 0; i < ms_nth; i++) if (ms_th[i].tid == ms_win_tid) wi = i;
    for (int r = 0; r < 4; r++) { int bi = -1; for (int i = 0; i < ms_nth; i++) { int d = 0; for (int q = 0; q < np; q++) if (pick[q] == i) d = 1; if (d) continue; if (bi < 0 || ct_dcpu[i] > ct_dcpu[bi]) bi = i; } if (bi < 0) break; pick[np++] = bi; }
    { int d = 0; for (int q = 0; q < np; q++) if (pick[q] == wi) d = 1; if (wi >= 0 && !d) pick[np++] = wi; }
    for (int q = 0; q < np; q++) { int i = pick[q]; u64 tk = (u64)(i + 1) << 52;
        p = pstr(p, "  tid "); p = pint(p, ms_th[i].tid); if (i == wi) p = pstr(p, " [window]"); p = pstr(p, " cpu ms "); p = pint(p, (i64)(ct_dcpu[i] / 10000));
        p = pstr(p, " running "); p = pint(p, ct_all[i] ? (i64)((u64)ct_run[i] * 100 / ct_all[i]) : 0); p = pstr(p, "% | IN"); ct_top(&p, tk, 1, 6, ct_all[i]);
        p = pstr(p, " | UNDER"); ct_top(&p, tk, 0, 14, ct_all[i]); p = pstr(p, "\r\n"); }
    ms_wr(h, b, p); ct_bytes += (u64)(p - b); CloseHandle(h);
}
static void ct_summary(void *h)
{
    static char b[4096]; static const char *PH[2] = { "IDLE", "PROGRESS" };
    for (int ph = 0; ph < 2; ph++) { if (!ag_catches[ph]) continue; char *p = b;
        p = pstr(p, "\r\n== LONG FRAMES 80ms+ / "); p = pstr(p, PH[ph]); p = pstr(p, " (catcher, every thread): frames "); p = pint(p, (i64)ag_catches[ph]);
        p = pstr(p, ", total ms "); p = pint(p, (i64)ag_ms[ph]); p = pstr(p, ", cpu ms of all threads "); p = pint(p, (i64)ag_cpu[ph]);
        p = pstr(p, ", disk read KB "); p = pint(p, (i64)ag_rd[ph]); p = pstr(p, ", page faults "); p = pint(p, (i64)ag_pf[ph]);
        p = pstr(p, ", running samples "); p = pint(p, (i64)ag_run[ph]); p = pstr(p, " of "); p = pint(p, (i64)ag_all[ph]); p = pstr(p, "\r\n"); ms_wr(h, b, p);
        for (int ex = 1; ex >= 0; ex--) { int ci = ph * 2 + (ex ? 0 : 1); p = b; p = pstr(p, ex ? "  RUNNING IN (permille of running samples):" : "  RUNNING UNDER (permille):");
            static int pk[30]; int np = 0;
            for (int r = 0; r < 30; r++) { u32 best = 0; int bi = -1; for (int i = 0; i < AG_HT; i++) if (ag_ht[i].key && ag_ht[i].c[ci] > best) { int d = 0; for (int q = 0; q < np; q++) if (pk[q] == i) d = 1; if (!d) { best = ag_ht[i].c[ci]; bi = i; } }
                if (bi < 0) break; pk[np++] = bi; *p++ = ' '; ms_name(&p, ag_ht[bi].key); *p++ = '='; p = pint(p, ag_run[ph] ? (i64)((u64)best * 1000 / ag_run[ph]) : 0); }
            p = pstr(p, "\r\n"); ms_wr(h, b, p); } }
}
/* ---- the sampler ---- */
static DWORD WINAPI ms_thread(void *arg)
{
    (void)arg; ms_self_tid = GetCurrentThreadId(); SetThreadPriority(GetCurrentThread(), 2);   /* highest: keeps sampling while the game is busy (no bias towards quiet moments) */
#ifndef MS_START_DELAY
#define MS_START_DELAY 20000
#endif
    Sleep(MS_START_DELAY);                                           /* after the title screen is up */
    ms_mod = (MSMOD *)VirtualAlloc(0, sizeof(MSMOD) * MS_MODS, 0x3000, 0x04); ms_ht = (MSENT *)VirtualAlloc(0, sizeof(MSENT) * MS_HT, 0x3000, 0x04);
    ct_ht = (CTENT *)VirtualAlloc(0, sizeof(CTENT) * CT_HT, 0x3000, 0x04); ag_ht = (AGENT *)VirtualAlloc(0, sizeof(AGENT) * AG_HT, 0x3000, 0x04);
    if (!ms_mod || !ms_ht || !ct_ht || !ag_ht) { logs("measure: no memory - off"); return 0; }
    { int n = wlen(logpath); while (n && logpath[n - 1] != '\\') n--; for (int i = 0; i < n; i++) { ms_path[i] = logpath[i]; ms_spath[i] = logpath[i]; } ms_path[n] = 0; ms_spath[n] = 0;
      for (int i = 0; i <= n; i++) ct_path[i] = ms_path[i]; wcat(ms_path, L"SeasonMeasure.txt"); wcat(ms_spath, L"SeasonMeasure_stalls.txt"); wcat(ct_path, L"SeasonMeasure_catch.txt"); DeleteFileW(ms_spath); DeleteFileW(ct_path); }
    { void *wm = LoadLibraryW(L"winmm.dll"); if (wm) { u32 (WINAPI *tbp)(u32) = (u32 (WINAPI *)(u32))GetProcAddress(wm, "timeBeginPeriod"); if (tbp) tbp(1); } }
    ms_ntqit = (NTQIT)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationThread");
    { void *k = GetModuleHandleW(L"kernel32.dll"); ct_gpioc = (GPIOC)GetProcAddress(k, "GetProcessIoCounters"); ct_gpmi = (GPMI)GetProcAddress(k, "K32GetProcessMemoryInfo"); }
    i64 qf = 0; QueryPerformanceFrequency(&qf); double qms = 1000.0 / (double)qf;
    ms_t0 = GetTickCount64(); u64 next_refresh = 0, next_write = ms_t0 + 60000, sec_end = ms_t0 + 1000;
    float last_fc = -1; i64 last_fq = 0; int sec_frames = 0; float sec_max = 0; int sec_prog = 0; u32 sec_samples[2] = { 0, 0 };
    logs("measure: frame and thread sampler running (SeasonMeasure.txt every 60 s, SeasonMeasure_stalls.txt per slow second)");
    for (;;) {
        u64 now = GetTickCount64(); i64 qn; QueryPerformanceCounter(&qn);
        int prog = day_tick_at != 0 && now - day_tick_at < 2500;
        if (now >= next_refresh && !ct_on) { next_refresh = now + 2000; ms_mods_refresh(); ms_threads_refresh(prog); }
        /* frames */
        float *L = dl_state == 1 ? dl_list(IP_END) : 0;
        if (L) { float fc = L[IP_CTRL * 4];
            if (last_fc < 0 || fc < last_fc || fc - last_fc > 1000) { last_fc = fc; last_fq = qn; }
            else if (fc > last_fc) { int nf = (int)(fc - last_fc + 0.5f); if (nf < 1) nf = 1; double ms = (double)(qn - last_fq) * qms / nf;
                if (ct_on) ct_end((double)(qn - last_fq) * qms);
                if (game_in_front()) for (int i = 0; i < nf; i++) { ms_frame(ms, prog); sec_frames++; if (ms > sec_max) sec_max = (float)ms; }
                last_fc = fc; last_fq = qn; } }
        if (L && !ct_on && last_fc >= 0 && (double)(qn - last_fq) * qms > 40.0 && game_in_front()) ct_begin(prog);
        if (ct_on && (!L || !game_in_front())) ct_on = 0;
        if (prog) sec_prog = 1;
        if (ct_on) ct_sample(); else
        /* samples */
        for (int sl = 0; sl < 4; sl++) { int ti = ms_slot_th[sl]; if (ti < 0) continue; MSTH *th = &ms_th[ti];
            u64 *ctx = (u64 *)(((u64)ms_ctx + 15) & ~15ULL); *(u32 *)((u8 *)ctx + 0x30) = 0x100001;   /* CONTEXT_CONTROL */
            static u64 stk[96]; int ns = 0; u64 rip = 0;
            if (SuspendThread(th->h) == (DWORD)-1) continue;
            if (GetThreadContext(th->h, ctx)) { rip = *(u64 *)((u8 *)ctx + 0xF8); u64 rsp = *(u64 *)((u8 *)ctx + 0x98);
                if (th->sbase && rsp && rsp < th->sbase) { u64 lim = (th->sbase - rsp) / 8; if (lim > 96) lim = 96; for (u64 i = 0; i < lim; i++) stk[i] = ((u64 *)rsp)[i]; ns = (int)lim; } }
            ResumeThread(th->h);
            if (!rip) continue;
            ms_samples_total++; ms_slot_samples[prog][sl]++;
            int mi; u64 f0 = ms_func(rip, &mi); u64 chain[12]; int nc = 0; chain[nc++] = f0;
            for (int i = 0; i < ns && nc < 12; i++) { u64 v = stk[i]; if (v < 0x10000 || !ms_is_ret(v)) continue; int m2; u64 f = ms_func(v - 1, &m2); int d = 0; for (int q = 0; q < nc; q++) if (chain[q] == f) d = 1; if (!d) chain[nc++] = f; }
            int ci = prog * 8 + sl * 2; MSENT *e = ms_get(f0); if (e) e->c[ci]++;
            for (int q = 0; q < nc; q++) { MSENT *e2 = ms_get(chain[q]); if (e2) e2->c[ci + 1]++; }
            if (sl < 2) { sec_samples[sl]++; sec_add(sl, f0, 1); for (int q = 0; q < nc; q++) sec_add(sl, chain[q], 0); } }
        ms_ticks++;
        if (now >= sec_end) { sec_end = now + 1000;
            if (sec_max >= 50.0f && game_in_front()) ms_stall_line(sec_frames, sec_max, sec_prog, sec_samples);
            sec_frames = 0; sec_max = 0; sec_prog = 0; sec_samples[0] = sec_samples[1] = 0; for (int w = 0; w < 2; w++) { for (int i = 0; i < SEC_HT; i++) ms_secs[w][i].key = 0; ms_secs_n[w] = 0; } }
        if (now >= next_write) { next_write = now + 60000; ms_write_summary(); }
        Sleep(1);
    }
    return 0;
}
static void measure_start(void) { HANDLE th = CreateThread(0, 0, (void *)ms_thread, 0, 0, 0); if (th) CloseHandle(th); else logs("measure: thread failed"); }
