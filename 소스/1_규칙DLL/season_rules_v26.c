/* SAN14 CE  SeasonRules v26  -  a SAN14 ModCore module (core ABI 0x00010000, rollback EXE image 0x29ff000)
 *
 * Seasonal game rules that go with the season map (v26: the living climate - see engine_v26.inc):
 *   - winter: the big northern rivers freeze. Frozen cells read as "river" (walkable) instead of "great river",
 *             cells with an army on them freeze/thaw late, armies caught on the ice at the thaw may be hurt once
 *   - settled snow slows armies (plain -15 %, hills/mountains -30 %; no slowdown in the dry northwest)
 *   - spring forest fire, summer flood and landslide, autumn earthquake (independent yearly chances,
 *     re-rolled every time a save is loaded), with damage and a line in Action Report > Important + the log
 *
 * Game access is done only on the game's own thread (inside the time step) except the speed factor, which
 * only reads our tables. Every game write is a single field write with floors so nothing reaches 0.
 */
typedef unsigned long long u64; typedef long long i64; typedef unsigned int u32; typedef int i32;
typedef unsigned short u16; typedef unsigned char u8; typedef u64 UPTR; typedef int BOOL; typedef void *HANDLE;
typedef unsigned long DWORD; typedef u64 SIZE_T;
#ifdef HOSTTEST
#define WINAPI
#define IMP
#define EXPORT
#define GAMEABI __attribute__((ms_abi))
#else
#define WINAPI __stdcall
#define IMP __declspec(dllimport)
#define EXPORT __declspec(dllexport)
#define GAMEABI
#endif

IMP void *WINAPI VirtualAlloc(void *, SIZE_T, DWORD, DWORD);
IMP BOOL WINAPI VirtualFree(void *, SIZE_T, DWORD);
IMP DWORD WINAPI GetModuleFileNameW(void *, u16 *, DWORD);
IMP HANDLE WINAPI CreateFileW(const u16 *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
IMP BOOL WINAPI ReadFile(HANDLE, void *, DWORD, DWORD *, void *);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
IMP BOOL WINAPI CloseHandle(HANDLE);
IMP DWORD WINAPI SetFilePointer(HANDLE, long, long *, DWORD);
IMP u64 WINAPI GetTickCount64(void);
IMP DWORD WINAPI GetFileAttributesW(const u16 *);
IMP BOOL WINAPI DeleteFileW(const u16 *);
IMP BOOL WINAPI FlushInstructionCache(HANDLE, const void *, SIZE_T);
IMP HANDLE WINAPI GetCurrentProcess(void);
IMP DWORD WINAPI GetCurrentThreadId(void);
typedef struct { u16 y, mo, dow, d, h, mi, s, ms; } SYSTEMTIME;
IMP void WINAPI GetLocalTime(SYSTEMTIME *);
IMP HANDLE WINAPI CreateThread(void *, SIZE_T, void *, void *, DWORD, DWORD *);
IMP void WINAPI Sleep(DWORD);
IMP HANDLE WINAPI CreateMutexW(void *, BOOL, const u16 *);
IMP DWORD WINAPI WaitForSingleObject(HANDLE, DWORD);
IMP BOOL WINAPI ReleaseMutex(HANDLE);
typedef struct { DWORD type; DWORD pad; u16 wVk, wScan; DWORD dwFlags, time; u64 extra; u8 pad2[8]; } INPUT;   /* 40 bytes on x64 */
IMP u32 WINAPI SendInput(u32, INPUT *, int);
IMP void *WINAPI GetForegroundWindow(void);
IMP DWORD WINAPI GetWindowThreadProcessId(void *, DWORD *);
IMP DWORD WINAPI GetCurrentProcessId(void);
IMP void *WINAPI GetModuleHandleW(const u16 *);
IMP BOOL WINAPI VirtualProtect(void *, SIZE_T, DWORD, DWORD *);
IMP BOOL WINAPI ReadProcessMemory(HANDLE, const void *, void *, SIZE_T, SIZE_T *);
IMP BOOL WINAPI QueryPerformanceCounter(i64 *); IMP BOOL WINAPI QueryPerformanceFrequency(i64 *);
typedef struct { void *Base, *AllocBase; DWORD AllocProtect, pad0; SIZE_T Size; DWORD State, Protect, Type, pad1; } MBI;   /* MEMORY_BASIC_INFORMATION, 48 bytes */
IMP SIZE_T WINAPI VirtualQuery(const void *, MBI *, SIZE_T);   /* v26k18: also used by the self-host (no shared mod loader needed) */

#ifndef HOSTTEST
void *memset(void *d, int c, SIZE_T n) { u8 *p = (u8 *)d; while (n--) *p++ = (u8)c; return d; }
void *memcpy(void *d, const void *s, SIZE_T n) { u8 *p = (u8 *)d; const u8 *q = (const u8 *)s; while (n--) *p++ = *q++; return d; }
int _fltused = 0;
#else
#include <string.h>
#endif

/* ---- ModCore ABI (same layout as the integrated patch's mod_api.h) ---- */
typedef struct {
    u32 size, abi, layout_abi, symbol_count, offset_count, image_size;
    u8 *image; const UPTR *symbols; const u32 *offsets; i32 (*ReadLimits)(void *); const char *profile;
} S14_Resolved;
typedef struct { u8 *target; u32 length; u8 expected[16], replacement[16]; } S14_Hook;
typedef struct {
    u32 size, abi; const S14_Resolved *resolved;
    i32 (GAMEABI *Readable)(const void *, UPTR);
    i32 (GAMEABI *InstallHooks)(const char *, const S14_Hook *, u32);
    u8 *(GAMEABI *MakeRelay)(const void *, const void *);
    void (GAMEABI *FreeRelay)(void *);
    void (GAMEABI *Log)(const char *);
} S14_CoreAPI;

/* ---- game layout (rollback CE KO exe) ---- */
/* v26k18: the game's own code / table places differ between game versions - picked at start from the exe's build stamp.
   (rollback exe 1.0.1.0 of 2026-08 = stamp 6a6d9f72, official 1.0.1.0 update of 2026-10-16 = stamp 6a983c25; found by matching the
   whole functions byte for byte with the relative jumps masked, and the tables by their class names) */
static u32 RVA_GAMEDB = 0x277df18;         /* -> game database (1.0.1 both; v26k41: 0x2780f60 in the 2026-11-05 update) */
#define DB_HEX         0xe270      /* 48400 pointers to 32-byte cells; cell+0x10 terrain, +0x1c army id */
#define DB_DATE        0x871e8     /* -> date: +0x34 year, +0x36 month, +0x37 day, +0x450 day count */
#define DB_FOOT        0x6caf0     /* 501 pointers to areas: +0x10 name, +0x3a capital cell, +0x54 public order (u8, 0..100) */
#define DB_BUILD       0x6da98     /* 3001 pointers to buildings: +0x10 cell, +0x14 endurance (u16) */
#define DB_ARMY        0x7e838     /* 501 pointers to armies: +0x16 troops, +0x1a morale, +0x2a cell */
#define DB_FOOTHOLD    0xdd38      /* 52 pointers to CFootholdData / CCityData, index = area byte +0x35 (as the game's own accessor 0x229d00 does); food at +0x38 */
#define DB_LOG         0x79e90     /* 2001 pointers to log entries: +0x10 date, +0x14 title (wstring), +0x82 flags, +0x86 kind */
typedef struct { u32 stamp; const char *name; u32 vt_army, vt_foot, time_site, time_fn, speed_fn, history, histmgr, emptystr, vt_foothold, vt_citydata, getfood, firerate, sethexfire, vt_logrow; } GameVer;
#define NGVER 3
static const GameVer GVER[NGVER] = {
    { 0x6a6d9f72u, "rollback 1.0.1.0 (stamp 6a6d9f72)", 0x1342458, 0x13a6930, 0x44b5e0, 0x33b7b0, 0x2af6a0, 0x9338c0, 0x16e290, 0x1a47b98, 0x13a6560, 0x13a6640, 0x22bce0, 0x267630, 0x2ed050, 0x13a75b8 },
    { 0x6a983c25u, "official 1.0.1.0 update 2026-10-16 (stamp 6a983c25)", 0x1342458, 0x13a6980, 0x44c590, 0x33c8b0, 0x2b0290, 0x934400, 0x16efc0, 0x1a47b98, 0x13a65b0, 0x13a6690, 0x3ea3b0, 0x268210, 0x2edd40, 0x13a7608 },
    /* v26k41: the game update of 2026-11-05 (stamp 6ad06f53, image 0x2a02000): the same functions moved (matched whole, relative fields
       masked); the unit speed function was rebuilt by the compiler (other registers, same arguments and result) and the food getter
       is now shared with another class (the same two instructions) */
    { 0x6ad06f53u, "update 2026-11-05 (stamp 6ad06f53)", 0x1345458, 0x13a99c0, 0x44e7d0, 0x33eb20, 0x2b2550, 0x936980, 0x16ff70, 0x1a4ab88, 0x13a95f0, 0x13a96d0, 0x22e1d0, 0x269bc0, 0x2f0100, 0x13aa648 } };
/* v26k41: the official-exe-only places below are written as the official 1.0.1 ones; GA() gives the same place in the running exe.
   The 2026-11-05 places were found by matching the whole functions of both exes (instructions the same, only relative fields moved). */
static int gv_off;                 /* 1 = the official exe (1.0.1 of 2026-10-16 or the 2026-11-05 update); 0 = rollback */
static int gv_new;                 /* 1 = the 2026-11-05 update */
static u32 gv_image = 0x29ff000;   /* the running exe's image size */
typedef struct { u32 o, n; } RvPair;
static const RvPair RV_N5[] = {
    { 0x4ba285, 0x4bc655 }, { 0x1864f0, 0x187880 }, { 0x1864ff, 0x18788f }, { 0x1865aa, 0x18793a }, { 0x1865ab, 0x18793b }, { 0x1865af, 0x18793f },
    { 0x9bbb0, 0x9cb10 }, { 0x9bbbc, 0x9cb1c }, { 0x9bbfe, 0x9cb5e }, { 0x9bc43, 0x9cba3 }, { 0x9bc5d, 0x9cbbd }, { 0x22dda0, 0x22f830 },
    { 0x183a30, 0x184d60 }, { 0x183d0a, 0x18503a }, { 0x183d11, 0x185041 }, { 0x183d68, 0x185098 }, { 0x1841b4, 0x1854e4 }, { 0x1841f8, 0x185528 },
    { 0x18ae60, 0x18c1f0 }, { 0x184228, 0x185558 }, { 0x1842e3, 0x185613 }, { 0x184298, 0x1855c8 }, { 0x1842d9, 0x185609 },
    { 0x1343670, 0x1346670 }, { 0x1bb7b00, 0x1bbab50 },
    { 0x1395990, 0x13989c8 }, { 0x1394b2c, 0x1397b4c }, { 0x1a20578, 0x1a23578 }, { 0x2141a80, 0x2144ad0 } };
static u32 ga_miss;                /* a place asked for that the table does not have (a mistake of ours: the part that asked stays off) */
static u32 GA(u32 o)
{
    if (!gv_new) return o;
    for (u32 i = 0; i < sizeof(RV_N5) / sizeof(RV_N5[0]); i++) if (RV_N5[i].o == o) return RV_N5[i].n;
    ga_miss = o; return 0;
}
static u32 VT_ARMY, VT_FOOT, RVA_TIME_SITE, RVA_TIME_FN, RVA_SPEED_FN, RVA_HISTORY, RVA_HISTMGR, RVA_EMPTYSTR, VT_FOOTHOLD, VT_CITYDATA, RVA_GETFOOD, RVA_FIRERATE, RVA_SETHEXFIRE, VT_LOGROW;
static const GameVer *gver;
#define NCELL 48400
#define T_GREATRIVER 14
#define T_RIVER      13
#define T_BLOCKED    19

static const S14_CoreAPI *api; static u8 *game; static void *self_module;
static u8 **dbp(void) { return (u8 **)(game + RVA_GAMEDB); }

/* ---- small helpers ---- */
static u16 logpath[600], datpath[600];
static void wcat(u16 *d, const u16 *s) { while (*d) d++; while (*s) *d++ = *s++; *d = 0; }
static int wlen(const u16 *s) { int n = 0; while (s[n]) n++; return n; }
static char *pstr(char *p, const char *s) { while (*s) *p++ = *s++; return p; }
static char *pint(char *p, i64 v) { char t[24]; int n = 0; if (v < 0) { *p++ = '-'; v = -v; } do { t[n++] = '0' + v % 10; v /= 10; } while (v); while (n) *p++ = t[--n]; return p; }
static char *putf8(char *p, const u16 *s, int max)
{
    for (int i = 0; i < max && s[i]; i++) { u32 c = s[i];
        if (c < 0x80) *p++ = (char)c; else if (c < 0x800) { *p++ = (char)(0xC0 | (c >> 6)); *p++ = (char)(0x80 | (c & 63)); }
        else { *p++ = (char)(0xE0 | (c >> 12)); *p++ = (char)(0x80 | ((c >> 6) & 63)); *p++ = (char)(0x80 | (c & 63)); } }
    return p;
}
static int nlog;
static HANDLE logh; static volatile long loglock;
static void logs(const char *s)
{
    if (nlog > 4000) return;
    while (__sync_lock_test_and_set(&loglock, 1)) Sleep(0);
    nlog++;
    if (!logh) { HANDLE h0 = CreateFileW(logpath, 4, 3, 0, 4, 0x80, 0); if (h0 == (HANDLE)(i64)-1) { __sync_lock_release(&loglock); return; } logh = h0; }   /* v25.4: opened once, kept open */
    HANDLE h = logh;
    SetFilePointer(h, 0, 0, 2);
    char b[32]; SYSTEMTIME t; GetLocalTime(&t); char *p = b;
    *p++ = '['; int v[3] = { t.h, t.mi, t.s };
    for (int i = 0; i < 3; i++) { *p++ = '0' + v[i] / 10; *p++ = '0' + v[i] % 10; if (i < 2) *p++ = ':'; } *p++ = ']'; *p++ = ' ';
    DWORD w; WriteFile(h, b, (DWORD)(p - b), &w, 0);
    int n = 0; while (s[n]) n++; WriteFile(h, s, n, &w, 0); WriteFile(h, "\r\n", 2, &w, 0);
    __sync_lock_release(&loglock);
}
#ifndef HOSTTEST
/* v26k38a: the module + offset of an address (the start notice names whatever holds a place we wanted; the test build's crash
   catcher uses it too) */
static int readable(const void *p, u64 n);
IMP BOOL WINAPI GetModuleHandleExW(DWORD, const void *, void **);
IMP int WINAPI MessageBoxW(void *, const u16 *, const u16 *, u32);
static char *sr_hex(char *p, u64 v) { char t[17]; int n = 0; do { t[n++] = "0123456789abcdef"[v & 15]; v >>= 4; } while (v && n < 16); while (n) *p++ = t[--n]; return p; }
static char *sr_where(char *p, u64 a)
{
    void *m = 0;
    if (!GetModuleHandleExW(6, (const void *)a, &m) || !m) { p = pstr(p, "?0x"); return sr_hex(p, a); }   /* FROM_ADDRESS | UNCHANGED_REFCOUNT */
    u16 w[300]; DWORD n = GetModuleFileNameW(m, w, 290); int k = (int)n; while (k > 0 && w[k - 1] != '\\') k--;
    for (int i = k; i < (int)n && i < k + 40; i++) *p++ = w[i] < 128 ? (char)w[i] : '_';
    p = pstr(p, "+0x"); return sr_hex(p, a - (u64)m);
}
/* where a jump / call written at a place goes (another mod's detour), or 0 */
static u64 sr_jump_dest(const u8 *at)
{
    if (!readable(at, 16)) return 0;
    if (at[0] == 0xe8 || at[0] == 0xe9) { u8 *t = (u8 *)at + 5 + *(int *)(at + 1);
        if (readable(t, 14) && t[0] == 0xff && t[1] == 0x25 && *(int *)(t + 2) == 0) return *(u64 *)(t + 6);   /* through a relay page */
        return (u64)t; }
    if (at[0] == 0xff && at[1] == 0x25) { u8 **pp = (u8 **)(at + 6 + *(int *)(at + 2)); if (readable(pp, 8)) return (u64)*pp; }
    return 0;
}
#endif
#if defined(SEASON_TEST) && !defined(HOSTTEST)
/* v26k38i (test build): a crash catcher. The integrated patch's crash log stopped writing (its file is from 10-26), so a crash left
   nothing. On a fatal fault (bad memory access, bad instruction, divide by zero, stack overflow, heap / buffer check) this writes
   to SeasonRules.log: the code, the faulting place as module + offset, the address touched, and the module + offset of the
   return addresses found on the stack (who called whom). It only looks - the fault goes on to the game / other handlers as before. */
IMP void *WINAPI AddVectoredExceptionHandler(u32, void *);
typedef struct { u32 code, flags; void *rec; void *addr; u32 np, pad; u64 info[15]; } SR_EXREC;
typedef struct { SR_EXREC *rec; u8 *ctx; } SR_EXPTRS;
static volatile long sr_veh_n;
static long GAMEABI sr_veh(SR_EXPTRS *ep)
{
    u32 c = ep->rec->code;
    if (c != 0xC0000005u && c != 0xC000001Du && c != 0xC0000094u && c != 0xC00000FDu && c != 0xC0000409u && c != 0xC0000374u && c != 0xC0000096u) return 0;
    if (__sync_add_and_fetch(&sr_veh_n, 1) > 4) return 0;
    static char b[1800]; char *p = b; u8 *x = ep->ctx;
    p = pstr(p, "FAULT code 0x"); p = sr_hex(p, c); p = pstr(p, " thread "); p = pint(p, (i64)GetCurrentThreadId());
    p = pstr(p, " at "); p = sr_where(p, (u64)ep->rec->addr);
    if (c == 0xC0000005u && ep->rec->np >= 2) { p = pstr(p, ep->rec->info[0] == 0 ? " reading 0x" : ep->rec->info[0] == 1 ? " writing 0x" : " running 0x"); p = sr_hex(p, ep->rec->info[1]); }
    static const char *rn[8] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi" };
    for (int i = 0; i < 8; i++) { p = pstr(p, " "); p = pstr(p, rn[i]); p = pstr(p, "="); p = sr_hex(p, *(u64 *)(x + 0x78 + 8 * i)); }
    p = pstr(p, " | stack:");
    u64 *sp = (u64 *)*(u64 *)(x + 0x98); int got = 0;
    for (int i = 0; i < 160 && got < 14 && p - b < 1600; i++) {
        if (!readable(sp + i, 8)) break; u64 v = sp[i]; if (v < 0x10000) continue;
        void *m = 0; if (!GetModuleHandleExW(6, (const void *)v, &m) || !m) continue;
        p = pstr(p, " "); p = sr_where(p, v); got++; }
    *p = 0;
    if (__sync_lock_test_and_set(&loglock, 1)) { for (int i = 0; i < 2000 && loglock; i++) Sleep(0); }   /* never wait forever: the fault may be inside logs */
    __sync_lock_release(&loglock);
    logs(b);
    return 0;                                                        /* EXCEPTION_CONTINUE_SEARCH */
}
static void sr_crash_catcher(void) { if (AddVectoredExceptionHandler(1, (void *)sr_veh)) logs("crash catcher: on (test build) - a crash writes its place here"); }
#endif
#ifdef HOSTTEST
static int readable(const void *p, u64 n) { return p && api->Readable(p, n); }
#else
/* v26k35: a memory check asks Windows for the region (a system call). The tile-info hook ran a few hundred of them every frame on
   the game's busy thread - measured in game: about half of that thread's time. A region found readable is remembered for 0.1 s
   (64 regions, shared by all threads); a range spanning regions, or anything not found, goes to the full check as before. */
typedef struct { u64 lo, hi, exp; } RCENT;
static RCENT rc_ent[64]; static volatile long rc_lock; static u32 rc_next;
static int readable(const void *p, u64 n)
{
    if (!p) return 0; u64 a = (u64)p, e = a + (n ? n : 1); if (e < a) return 0;
    u64 now = GetTickCount64();
    while (__sync_lock_test_and_set(&rc_lock, 1)) Sleep(0);
    for (int i = 0; i < 64; i++) if (rc_ent[i].exp > now && a >= rc_ent[i].lo && e <= rc_ent[i].hi) { __sync_lock_release(&rc_lock); return 1; }
    __sync_lock_release(&rc_lock);
    MBI m; if (VirtualQuery(p, &m, sizeof(m)) == sizeof(m) && m.State == 0x1000 && !(m.Protect & 0x101) && (m.Protect & 0xEE)) {
        u64 lo = (u64)m.Base, hi = lo + m.Size;
        if (e <= hi) { while (__sync_lock_test_and_set(&rc_lock, 1)) Sleep(0); RCENT *r = &rc_ent[rc_next++ & 63]; r->lo = lo; r->hi = hi; r->exp = now + 100; __sync_lock_release(&rc_lock); return 1; } }
    return api->Readable(p, n);
}
#endif
/* v26: a fast check for the small game objects (one read-memory call, a few microseconds; Readable() walks the whole address
   map and took 60 us each - 2000 log entries made the first turn after a load stall for 0.28 s) */
static int rok(const void *p, u64 n) { u8 tmp[0x800]; SIZE_T got = 0; if (!p || (u64)p < 0x10000 || n > sizeof(tmp)) return 0; return ReadProcessMemory(GetCurrentProcess(), p, tmp, n, &got) && got == n; }

/* ---- random (xorshift, seeded per load) ---- */
static u64 rs = 0x9E3779B97F4A7C15ULL;
static u32 rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (u32)(rs >> 11); }
static int chance(int per10000) { return (int)(rnd() % 10000) < per10000; }
static int rint(int lo, int hi) { return lo + (int)(rnd() % (u32)(hi - lo + 1)); }

/* ---- tables from SeasonRules.dat ---- */
static u8 *dat; static u8 *vterr, *prov, *floodz, *slidez, *firez, *frz[36], *snw[36];
static u16 *fcells; static int nf;            /* cells that can ever freeze */
static u8 *v2base; static u32 v2len;
static int load_tables(void)
{
    HANDLE h = CreateFileW(datpath, 0x80000000, 1, 0, 3, 0x80, 0);
    if (h == (HANDLE)(i64)-1) return 0;
    u32 need = 24 + NCELL * 5 + 36 * (NCELL / 8) + 36 * NCELL;
    u32 need2 = need + NCELL * 64 + NCELL * 4 + 81 * 2;              /* v26: the climate records and the small tables behind the v1 block */
    dat = (u8 *)VirtualAlloc(0, need2 + 4 * NCELL + 131072, 0x3000, 4);   /* v26k14: + the rivers' stretches */
    if (!dat) { CloseHandle(h); return 0; }
    u32 got = 0; DWORD r = 0;
    while (got < need2 + 4 * NCELL + 65536 && ReadFile(h, dat + got, need2 + 4 * NCELL + 65536 - got, &r, 0) && r) got += r;
    CloseHandle(h);
    if (got < need || dat[0] != 'S' || (dat[7] != '1' && dat[7] != '2') || *(u32 *)(dat + 12) != 220 || *(u32 *)(dat + 20) != 36) return 0;
    u8 *p = dat + 24;
    vterr = p; p += NCELL; prov = p; p += NCELL; floodz = p; p += NCELL; slidez = p; p += NCELL; firez = p; p += NCELL;
    for (int t = 0; t < 36; t++) { frz[t] = p; p += NCELL / 8; }
    for (int t = 0; t < 36; t++) { snw[t] = p; p += NCELL; }
    v2base = (dat[7] == '2' && got >= need2) ? p : 0; v2len = got - need;
    fcells = (u16 *)VirtualAlloc(0, NCELL * 2, 0x3000, 4); nf = 0;
    for (int c = 0; c < NCELL; c++) { int any = 0; for (int t = 0; t < 36; t++) if (frz[t][c >> 3] & (1 << (c & 7))) { any = 1; break; }
        if (any && vterr[c] == T_GREATRIVER) fcells[nf++] = (u16)c; }
    return 1;
}
static int frozen_wanted(int t, int c) { return (frz[t][c >> 3] >> (c & 7)) & 1; }

/* ---- game access ---- */
/* Readable() is a VirtualQuery call (a few microseconds). The hot paths below are hit many thousand times per
   turn, so every pointer is checked once and then remembered; a check is repeated only when the pointer changed. */
static u8 *db_ok;                         /* last verified database pointer */
static u8 *db(void) { u8 *d = *dbp(); if (d == db_ok) return d; if (readable(d, 0x80000)) { db_ok = d; return d; } return 0; }
static u8 *hex0;                          /* first cell when the 48400 cells are one contiguous block (checked per session) */
static u8 *hex_db;                        /* database the hex0 check was made for */
static void hex_check(u8 *d)
{
    hex0 = 0; hex_db = d; u8 *h = *(u8 **)(d + DB_HEX); if (!readable(h, (u64)NCELL * 32)) return;
    for (int c = 1; c < NCELL; c++) if (*(u8 **)(d + DB_HEX + 8 * (u64)c) != h + 32 * (u64)c) return;
    hex0 = h;
}
static u8 *cellp(u8 *d, int c) { if (d != hex_db) hex_check(d); if (hex0) return hex0 + 32 * (u64)c;
    u8 *h = *(u8 **)(d + DB_HEX + 8 * (u64)c); return readable(h, 0x20) ? h : 0; }
/* ---- v26k42: cells the player retyped in the terrain editor (TileEdits.dat next to SeasonRules.dat: "S14TILE1", count, then
   cell u16 / from u8 / to u8). The game's cell only changes while it still holds the old type, so a landslide or a frozen river
   on it, or another mod that changed the same cell, is never overwritten. The rules use the new type too (vterr). ---- */
static u16 te_cell[1024]; static u8 te_from[1024], te_to[1024]; static int te_n; static u8 *te_done_db; static int te_done_n;
static void te_load(void)
{
    u16 fp[600]; fp[0] = 0; wcat(fp, datpath); int n = wlen(fp); while (n && fp[n - 1] != '\\') n--; fp[n] = 0; wcat(fp, L"TileEdits.dat");
    HANDLE h = CreateFileW(fp, 0x80000000, 1, 0, 3, 0x80, 0); if (h == (HANDLE)(i64)-1) return;
    static u8 b[16 + 4 * 1024]; DWORD r = 0, got = 0; while (got < sizeof(b) && ReadFile(h, b + got, sizeof(b) - got, &r, 0) && r) got += r; CloseHandle(h);
    if (got < 12 || b[0] != 'S' || b[1] != '1' || b[2] != '4' || b[3] != 'T' || b[4] != 'I' || b[5] != 'L' || b[6] != 'E' || b[7] != '1') { logs("tile edits: file damaged - ignored"); return; }
    u32 cnt = *(u32 *)(b + 8); if (cnt > 1024 || 12 + 4 * cnt > got) { logs("tile edits: file damaged - ignored"); return; }
    for (u32 i = 0; i < cnt; i++) { const u8 *e = b + 12 + 4 * i; u16 c = (u16)(e[0] | (e[1] << 8)); u8 f = e[2], to = e[3];
        if (c >= NCELL || !f || !to || f > 24 || to > 24 || f == to) continue;
        te_cell[te_n] = c; te_from[te_n] = f; te_to[te_n] = to; te_n++; vterr[c] = to; }
    char m[80], *p = m; p = pstr(p, "tile edits: "); p = pint(p, te_n); p = pstr(p, " cells retyped"); *p = 0; logs(m);
}
static void te_apply(void)                  /* any thread: one byte per edited cell, only while it still holds the old type */
{
    if (!te_n) return; u8 **pp = dbp(); if (!readable(pp, 8)) return; u8 *d = *pp; if (!d || !readable(d + DB_HEX, 8 * (u64)NCELL)) return;
    int n = 0; for (int i = 0; i < te_n; i++) { u8 *h = *(u8 **)(d + DB_HEX + 8 * (u64)te_cell[i]); if (!readable(h, 0x20)) continue;
        if (h[0x10] == te_from[i]) { h[0x10] = te_to[i]; n++; } }
    if (n && (d != te_done_db || n != te_done_n)) { te_done_db = d; te_done_n = n; char m[80], *p = m; p = pstr(p, "tile edits: "); p = pint(p, n); p = pstr(p, " game cells set to the new type"); *p = 0; logs(m); }
}
static u8 *army_ok[501];                  /* army pointers already verified (re-verified when the slot changes) */
static u8 *army(u8 *d, int i) { u8 *a = *(u8 **)(d + DB_ARMY + 8 * (u64)i); if (!a) return 0;
    if (a != army_ok[i]) { if (!rok(a, 0x200)) return 0; army_ok[i] = a; }
    if (*(u8 **)a != game + VT_ARMY || *(u16 *)(a + 0x2a) >= NCELL || *(u16 *)(a + 0x16) == 0) return 0; return a; }
/* sorted copy of the live army pointers, rebuilt each day, so the speed hook can check a pointer without a system call */
static u8 *army_idx[501]; static int n_army_idx;
static int army_known(const void *a) { int lo = 0, hi = n_army_idx - 1; while (lo <= hi) { int m = (lo + hi) >> 1; if (army_idx[m] == a) return 1; if (army_idx[m] < (u8 *)a) lo = m + 1; else hi = m - 1; } return 0; }
static u8 *foot_ok[501];
static u8 *foot(u8 *d, int i) { u8 *f = *(u8 **)(d + DB_FOOT + 8 * (u64)i); if (!f) return 0;
    if (f != foot_ok[i]) { if (!rok(f, 0x98)) return 0; foot_ok[i] = f; }
    if (*(u8 **)f != game + VT_FOOT || *(u16 *)(f + 0x3a) >= NCELL) return 0; return f; }
static u8 *bld_ok[3001];
static u8 *building_at(u8 *d, int cell)
{
    for (int i = 1; i <= 3000; i++) { u8 *b = *(u8 **)(d + DB_BUILD + 8 * (u64)i); if (!b) continue;
        if (b != bld_ok[i]) { if (!rok(b, 0x20)) continue; bld_ok[i] = b; }
        if (*(u16 *)(b + 0x10) == cell) return b; }
    return 0;
}
/* the city's food store through the game's own foothold object; returns the amount taken */
static int city_take_food(u8 *d, u8 *area, int pct)
{
    typedef void (GAMEABI *SetF)(u8 *, int);
    int fid = area[0x35]; if (fid > 0x33) return 0;                      /* v25.3: the right field and table (v25.1/25.2 called into the middle of a game function here) */
    u8 *f = *(u8 **)(d + DB_FOOTHOLD + 8 * (u64)fid); if (!rok(f, 0x60)) return 0;
    void **vt = *(void ***)f; if ((u8 *)vt != game + VT_FOOTHOLD && (u8 *)vt != game + VT_CITYDATA) return 0;
    if (!readable(vt, 8 * 25) || (u8 *)vt[20] != game + RVA_GETFOOD) return 0;
    int food = *(i32 *)(f + 0x38); if (food <= 0) return 0;
    int take = food * pct / 100; if (take == 0) return 0; if (take < 0 && food - take > 1500000) return 0;   /* v26k6: a negative share adds (the harvest bonus) */
    ((SetF)vt[21])(f, food - take);
#ifdef SEASON_TEST
    { int now = *(i32 *)(f + 0x38); char b[120], *p = b; p = pstr(p, "test food: before "); p = pint(p, food); p = pstr(p, " wanted "); p = pint(p, food - take);
      p = pstr(p, " now "); p = pint(p, now); p = pstr(p, now == food - take ? " OK" : " MISMATCH"); *p = 0; logs(b); if (now != food - take) take = food - now; }
#endif
    return take;
}
/* a natural disaster of the game itself reported in the last ten days (the title says plague, locusts, flood, drought, quake) */
static u8 *log_ok[2001]; static u8 *last_vanilla;
static const u16 *VANILLA_WORDS[] = { L"역병", L"메뚜기", L"황충", L"홍수", L"범람", L"가뭄", L"지진", L"폭풍", 0 };
static int wfind(const u16 *s, int n, const u16 *w) { int m = wlen(w); for (int i = 0; i + m <= n; i++) { int k = 0; while (k < m && s[i + k] == w[k]) k++; if (k == m) return 1; } return 0; }
static int vanilla_disaster_recent(u8 *d, u32 today)
{
    for (int i = 0; i < 2001; i++) {
        u8 *e = *(u8 **)(d + DB_LOG + 8 * (u64)i); if (!e) continue;
        if (e != log_ok[i]) { if (!rok(e, 0x140)) continue; log_ok[i] = e; }
        if (e[0x86] != 0) continue;                                  /* kind 0 = the important reports */
        u16 dt = *(u16 *)(e + 0x10); int diff = (int)(u16)today - (int)dt; if (diff < 0 || diff > 10) continue;
        u64 size = *(u64 *)(e + 0x24), cap = *(u64 *)(e + 0x2c); if (size == 0 || size > 300) continue;
        const u16 *t = (cap < 8) ? (const u16 *)(e + 0x14) : *(const u16 **)(e + 0x14);
        if (cap >= 8 && !rok(t, size * 2)) continue;
        if (size > 4 && t[0] == '[' && t[1] == 0xc7ac && t[2] == 0xd574 && t[3] == ']') continue;   /* "[재해]" = ours */
        for (int k = 0; VANILLA_WORDS[k]; k++) if (wfind(t, (int)size, VANILLA_WORDS[k])) {
            if (e != last_vanilla) { last_vanilla = e; char b[400], *q = b; q = pstr(q, "vanilla disaster entry: kind "); q = pint(q, e[0x86]); q = pstr(q, " flags "); q = pint(q, e[0x82]);
                q = pstr(q, " speaker "); q = pint(q, *(u16 *)(e + 0x88)); q = pstr(q, " district "); q = pint(q, e[0x12]); q = pstr(q, " title "); q = putf8(q, t, (int)(size < 80 ? size : 80)); *q = 0; logs(b); }
            return 1; }
    }
    return 0;
}
static volatile int g_turn = -1;        /* 0..35, read by the speed hook */
static u16 occ[NCELL];                   /* army index standing on the cell (1..500), 0 none */
static u8 mine[NCELL];                   /* 1 = we froze it, 2 = we blocked it (landslide) */
static u8 flooded[NCELL];                /* 0 none, 1 normal, 2 big - slows armies */
static u8 burning[NCELL];
static u8 quaked[NCELL];
static u8 mud[NCELL];                    /* v26k6: ground the flood water just left - turns of mud, armies at 90% */                 /* v26k3: shaken ground - (strength+1) | turns left << 2: armies slower there (every disaster slows the march, so the AI sees it too) */
/* v26k11: the climate ledger (filled in engine_v26.inc, written by stats_write) */
typedef struct { int year, turns; double rainAct, rainExp; int flStart, fl[4], flTurns[4]; int fires, fireBig, fireGreat, fireCells;
                 int slides, slideMelt, quakes[3]; int snowA, snowN, iceA, iceN, crackMax; int harv[5], harvDone; int moraleArmies; int floodFood; int brake[4]; } YearStat;
static YearStat YS, YSH[12]; static int nYSH;
enum { BR_FIRE, BR_FLOOD, BR_SLIDE, BR_QUAKE };
#ifdef SEASON_TEST
/* ---- test build only (needs SeasonRules_test.flag): forced disasters next to armies + counters ---- */
IMP BOOL WINAPI QueryPerformanceCounter(i64 *); IMP BOOL WINAPI QueryPerformanceFrequency(i64 *);
static int test_on, test_done;
static volatile u32 tq_calls, tq_burn_empty, tq_burn_army, tq_ice_empty, tq_thaw_empty;
static u32 tt_calls, tt_burn_empty, tt_burn_army, tt_ice_empty, tt_thaw_empty, tt_entered, tt_days; static i64 tt_us_max, tt_us_sum;
static u8 test_lit[NCELL];
/* link test: where the time of a disaster day goes (each part on the game thread, in microseconds) */
static i64 q_freq;
static i64 qnow(void) { i64 v; QueryPerformanceCounter(&v); return v; }
static i64 qus(i64 t0) { if (!q_freq) QueryPerformanceFrequency(&q_freq); return q_freq > 0 ? (qnow() - t0) * 1000000 / q_freq : 0; }
static int ntime;
static void tlog(const char *label, i64 t0, int always)
{
    if (!test_on) return; i64 us = qus(t0); if (!always && us < 1000) return; if (ntime++ > 800) return;
    char b[120], *p = b; p = pstr(p, "time: "); p = pstr(p, label); p = pstr(p, " us "); p = pint(p, us); *p = 0; logs(b);
}
#define TB(n) i64 n = test_on ? qnow() : 0
#define TE(n, label, always) tlog(label, n, always)
#else
#define TB(n)
#define TE(n, label, always)
#endif

/* ---- reports: log window; important ones also go to the turn report's Important tab (5th arg = shown to every player) ---- */
static u16 rbuf[256];
static void report_now(const u16 *text, int cell, int important);
typedef struct { u16 text[256]; int cell, important; u32 seq; u64 t0; } PendRep;
static PendRep pend[24]; static int npend;
static u32 ev_seq;                              /* bumped at every disaster start; the visuals of that start carry it */
static int  slot_has_seq(u32 seq);
static int  seq_shown(u32 seq);
static void report_ex(int cell, int important)
{
    if (npend >= 24) { report_now(rbuf, cell, important); return; }
    PendRep *r = &pend[npend++]; memcpy(r->text, rbuf, sizeof(rbuf)); r->cell = cell; r->important = important; r->seq = ev_seq; r->t0 = GetTickCount64();
}
static void flush_reports(int force)            /* game thread: the turn's reports go out together once all their pictures are on the map */
{                                               /* (together, because the game may pause on the first one; at the latest after 12 s) */
    if (!npend) return;
    int ready = force || GetTickCount64() - pend[0].t0 > 12000;
    if (!ready) { ready = 1; for (int i = 0; i < npend; i++) if (slot_has_seq(pend[i].seq) && !seq_shown(pend[i].seq)) { ready = 0; break; } }
    if (!ready) return;
    for (int i = 0; i < npend; i++) report_now(pend[i].text, pend[i].cell, pend[i].important);
    npend = 0;
}
static void report_now(const u16 *text, int cell, int important)
{
    u8 *d = db(); if (!d) return;
    u8 *date = *(u8 **)(d + DB_DATE); if (!readable(date, 0x460)) return;
    typedef u8 *(GAMEABI *Hist)(void *, int, int, int, int, int, const u16 *, const u16 *, int, int, int, int, int);
    typedef void *(GAMEABI *MgrFn)(void);
    void *mgr = ((MgrFn)(game + RVA_HISTMGR))();
    const u16 *empty = *(const u16 **)(game + RVA_EMPTYSTR);
    if (!mgr || !readable(empty, 2)) return;
    int has_place = (cell >= 0 && cell < NCELL);   /* arg 11 sets the "jump to the place" flag (bit 0x10) the log window uses */
    if (!has_place) cell = 0;
    TB(t_rep);
    ((Hist)(game + RVA_HISTORY))(mgr, 0, 0, cell, important ? 1 : 0, *(i32 *)(date + 0x450), text, empty, 0x1394, 0, has_place, 0, 0);
    TE(t_rep, important ? "one news line (log + important tab)" : "one news line (log only)", 1);
    char b[600], *p = b; p = pstr(p, important ? "report: " : "report(log): "); p = putf8(p, text, 200); *p = 0; logs(b);
}
static void report(int cell) { report_ex(cell, 1); }
static void rset(const u16 *s) { rbuf[0] = 0; wcat(rbuf, s); }
static void radd(const u16 *s) { if (wlen(rbuf) + wlen(s) < 250) wcat(rbuf, s); }
static void raddn(int v) { u16 t[16]; int n = 0; char c[16]; char *e = pint(c, v); for (char *q = c; q < e; q++) t[n++] = (u16)*q; t[n] = 0; radd(t); }
static void raddname(u8 *f) { u16 n[8]; for (int i = 0; i < 4; i++) n[i] = ((u16 *)(f + 0x10))[i]; n[4] = 0; radd(n); }
/* v26k13 (asked for): the day a disaster ends - written only when it is over (never ahead: no line says how long a disaster
   WILL last, so nobody can foresee it) */
static u32 today_day(void) { u8 *d = db(); if (!d) return 0; u8 *date = *(u8 **)(d + DB_DATE); if (!readable(date, 0x460)) return 0; return *(u32 *)(date + 0x450); }
static void radd_when(u32 day0)
{
    u8 *d = db(); if (!d) return; u8 *date = *(u8 **)(d + DB_DATE); if (!readable(date, 0x460)) return;
    u32 now = *(u32 *)(date + 0x450); radd(L" ("); raddn(date[0x36]); radd(L"월 "); raddn(date[0x37]); radd(L"일");
    radd(L")"); (void)now; (void)day0;   /* the date only: how long it lasted is left out too (slides and quakes have fixed lengths - a number would teach them) */
}

/* ---- cell geometry (offset rows, approximate) ---- */
static int ccol(int c) { return c % 220; } static int crow(int c) { return c / 220; }
static int cdist2(int a, int b) { int dx = ccol(a) - ccol(b), dy = crow(a) - crow(b); return dx * dx + dy * dy; }
static u8 *nearest_city(u8 *d, int cell, int maxd2)
{
    u8 *best = 0; int bd = maxd2 + 1;
    for (int i = 1; i <= 500; i++) { u8 *f = foot(d, i); if (!f) continue; int c = *(u16 *)(f + 0x3a);
        u8 *h = cellp(d, c); if (!h || (h[0x10] != 16 && h[0x10] != 17)) continue;
        int dd = cdist2(c, cell); if (dd < bd) { bd = dd; best = f; } }
    return best;
}

/* ---- army damage with floors (never to zero) ---- */
static void hurt(u8 *a, int troop_pct, int morale, const char *why)
{
    u16 t0 = *(u16 *)(a + 0x16); u8 m0 = a[0x1a];
    int t1 = t0 - (int)((u32)t0 * (u32)troop_pct / 100u);
    int floor = (int)t0 / 10; if (floor < 1) floor = 1; if (t1 < floor) t1 = floor;
    *(u16 *)(a + 0x16) = (u16)t1;
    int m1 = m0;
    if (m0 >= 1 && m0 <= 150) { m1 = m0 - morale; if (m1 < 10) m1 = m0 < 10 ? m0 : 10; a[0x1a] = (u8)m1; }
    char b[160], *p = b; p = pstr(p, why); p = pstr(p, ": army officer "); p = pint(p, *(u16 *)(a + 0x12));
    p = pstr(p, " cell "); p = pint(p, *(u16 *)(a + 0x2a)); p = pstr(p, " troops "); p = pint(p, t0); p = pstr(p, "->"); p = pint(p, t1);
    p = pstr(p, " morale "); p = pint(p, m0); p = pstr(p, "->"); p = pint(p, m1); *p = 0; logs(b);
}

static void hurt_pct(u8 *a, int troop_pct, int morale_pct, const char *why)   /* v26k38c: morale by a share of what the army has (floors as above) */
{
    u8 m0 = a[0x1a]; int mp = (m0 >= 1 && m0 <= 150) ? ((int)m0 * morale_pct + 50) / 100 : 0; hurt(a, troop_pct, mp, why);
}

#ifdef DIAG28
static volatile i64 th_ticks, th_maxt; static volatile u64 th_n;   /* v26k28d: our own work on the game's time step (every call) */
#endif
/* v26 engine state read by the hooks (defined in engine_v26.inc) */
static int v2; static int cur_turn; static u8 cSnow[NCELL]; static u8 rain[NCELL]; static u8 snowing[NCELL]; static int frz_cur(int c); static int frz_next(int c);
/* ---- the speed hook (any thread): settled snow slows, floods slow; ice is already 'river' terrain ---- */
typedef float (GAMEABI *SpeedFn)(void *, void *, int, int, void *);
static SpeedFn speed_orig;
/* v26k28 (speed): the game asks this for every cell of every route it weighs - during a turn with many armies on the march that is
   hundreds of thousands of calls a step. The slow-down of each cell is now worked out once a day into a table (spd_fac), so a
   call is the game's own answer times one number from the table. The rule itself is unchanged (spd_cell). */
static float spd_fac[NCELL]; static volatile int spd_turn = -1;
static volatile int hook_mode;                     /* 0 table (normal), 1 worked out per call (as before), 2 off (speed test only) */
static volatile u64 sp_calls, sp_samp, sp_tot, sp_orig, fr_calls;
/* v26k30: the lying snow of a cell as the speed rule reads it (the same chain as spd_cell: a flood or the river ice come first; today's
   snowfall raises it - snow thin, a snowstorm deep). One value for the map's snow picture, the tile info and the speed. */
static int cover_lvl(int c, int t)
{
    if (flooded[c] || mine[c] == 1) return 0;
    u8 l = (v2 && cur_turn == t) ? cSnow[c] : snw[t][c]; if (v2 && snowing[c]) { u8 sl = rain[c] >= 3 ? 2 : rain[c] == 2 ? 1 : 0; if (sl > l) l = sl; }
    return l > 2 ? 2 : l;
}
static float spd_cell_nr(int c, int t)               /* v26k38c: everything but the rain (the route cost weighs rain on its own) */
{
    float f = 1.0f;
    if (flooded[c]) f = flooded[c] >= 3 ? 0.35f : flooded[c] == 2 ? 0.45f : 0.60f;
    else if (mine[c] == 1) { f = 0.5f; int thaw = (v2 && cur_turn == t) ? !frz_next(c) : !frozen_wanted(t == 35 ? 0 : t + 1, c); if (thaw) f = 0.3f; }   /* on the ice: half speed; the turn before it thaws, slower still (keeps the AI off thinning ice) */
    else { u8 l = (v2 && cur_turn == t) ? cSnow[c] : snw[t][c]; if (v2 && snowing[c]) { u8 sl = rain[c] >= 3 ? 2 : rain[c] == 2 ? 1 : 0; if (sl > l) l = sl; }   /* v26k5: today's snowfall - sleet nothing, snow thin, a snowstorm deep */
        if (l == 1) f = 0.85f; else if (l == 2) f = 0.70f; }
    if (burning[c] && f > 0.5f) f = 0.5f;                                                        /* a burning cell: everyone, the AI included, routes around it */
    if (quaked[c]) { float fq = (quaked[c] & 3) == 1 ? 0.50f : (quaked[c] & 3) == 2 ? 0.30f : 0.10f; if (f > fq) f = fq; }   /* v26k38c (asked for): shaken ground - quake -50%, strong -70%, great -90% */
    if (mud[c] && f > 0.9f) f = 0.9f;                                                           /* v26k6: mud where the flood drained */
    return f;
}
static float spd_rain(int c)                         /* v26 rain: a little slower, more so in forests and hills (snowfall too: rain[] holds its tier) */
{
    u8 rt = rain[c]; if (rt < 2) return 1.0f; u8 v = vterr[c]; int rough = (v == 5) || (v == 6) || (v >= 8 && v <= 11);
    return rt == 2 ? (rough ? 0.90f : 0.95f) : (rough ? 0.80f : 0.90f);
}
static float spd_cell(int c, int t) { float f = spd_cell_nr(c, t); if (rain[c] >= 2) f *= spd_rain(c); return f; }
/* v26k38c: the route search's step cost per cell (AI and the player's own route). The game weighs a step by the speed of the cell's
   terrain type and keeps one number per type for the whole search (0x1864f0) - so the slow-down of one cell used to stand for every
   cell of that type, whichever came first. Now the speed hook gives that call the terrain's plain speed and the three step-cost calls
   multiply the cost by the cell's own number: 1 / its slow-down, and rain x1.3, a downpour x1.6 (the morale they cost). Nothing is
   blocked by it - only dearer. */
static float rc_mul[NCELL];
static volatile int rc_ok, rc_off, rg_ok;                  /* rc_off: the 'off' way of the test build's A/B (the old weighing) */
static volatile u64 rc_calls, rc_mulled;
static u8 *rc_ret;
static void spd_build(void)                        /* after each day's rules (game thread): the table for this turn */
{
    int t = g_turn; if (t < 0 || !dat) { spd_turn = -1; return; }
    spd_turn = -1; __sync_synchronize();
    for (int c = 0; c < NCELL; c++) { float fn = spd_cell_nr(c, t), f = fn; u8 rt = rain[c]; if (rt >= 2) f *= spd_rain(c); spd_fac[c] = f;
        float m = rt >= 3 ? 1.6f : rt == 2 ? 1.3f : 1.0f; float q = fn < 1.0f ? m / fn : m; rc_mul[c] = q > 4.0f ? 4.0f : q; }   /* v26k38c: at most x4 */
    __sync_synchronize(); spd_turn = t;
}
static float speed_apply(void *a, void *cell, float s)
{
    if (hook_mode == 2) return s;
    int t = g_turn; if (t < 0 || !dat) return s;
    u8 *d = *dbp(); if (!d) return s;
    int c = -1;
    if (cell) { u8 *h0 = *(u8 **)(d + DB_HEX); if (h0) { i64 o = (u8 *)cell - h0; if (o >= 0 && o < (i64)NCELL * 32 && !(o & 31)) c = (int)(o >> 5); } }
    else if (a && army_known(a)) { u16 x = *(u16 *)((u8 *)a + 0x2a); if (x < NCELL) c = x; }
    if (c < 0) return s;
#ifdef SEASON_TEST
    if (test_on) { tq_calls++;
        if (burning[c]) { if (occ[c]) tq_burn_army++; else tq_burn_empty++; }
        if (mine[c] == 1 && !occ[c]) { tq_ice_empty++; if (!frozen_wanted(t == 35 ? 0 : t + 1, c)) tq_thaw_empty++; } }
#endif
    float f = (hook_mode == 0 && spd_turn == t) ? spd_fac[c] : spd_cell(c, t);
    return s * f;
}
static i64 hq_now(void) { i64 v; QueryPerformanceCounter(&v); return v; }
static float GAMEABI SpeedHook(void *a, void *form, int a3, int a4, void *cell)
{
    if (rc_ok && !rc_off && __builtin_return_address(0) == (void *)rc_ret) return speed_orig(a, form, a3, a4, cell);   /* v26k38c: the step cost's terrain speed - plain (the cell's own number comes after) */
    u64 n = ++sp_calls;
    if (n & 1023) return speed_apply(a, cell, speed_orig(a, form, a3, a4, cell));
    i64 t0 = hq_now(); float s = speed_orig(a, form, a3, a4, cell); i64 t1 = hq_now(); s = speed_apply(a, cell, s); i64 t2 = hq_now();   /* one call in 1024 is timed */
    sp_samp++; sp_orig += (u64)(t1 - t0); sp_tot += (u64)(t2 - t0);
    return s;
}
/* v26k38c: the step cost of the route search, per cell (official exe: 0x1864f0 called at 0x1841af, 0x184acc, 0x1853ec - the three
   expansions of the game's route searches; each adds the cost to the cell's distance and compares). cost = the game's own cost
   (from the terrain's plain speed) x the cell's number from the day's table. The number is 1 on most cells: the cost is then the game's own. */
typedef int (GAMEABI *RouteCostFn)(int base, int flag, void *cell, void *unit, void *a5, void *a6, void *a7);
static RouteCostFn rc_orig;
static int GAMEABI RouteCost(int base, int flag, void *cell, void *unit, void *a5, void *a6, void *a7)
{
    /* v26k51: the game writes the 5th-7th arguments 8 bytes at a time; read them back 8 bytes at a time as well. The compiler read two
       of them as one 16-byte load, which the CPU cannot take straight from the two 8-byte writes just made - it waited for the writes
       to finish (seen on the big map as a third of the busy samples at the top of this function). Same values, same result. */
    void *b5 = *(void *volatile *)&a5, *b6 = *(void *volatile *)&a6, *b7 = *(void *volatile *)&a7;
    int r = rc_orig(base, flag, cell, unit, b5, b6, b7);
    rc_calls++;
    if (!rc_ok || rc_off || !cell || r <= 0 || spd_turn != g_turn) return r;
    u8 *d = *dbp(); if (!d) return r; u8 *h0 = *(u8 **)(d + DB_HEX); if (!h0) return r;
    i64 o = (u8 *)cell - h0; if (o < 0 || o >= (i64)NCELL * 32 || (o & 31)) return r;
    float m = rc_mul[o >> 5]; if (m == 1.0f) return r;
    float v = (float)r * m + 0.5f; if (v > 100000000.0f) v = 100000000.0f; rc_mulled++;
    return (int)v;
}
/* v26k38c: the second place for the same weight, used only when the function's first bytes are taken: a few bytes in, where the base
   cost is taken - the base is scaled and the function divides it by the terrain speed as usual (cost x factor, the same result) */
static int GAMEABI rc_scale(int base, int flag, void *cell, void *unit)
{
    rc_calls++;
    if (!rc_ok || rc_off || !cell || !unit || !(flag & 1) || base <= 0 || spd_turn != g_turn) return base;
    u8 *d = *dbp(); if (!d) return base; u8 *h0 = *(u8 **)(d + DB_HEX); if (!h0) return base;
    i64 o = (u8 *)cell - h0; if (o < 0 || o >= (i64)NCELL * 32 || (o & 31)) return base;
    float m = rc_mul[o >> 5]; if (m == 1.0f) return base;
    float v = (float)base * m + 0.5f; if (v > 100000000.0f) v = 100000000.0f; rc_mulled++;
    return (int)v;
}

/* ---- winter rivers ---- */
static int thaw_year[512];
static int g_year;
static void rivers(u8 *d, int t)
{
    int froze = 0, thawed = 0, held = 0;
    for (int k = 0; k < nf; k++) {
        int c = fcells[k]; u8 *h = cellp(d, c); if (!h) continue;
        u8 cur = h[0x10]; int want = (v2 && cur_turn == t) ? frz_cur(c) : frozen_wanted(t, c);
        if (want) {
            if (cur == T_GREATRIVER && !occ[c]) { h[0x10] = T_RIVER; mine[c] = 1; froze++; }
            else if (cur == T_RIVER) mine[c] = 1;
        } else if (cur == T_RIVER && vterr[c] == T_GREATRIVER) {
            if (occ[c]) {                                       /* army on the ice: keep the ice until it leaves */
                held++;
                u8 *a = army(d, occ[c]);
                if (a && thaw_year[occ[c]] != g_year) { thaw_year[occ[c]] = g_year; if (chance(2500)) hurt(a, 15, 15, "thaw"); }
            } else { h[0x10] = T_GREATRIVER; mine[c] = 0; thawed++; }
        }
    }
    if (froze || thawed || held) { char b[120], *p = b; p = pstr(p, "rivers turn "); p = pint(p, t); p = pstr(p, ": froze "); p = pint(p, froze);
        p = pstr(p, ", thawed "); p = pint(p, thawed); p = pstr(p, ", kept for armies "); p = pint(p, held); *p = 0; logs(b); }
}

/* ---- disasters ---- */
enum { EV_FIRE = 1, EV_FLOOD, EV_SLIDE, EV_QUAKE };
typedef struct { int type, region, start, turns_left, big, cell, active; u16 cells[8]; int ncells; u8 saved[8]; u32 day0; } Event;
typedef struct { int cell, region, tier, left; u32 day0; } QTrack; static QTrack qtrack[8];   /* v26k13: the shaken ground of each quake, until it settles */
static Event ev[48]; static int nev;

/* ---- what the map shows: up to 12 slots sent to d3dx.ini as key presses (see the ini) ----
   value = (type*16 + n) * 65536 + cell;  1 fire (n turns burning), 2 flood (cell = river, n = big), 3 landslide, 4 quake (n = direction),
   5 burnt forest (n = turns since it went out). The batch is resent every turn so a missed key press heals itself. */
typedef struct { int type, cell, p, left, born; u32 seq; } Vis;   /* p: the turn it began (fire, landslide, burnt) or a parameter */
static Vis vis[16];
static volatile int vis_send;                  /* set on the game thread, consumed by the sender thread */
static u32 vis_val[16], vis_vseq[16]; static int vis_evn;   /* snapshot for the sender thread */
static u32 sent_val[16]; static int sent_evn; static int resend[16];   /* sender thread only */
static volatile u32 shown_seq[16];             /* sender -> game thread: the event a slot showed when it was sent */
static int  slot_has_seq(u32 seq) { if (!seq) return 0; for (int i = 0; i < 16; i++) if (vis[i].type && vis[i].seq == seq) return 1; return 0; }
static int  seq_shown(u32 seq) { for (int i = 0; i < 16; i++) if (vis[i].type && vis[i].seq == seq && shown_seq[i] == seq) return 1; return 0; }
/* ---- v26 (from the link test, now always on): the direct link to the map. The season patch ships its own graphics tool (3DMigoto 1.3.16 x64, the official
   build, d3d11.dll in the game folder). The numbers it hands to the map shaders live in one list inside it. Instead of pressing
   keys, the disaster slots are written straight into that list (list index 60: x marker 7171 set by d3dx.ini, y echo = x61 + 1
   computed by d3dx.ini every frame, z = bank * 256 + slots in use, w = 1 while the link is on; 64..79 bank 0, 80..95 bank 1).
   Checked first: the exact build (size + the code that reads the list), the marker, and an echo through d3dx.ini.
   On any doubt the key path stays. ---- */
static volatile int dl_state;            /* 0 not decided, 1 working, -1 off (keys) */
static volatile int weather_resend; static void rain_calibrate(void); static void perf_ab(void); static void diag_tick(void);
static int dl_tries, dl_writes, dl_rearm, dl_said;
static u8 *dl_mod; static u32 dl_bank; static volatile long dl_lock; static u64 dl_next;
static char *phex(char *p, u64 v) { char t[20]; int n = 0; do { int k = (int)(v & 15); t[n++] = (char)(k < 10 ? '0' + k : 'a' + k - 10); v >>= 4; } while (v); *p++ = '0'; *p++ = 'x'; while (n) *p++ = t[--n]; return p; }
static void dl_say(int bit, const char *s) { if (dl_said & bit) return; dl_said |= bit; logs(s); }
static float *dl_list(int need)
{
    if (!dl_mod) return 0;
    u8 *G = *(u8 **)(dl_mod + 0x2eb8e8); if (!readable(G, 0xbb0)) return 0;
    float *b = *(float **)(G + 0xb98), *e = *(float **)(G + 0xba0);
    if (!b || e < b || (e - b) / 4 < need) return 0;
    return readable(b, (u64)need * 16) ? b : 0;
}
static int dl_check_build(u8 *m)
{
    if (!m || !readable(m, 0x400)) return 0;
    u32 pe = *(u32 *)(m + 0x3c); u8 *nt = m + pe; if (pe > 0x1000 || !readable(nt, 0x108) || *(u32 *)nt != 0x4550) return 0;
    static const u32 RV[4] = { 0xf7f43, 0xf7f4f, 0xf7f56, 0x8638f };
    static const u8 BY[4][7] = { {0x48,0x8b,0x15,0x9e,0x39,0x1f,0x00}, {0x4c,0x8b,0x82,0xa0,0x0b,0x00,0x00}, {0x4c,0x2b,0x82,0x98,0x0b,0x00,0x00}, {0x48,0x8b,0x0d,0x52,0x55,0x26,0x00} };
    int ok = *(u32 *)(nt + 0x50) == 0x32b000;
    for (int i = 0; i < 4 && ok; i++) { u8 *c = m + RV[i]; if (!readable(c, 8)) return 0; for (int j = 0; j < 7; j++) if (c[j] != BY[i][j]) ok = 0; }
    char b[160], *p = b; p = pstr(p, "direct: graphics tool stamp "); p = phex(p, *(u32 *)(nt + 8)); p = pstr(p, " size "); p = phex(p, *(u32 *)(nt + 0x50));
    p = pstr(p, ok ? " - the known build" : " - a different build (keys stay)"); *p = 0; logs(b);
    return ok;
}
/* v26k29: list entry 97.x counts every change the rules module makes to the map's data (day weather, cell states, picture slots).
   The ground shader keeps each pixel's season colours from the frame before and uses them again only while this count (and the
   pixel's own inputs) stay the same. The count only grows during a game session, so an old value never comes back. */
static u32 dl_epoch;
static void dl_epoch_bump(float *L) { __sync_synchronize(); dl_epoch = (dl_epoch + 1u) & 0xFFFFFFu; if (!dl_epoch) dl_epoch = 1u; L[97 * 4] = (float)dl_epoch; __sync_synchronize(); }
static void dl_write(const u32 *vals, int n, int from_game)
{
    while (__sync_lock_test_and_set(&dl_lock, 1)) Sleep(0);
    float *L = dl_list(98);
    if (!L || L[240] != 7171.0f) { __sync_lock_release(&dl_lock); dl_state = -1; vis_send = 1; logs("direct: the numbers list changed - back to keys"); return; }
    u32 nb = dl_bank ^ 1u; float *bank = L + (64 + nb * 16) * 4;
    for (int i = 0; i < 64; i++) bank[i] = (i < n && i < 16) ? (float)vals[i] : 0.0f;
    __sync_synchronize();
    L[242] = (float)(nb * 256u + (u32)(n > 16 ? 16 : n));
    L[243] = 1.0f;
    dl_epoch_bump(L);                          /* v26k29: new picture slots: the ground's kept colours are drawn afresh */
    dl_bank = nb; dl_writes++;
    __sync_lock_release(&dl_lock);
    for (int i = 0; i < 16; i++) if (vis_vseq[i]) shown_seq[i] = vis_vseq[i];   /* on the map now: the news may go out */
    char b[100], *p = b; p = pstr(p, "direct: wrote "); p = pint(p, n); p = pstr(p, " slot(s), bank "); p = pint(p, nb); p = pstr(p, from_game ? "" : " (sender)"); *p = 0; logs(b);
}
static void dl_probe(void)              /* sender thread, every 2 s until decided (the game may still be loading) */
{
    if (++dl_tries > 450) { dl_state = -1; vis_send = 1; logs("direct: gave up after 15 minutes - keys stay"); return; }
    if (!dl_mod) {
        u16 p[600]; DWORD n = GetModuleFileNameW(self_module, p, 560); if (!n) return;
        while (n && p[n - 1] != '\\' && p[n - 1] != '/') n--; p[n] = 0; wcat(p, L"d3d11.dll");
        u8 *m = (u8 *)GetModuleHandleW(p); if (!m) m = (u8 *)GetModuleHandleW(L"d3d11.dll");
        if (!m) { dl_say(1, "direct: the graphics tool is not loaded yet"); return; }
        if (!dl_check_build(m)) { dl_state = -1; vis_send = 1; return; }
        dl_mod = m;
    }
    float *L = dl_list(96);
    if (!L) { dl_say(2, "direct: the numbers list is not there yet (or shorter than 96)"); return; }
    if (L[240] != 7171.0f) { dl_say(4, "direct: marker 7171 not set yet (d3dx.ini not read yet?)"); return; }
    float nonce = (float)(1000 + (int)(GetTickCount64() % 50000));
    L[244] = nonce; Sleep(500);
    L = dl_list(96); if (!L) return;
    float echo = L[241];
    { char b[120], *p = b; p = pstr(p, "direct: echo test wrote "); p = pint(p, (i64)nonce); p = pstr(p, " read back "); p = pint(p, (i64)echo); p = pstr(p, echo == nonce + 1.0f ? " (+1 = the graphics tool used it)" : " (not yet)"); *p = 0;
      if (echo == nonce + 1.0f || dl_tries % 10 == 1) logs(b); }
    if (echo != nonce + 1.0f) return;
    dl_state = 1;
    logs("direct: link works - the map gets its disaster slots without key presses");
    dl_write(vis_val, vis_evn, 0);
}
static void dl_tick(void)               /* sender thread */
{
    u64 now = GetTickCount64(); if (now < dl_next) return; dl_next = now + 2000;
    if (dl_state == 0) { dl_probe(); return; }
    if (dl_state != 1) return;
    float *L = dl_list(96);                                       /* the graphics tool zeroes its list when it starts over: write again */
    if (!L || L[240] != 7171.0f) { dl_state = -1; vis_send = 1; logs("direct: the numbers list is gone - back to keys"); return; }
    if (L[243] != 1.0f) { dl_rearm++; logs("direct: the graphics tool started over - slots written again"); dl_write(vis_val, vis_evn, 0); weather_resend = 1; }
}
static void vis_snapshot(void)
{
    int n = 0;
    for (int i = 0; i < 16; i++) { u32 v = 0;
        if (vis[i].type) { v = (u32)vis[i].cell + 48400u * (u32)(vis[i].type * 36 + vis[i].p); n = i + 1; }
        vis_val[i] = v; vis_vseq[i] = vis[i].type ? vis[i].seq : 0; }
    vis_evn = n;
    if (dl_state == 1) { dl_write(vis_val, n, 1); return; }      /* the direct link: on the map at once, no key presses */
    vis_send = 1;
}
static void vis_put(int i, int type, int cell, int p, int left) { vis[i].type = type; vis[i].cell = cell; vis[i].p = p; vis[i].left = left; vis[i].born = 1; vis[i].seq = ev_seq; shown_seq[i] = 0; }
static void vis_add(int type, int cell, int p, int left)
{
    /* v26b: with the direct link the map draws fires, ash and floods from the per-cell state, so they take no picture slot
       (three forced fires alone filled all 16 slots and the landslides and earthquakes behind them were never drawn) */
    if (dl_state == 1 && (type == 1 || type == 2 || type == 3 || type == 5)) return;   /* v26k38c: landslides too (state 7) */
    for (int i = 0; i < 16; i++) if (!vis[i].type) { vis_put(i, type, cell, p, left); return; }
    if (type == 3 || type == 4)                          /* full: a landslide or an earthquake takes a fire / ash / flood slot */
        for (int i = 0; i < 16; i++) if (vis[i].type == 5 || vis[i].type == 1 || vis[i].type == 2) { vis_put(i, type, cell, p, left); return; }
}
static void vis_clear(void) { memset(vis, 0, sizeof(vis)); }
static void vis_turn(void)                     /* once per turn: drop the finished ones, burning cells turn into burnt ones; then send what changed */
{
    for (int i = 0; i < 16; i++) { Vis *v = &vis[i]; if (!v->type) continue;
        if (v->born) { v->born = 0; continue; }
        if (v->left > 0 && --v->left == 0) { if (v->type == 1) { v->type = 5; v->p = g_turn < 0 ? 0 : g_turn; v->left = 5; v->seq = 0; } else v->type = 0; } }
    vis_snapshot();
}
static volatile u64 quiet_after;           /* v25.9: no key bursts while the game is busy with a new turn (frames stall, key presses get lost) */
static int game_in_front(void) { DWORD pid = 0; void *w = GetForegroundWindow(); if (!w) return 0; GetWindowThreadProcessId(w, &pid); return pid == GetCurrentProcessId(); }
static void key_tap(u16 vk)
{
    INPUT in; memset(&in, 0, sizeof(in)); in.type = 1; in.wVk = vk;
    SendInput(1, &in, sizeof(in)); Sleep(60);   /* held across several frames */
    in.dwFlags = 2; SendInput(1, &in, sizeof(in)); Sleep(35);
}
static void key_combo(u16 mod, u16 vk)   /* v26k28c: a key with Ctrl or Shift held */
{
    INPUT in[2]; memset(in, 0, sizeof(in)); in[0].type = 1; in[0].wVk = mod; in[1].type = 1; in[1].wVk = vk;
    SendInput(2, in, sizeof(INPUT)); Sleep(80);
    in[0].dwFlags = 2; in[1].dwFlags = 2; { INPUT u[2]; u[0] = in[1]; u[1] = in[0]; SendInput(2, u, sizeof(INPUT)); } Sleep(40);
}
static void key_digits(u32 v) { char b[16]; char *e = pint(b, (i64)v); for (char *q = b; q < e; q++) key_tap((u16)(0x7C + (*q - '0'))); }   /* F13..F22 */
static void wl_watch(void);   /* v26k37: engine_v26.inc */
#ifndef HOSTTEST
static volatile int pf_log_due; static void pf_log(void);   /* v26k38c */
#endif
static DWORD WINAPI sender_thread(void *arg)
{
    (void)arg;
    HANDLE mx = CreateMutexW(0, 0, L"SAN14CE_SeasonKeys");   /* shared with SeasonSync so month keys and data keys never interleave */
    u64 resend_at = 0; int rs_round = 0;
    for (;;) {
        Sleep(50);
        dl_tick(); rain_calibrate(); wl_watch(); perf_ab(); diag_tick();
        { static u64 te_next; u64 now = GetTickCount64(); if (te_n && now >= te_next) { te_next = now + 1000; te_apply(); } }   /* v26k42 */
#ifndef HOSTTEST
        if (pf_log_due) { pf_log_due = 0; pf_log(); }   /* v26k38c: the route-search line is written here, not inside a route search */
#endif
        if (dl_state == 1) { vis_send = 0; resend_at = 0; continue; }   /* the direct link carries the slots: no key presses */
        if (resend_at && GetTickCount64() >= resend_at) { resend_at = 0; vis_send = 1; }
        if (!vis_send) continue;
        if (!game_in_front()) continue;           /* the keys go to the window in front; wait for the game */
        if (GetTickCount64() < quiet_after) continue;   /* v25.9: wait until the turn change is over */
        vis_send = 0;
        u32 vals[16], seqs[16]; int evn = vis_evn; memcpy(vals, vis_val, sizeof(vals)); memcpy(seqs, vis_vseq, sizeof(seqs));
        int order[16], n = 0;
        for (int pass = 0; pass < 3; pass++)      /* new disasters first, then other changes and one confirming resend, cleared slots last */
            for (int i = 0; i < 16; i++) {
                int changed = vals[i] != sent_val[i], again = resend[i] > 0;
                if (!changed && !again) continue;
                int isnew = changed && vals[i] && seqs[i] && shown_seq[i] != seqs[i];
                if ((pass == 0 && isnew) || (pass == 1 && !isnew && vals[i]) || (pass == 2 && !vals[i])) order[n++] = i; }
        if (!n && evn == sent_evn) continue;
        if (mx) WaitForSingleObject(mx, 10000);
        key_tap(0xFC);                            /* NONAME: begin */
        for (int j = 0; j < n; j++) {
            int i = order[j]; u32 v = vals[i], ds = (v + (u32)i * 7919u) % 9973u;   /* v25.9c: check = (value + slot * 7919) mod 9973, the map checks it before storing */
            key_digits((u32)i); key_tap(0xF6);    /* ATTN: slot */
            key_digits(v); key_tap(0x7C + 10);    /* F23: value */
            key_digits(ds); key_tap(0x7C + 11);        /* F24: check, stored and shown at once only if it matches */
            if (resend[i] > 0 && vals[i] == sent_val[i]) resend[i]--; else { resend[i] = 3; rs_round = 0; }   /* v25.9: three confirming resends (cleared slots too) */
            sent_val[i] = v; shown_seq[i] = seqs[i];
            if (j == 0 && evn > sent_evn) { key_digits((u32)evn * 1000u + ((u32)evn * 37u + 11u) % 97u); key_tap(0xF7); sent_evn = evn; }   /* CRSEL: slots in use (sent as n*1000 + (n*37+11) mod 97: a lost digit breaks the pattern) */
        }
        key_digits((u32)evn * 1000u + ((u32)evn * 37u + 11u) % 97u); key_tap(0xF7); sent_evn = evn;   /* v25.9: slots in use, every time (a lost key no longer hides everything) */
        key_tap(0xF9);                            /* EREOF: end */
        /* v25.9: if a key was lost the map may have read data digits as a month; a moment later (the map has left data mode by
           itself by then) put the right month and ten-day period back */
        Sleep(1800);
        { int t = g_turn; if (t >= 0 && t < 36 && game_in_front()) { key_tap((u16)(0x7C + t / 3)); key_tap((u16)(0xF6 + t % 3)); } }
        if (mx) ReleaseMutex(mx);
        for (int i = 0; i < 16; i++) if (resend[i] > 0) { resend_at = GetTickCount64() + (2500u << (rs_round < 3 ? rs_round : 3)); rs_round++; break; }   /* resends after 2.5 s, 5 s, 10 s */
        char b[80], *p = b; p = pstr(p, "map: sent "); p = pint(p, n); p = pstr(p, " slot(s), in use "); p = pint(p, evn); *p = 0; logs(b);
    }
    return 0;
}
static int rolled_year[4]; static int harvest_force;   /* v26k6: the test build shows the harvest verdict at once */
static u8 hit_city_season[512];
static int force_quake_cell = -1;            /* v26k38c: the test build puts its small quake among hills */
static int force_quake_tier = -1;                /* test build: the strength of the forced quake */
static u16 food_season[512];  /* season mark per base: one of our disasters per city per season */
/* v25.9i: the game's own (lunar) seasons, as in KOEI's manual: spring 1-3, summer 4-6, autumn 7-9, winter 10-12 */
static int season_of(int t) { int m = t / 3 + 1; return (m <= 3) ? 1 : (m <= 6) ? 2 : (m <= 9) ? 3 : 0; }
static int g_syear;   /* the year the current season belongs to (the last day of 12월 already counts as next year's spring) */
static int banned_turn(int t) { return t == 0 || t == 9 || t == 18 || t == 27; }   /* first turn of 1/4/7/10: the game's own events */
static int pick_turn(int lo, int hi, int now)
{
    int cand[36], n = 0;
    for (int t = lo; t <= hi && t < 36; t++) if (t > now && !banned_turn(t)) cand[n++] = t;
    return n ? cand[rnd() % n] : -1;
}
static const u16 *PROVN[17] = { L"유주", L"요동", L"기주", L"병주", L"청주", L"서주", L"연주", L"예주", L"사례", L"옹주", L"양주(서북)", L"익주", L"남중", L"형주", L"양주(강동)", L"강남", L"교주" };
static const int QUAKEP[17] = { 325, 250, 300, 300, 250, 275, 200, 200, 225, 300, 350, 350, 350, 150, 175, 175, 250 };   /* per 10000 */
static const int FIREP[4] = { 5000, 3000, 2500, 500 };
static const int SLIDEP[5] = { 4000, 3000, 3000, 2000, 2000 };
static const u16 *SLIDEN[5] = { L"촉도", L"진령", L"황토고원", L"삼협", L"운귀" };
static const int FLOODP[3] = { 3000, 3000, 1500 };              /* Yellow lower, Yangtze middle, Han */
static const u16 *RIVN[3] = { L"황하가", L"장강이", L"한수가" };
static int burnt_region[6];                                     /* landslide region burnt this spring -> double chance */

static void add_event(int type, int region, int start)
{
    if (nev >= 48 || start < 0) return;
    Event *e = &ev[nev++]; memset(e, 0, sizeof(*e)); e->type = type; e->region = region; e->start = start;
    char b[100], *p = b; p = pstr(p, "scheduled type "); p = pint(p, type); p = pstr(p, " region "); p = pint(p, region); p = pstr(p, " turn "); p = pint(p, start); *p = 0; logs(b);
}
static void roll_season(int s, int t)
{
    if (!v2) {           /* the old fixed-season rolls stay for an old data file only */
    if (s == 1) {        /* spring fire: 1월 .. 3월 (v25.9i: the game's spring) */
        for (int r = 0; r < 4; r++) if (chance(FIREP[r])) add_event(EV_FIRE, r + 1, pick_turn(0, 8, t));
    } else if (s == 2) { /* floods: 4월 .. 5월; landslides any summer turn 4월 .. 6월 */
        for (int r = 0; r < 3; r++) if (chance(FLOODP[r])) add_event(EV_FLOOD, r + 1, pick_turn(9, 14, t));
        for (int r = 0; r < 5; r++) { int p = SLIDEP[r]; if (burnt_region[r]) p *= 2; if (chance(p)) add_event(EV_SLIDE, r + 1, pick_turn(9, 17, t)); }
    } }
    (void)s; (void)t;    /* v26k2: earthquakes are rolled once a year for any turn of the year (roll_quakes), no season of their own */
}
static int quake_year = -0x7fffffff;
static void roll_quakes(int t)   /* v26k2 (asked for): the yearly chance per region as before (QUAKEP), the turn any one left in the year */
{
    if (quake_year == g_syear) return; quake_year = g_syear;
    for (int r = 0; r < 17; r++) if (chance(QUAKEP[r])) add_event(EV_QUAKE, r, pick_turn(0, 35, t));
}
static u8 near_gate[NCELL];      /* 1 = within 3 cells of a gate or a city (landslides keep off these roads) */
static int near_gate_ready;
static void build_near_gate(void)
{
    if (near_gate_ready) return; near_gate_ready = 1;
    for (int c = 0; c < NCELL; c++) { if (vterr[c] != 16 && vterr[c] != 17) continue;
        int cx = c % 220, cy = c / 220;
        for (int dy = -3; dy <= 3; dy++) for (int dx = -3; dx <= 3; dx++) { int x = cx + dx, y = cy + dy; if (x < 0 || x >= 220 || y < 0 || y >= 220) continue; near_gate[y * 220 + x] = 1; } }
}
static int random_cell(u8 *zone, int value, int prov_filter)
{
    int n = 0, pick = -1;
    if (zone == slidez) build_near_gate();
    for (int c = 0; c < NCELL; c++) {
        if (zone && zone[c] != value) continue;
        if (zone == slidez && near_gate[c]) continue;
        if (!zone && prov[c] != prov_filter) continue;
        if (!zone && (vterr[c] == 0 || vterr[c] >= 19 || vterr[c] == 14 || vterr[c] == 24)) continue;
        n++; if (rnd() % (u32)n == 0) pick = c;
    }
    return pick;
}
static int nb6(int c, int k);
static void slide_at(u8 *d, Event *e, int c, int season)
{
    u8 *h = cellp(d, c); if (!h) return;
    if (occ[c]) { u8 *a = army(d, occ[c]); if (a && chance(5000)) hurt(a, 20, 20, "landslide"); return; }   /* the army is not blocked */
    if (e->ncells < 8 && h[0x10] != T_BLOCKED) { e->cells[e->ncells] = (u16)c; e->saved[e->ncells] = h[0x10]; e->ncells++; h[0x10] = T_BLOCKED; mine[c] = 2; vis_add(3, c, g_turn < 0 ? 0 : g_turn, 0); }
    (void)season;
}
static void start_event0(u8 *d, Event *e, int t)
{
    int s = season_of(t); build_near_gate(); ev_seq++;
    if (e->type == EV_FIRE) {
        int c = random_cell(firez, (u8)e->region, 0); if (c < 0) { e->active = 0; return; }
        e->cell = c; e->cells[0] = (u16)c; e->ncells = 1; int want = rint(2, 4);
        for (int k = 0; k < 400 && e->ncells < want; k++) { int n = c + rint(-2, 2) + 220 * rint(-2, 2); if (n < 0 || n >= NCELL) continue;
            if (firez[n] != e->region) continue; int dup = 0; for (int j = 0; j < e->ncells; j++) if (e->cells[j] == n) dup = 1; if (!dup) e->cells[e->ncells++] = (u16)n; }
        for (int j = 0; j < e->ncells; j++) { int bc = e->cells[j]; burning[bc] = 1; if (occ[bc]) { u8 *a = army(d, occ[bc]); if (a) hurt(a, 10, 10, "fire"); } }
        e->turns_left = rint(2, 3); e->active = 1;
        for (int j = 0; j < e->ncells; j++) vis_add(1, e->cells[j], t, e->turns_left);
        u8 *f = nearest_city(d, c, 30 * 30);
        rset(L"[재해] "); if (f) { raddname(f); radd(L" 부근 "); } radd(L"숲에 산불이 났습니다. ("); raddn(e->ncells); radd(L"칸)");
        report(c);
        if (e->region == 1) burnt_region[4] = 1; else if (e->region == 3) { burnt_region[1] = 1; burnt_region[2] = 1; }
    } else if (e->type == EV_FLOOD) {
        e->big = chance(e->region == 1 ? 4000 : 2500);
        int n = 0, swept = 0, cell0 = -1;
        for (int c = 0; c < NCELL; c++) {
            u8 z = floodz[c]; if ((z >> 4) != e->region) continue; z &= 15; if (!z || (z == 3 && !e->big)) continue;
            flooded[c] = (u8)(e->big ? 2 : 1); n++; if (cell0 < 0 || (z == 1 && rnd() % 64 == 0)) cell0 = c;
            if (occ[c]) { u8 *a = army(d, occ[c]); if (!a) continue;
                int p = (z == 1) ? (e->big ? 3000 : 1000) : (e->big ? 6000 : 3000);
                if (chance(p)) { swept++; int heavy = chance(e->big ? 6000 : 2500);
                    if (heavy) hurt(a, 50, 40, "flood heavy"); else hurt(a, 30, 25, "flood light"); } }
        }
        e->cell = cell0; e->turns_left = e->big ? 3 : 2; e->active = n > 0;
        if (e->active) {                                           /* the harvest: cities beside the flood lose part of their food store */
            int hit = 0;
            for (int i = 1; i <= 500 && hit < 3; i++) { u8 *f = foot(d, i); if (!f) continue; int fc = *(u16 *)(f + 0x3a);
                u8 *h = cellp(d, fc); if (!h || h[0x10] != 16) continue;
                int near = 0; for (int dy = -3; dy <= 3 && !near; dy++) for (int dx = -3; dx <= 3; dx++) { int x = fc % 220 + dx, y = fc / 220 + dy;
                    if (x < 0 || x >= 220 || y < 0 || y >= 220) continue; int q = y * 220 + x; if (flooded[q] && (floodz[q] >> 4) == e->region) { near = 1; break; } }
                if (!near) continue;
                if (food_season[i] == (u16)(2 + 4 * (g_year & 0x3fff))) continue;      /* v25.4: one harvest loss per city per summer */
                food_season[i] = (u16)(2 + 4 * (g_year & 0x3fff));
                int took = city_take_food(d, f, e->big ? 20 : 10); if (!took) continue; hit++;
                rset(L"[재해] 홍수로 "); raddname(f); radd(L"의 병량이 "); raddn(took); radd(L" 줄었습니다."); report_ex(fc, 0);
                { char b[120], *q = b; q = pstr(q, "flood: city "); q = putf8(q, (u16 *)(f + 0x10), 4); q = pstr(q, " food -"); q = pint(q, took); *q = 0; logs(b); }
            }
        }
        if (e->active) vis_add(2, e->region, e->big ? 1 : 0, e->turns_left);
        rset(L"[재해] "); radd(RIVN[e->region - 1]); radd(e->big ? L" 크게 범람했습니다." : L" 범람했습니다.");
        if (swept) { radd(L" 휩쓸린 부대 "); raddn(swept); radd(L"곳"); }
        report(cell0);
    } else if (e->type == EV_SLIDE) {
        int c = random_cell(slidez, (u8)e->region, 0); if (c < 0) { e->active = 0; return; }
        e->cell = c; slide_at(d, e, c, s);
        if (chance(5000)) { for (int k = 0; k < 200; k++) { int n = c + rint(-2, 2) + 220 * rint(-2, 2); if (n >= 0 && n < NCELL && n != c && slidez[n] == e->region && !near_gate[n]) { slide_at(d, e, n, s); break; } } }
        e->turns_left = 3; e->active = 1;
        for (int i = 0; i < 16; i++) if (vis[i].type == 3 && vis[i].left == 0) vis[i].left = e->turns_left;
        u8 *f = nearest_city(d, c, 30 * 30);
        rset(L"[재해] "); radd(SLIDEN[e->region - 1]); radd(L" ");
        if (f) { raddname(f); radd(L" 부근 "); } radd(L"산길이 산사태로 막혔습니다.");
        report(c);
    } else if (e->type == EV_QUAKE) {
        int c = force_quake_cell >= 0 ? force_quake_cell : random_cell(0, 0, e->region); force_quake_cell = -1; if (c < 0) { e->active = 0; return; }
        e->cell = c; e->active = 0;
        /* v25.9: three strengths inside the yearly quake chance (like a flood and a big flood): 60% quake, 30% strong, 10% great */
        int tier = force_quake_tier >= 0 ? force_quake_tier : (chance(6000) ? 0 : (chance(7500) ? 1 : 2));
        force_quake_tier = -1; e->big = tier;
        /* v26k38c (asked for): every army inside is hit (troops and morale -30% / -40% / -50%), order and endurance -30% / -50% / -70%
           of what they are, 3 / 5 / 10 landslides; the hurt is at the moment of the quake only (the shaken ground after it only slows) */
        static const int QAL[3] = { 6, 10, 14 }, QAC[3] = { 3, 5, 7 }, QORD[3] = { 30, 50, 70 }, QEND[3] = { 30, 50, 70 },
                         QSLIDE[3] = { 3, 5, 10 }, QTURNS[3] = { 2, 3, 4 }, QHURT[3] = { 30, 40, 50 };
        static const float COS12[12] = { 1, .966f, .866f, .707f, .5f, .259f, 0, -.259f, -.5f, -.707f, -.866f, -.966f };
        static const float SIN12[12] = { 0, .259f, .5f, .707f, .866f, .966f, 1, .966f, .866f, .707f, .5f, .259f };
        int k12 = rint(0, 11); float ca = COS12[k12], sa = SIN12[k12];      /* fault direction, 15 degree steps */
        float A2 = (float)(QAL[tier] * QAL[tier]), C2 = (float)(QAC[tier] * QAC[tier]);
        vis_add(4, c, k12 + 12 * tier, QTURNS[tier]);
        /* v26k3: the shaken ground slows every army for as long as the cracks show (quake 80%, strong 70%, great 60%) */
        { int nq = 0; for (int dy = -QAL[tier]; dy <= QAL[tier]; dy++) for (int dx = -QAL[tier]; dx <= QAL[tier]; dx++) {
            int x = ccol(c) + dx, y = crow(c) + dy; if (x < 0 || x >= 220 || y < 0 || y >= 220) continue;
            float al = dx * ca + dy * sa, ac = -dx * sa + dy * ca; if ((al * al) / A2 + (ac * ac) / C2 > 1.0f) continue;
            int n = y * 220 + x; if (vterr[n] == 0 || vterr[n] >= 19 || vterr[n] == 14) continue;
            quaked[n] = (u8)((tier + 1) | (QTURNS[tier] << 2)); nq++; }
          for (int k = 0; k < 8; k++) if (!qtrack[k].left) { qtrack[k].left = QTURNS[tier]; qtrack[k].cell = c; qtrack[k].region = e->region; qtrack[k].tier = tier; qtrack[k].day0 = e->day0; break; }   /* v26k13 */
          char b[80], *q = b; q = pstr(q, "quake: shaken cells "); q = pint(q, nq); *q = 0; logs(b); }
        /* armies inside the shaken area: thrown down by falling rock and opening ground - every one of them */
        int hurt_n = 0;
        for (int dy = -QAL[tier]; dy <= QAL[tier]; dy++) for (int dx = -QAL[tier]; dx <= QAL[tier]; dx++) {
            int x = ccol(c) + dx, y = crow(c) + dy; if (x < 0 || x >= 220 || y < 0 || y >= 220) continue;
            int n = y * 220 + x; if (!occ[n]) continue;
            float al = dx * ca + dy * sa, ac = -dx * sa + dy * ca; if ((al * al) / A2 + (ac * ac) / C2 > 1.0f) continue;
            u8 *a = army(d, occ[n]); if (!a) continue;
            hurt_n++; hurt_pct(a, QHURT[tier], QHURT[tier], tier == 2 ? "great quake" : tier == 1 ? "strong quake" : "quake"); }
        rset(L"[재해] "); radd(PROVN[e->region]); radd(tier == 0 ? L"에서 지진이 일어났습니다." : (tier == 1 ? L"에서 강진이 일어났습니다." : L"에서 대지진이 일어났습니다!"));
        if (hurt_n) { radd(L" 피해 입은 부대 "); raddn(hurt_n); radd(L"곳"); }
        report(c);
        /* cities, gates and ports inside: fixed losses by strength, the three nearest the epicentre, once per city per autumn */
        int ci[64], nci = 0; float ce[64];
        for (int i = 1; i <= 500; i++) {
            u8 *f = foot(d, i); if (!f) continue; int fc = *(u16 *)(f + 0x3a);
            float dx = (float)(ccol(fc) - ccol(c)), dy = (float)(crow(fc) - crow(c));
            float al = dx * ca + dy * sa, ac = -dx * sa + dy * ca;
            float e2 = (al * al) / A2 + (ac * ac) / C2; if (e2 > 1.0f) continue;
            if (hit_city_season[i] == 3 + 4 * (g_year & 63)) continue;
            u8 *h = cellp(d, fc); int ttype = h ? h[0x10] : 0; if (ttype != 16 && ttype != 17 && ttype != 18) continue;
            if (nci < 64) { ci[nci] = i; ce[nci] = e2; nci++; } }
        for (int pick = 0; pick < 3 && nci > 0; pick++) {
            int bi = 0; for (int k = 1; k < nci; k++) if (ce[k] < ce[bi]) bi = k;
            int i = ci[bi]; ci[bi] = ci[nci - 1]; ce[bi] = ce[nci - 1]; nci--;
            u8 *f = foot(d, i); if (!f) continue; int fc = *(u16 *)(f + 0x3a);
            hit_city_season[i] = (u8)(3 + 4 * (g_year & 63));
            int o0 = f[0x54], o1 = o0;                       /* public order: u8 0..100 (same field the game's order proposals use) */
            u8 *h = cellp(d, fc); int ttype = h ? h[0x10] : 0;
            if (ttype == 16 && o0 <= 100) { o1 = o0 - o0 * QORD[tier] / 100; if (o1 < 0) o1 = 0; f[0x54] = (u8)o1; }   /* public order exists for cities only */
            int e0 = -1, e1 = -1;                             /* endurance of the city / gate / port building on the capital cell */
            u8 *bld = building_at(d, fc);
            if (bld) { e0 = *(u16 *)(bld + 0x14); e1 = e0 - e0 * QEND[tier] / 100; if (e1 < 1) e1 = e0 < 1 ? e0 : 1; if (e0 > 0 && e0 <= 20000) *(u16 *)(bld + 0x14) = (u16)e1; else e1 = e0; }
            { char b[160], *p = b; p = pstr(p, "quake: tier "); p = pint(p, tier); p = pstr(p, " area "); p = putf8(p, (u16 *)(f + 0x10), 4); p = pstr(p, " terrain "); p = pint(p, ttype); p = pstr(p, " order "); p = pint(p, o0); p = pstr(p, "->"); p = pint(p, o1);
              if (e0 >= 0) { p = pstr(p, " endurance "); p = pint(p, e0); p = pstr(p, "->"); p = pint(p, e1); } *p = 0; logs(b); }
            if (o0 != o1 || e0 != e1) {
                rset(L"[재해] 지진으로 "); raddname(f); radd(L"의 ");
                if (o0 != o1) { radd(L"치안이 "); raddn(o0 - o1); if (e0 != e1) radd(L", "); }
                if (e0 != e1) { radd(L"내구가 "); raddn(e0 - e1); } radd(L" 떨어졌습니다.");
                report_ex(fc, 0); }
        }
        { char b[80], *p = b; p = pstr(p, "quake: tier "); p = pint(p, tier); p = pstr(p, " armies hurt "); p = pint(p, hurt_n); *p = 0; logs(b); }
        /* landslides on the mountain roads of the shaken area (v26k38c: picked among all of them, nearest the fault first, two cells apart,
           never on an army - the quake has hurt it already - nor on a cell already blocked) */
        int made = 0; static u16 cand[1024]; static float cw[1024]; int ncand = 0;
        for (int dy = -QAL[tier]; dy <= QAL[tier]; dy++) for (int dx = -QAL[tier]; dx <= QAL[tier]; dx++) {
            int x = ccol(c) + dx, y = crow(c) + dy; if (x < 0 || x >= 220 || y < 0 || y >= 220) continue; int n = y * 220 + x;
            if (near_gate[n] || occ[n] || mine[n] || vterr[n] == T_BLOCKED || flooded[n]) continue;
            float pri = 0.0f; if (!slidez[n]) { u8 v = vterr[n]; if (!(v == 5 || v == 6 || (v >= 8 && v <= 11))) continue;   /* off the mountain roads: forest / hill ground, at a mountain's foot first */
                int foot = 0; for (int k = 0; k < 6; k++) { int q = nb6(n, k); if (q >= 0 && vterr[q] == T_BLOCKED) foot = 1; } pri = foot ? 1000.0f : 2000.0f; }
            float al = dx * ca + dy * sa, ac = -dx * sa + dy * ca; if ((al * al) / A2 + (ac * ac) / C2 > 1.0f) continue;
            if (ncand < 1024) { cand[ncand] = (u16)n; cw[ncand] = ac * ac + (float)(rnd() % 1000) * 0.004f + pri; ncand++; } }   /* the mountain roads first */
        for (int k = 0; k < 3000 && made < QSLIDE[tier] && ncand > 0; k++) {
            int bi = 0; for (int j = 1; j < ncand; j++) if (cw[j] < cw[bi]) bi = j;
            int n = cand[bi]; cand[bi] = cand[ncand - 1]; cw[bi] = cw[ncand - 1]; ncand--;
            { int near = 0; for (int j = 0; j < nev && !near; j++) if (ev[j].type == EV_SLIDE && ev[j].active) for (int q = 0; q < ev[j].ncells; q++) if (cdist2(ev[j].cells[q], n) < 4) near = 1; if (near) continue; }
            Event *s2 = (nev < 48) ? &ev[nev++] : 0; if (!s2) break; memset(s2, 0, sizeof(*s2)); s2->day0 = today_day(); s2->type = EV_SLIDE; s2->region = slidez[n]; s2->start = t; s2->cell = n;
            slide_at(d, s2, n, s); s2->turns_left = 3; s2->active = 1; made++;
            for (int i = 0; i < 16; i++) if (vis[i].type == 3 && vis[i].left == 0) vis[i].left = s2->turns_left; }
        if (made) { int c1 = ev[nev - 1].cell; u8 *f = nearest_city(d, c1, 30 * 30); rset(L"[재해] "); radd(tier == 2 ? L"대지진" : tier == 1 ? L"강진" : L"지진"); radd(L"으로 ");
            if (f) { raddname(f); radd(L" 부근 "); } radd(L"산사태 "); raddn(made); radd(L"곳 - 길이 막혔습니다."); report_ex(c1, 0); }
        { char b[100], *p = b; p = pstr(p, "quake: landslides "); p = pint(p, made); p = pstr(p, " of "); p = pint(p, QSLIDE[tier]); p = pstr(p, " (mountain-road cells in the area "); p = pint(p, ncand + made); p = pstr(p, ")"); *p = 0; logs(b); }
    }
    char b[120], *p = b; p = pstr(p, "start type "); p = pint(p, e->type); p = pstr(p, " region "); p = pint(p, e->region); p = pstr(p, " cell "); p = pint(p, e->cell);
    p = pstr(p, " cells "); p = pint(p, e->ncells); p = pstr(p, e->big ? " big" : ""); *p = 0; logs(b);
}
static void start_event(u8 *d, Event *e, int t)
{
    TB(t_ev); int ty = e->type, rg = e->region; (void)ty; (void)rg;
    if (ty == EV_QUAKE && v2 && YS.quakes[0] + YS.quakes[1] + YS.quakes[2] >= 6) { if (!YS.brake[BR_QUAKE]) { YS.brake[BR_QUAKE] = 1; logs("ledger brake: quakes"); } e->active = 0; TE(t_ev, "quake braked", 0); return; }   /* v26k11 */
    if (ty == EV_QUAKE) e->cell = -1;
    e->day0 = today_day();   /* v26k13 */
    start_event0(d, e, t);
    if (ty == EV_QUAKE && e->cell >= 0 && e->big >= 0 && e->big <= 2) YS.quakes[e->big]++;   /* a quake is over at once (never 'active') */
#ifdef SEASON_TEST
    { char lb[60], *p = lb; p = pstr(p, ty == EV_FIRE ? "start fire" : ty == EV_FLOOD ? "start flood" : ty == EV_SLIDE ? "start landslide" : "start quake");
      p = pstr(p, " region "); p = pint(p, rg); *p = 0; tlog(lb, t_ev, 1); }
#endif
}
static int end_quiet;                          /* v26k13: a reload clears everything without news */
static void end_event(u8 *d, Event *e)
{
    if (e->type == EV_SLIDE && !end_quiet && e->cell >= 0 && e->cell < NCELL) {   /* v26k13 (asked for): the road opens again - said, with the day */
        u8 *f = nearest_city(d, e->cell, 30 * 30); rset(L"[재해] "); if (e->region >= 1 && e->region <= 5) { radd(SLIDEN[e->region - 1]); radd(L" "); }
        if (f) { raddname(f); radd(L" 부근 "); } radd(L"산길이 다시 열렸습니다."); radd_when(e->day0); report_ex(e->cell, 0); }
    if (e->type == EV_FIRE) for (int j = 0; j < e->ncells; j++) burning[e->cells[j]] = 0;
    if (e->type == EV_FLOOD) for (int c = 0; c < NCELL; c++) if ((floodz[c] >> 4) == e->region) flooded[c] = 0;
    if (e->type == EV_SLIDE) for (int j = 0; j < e->ncells; j++) { int c = e->cells[j]; u8 *h = cellp(d, c);
        if (h && h[0x10] == T_BLOCKED && mine[c] == 2) { h[0x10] = e->saved[j]; mine[c] = 0; } }
    e->active = 0; e->type = 0;
    logs("event ended");
}
#include "engine_v26.inc"
#ifdef SEASON_TEST
static int land_ok(int c) { u8 v = vterr[c]; return v == 3 || v == 5 || v == 6; }   /* forest, hills, mountains: never bare ground */
static int test_fire_by(u8 *d, int t, int ai)
{
    u8 *a = army(d, ai); if (!a || nev >= 48) return 0; int ac = *(u16 *)(a + 0x2a);
    Event *e = &ev[nev]; memset(e, 0, sizeof(*e));
    for (int r = 1; r <= 4 && e->ncells < 3; r++)
        for (int dy = -r; dy <= r && e->ncells < 3; dy++) for (int dx = -r; dx <= r && e->ncells < 3; dx++) {
            if ((dx < 0 ? -dx : dx) != r && (dy < 0 ? -dy : dy) != r) continue;
            int x = ac % 220 + dx, y = ac / 220 + dy; if (x < 0 || x >= 220 || y < 0 || y >= 220) continue;
            int n = y * 220 + x; if (!land_ok(n) || occ[n] || burning[n]) continue;
            u8 *h = cellp(d, n); if (!h || h[0x10] != vterr[n]) continue;
            e->cells[e->ncells++] = (u16)n; }
    if (e->ncells < 2) return 0;
    nev++; e->type = EV_FIRE; e->region = 1; e->start = t; e->cell = e->cells[0]; e->turns_left = 3; e->active = 1;
    ev_seq++; for (int j = 0; j < e->ncells; j++) { burning[e->cells[j]] = 1; test_lit[e->cells[j]] = 1; vis_add(1, e->cells[j], t, 3); }
    u8 *f = nearest_city(d, ac, 30 * 30);
    rset(L"[재해] (시험) "); if (f) { raddname(f); radd(L" 부근 "); } radd(L"부대 바로 옆 숲에 산불 ("); raddn(e->ncells); radd(L"칸)");
    report(e->cells[0]);
    char b[120], *p = b; p = pstr(p, "test fire next to army "); p = pint(p, ai); p = pstr(p, " at cell "); p = pint(p, ac); p = pstr(p, " cells "); p = pint(p, e->ncells); *p = 0; logs(b);
    return 1;
}
static int test_rain_days;
/* v26k30b: forced ground states of the test: lying snow (thin / deep), river ice, mud (fresh / drying), a burnt slope on a landslide road.
   Each goes where nothing else of the test is (no fire, flood, quake, landslide, rain, crack nearby), so every tile shows one thing.
   They are held for the whole session (the turn's climate would put the real season back; letting the snow go would also "melt" it). */
static u16 tcov_c[64]; static u8 tcov_l[64]; static int ntcov; static u16 tice_c[160]; static int ntice; static u16 tmud_c[64]; static u8 tmud_l[64]; static int ntmud;
static u8 tmark[NCELL]; static u16 tash_c[8]; static int ntash;
static void test_states(void)
{
    for (int i = 0; i < ntcov; i++) { int c = tcov_c[i]; cSnow[c] = tcov_l[i]; cCR[c] = 0.0f; }   /* the climate's own rule: no fire risk under snow */
    for (int i = 0; i < ntice; i++) { int c = tice_c[i]; cFrz[c >> 3] |= (u8)(1 << (c & 7)); cFrzN[c >> 3] |= (u8)(1 << (c & 7)); }
    for (int i = 0; i < ntmud; i++) if (!flooded[tmud_c[i]]) mud[tmud_c[i]] = tmud_l[i];
    for (int i = 0; i < ntash; i++) { int c = tash_c[i]; if (!burning[c] && !flooded[c]) { ash[c] = 1; ashLeft[c] = 8; } }
}
static int test_busy(int c0, int rx, int ry)
{
    int n = 0, x0 = c0 % 220, y0 = c0 / 220;
    for (int dy = -ry; dy <= ry; dy++) for (int dx = -rx; dx <= rx; dx++) { int x = x0 + dx, y = y0 + dy; if (x < 0 || x >= 220 || y < 0 || y >= 220) { n += 4; continue; } int c = y * 220 + x;
        if (flooded[c] || burning[c] || quaked[c] || mine[c] == 2 || rain[c] || crack[c] || ash[c] || mud[c] || tmark[c] || vterr[c] == T_BLOCKED) n++; }
    return n;
}
static u16 tfl[4096]; static int ntfl;          /* the flooded cells when the test starts */
static int test_flood_d2(int c)               /* squared distance to the nearest flooded cell (the floods spread along their river) */
{
    int b = 1 << 30; for (int i = 0; i < ntfl; i++) { int dd = cdist2(c, tfl[i]); if (dd < b) b = dd; } return b;
}
static int test_pick_city(u8 *d, u32 pmask, int rx, int ry)
{
    int best = -1, bs = 1 << 30, n = 0;
    for (int i = 1; i <= 500; i++) { u8 *f = foot(d, i); if (!f) continue; int fc = *(u16 *)(f + 0x3a); if (fc >= NCELL || prov[fc] > 16 || !((pmask >> prov[fc]) & 1)) continue;
        u8 *h = cellp(d, fc); if (!h || h[0x10] != 16) continue; int x = fc % 220, y = fc / 220; if (x < rx + 2 || x > 217 - rx || y < ry + 2 || y > 217 - ry) continue;
        int sc = test_busy(fc, rx, ry) * 8 + (test_flood_d2(fc) < 18 * 18 ? 1000 : 0);
        if (sc < bs) { bs = sc; best = fc; n = 1; } else if (sc == bs && rnd() % (u32)(++n) == 0) best = fc; }
    return best;
}
static void test_line(u8 *d, int c, const u16 *what) { u8 *f = nearest_city(d, c, 40 * 40); rset(L"[시험] "); if (f) { raddname(f); radd(L" 부근: "); } radd(what); report_ex(c, 0); }
static void test_ground(u8 *d, int t)
{
    ntcov = 0; ntice = 0; ntmud = 0; ntash = 0; memset(tmark, 0, sizeof(tmark));
    ntfl = 0; for (int c = 0; c < NCELL && ntfl < 4096; c++) if (flooded[c]) tfl[ntfl++] = (u16)c;
    /* river ice: v26k38c - up to 120 freezable big-river cells within 12 of the quietest one (a long stretch, bank to bank), far from the floods */
    int ci = -1; { int bs = 1 << 30, n = 0;
      for (int k = 0; k < nf; k++) { int c = fcells[k]; if (occ[c]) continue; int sc = test_busy(c, 6, 4) * 8 + (test_flood_d2(c) < 20 * 20 ? 1000 : 0);
          if (sc < bs) { bs = sc; ci = c; n = 1; } else if (sc == bs && rnd() % (u32)(++n) == 0) ci = c; } }
    if (ci >= 0) { int bd[160]; int nb = 0;
        for (int k = 0; k < nf; k++) { int c = fcells[k]; if (occ[c]) continue; int dd = cdist2(c, ci); if (dd > 12 * 12) continue;
            if (nb < 120) { tice_c[nb] = (u16)c; bd[nb] = dd; nb++; continue; } int w = 0; for (int j = 1; j < nb; j++) if (bd[j] > bd[w]) w = j; if (dd < bd[w]) { tice_c[w] = (u16)c; bd[w] = dd; } }
        ntice = nb; for (int j = 0; j < nb; j++) tmark[tice_c[j]] = 1; }
    /* lying snow: a city of 병주 / 기주 / 청주 / 유주 - thin snow 6 cells west of it, deep snow 6 cells east */
    int cs = test_pick_city(d, (1u << 3) | (1u << 2) | (1u << 4) | (1u << 0), 9, 3), sw = -1, se = -1;
    if (cs >= 0) { int cx = cs % 220, cy = cs / 220;
        for (int side = 0; side < 2; side++) { int ox = cx + (side ? 6 : -6);
            for (int dy = -2; dy <= 2; dy++) for (int dx = -2; dx <= 2; dx++) { int x = ox + dx, y = cy + dy, c = y * 220 + x;
                if (!cl_land(c) || vterr[c] >= 13 || ntcov >= 64 || test_busy(c, 0, 0)) continue; tcov_c[ntcov] = (u16)c; tcov_l[ntcov] = (u8)(side ? 2 : 1); ntcov++;
                if (dx == 0 && dy == 0) { if (side) se = c; else sw = c; } } }
        for (int i = 0; i < ntcov; i++) tmark[tcov_c[i]] = 1; }
    /* mud: a city of 예주 / 서주 / 청주 / 연주 - fresh mud (the water just left) 6 cells west, drying mud 6 cells east */
    int cm = test_pick_city(d, (1u << 7) | (1u << 5) | (1u << 4) | (1u << 6), 9, 3), mw = -1, me = -1;
    if (cm >= 0) { int cx = cm % 220, cy = cm / 220;
        for (int side = 0; side < 2; side++) { int ox = cx + (side ? 6 : -6);
            for (int dy = -2; dy <= 2; dy++) for (int dx = -2; dx <= 2; dx++) { int x = ox + dx, y = cy + dy, c = y * 220 + x;
                if (!cl_land(c) || vterr[c] >= 13 || ntmud >= 64 || test_busy(c, 0, 0)) continue; tmud_c[ntmud] = (u16)c; tmud_l[ntmud] = (u8)(side ? 1 : 2); ntmud++;
                if (dx == 0 && dy == 0) { if (side) me = c; else mw = c; } } }
        for (int i = 0; i < ntmud; i++) tmark[tmud_c[i]] = 1; }
    /* a burnt slope on a landslide road (산사태 위험 x3): the quietest such cell, with its burnt neighbours on the road */
    int cb = -1; build_near_gate();
    { int bs = 1 << 30, n = 0;
      for (int c = 0; c < NCELL; c++) { if (!slidez[c] || near_gate[c] || !fuelT[c] || occ[c]) continue; int sc = test_busy(c, 3, 3);
          if (sc < bs) { bs = sc; cb = c; n = 1; } else if (sc == bs && rnd() % (u32)(++n) == 0) cb = c; } }
    int nash = 0;
    if (cb >= 0) { tash_c[ntash++] = (u16)cb; for (int k = 0; k < 6; k++) { int q = nb6(cb, k); if (q >= 0 && slidez[q] && !near_gate[q] && fuelT[q] && !occ[q] && !test_busy(q, 0, 0)) tash_c[ntash++] = (u16)q; } nash = ntash; }
    test_states(); rivers(d, t);
    if (sw >= 0) test_line(d, sw, L"얕은 눈 - 정보창 '얕은 눈   이속 -15%'");
    if (se >= 0) test_line(d, se, L"깊은 눈 - 정보창 '깊은 눈   이속 -30%'");
    if (ntice) test_line(d, ci, L"언 강 (긴 구간) - 큰 강이 얼어 걸어서 건널 수 있음, 정보창 '언 강   이속 -50%'. 40일 뒤 녹기 시작 (얼음 위 부대는 내릴 때까지 얼음 유지)");
    if (mw >= 0) test_line(d, mw, L"진흙 (막 물 빠진 땅) - 정보창 '진흙   이속 -10%'");
    if (me >= 0) test_line(d, me, L"진흙 (마르는 중) - 그림이 옅고, 정보창 '진흙   이속 -10%'");
    if (cb >= 0) test_line(d, cb, L"불탄 산길 - 정보창 '잿더미' 그리고 아랫줄 '산사태 위험 ×3'");
    char b[200], *p = b; p = pstr(p, "TEST ground: snow city "); p = pint(p, cs); p = pstr(p, " cells "); p = pint(p, ntcov); p = pstr(p, ", river ice at "); p = pint(p, ci); p = pstr(p, " cells "); p = pint(p, ntice);
    p = pstr(p, ", mud city "); p = pint(p, cm); p = pstr(p, " cells "); p = pint(p, ntmud); p = pstr(p, ", burnt slope "); p = pint(p, cb); p = pstr(p, " cells "); p = pint(p, nash); *p = 0; logs(b);
}
static void test_start(u8 *d, int t)
{
    test_done = 1; logs("TEST: forcing every disaster in every size now");
    TB(t_all);
    if (!v2) { for (int r = 1; r <= 3; r++) { add_event(EV_FLOOD, r, t); Event *e = &ev[nev - 1]; start_event(d, e, t); if (!e->active) e->type = 0; } }
    else {
        /* floods: 황하 홍수 (1), 장강 큰 홍수 (2), 한수 대홍수 (3) - each stays at its level for a few turns */
        /* v26k16: the Yellow River at 복양 an ice jam (10 days, then it drains and rises again: "다시 범람"), the Yangtze at 강릉 a big flood,
           the Han at 양양 a great flood (40 days each, then they drain) */
        force_flood(d, 1, 1, 14882, t, 2, 10); test_reflood = 1; force_flood(d, 2, 2, 33544, t, 9, 40); force_flood(d, 3, 3, 29578, t, 9, 40);
        /* v26k30b: no picture-only snow / ice preview any more - the map draws the rules' own cells; the lying snow and the river ice are forced below */
        for (int i = 0; i < 16; i++) if (vis[i].type == 2) vis[i].left = 5;
        /* fires: 산불 (3 cells) in the north-east woods, 큰 산불 (18) in the Qinling, 대산불 (45) in the south-west */
        int anchors[3] = { 0, 0, 0 }, sizes[3] = { 3, 18, 45 }; int cnt[3] = { 0, 0, 0 };
        for (int c = 0; c < NCELL; c++) { int k = (prov[c] <= 2) ? 0 : (prov[c] == 8 || prov[c] == 9) ? 1 : (prov[c] == 12 || prov[c] == 13) ? 2 : -1; if (k < 0 || fuelT[c] < 120 || occ[c]) continue;
            int nn = 0; for (int q = 0; q < 6; q++) { int b = nb6(c, q); if (b >= 0 && fuelT[b] >= 100) nn++; } if (nn < 4) continue; cnt[k]++; if (rnd() % (u32)cnt[k] == 0) anchors[k] = c; }
        for (int k = 0; k < 3; k++) if (cnt[k]) force_fire_size(d, anchors[k], sizes[k], t);
        /* landslides in three regions (촉도, 진령, 황토고원) */
        for (int r = 1; r <= 3; r++) { add_event(EV_SLIDE, r, t); Event *e = &ev[nev - 1]; start_event(d, e, t); if (!e->active) e->type = 0; }
        /* rain: a downpour over the Yellow River flood, rain over the Yangtze, drizzle over the Han; drought cracks around Chang'an / Liangzhou */
        int rv_cell[4] = { -1, -1, -1, -1 };
        for (int rv = 1; rv <= 3; rv++) { int n = 0; for (int c = 0; c < NCELL; c++) if ((floodz[c] >> 4) == rv && (floodz[c] & 15) == 1) { n++; if (rnd() % (u32)n == 0) rv_cell[rv] = c; } }
        force_rain_on = 1; force_rain_t[0] = 3; force_rain_t[1] = 2; force_rain_t[2] = 1; force_rain_r[0] = 9; force_rain_r[1] = 9; force_rain_r[2] = 8;
        for (int rv = 1; rv <= 3; rv++) { int c = rv_cell[rv] >= 0 ? rv_cell[rv] : 15000 + rv * 3000; force_rain_cx[rv - 1] = c % 220; force_rain_cy[rv - 1] = c / 220; }
        /* the fourth: heavy rain on the small fire, so the "rain puts the fire out" rule can be seen (after the first day) */
        if (anchors[0]) { force_rain_cx[3] = anchors[0] % 220; force_rain_cy[3] = anchors[0] / 220; force_rain_r[3] = 5; force_rain_t[3] = 3; }   /* on the small fire: it goes out in the rain, the big ones keep burning */
        /* v26k7: two rows to compare side by side, west to east, 12 cells apart:
             snow  - sleet / snow / snowstorm, the snowstorm on the coldest city of 유주 / 요동 (not on the map's top edge)
             rain  - drizzle / rain / downpour, the downpour on a city of 연주 / 예주 / 사례 (the weather notices show too) */
        { int best = -1; float bt = 99.0f;
          for (int i = 1; i <= 500; i++) { u8 *f = foot(d, i); if (!f) continue; int fc = *(u16 *)(f + 0x3a); if (fc >= NCELL || prov[fc] > 1) continue; if (fc / 220 < 8 || fc % 220 < 30) continue; u8 *h = cellp(d, fc); if (!h || h[0x10] != 16) continue; if (cT[fc] < bt) { bt = cT[fc]; best = fc; } }
          if (best >= 0) { int bx = best % 220, by = best / 220;
            force_rain_cx[4] = bx - 24; force_rain_cy[4] = by; force_rain_r[4] = 5; force_rain_t[4] = 1;
            force_rain_cx[5] = bx - 12; force_rain_cy[5] = by; force_rain_r[5] = 5; force_rain_t[5] = 2;
            force_rain_cx[6] = bx;      force_rain_cy[6] = by; force_rain_r[6] = 5; force_rain_t[6] = 3;
            char b[100], *q = b; q = pstr(q, "TEST snow spots around cell "); q = pint(q, best); q = pstr(q, " temp x10 "); q = pint(q, (int)(bt * 10)); *q = 0; logs(b); }
          int cityc = -1; for (int i = 1; i <= 500 && cityc < 0; i++) { u8 *f = foot(d, i); if (!f) continue; int fc = *(u16 *)(f + 0x3a); if (fc >= NCELL || (prov[fc] != 6 && prov[fc] != 7 && prov[fc] != 8)) continue; if (fc % 220 < 30) continue; u8 *h = cellp(d, fc); if (h && h[0x10] == 16) cityc = fc; }
          if (cityc >= 0) { int cx = cityc % 220, cy = cityc / 220;
            force_rain_cx[7] = cx - 24; force_rain_cy[7] = cy; force_rain_r[7] = 5; force_rain_t[7] = 1;
            force_rain_cx[8] = cx - 12; force_rain_cy[8] = cy; force_rain_r[8] = 5; force_rain_t[8] = 2;
            force_rain_cx[9] = cx;      force_rain_cy[9] = cy; force_rain_r[9] = 5; force_rain_t[9] = 3; } }
        for (int k = 0; k < 10; k++) if (force_rain_r[k]) { force_snow_k = (k >= 4 && k <= 6) ? 1 : 0; force_rain_at(force_rain_cx[k], force_rain_cy[k], force_rain_r[k], force_rain_t[k]); } force_snow_k = 0;
        int ncr = 0; for (int c = 0; c < NCELL; c++) { if (!cl_land(c) || (prov[c] != 10 && prov[c] != 9)) continue; if (vterr[c] == 16 || vterr[c] == 17 || vterr[c] == 18) continue; int x = c % 220, y = c / 220; if (x < 20 || x > 75 || y < 55 || y > 85) continue; crack[c] = 1; dryd[c] = 60; ncr++; }
        test_rain_days = 30;                   /* v26k16: 30 days (enough to visit every spot), then they stop - so the 'stopped' lines show too */
        /* v26k6: one clickable line per weather spot in the record window, so each strength can be visited */
        { static const u16 *WN[10] = { L"폭우 (황하 홍수 자리)", L"비 (장강 홍수 자리)", L"가랑비 (한수 홍수 자리)", L"폭우 (작은 산불 위 - 불이 칸마다 꺼짐)",
                                       L"진눈깨비", L"눈", L"눈보라 (도시 위)", L"가랑비", L"비", L"폭우 (도시 위)" };   /* v26k30b: the names the tile info uses */
          for (int k = 0; k < 10; k++) { if (!force_rain_r[k]) continue; int c = force_rain_cy[k] * 220 + force_rain_cx[k]; if (c < 0 || c >= NCELL) continue; u8 *f = nearest_city(d, c, 40 * 40);
            rset(L"[시험] "); if (f) { raddname(f); radd(L" 부근: "); } radd(WN[k]); report_ex(c, 0); } }
        /* v26k12: two more points to check the flood picture: a cell that wrongly carried the Yellow River's mud before (now its own
           river's clear water), and the Yellow River's bank (the water starts at the bank) */
        if (test_relabel_cell >= 0) { int c = test_relabel_cell; u8 *f = nearest_city(d, c, 40 * 40); rset(L"[시험] "); if (f) { raddname(f); radd(L" 부근: "); }
            radd((floodz[c] >> 4) == 3 ? L"한수 홍수 - 흙탕물(황하 색)이 아니면 정상" : L"장강 홍수 - 흙탕물(황하 색)이 아니면 정상"); report_ex(c, 0); }
        { int pick = -1, n = 0; for (int c = 0; c < NCELL; c++) { if ((floodz[c] >> 4) != 1 || (floodz[c] & 15) != 1 || c % 220 < 120) continue; int w = 0; for (int k = 0; k < 6; k++) { int q = nb6(c, k); if (q >= 0 && (vterr[q] == 13 || vterr[q] == 14)) w = 1; } if (!w) continue; n++; if (rnd() % (u32)n == 0) pick = c; }
          if (pick >= 0) { u8 *f = nearest_city(d, pick, 40 * 40); rset(L"[시험] "); if (f) { raddname(f); radd(L" 부근: "); } radd(L"황하 강가 - 강에 붙은 땅부터 잠기고, 물과 강 사이에 빈틈이 없으면 정상"); report_ex(pick, 0); } }
        harvest_force = 1;                     /* v26k6: the year's harvest verdict shown on the test day (in play: 7월 중순) */
        char b[160], *p = b; p = pstr(p, "TEST: fires "); p = pint(p, cnt[0] ? 1 : 0); p = pint(p, cnt[1] ? 1 : 0); p = pint(p, cnt[2] ? 1 : 0); p = pstr(p, " (anchors "); p = pint(p, anchors[0]); *p++ = ' '; p = pint(p, anchors[1]); *p++ = ' '; p = pint(p, anchors[2]);
        p = pstr(p, "), cracked cells "); p = pint(p, ncr); p = pstr(p, ", rain centres "); for (int k = 0; k < 10; k++) { if (!force_rain_r[k]) continue; p = pint(p, force_rain_cx[k]); *p++ = ','; p = pint(p, force_rain_cy[k]); *p++ = ' '; } *p = 0; logs(b);
    }
    force_quake_tier = 2;                          /* quakes: 대지진 in 익주, 강진 in 형주, 지진 in 병주 (v26k38c: hills - the plain of 기주 has no slope to slide) */
    add_event(EV_QUAKE, 11, t); { Event *e = &ev[nev - 1]; start_event(d, e, t); if (!e->active) e->type = 0; }
    force_quake_tier = 1; add_event(EV_QUAKE, 13, t); { Event *e = &ev[nev - 1]; start_event(d, e, t); if (!e->active) e->type = 0; }
    { int best = -1, bn = 0;                     /* the cell of 병주 with the most mountain feet within 3 */
      for (int c = 0; c < NCELL; c += 3) { if (prov[c] != 3 || !cl_land(c)) continue; int n = 0, x0 = c % 220, y0 = c / 220;
          for (int dy = -3; dy <= 3; dy++) for (int dx = -3; dx <= 3; dx++) { int x = x0 + dx, y = y0 + dy; if (x < 0 || x >= 220 || y < 0 || y >= 220) continue; int q = y * 220 + x; u8 v = vterr[q];
              if (!(v == 5 || v == 6 || (v >= 8 && v <= 11))) continue; for (int k = 0; k < 6; k++) { int r = nb6(q, k); if (r >= 0 && vterr[r] == T_BLOCKED) { n++; break; } } }
          if (n > bn) { bn = n; best = c; } }
      force_quake_cell = best; }
    force_quake_tier = 0; add_event(EV_QUAKE, 3, t); { Event *e = &ev[nev - 1]; start_event(d, e, t); if (!e->active) e->type = 0; }
    if (v2) test_ground(d, t);                     /* v26k30b: lying snow, river ice, mud, a burnt slope - after the quakes, so they keep clear of them */
    vis_snapshot(); weather_write();
    TE(t_all, "forced disasters (all, with the news lines and the map)", 1);
    logs("TEST: fires placed 3");
}
static void test_day(u8 *d, i64 us)
{
    u32 c0 = tq_calls, b0 = tq_burn_empty, b1 = tq_burn_army, i0 = tq_ice_empty, i1 = tq_thaw_empty;
    tq_calls = tq_burn_empty = tq_burn_army = tq_ice_empty = tq_thaw_empty = 0;
    int entered = 0;
    for (int i = 1; i <= 500; i++) { u8 *a = army(d, i); if (!a) continue; int c = *(u16 *)(a + 0x2a); if (c < NCELL && test_lit[c] && burning[c]) entered++; }
    tt_calls += c0; tt_burn_empty += b0; tt_burn_army += b1; tt_ice_empty += i0; tt_thaw_empty += i1; tt_entered += (u32)entered; tt_days++;
    tt_us_sum += us; if (us > tt_us_max) tt_us_max = us;
    char b[260], *p = b; p = pstr(p, "test day: speed calls "); p = pint(p, c0); p = pstr(p, ", fire-cell checks (no army there) "); p = pint(p, b0);
    p = pstr(p, ", fire-cell checks (army there) "); p = pint(p, b1); p = pstr(p, ", ice checks "); p = pint(p, i0); p = pstr(p, ", thin-ice checks "); p = pint(p, i1);
    p = pstr(p, ", armies on test fire "); p = pint(p, entered); p = pstr(p, ", rules time us "); p = pint(p, us); *p = 0; logs(b);
    p = b; p = pstr(p, "test total: days "); p = pint(p, tt_days); p = pstr(p, " fire_checks_empty "); p = pint(p, tt_burn_empty); p = pstr(p, " fire_checks_army "); p = pint(p, tt_burn_army);
    p = pstr(p, " ice_checks "); p = pint(p, tt_ice_empty); p = pstr(p, " thin_ice_checks "); p = pint(p, tt_thaw_empty); p = pstr(p, " army_days_on_fire "); p = pint(p, tt_entered);
    p = pstr(p, " time_max_us "); p = pint(p, tt_us_max); p = pstr(p, " time_avg_us "); p = pint(p, tt_days ? tt_us_sum / tt_days : 0); *p = 0; logs(b);
}
/* ---- link test: game data for the next version, written to SeasonRules_dump.txt by a side thread (read-only; every read goes
   through ReadProcessMemory, so a page that moves away is skipped instead of crashing):
   areas (names, capital cells), the city objects (population is in one of them), buildings (the type field for 토성 / 석벽),
   and a search of memory for the building table (13 days / 500 / 500 next to 21 days / 800 / 800). ---- */
static volatile int dump_started, dump_done;
static HANDLE dmp; static char *dbuf; static u32 dlen;
static int rpm(const void *a, void *buf, u64 n) { SIZE_T got = 0; if (!a || (u64)a < 0x10000) return 0; return ReadProcessMemory(GetCurrentProcess(), a, buf, n, &got) && got == n; }
static void dflush(void) { if (dlen && dmp) { DWORD w; WriteFile(dmp, dbuf, dlen, &w, 0); } dlen = 0; }
static void dput(const char *s, int n) { if (dlen + (u32)n > 60000) dflush(); if (n > 60000) n = 60000; memcpy(dbuf + dlen, s, n); dlen += (u32)n; }
static void dline(const char *s) { int n = 0; while (s[n]) n++; dput(s, n); dput("\r\n", 2); }
static char *hb(char *p, u32 v) { static const char H[] = "0123456789abcdef"; *p++ = H[(v >> 4) & 15]; *p++ = H[v & 15]; return p; }
static void dhex(const u8 *b, int n)
{
    for (int o = 0; o < n; o += 32) { char l[160], *p = l; p = pstr(p, "  +"); p = hb(p, (u32)o >> 8); p = hb(p, (u32)o);
        *p++ = ':'; for (int k = o; k < o + 32 && k < n; k++) { *p++ = ' '; p = hb(p, b[k]); } *p = 0; dline(l); }
}
static char *pname(char *p, const u16 *s, int max) { for (int i = 0; i < max && s[i]; i++) { if (s[i] < 0x20) break; p = putf8(p, s + i, 1); } return p; }
static int cell_terrain(u8 *d, int c) { u8 *h = 0; u8 cb[32]; if (c < 0 || c >= NCELL || !rpm(d + DB_HEX + 8 * (u64)c, &h, 8) || !rpm(h, cb, 32)) return -1; return cb[0x10]; }
static int rpm_upto(const u8 *a, u8 *buf, int want) { for (int n = want; n >= 0x40; n >>= 1) if (rpm(a, buf, (u64)n)) return n; return 0; }
static int sig_n, str_n[2];
static int vat(const u8 *b, int i, int w) { return w == 2 ? (int)*(const u16 *)(b + i) : *(const i32 *)(b + i); }
static void sig_at(const u8 *b, int len, int i, int w, const u8 *va)
{
    for (int o1 = -64; o1 <= 64; o1 += w) { if (!o1) continue; int p1 = i + o1; if (p1 < 0 || p1 + w > len || vat(b, p1, w) != 800) continue;
        for (int o2 = o1 + w; o2 <= 64; o2 += w) { if (!o2) continue; int p2 = i + o2; if (p2 < 0 || p2 + w > len || vat(b, p2, w) != 800) continue;
            int mn = o1 < 0 ? o1 : 0, mx = o2 > 0 ? o2 : 0;
            for (int dl = w; dl <= 0x400; dl += w) for (int sg = -1; sg <= 1; sg += 2) { int j = i + sg * dl;
                if (j + mn < 0 || j + mx + w > len) continue;
                if (vat(b, j, w) != 13 || vat(b, j + o1, w) != 500 || vat(b, j + o2, w) != 500) continue;
                if (sig_n++ >= 24) return;
                char l[220], *p = l; p = pstr(p, "TABLE? 21 at "); p = phex(p, (u64)(va + i)); p = pstr(p, " width "); p = pint(p, w); p = pstr(p, ", 800 at "); p = pint(p, o1);
                p = pstr(p, " and "); p = pint(p, o2); p = pstr(p, ", 13/500/500 record "); p = pint(p, sg * dl); p = pstr(p, " bytes away"); *p = 0; dline(l);
                int a0 = (i < j ? i : j) + mn - 6 * dl, a1 = (i > j ? i : j) + mx + w + 6 * dl; if (a0 < 0) a0 = 0; if (a1 > len) a1 = len; if (a1 - a0 > 0x800) a1 = a0 + 0x800;
                p = l; p = pstr(p, "  window from "); p = phex(p, (u64)(va + a0)); *p = 0; dline(l); dhex(b + a0, a1 - a0);
                return; } } }
}
static void str_at(const u8 *b, int len, int i, int which, const u8 *va, u32 type)
{
    if (str_n[which]++ >= 30) return;
    char l[400], *p = l; p = pstr(p, which ? "STRING 석벽 at " : "STRING 토성 at "); p = phex(p, (u64)(va + i)); p = pstr(p, type == 0x1000000 ? " (image)" : type == 0x40000 ? " (mapped)" : " (private)"); p = pstr(p, " text: ");
    int a0 = i - 24; if (a0 < 0) a0 = 0; a0 &= ~1; int a1 = i + 48; if (a1 > len) a1 = len;
    for (int k = a0; k + 1 < a1; k += 2) { u16 c = *(const u16 *)(b + k); if (c < 0x20 || (c >= 0x7f && c < 0xa0)) *p++ = '.'; else p = putf8(p, &c, 1); }
    *p = 0; dline(l); dhex(b + a0, a1 - a0);
}
static void scan_mem(void)
{
    const u64 CH = 0x100000, OV = 0x600;
    u8 *buf = (u8 *)VirtualAlloc(0, CH + 2 * OV + 64, 0x3000, 4); if (!buf) { logs("scan: no memory"); return; }
    i64 t0 = qnow(); u64 total = 0; int nreg = 0; MBI m; u8 *a = (u8 *)0x10000;
    dline(""); dline("== memory search: UTF-16 text 토성 / 석벽, and a building table (13 days / 500 / 500 next to 21 days / 800 / 800) ==");
    while ((u64)a < 0x7ffffffe0000ULL) {
        if (VirtualQuery(a, &m, sizeof(m)) != sizeof(m)) break;
        u8 *base = (u8 *)m.Base; u64 size = m.Size; if (!size) break;
        int ok = m.State == 0x1000 && (m.Protect & 0xEE) && !(m.Protect & 0x701) && m.AllocBase != (void *)buf && m.AllocBase != (void *)dbuf && size <= 0x20000000ULL;   /* no guard / uncached / write-combined (graphics) pages */
        if (ok) { nreg++;
            for (u64 off = 0; off < size; off += CH) {
                u64 st = off > OV ? off - OV : 0, en = off + CH + OV; if (en > size) en = size;
                if (!rpm(base + st, buf, en - st)) continue;
                int len = (int)(en - st), from = (int)(off - st), to = (int)((off + CH > size ? size : off + CH) - st);
                for (int i = from & ~1; i + 1 < to; i += 2) { u16 v = *(u16 *)(buf + i);
                    if (v == 0xD1A0 && i + 3 < len && *(u16 *)(buf + i + 2) == 0xC131) str_at(buf, len, i, 0, base + st, m.Type);
                    else if (v == 0xC11D && i + 3 < len && *(u16 *)(buf + i + 2) == 0xBCBD) str_at(buf, len, i, 1, base + st, m.Type);
                    else if (v == 21 && sig_n < 24) { sig_at(buf, len, i, 2, base + st); if (!(i & 3) && i + 3 < len && *(u16 *)(buf + i + 2) == 0) sig_at(buf, len, i, 4, base + st); } }
                total += (u64)(to - from);
            }
        }
        a = base + size;
        if (qus(t0) > 150000000) { dline("(search stopped after 150 s)"); break; }
    }
    VirtualFree(buf, 0, 0x8000);
    char b[200], *p = b; p = pstr(p, "scan: done, "); p = pint(p, (i64)(total >> 20)); p = pstr(p, " MB in "); p = pint(p, nreg); p = pstr(p, " areas, "); p = pint(p, qus(t0) / 1000);
    p = pstr(p, " ms; text 토성 "); p = pint(p, str_n[0]); p = pstr(p, ", 석벽 "); p = pint(p, str_n[1]); p = pstr(p, "; table candidates "); p = pint(p, sig_n); *p = 0; logs(b); dline(b);
}
static DWORD WINAPI dump_thread(void *arg)
{
    (void)arg; Sleep(3000); if (!q_freq) QueryPerformanceFrequency(&q_freq);
    i64 t0 = qnow();
    u16 fp[600]; fp[0] = 0; wcat(fp, datpath); int n = wlen(fp); while (n && fp[n - 1] != '\\') n--; fp[n] = 0; wcat(fp, L"SeasonRules_dump.txt");
    dmp = CreateFileW(fp, 0x40000000, 1, 0, 2, 0x80, 0); if (dmp == (HANDLE)(i64)-1) { dmp = 0; logs("dump: cannot write the file"); dump_done = 1; return 0; }
    dbuf = (char *)VirtualAlloc(0, 0x10000, 0x3000, 4); if (!dbuf) { CloseHandle(dmp); dmp = 0; dump_done = 1; return 0; }
    u8 *d = 0; if (!rpm(dbp(), &d, 8) || !d) { dline("no game database"); dflush(); CloseHandle(dmp); dump_done = 1; return 0; }
    char l[400], *p; u8 ob[0x800];
    { u8 *date = 0; u8 dt[0x40]; if (rpm(d + DB_DATE, &date, 8) && rpm(date + 0x30, dt, 0x10)) { p = l; p = pstr(p, "game date "); p = pint(p, *(u16 *)(dt + 4)); *p++ = '/'; p = pint(p, dt[6]); *p++ = '/'; p = pint(p, dt[7]); *p = 0; dline(l); } }
    p = l; p = pstr(p, "game image "); p = phex(p, (u64)game); p = pstr(p, " database "); p = phex(p, (u64)d); *p = 0; dline(l);
    /* areas */
    dline(""); dline("== areas (DB_FOOT 1..500): name, capital cell (col,row), terrain there (16 city 17 gate 18 port 15 small base), foothold index (+0x35), order (+0x54), class; then the object's bytes ==");
    int na = 0, fid_of[501]; for (int i = 0; i <= 500; i++) fid_of[i] = -1;
    for (int i = 1; i <= 500; i++) { u8 *f = 0; if (!rpm(d + DB_FOOT + 8 * (u64)i, &f, 8) || !f) continue;
        int got = rpm_upto(f, ob, 0x200); if (got < 0x98) continue; na++;
        int c = *(u16 *)(ob + 0x3a); fid_of[i] = ob[0x35];
        p = l; p = pstr(p, "area "); p = pint(p, i); p = pstr(p, " name "); p = pname(p, (u16 *)(ob + 0x10), 8); p = pstr(p, " cell "); p = pint(p, c);
        p = pstr(p, " ("); p = pint(p, c % 220); *p++ = ','; p = pint(p, c / 220); p = pstr(p, ") terrain "); p = pint(p, cell_terrain(d, c));
        p = pstr(p, " foothold "); p = pint(p, ob[0x35]); p = pstr(p, " order "); p = pint(p, ob[0x54]); p = pstr(p, " class "); p = phex(p, *(u64 *)ob - (u64)game); *p = 0; dline(l);
        dhex(ob, got); }
    /* footholds / cities */
    dline(""); dline("== footholds (DB_FOOTHOLD 0..51): class (city / other), food (+0x38), the areas using it; then the object's bytes ==");
    int nf2 = 0;
    for (int j = 0; j <= 51; j++) { u8 *f = 0; if (!rpm(d + DB_FOOTHOLD + 8 * (u64)j, &f, 8) || !f) continue;
        int got = rpm_upto(f, ob, 0x800); if (!got) continue; nf2++;
        u64 vt = *(u64 *)ob;
        p = l; p = pstr(p, "foothold "); p = pint(p, j); p = pstr(p, (u8 *)vt == game + VT_CITYDATA ? " city" : (u8 *)vt == game + VT_FOOTHOLD ? " foothold" : " other"); p = pstr(p, " class "); p = phex(p, vt - (u64)game);
        p = pstr(p, " food "); p = pint(p, *(i32 *)(ob + 0x38)); p = pstr(p, " bytes "); p = pint(p, got); p = pstr(p, " areas:");
        for (int i = 1; i <= 500 && p - l < 300; i++) if (fid_of[i] == j) { u8 *fa = 0; u8 nb[0x20]; if (rpm(d + DB_FOOT + 8 * (u64)i, &fa, 8) && rpm(fa + 0x10, nb, 0x10)) { *p++ = ' '; p = pname(p, (u16 *)nb, 8); } }
        *p = 0; dline(l); dhex(ob, got); }
    /* buildings */
    dline(""); dline("== buildings (DB_BUILD 1..3000): cell (col,row), terrain there, endurance (+0x14), class; then the first bytes ==");
    int nb2 = 0;
    for (int i = 1; i <= 3000; i++) { u8 *b = 0; if (!rpm(d + DB_BUILD + 8 * (u64)i, &b, 8) || !b) continue;
        int got = rpm_upto(b, ob, 0x100); if (got < 0x40) continue; nb2++;
        int c = *(u16 *)(ob + 0x10);
        p = l; p = pstr(p, "building "); p = pint(p, i); p = pstr(p, " cell "); p = pint(p, c); p = pstr(p, " ("); p = pint(p, c % 220); *p++ = ','; p = pint(p, c / 220);
        p = pstr(p, ") terrain "); p = pint(p, cell_terrain(d, c)); p = pstr(p, " endurance "); p = pint(p, *(u16 *)(ob + 0x14)); p = pstr(p, " class "); p = phex(p, *(u64 *)ob - (u64)game); *p = 0; dline(l);
        dhex(ob, got > 0x80 ? 0x80 : got); }
    /* a map of the database: runs of pointers to objects of one class (the other tables of the game, e.g. building types) */
    dline(""); dline("== database map: offset, how many pointers in a row, the class of the objects (image offset of the first table entry) ==");
    { u8 *root = (u8 *)VirtualAlloc(0, 0x90000, 0x3000, 4);
      int lim = 0; if (root) { if (rpm(d, root, 0x90000)) lim = 0x90000; else if (rpm(d, root, 0x88000)) lim = 0x88000; }
      if (lim) {
        u64 run_cls = 0; int run_off = -1, run_n = 0, nruns = 0;
        for (int off = 0; off <= lim; off += 8) {
            u64 cls = 0;
            if (off < lim) { u8 *q = *(u8 **)(root + off); u64 vt = 0;
                if (q && rpm(q, &vt, 8) && vt > (u64)game && vt < (u64)game + gv_image) cls = vt - (u64)game; }
            if (cls && cls == run_cls) { run_n++; continue; }
            if (run_cls && run_n >= 3 && nruns++ < 400) { p = l; p = pstr(p, "db +"); p = phex(p, (u64)run_off); p = pstr(p, " x"); p = pint(p, run_n); p = pstr(p, " class "); p = phex(p, run_cls); *p = 0; dline(l); }
            run_cls = cls; run_off = off; run_n = cls ? 1 : 0; }
      }
      if (root) VirtualFree(root, 0, 0x8000); }
    p = l; p = pstr(p, "dump: areas "); p = pint(p, na); p = pstr(p, ", footholds "); p = pint(p, nf2); p = pstr(p, ", buildings "); p = pint(p, nb2); p = pstr(p, ", ms "); p = pint(p, qus(t0) / 1000); *p = 0; logs(l); dline(l);
    dflush();
    scan_mem();
    dflush(); CloseHandle(dmp); dmp = 0;
    dump_done = 1; logs("dump: finished (SeasonRules_dump.txt)");
    return 0;
}
#endif
/* v26k38c (asked for): the weather's morale, day by day, as simple as can be - no state kept per army. On the cell the army stands on today:
   rain / snow -1 on even days, a downpour / snowstorm -1 every day, drizzle / sleet nothing. The weather never takes morale below 50
   (an army already under 50 loses nothing to it). A month in a downpour: -30, in rain: -15. */
static int wm_n1, wm_n3;
static void weather_morale(u8 *d, u32 day)
{
    int even = !(day & 1), n1 = 0, n3 = 0;
    for (int i = 1; i <= 500; i++) { u8 *a = army(d, i); if (!a) continue; int c = *(u16 *)(a + 0x2a); if (c >= NCELL) continue; u8 rt = rain[c]; if (rt < 2 || (rt == 2 && !even)) continue;
        u8 m0 = a[0x1a]; if (m0 <= 50 || m0 > 150) continue; a[0x1a] = (u8)(m0 - 1); if (rt >= 3) n3++; else n1++; }
    wm_n1 += n1; wm_n3 += n3; YS.moraleArmies += n1 + n3;
}
static void rain_morale(u8 *d)   /* v26k5: a turn spent in the rain costs morale - rain 1, a downpour 3 (never below 10); no troop loss */
{
    int n1 = 0, n3 = 0;
    for (int i = 1; i <= 500; i++) { u8 *a = army(d, i); if (!a) continue; int c = *(u16 *)(a + 0x2a); if (c >= NCELL) continue; u8 rt = rain[c]; if (rt < 2) continue;
        u8 m0 = a[0x1a]; if (m0 < 1 || m0 > 150) continue; int m1 = m0 - (rt >= 3 ? 3 : 1); if (m1 < 10) m1 = m0 < 10 ? m0 : 10; a[0x1a] = (u8)m1; if (rt >= 3) n3++; else n1++; }
    YS.moraleArmies += n1 + n3;
    if (n1 || n3) { char b[100], *p = b; p = pstr(p, "rain morale: armies in rain "); p = pint(p, n1); p = pstr(p, ", in a downpour "); p = pint(p, n3); *p = 0; logs(b); }
}
static int harvest_year_done = -0x7fffffff;
static void harvest_year(u8 *d, int t)   /* v26k6: the climate reaches the granary - at 7월 중순 every city's store follows its region's rain grade this year:
                                            big drought -12%, dry -6%, normal 0, wet +2%, very wet +4% (once a year) */
{
    if (!harvest_force && (t != 19 || harvest_year_done == g_syear)) return; harvest_year_done = g_syear; harvest_force = 0;
    static const int PCT[5] = { -12, -6, 0, 2, 4 }; int cnt[5] = { 0, 0, 0, 0, 0 }; u8 *first[5] = { 0, 0, 0, 0, 0 }; int firstc[5] = { 0, 0, 0, 0, 0 }; u16 names[5][4][8]; int nn[5] = { 0, 0, 0, 0, 0 };
    for (int i = 1; i <= 500; i++) { u8 *f = foot(d, i); if (!f) continue; int fc = *(u16 *)(f + 0x3a); if (fc >= NCELL) continue; u8 *h = cellp(d, fc); if (!h || h[0x10] != 16) continue;
        int n[4]; float w[4]; node_mix(fc, n, w); float zr = 0; for (int k = 0; k < 4; k++) zr += Y.zr[n[k]] * w[k];
        int g = grade_of(zr); int pct = PCT[g]; if (!pct) { cnt[g]++; continue; }
        int fid = f[0x35]; if (fid > 0x33) continue; u8 *fd = *(u8 **)(d + DB_FOOTHOLD + 8 * (u64)fid); if (!rok(fd, 0x60)) continue;
        int food = *(i32 *)(fd + 0x38); if (food <= 0) continue;
        int took = city_take_food(d, f, pct);          /* a negative share adds (the setter is the game's own) */
        if (!took) continue; cnt[g]++; if (!first[g]) { first[g] = f; firstc[g] = fc; } if (nn[g] < 4) { for (int q = 0; q < 8; q++) names[g][nn[g]][q] = *(u16 *)(f + 0x10 + 2 * q); nn[g]++; } }
    for (int g = 0; g < 5; g++) { if (!cnt[g] || !PCT[g]) continue;
        rset(g <= 1 ? L"[재해] " : L"[소식] "); radd(g == 0 ? L"큰 가뭄으로 흉작: " : g == 1 ? L"가뭄으로 흉작: " : g == 3 ? L"비가 알맞아 풍작: " : L"비가 넉넉해 풍작: ");
        for (int k = 0; k < nn[g]; k++) { if (k) radd(L"·"); radd(names[g][k]); } if (cnt[g] > nn[g]) { radd(L" 외 "); raddn(cnt[g] - nn[g]); radd(L"곳"); }
        radd(L" (병량 "); if (PCT[g] > 0) radd(L"+"); raddn(PCT[g]); radd(L"%)"); report_ex(firstc[g], g <= 1); }
    for (int g = 0; g < 5; g++) YS.harv[g] = cnt[g]; YS.harvDone = 1;   /* v26k11 ledger */
    char b[120], *p = b; p = pstr(p, "harvest: big drought "); p = pint(p, cnt[0]); p = pstr(p, " dry "); p = pint(p, cnt[1]); p = pstr(p, " normal "); p = pint(p, cnt[2]); p = pstr(p, " wet "); p = pint(p, cnt[3]); p = pstr(p, " very wet "); p = pint(p, cnt[4]); *p = 0; logs(b);
}
static void weather_notice_one(u8 *d, int k, int days, int snow)
{
    int c = cityCellL[k]; u8 *f = nearest_city(d, c, 2 * 2);
    /* v26k13 (asked for): no number of days - the line read like a forecast ("이번 턴 6일"). It tells what has been (the turn just
       over); if it is still going, its end is said on the day it stops */
    u32 now = today_day(); int going = wLast[snow ? 1 : 0][k] && now >= wLast[snow ? 1 : 0][k] && now - wLast[snow ? 1 : 0][k] <= 1;
    rset(L"[날씨] "); if (f) raddname(f); radd(snow ? (going ? L" 일대에 눈보라가 이어지고 있습니다." : L" 일대에 눈보라가 이어지다 그쳤습니다.") : (going ? L" 일대에 폭우가 이어지고 있습니다." : L" 일대에 폭우가 이어지다 그쳤습니다.")); report_ex(c, 0);
    if (going) { wTrack[snow ? 1 : 0] = k; wGap[snow ? 1 : 0] = 0; } (void)days;
}
static void weather_notice(u8 *d)        /* v26k6: one line a turn at most - the city under the most days (5+) of a downpour or a snowstorm */
{
    int need = 4;
#ifdef SEASON_TEST
    if (test_done) need = 1;             /* the test: both kinds shown, the forced weather may straddle a turn change */
#endif
    int bs = -1, vs = need, br = -1, vr = need;
    for (int k = 0; k < ncityL && k < 128; k++) { if (snowCity[k] > vs) { vs = snowCity[k]; bs = k; } if (heavyCity[k] > vr) { vr = heavyCity[k]; br = k; } }
    int both = 0;
#ifdef SEASON_TEST
    both = test_done;
#endif
    if (both) { if (bs >= 0) weather_notice_one(d, bs, vs, 1); if (br >= 0) weather_notice_one(d, br, vr, 0); }
    else if (bs >= 0 && (br < 0 || vs >= vr)) weather_notice_one(d, bs, vs, 1);
    else if (br >= 0) weather_notice_one(d, br, vr, 0);
    memset(heavyCity, 0, sizeof(heavyCity)); memset(snowCity, 0, sizeof(snowCity));
}
/* v26k11: the ledger file. Normal ranges: 5..95% of 40 simulated years (normal_ranges in the build notes). */
typedef struct { const char *name; int lo, hi; const char *unit; } StatRow;
static const StatRow SROW[18] = {   /* v26k15: ranges from 100 simulated years (5..95%, widened a little) - the water-flow floods and the rain by each region's climate */
    { "비 오는 날 (기후 기대값 대비)", 95, 108, "%" }, { "홍수 발생 (세 강 합)", 0, 6, "번" }, { "  끝난 홍수: 홍수에서 그침", 0, 5, "번" },
    { "  끝난 홍수: 큰 홍수까지 커짐", 0, 3, "번" }, { "  끝난 홍수: 대홍수까지 커짐", 0, 1, "번" }, { "홍수 평균 지속", 10, 30, "턴x10" },
    { "산불 발생", 0, 5, "번" }, { "  그중 큰 산불 (10칸+)", 0, 2, "번" }, { "  그중 대산불 (30칸+)", 0, 1, "번" },
    { "산불 탄 칸 합", 0, 60, "칸" }, { "산사태", 0, 3, "번" }, { "지진", 0, 2, "번" },
    { "눈 쌓인 칸 (보통 해 대비)", 85, 120, "%" }, { "황하 언 칸 (보통 해 대비)", 90, 135, "%" }, { "가뭄 갈라진 땅 (가장 넓을 때)", 100, 900, "칸" },
    { "흉작 도시 비율 (가뭄)", 0, 50, "%" }, { "풍작 도시 비율 (비 넉넉)", 5, 75, "%" }, { "비 속에서 사기 깎인 부대 (판정 없음)", -1, -1, "번" } };
static int stat_val(const YearStat *y, int k)
{
    int nf = y->fl[1] + y->fl[2] + y->fl[3];
    switch (k) {
    case 0: return y->rainExp > 1 ? (int)(y->rainAct * 100.0 / y->rainExp + 0.5) : -1;
    case 1: return y->flStart; case 2: return y->fl[1]; case 3: return y->fl[2]; case 4: return y->fl[3];
    case 5: return nf ? (y->flTurns[1] + y->flTurns[2] + y->flTurns[3]) * 10 / nf : -1;
    case 6: return y->fires; case 7: return y->fireBig; case 8: return y->fireGreat; case 9: return y->fireCells;
    case 10: return y->slides; case 11: return y->quakes[0] + y->quakes[1] + y->quakes[2];
    case 12: return y->snowN > 50 ? (int)((double)y->snowA * 100.0 / y->snowN + 0.5) : -1;
    case 13: return y->iceN > 20 ? (int)((double)y->iceA * 100.0 / y->iceN + 0.5) : -1;
    case 14: return y->crackMax;
    case 15: case 16: { int tot = y->harv[0] + y->harv[1] + y->harv[2] + y->harv[3] + y->harv[4]; if (!y->harvDone || !tot) return -1;
        return (k == 15 ? y->harv[0] + y->harv[1] : y->harv[3] + y->harv[4]) * 100 / tot; }
    default: return y->moraleArmies; }
}
static int stat_side(const YearStat *y, int k) { int v = stat_val(y, k); if (v < 0 || SROW[k].lo < 0) return 0; return v > SROW[k].hi ? 1 : v < SROW[k].lo ? -1 : 0; }
static u16 statpath[600];
static void stats_write(void)
{
    if (!statpath[0]) { int n = wlen(logpath); while (n && logpath[n - 1] != '\\') n--; for (int i = 0; i < n; i++) statpath[i] = logpath[i]; statpath[n] = 0; wcat(statpath, L"SeasonStats.txt"); }
    static char B[16384]; char *p = B; *p++ = (char)0xEF; *p++ = (char)0xBB; *p++ = (char)0xBF;
    p = pstr(p, "=== 기후 장부 (계절 맵 v26) - 세이브를 불러온 뒤부터, 해마다 ===\r\n");
    p = pstr(p, "[정상] 보통 범위 안 / [많음]·[적음] 범위 밖 (한 해만이면 운일 수 있음) / [확인 필요] 두 해 연속 같은 쪽 = 설정을 볼 자리\r\n");
    p = pstr(p, "범위는 기후를 여러 해 돌려 본 값 (해마다 20번 중 19번 들어오는 폭). [제동] = 그해 너무 많아 그 재해를 멈춤\r\n");
    for (int h = 0; h <= nYSH; h++) {
        const YearStat *y = h < nYSH ? &YSH[h] : &YS; if (y->year < 0 || !y->turns) continue;
        const YearStat *pv = h > 0 ? &YSH[h - 1] : 0; if (h == nYSH && nYSH > 0) pv = &YSH[nYSH - 1];
        p = pstr(p, "\r\n--- "); p = pint(p, y->year); p = pstr(p, "년");
        if (h == nYSH) { p = pstr(p, " (진행 중, "); p = pint(p, y->turns); p = pstr(p, "턴째 - 아직 판정 안 함)"); }
        else if (y->turns < 33) { p = pstr(p, " (일부 "); p = pint(p, y->turns); p = pstr(p, "턴만 - 참고)"); }
        p = pstr(p, " ---\r\n");
        for (int k = 0; k < 18; k++) { int v = stat_val(y, k); p = pstr(p, SROW[k].name); p = pstr(p, ": ");
            if (v < 0) { p = pstr(p, "-\r\n"); continue; }
            if (k == 5) { p = pint(p, v / 10); *p++ = '.'; p = pint(p, v % 10); p = pstr(p, "턴"); } else { p = pint(p, v); p = pstr(p, SROW[k].unit); }
            if (SROW[k].lo < 0) { p = pstr(p, "\r\n"); continue; }
            p = pstr(p, "   (보통 "); if (k == 5) { p = pint(p, SROW[k].lo / 10); p = pstr(p, "~"); p = pint(p, SROW[k].hi / 10); p = pstr(p, "턴"); } else { p = pint(p, SROW[k].lo); p = pstr(p, "~"); p = pint(p, SROW[k].hi); p = pstr(p, SROW[k].unit); } p = pstr(p, ")");
            if (h < nYSH && y->turns >= 33) { int sd = stat_side(y, k);
                if (!sd) p = pstr(p, "  [정상]");
                else { p = pstr(p, sd > 0 ? "  [많음]" : "  [적음]"); if (pv && pv->turns >= 33 && stat_side(pv, k) == sd) p = pstr(p, " [확인 필요]"); } }
            p = pstr(p, "\r\n"); if (p - B > 15000) break; }
        if (y->floodFood) { p = pstr(p, "홍수로 줄어든 병량 합: "); p = pint(p, y->floodFood); p = pstr(p, "\r\n"); }
        static const char *BRN[4] = { "산불", "홍수", "산사태", "지진" };
        for (int b2 = 0; b2 < 4; b2++) if (y->brake[b2]) { p = pstr(p, "[제동] "); p = pstr(p, BRN[b2]); p = pstr(p, "이(가) 그해 너무 많아 남은 기간 새로 일으키지 않음\r\n"); } }
#ifdef HOSTTEST
    { char a8[600]; int i = 0; for (; statpath[i] && i < 599; i++) a8[i] = statpath[i] == '\\' ? '/' : (char)statpath[i]; a8[i] = 0; remove(a8); }
#endif
    HANDLE f = CreateFileW(statpath, 0x40000000, 1, 0, 2, 0x80, 0); if (f == (HANDLE)(i64)-1) return;
    DWORD w; WriteFile(f, B, (DWORD)(p - B), &w, 0); CloseHandle(f);
}
static void stats_turn(void)
{
    if (YS.year != g_syear) {
        if (YS.year >= 0 && YS.turns > 0) {
            char b[300], *q = b; q = pstr(q, "STATYEAR y="); q = pint(q, YS.year); q = pstr(q, " turns="); q = pint(q, YS.turns);
            for (int k = 0; k < 18; k++) { *q++ = ' '; q = pint(q, stat_val(&YS, k)); } *q = 0; logs(b);
#ifdef HOSTTEST
            printf("%s slmelt %d\n", b, YS.slideMelt);
#endif
            if (nYSH == 12) { for (int i9 = 0; i9 < 11; i9++) YSH[i9] = YSH[i9 + 1]; nYSH = 11; } YSH[nYSH++] = YS; }
        memset(&YS, 0, sizeof(YS)); YS.year = g_syear; }
    YS.turns++;
}
static void per_turn(u8 *d, int t)
{
    if (v2) stats_turn();
    int s = season_of(t);
    if (v2 && (wm_n1 || wm_n3)) { char b[120], *p = b; p = pstr(p, "weather morale last turn (-1 each): army-days in rain / snow "); p = pint(p, wm_n1); p = pstr(p, ", in a downpour / snowstorm "); p = pint(p, wm_n3); *p = 0; logs(b); }
    wm_n1 = 0; wm_n3 = 0; (void)rain_morale;
    if (v2) weather_notice(d);
    for (int c = 0; c < NCELL; c++) if (mud[c]) mud[c]--;                                       /* v26k6 */
    if (v2 && (ncityL == 0 || (t % 9) == 0)) { ncityL = 0; for (int i = 1; i <= 500 && ncityL < 128; i++) { u8 *f = foot(d, i); if (!f) continue; int fc = *(u16 *)(f + 0x3a); if (fc >= NCELL) continue; u8 *h = cellp(d, fc); if (h && h[0x10] == 16) cityCellL[ncityL++] = (u16)fc; } }
    for (int c = 0; c < NCELL; c++) if (quaked[c]) { int left = (quaked[c] >> 2) - 1; quaked[c] = left > 0 ? (u8)((quaked[c] & 3) | (left << 2)) : 0; }   /* v26k3 */
    for (int k = 0; k < 8; k++) if (qtrack[k].left && --qtrack[k].left == 0) {   /* v26k13 (asked for): the shaken ground settles - said, with the day */
        rset(L"[재해] "); if (qtrack[k].region >= 0 && qtrack[k].region < 17) radd(PROVN[qtrack[k].region]); radd(qtrack[k].tier == 2 ? L"의 대지진으로 흔들린 땅이 가라앉았습니다." : qtrack[k].tier == 1 ? L"의 강진으로 흔들린 땅이 가라앉았습니다." : L"의 지진으로 흔들린 땅이 가라앉았습니다.");
        radd_when(qtrack[k].day0); report_ex(qtrack[k].cell, 0); }
#ifdef SEASON_TEST
    if (test_on) { if (s) rolled_year[s] = g_syear; }
#endif
    if (v2) { if (!Y.valid || Y.year != g_syear) year_roll(g_syear); TB(t_cl); climate_turn(t); TE(t_cl, "turn climate", 0); }
#ifdef SEASON_TEST
    if (v2 && test_done) test_states();       /* v26k30b: the forced lying snow and river ice stay (the turn's climate would put the real ones back) */
#endif
    int vanilla_today = -1;                 /* v26: the game's own news is read only when one of ours is about to start (the check cost 0.28 s on the first turn after a load) */
    if (v2) roll_quakes(t - 1);
    if (s && rolled_year[s] != g_syear) {
        rolled_year[s] = g_syear;
        if (s == 1) for (int i = 0; i < 6; i++) burnt_region[i] = 0;
        roll_season(s, t - 1);
    }
    for (int i = 0; i < nev; i++) {
        Event *e = &ev[i]; if (!e->type) continue;
        if (!e->active && e->start == t) {
            if (vanilla_today < 0) vanilla_today = vanilla_now(d);
            if (vanilla_today) { e->start = t + 1; logs("a disaster of the game itself is in the news: ours waits a turn"); continue; }
            start_event(d, e, t); if (!e->active) e->type = 0; continue; }
        if (e->active && e->start != t) {
            if (e->type == EV_FIRE) for (int j = 0; j < e->ncells; j++) { int c = e->cells[j]; if (occ[c]) { u8 *a = army(d, occ[c]); if (a) hurt(a, 10, 10, "fire"); } }
            if (--e->turns_left <= 0) end_event(d, e);
        }
    }
    { int j = 0; for (int i = 0; i < nev; i++) if (ev[i].type) { if (j != i) ev[j] = ev[i]; j++; } nev = j; }   /* v26k38c: finished ones out (the list only grew: after 48 in one session no new disaster could start) */
    if (v2) { TB(t_w); fires_turn(d, t); floods_turn(d, t); slides_turn(d, t); drought_turn(d, t); TE(t_w, "turn weather disasters", 0); }
    if (v2) harvest_year(d, t);
    if (v2) stats_write();   /* v26k11 */
    vis_turn();
}


/* ---- v25.9c: the integrated patch's fog-of-war module hides log lines whose place the player cannot see. Disaster news is
   news everyone hears, so our "[재해]" lines stay in the log at once. Done by pointing the fog module's own calls of its
   log-line filter at a small check of ours (its function itself is left untouched, so the integrated patch's own hook on
   it keeps working); every other line goes through the fog filter exactly as before. No fog module = nothing done. ---- */
typedef u8 (*FogVisFn)(u8 *);
static u8 *fog_entry; static int fog_state, fog_tries;
static int our_log_row(u8 *row)
{
    if (!row || !readable(row, 0x40) || *(u8 **)row != game + VT_LOGROW) return 0;
    const u16 *t = (const u16 *)(row + 0x14);
    return t[0] == '[' && t[1] == 0xC7AC && t[2] == 0xD574 && t[3] == ']';      /* "[재해]" */
}
static u8 FogVisWrap(u8 *row) { if (our_log_row(row)) return 1; return ((FogVisFn)fog_entry)(row); }
static u8 *near_relay(u8 *near_at, void *dest)
{
    u8 *r = api->MakeRelay(near_at, dest); if (r) return r;
    for (u64 k = 1; k < 0x7000; k++) {             /* our own 14-byte jump, within 2 GB below the module */
        u8 *want = (u8 *)((((u64)near_at) & ~0xffffULL) - k * 0x10000ULL);
        u8 *m = (u8 *)VirtualAlloc(want, 0x1000, 0x3000, 0x40); if (!m) continue;
        if (m != want) { VirtualFree(m, 0, 0x8000); continue; }
        m[0] = 0xFF; m[1] = 0x25; m[2] = m[3] = m[4] = m[5] = 0; memcpy(m + 6, &dest, 8);
        return m;
    }
    return 0;
}
static void fog_try(void)
{
    if (fog_state || ++fog_tries > 600) { if (!fog_state) fog_state = -1; return; }
    u8 *m = (u8 *)GetModuleHandleW(L"SAN14_FogEngine121.dll"); if (!m || !readable(m, 0x400)) return;
    fog_state = -1;
    u32 pe = *(u32 *)(m + 0x3c); u8 *nt = m + pe; if (!readable(nt, 0x200) || *(u32 *)nt != 0x4550) return;
    u16 nsec = *(u16 *)(nt + 6), opt = *(u16 *)(nt + 20); u8 *sec = nt + 24 + opt; u8 *tx = 0; u32 tsz = 0;
    for (int i = 0; i < nsec && i < 32; i++) { u8 *e = sec + i * 40; if (!readable(e, 40)) return;
        if (e[0] == '.' && e[1] == 't' && e[2] == 'e' && e[3] == 'x' && e[4] == 't') { tx = m + *(u32 *)(e + 12); tsz = *(u32 *)(e + 8); } }
    if (!tx || tsz < 0x1000 || !readable(tx, tsz)) return;
    static const u8 P[7] = { 0x0F, 0xB7, 0x9A, 0x84, 0x00, 0x00, 0x00 };      /* movzx ebx, word [rdx+84h]: the line's place */
    u8 *hit = 0; int nh = 0;
    for (u32 k = 0x6e; k + 7 <= tsz; k++) if (tx[k] == 0x0F && tx[k + 1] == 0xB7 && tx[k + 2] == 0x9A) {
        int ok = 1; for (int j = 3; j < 7; j++) if (tx[k + j] != P[j]) ok = 0; if (ok) { hit = tx + k; nh++; } }
    char b[160], *p;
    if (nh != 1) { p = b; p = pstr(p, "fog: log filter not found ("); p = pint(p, nh); p = pstr(p, "), fog rules unchanged"); *p = 0; logs(b); return; }
    u8 *entry = hit - 0x6e;
    if (!(entry[7] == 0x48 && entry[8] == 0x8B && entry[9] == 0x3D)) { logs("fog: unknown log filter, fog rules unchanged"); return; }
    u8 *site[16]; int ns = 0;
    for (u32 k = 0; k + 5 <= tsz && ns < 16; k++) if (tx[k] == 0xE8) { i32 rel; memcpy(&rel, tx + k + 1, 4); if (tx + k + 5 + rel == entry) site[ns++] = tx + k; }
    if (!ns || ns > 12) { logs("fog: log filter calls not found, fog rules unchanged"); return; }
    u8 *relay = near_relay(site[0], (void *)FogVisWrap); if (!relay) { logs("fog: no room for a jump, fog rules unchanged"); return; }
    for (int i = 0; i < ns; i++) { i64 dl = relay - (site[i] + 5); if (dl < -0x7fffffffLL || dl > 0x7fffffffLL) { logs("fog: jump too far, fog rules unchanged"); return; } }
    fog_entry = entry;
    for (int i = 0; i < ns; i++) {
        i32 nd = (i32)(relay - (site[i] + 5)); DWORD old = 0;
        if (!VirtualProtect(site[i] + 1, 4, 0x40, &old)) continue;
        *(volatile i32 *)(site[i] + 1) = nd;
        VirtualProtect(site[i] + 1, 4, old, &old);
    }
    FlushInstructionCache(GetCurrentProcess(), tx, tsz);
    fog_state = 1;
    p = b; p = pstr(p, "fog: disaster news shown at once ("); p = pint(p, ns); p = pstr(p, " filter calls)"); *p = 0; logs(b);
}
/* ---- the time step hook (game thread) ---- */
static u32 last_day, last_turn_seen = 0xffffffff, hold_day;
static void on_time(void)
{
    u8 *d = db(); if (!d || !dat) return;
    if (fog_state == 0) fog_try();
    if (npend && last_day != hold_day) flush_reports(0);   /* v25.9e: never on a held day */
    static u8 *date_ok; u8 *date = *(u8 **)(d + DB_DATE);
    if (date != date_ok) { if (!readable(date, 0x460)) return; date_ok = date; }
    u32 day = *(u32 *)(date + 0x450); if (day == last_day) return;
    int fresh = last_day == 0 || day < last_day || day > last_day + 40;
    last_day = day;
    int month = date[0x36], dd = date[0x37]; if (month < 1 || month > 12) return;
    int t = (month - 1) * 3 + (dd <= 10 ? 0 : dd <= 20 ? 1 : 2);
    /* v25.9d: a new ten-day period is entered on the LAST day of the old one. On the first day of a period the game rebuilds
       its log and stops for the new turn, and a line added on that very day only showed up a turn later. One day early,
       the news is in the log when the new turn begins. */
    if (dd == 10 || dd == 20 || dd >= 29) t = (t + 1) % 36;
    int first_day = (dd == 1 || dd == 11 || dd == 21);
    static int sess_days; sess_days++;
    day_tick_at = GetTickCount64();            /* v26h: the frame watch skips the seconds around a day tick */
    g_year = *(u16 *)(date + 0x34);
    if (fresh) {                               /* a save was loaded: forget everything, roll again */
        end_quiet = 1; for (int i = 0; i < nev; i++) if (ev[i].type && ev[i].active) end_event(d, &ev[i]); end_quiet = 0; memset(qtrack, 0, sizeof(qtrack));
        npend = 0;
        nev = 0; for (int i = 0; i < 4; i++) rolled_year[i] = 0; for (int i = 0; i < 6; i++) burnt_region[i] = 0; quake_year = -0x7fffffff; harvest_year_done = -0x7fffffff;
        memset(flooded, 0, sizeof(flooded)); memset(burning, 0, sizeof(burning)); memset(quaked, 0, sizeof(quaked)); memset(mud, 0, sizeof(mud)); vis_clear();
#ifdef SEASON_TEST
        test_rain_days = 0;                    /* v26k9: a reload ends the forced weather */
#endif
#ifdef HOSTFIX
        rs = 0x9E3779B97F4A7C15ULL ^ day; rnd(); rnd();
#else
        rs ^= GetTickCount64() * 0x2545F4914F6CDD1DULL ^ day; rnd(); rnd();
#endif
        if (v2) engine_reset();
        last_turn_seen = 0xffffffff; sess_days = 1;
        for (int c = 0; c < NCELL; c++) {      /* a cell we blocked must never stay blocked across loads */
            if (vterr[c] == T_BLOCKED) continue; u8 *h = cellp(d, c); if (h && h[0x10] == T_BLOCKED) { h[0x10] = vterr[c]; mine[c] = 0; } }
        te_apply();                            /* v26k42: the retyped cells, right away after a load */
        char b[80], *p = b; p = pstr(p, "game date "); p = pint(p, g_year); *p++ = '/'; p = pint(p, month); *p++ = '/'; p = pint(p, dd); p = pstr(p, " (new session)"); *p = 0; logs(b);
#ifdef SEASON_TEST
        if (test_on && !dump_started) { dump_started = 1; dump_done = 1; (void)dump_thread; }   /* v26: the data search of the link test is done; nothing to dump */
#endif
    }
    memset(occ, 0, sizeof(occ));
    { u8 *tmp[501]; int n = 0;
      for (int i = 1; i <= 500; i++) { u8 *a = army(d, i); if (a) { occ[*(u16 *)(a + 0x2a)] = (u16)i; tmp[n++] = a; } }
      for (int i = 1; i < n; i++) { u8 *v = tmp[i]; int j = i - 1; while (j >= 0 && tmp[j] > v) { tmp[j + 1] = tmp[j]; j--; } tmp[j + 1] = v; }   /* insertion sort, n <= 500 */
      n_army_idx = 0; memcpy(army_idx, tmp, n * sizeof(u8 *)); n_army_idx = n; }
    g_turn = t; g_syear = g_year + (month == 12 && t < 3);   /* v25.9i: 12월 하순's last day already belongs to next year's spring */
    int held = first_day || sess_days <= 1;      /* a period's first day or the first tick after loading: news goes out on the next day */
    if ((u32)t != last_turn_seen) {
        last_turn_seen = (u32)t;
        TB(t_turn); per_turn(d, t); TE(t_turn, "turn change (all)", 1);
        quiet_after = GetTickCount64() + 2000;
    }
    { TB(t_riv); rivers(d, t); TE(t_riv, "rivers", 0); }
    int dit = dd >= 29 ? dd - 29 : dd < 10 ? dd + 1 : dd < 20 ? dd - 10 : dd - 20, dlen = dd >= 29 || dd < 10 ? 11 : dd < 20 ? 10 : 9;   /* v26k12: the day within the turn */
    if (dit > dlen - 1) dit = dlen - 1;
    if (v2) { g_dayfrac = (dit + 0.5f) / dlen; TB(t_day); rain_day(t, dit);
#ifdef SEASON_TEST
        if (test_done && v2) { static int tday; tday++;
            if (prev_on) { int step = prev_day / 2, pt = (26 + step) % 36;
                if (step != prev_step) { prev_step = step; int cnt = 0;   /* where the cover spreads during this preview turn, the snow falls first */
                    for (int c = 0; c < NCELL; c++) { u8 w = 0; if (cl_land(c)) { float sC = vis_snow(pt, 0.0f, c), sN = vis_snow((pt + 1) % 36, 0.0f, c); if (sN >= 0.20f && (sC < 0.20f ? sN - sC > 0.02f : sN - sC >= 0.15f)) w = 1; } willCov[c] = w ? 2 : willCov[c] == 2 ? 0 : willCov[c]; cnt += w; }
                    char b[100], *p = b; p = pstr(p, "TEST snow preview step "); p = pint(p, step); p = pstr(p, " turn "); p = pint(p, pt); p = pstr(p, " snowfall cells "); p = pint(p, cnt); *p = 0; logs(b);
                    static const u16 *PM[18] = { L"눈·얼음 미리보기 시작 (9월 하순) - 34일 동안 이틀에 한 턴씩 3월 상순까지. 그림만 미리 보여 주고 규칙은 지금 계절 그대로", 0, 0, 0,
                        L"눈·얼음 미리보기: 11월 상순 - 첫눈이 내리고 덮이기 시작", 0, 0, L"눈·얼음 미리보기: 12월 상순 - 한겨울, 하북이 하얗고 황하가 얼어붙음", 0, 0, 0, 0,
                        L"눈·얼음 미리보기: 1월 하순 - 기온 따라 서서히 녹기 시작", 0, 0, 0, 0, 0 };
                    if (step < 18 && PM[step]) { rset(L"[시험] "); radd(PM[step]); report_ex(10905, 0); } }
                for (int c = 0; c < NCELL; c++) if (willCov[c] == 2) { rain[c] = 2; snowing[c] = 1; }
                if (++prev_day >= 34) { prev_on = 0; for (int c = 0; c < NCELL; c++) if (willCov[c] == 2) willCov[c] = 0;
                    rset(L"[시험] 눈·얼음 미리보기 끝 (3월 상순까지) - 이제 지금 계절 그림으로 돌아감"); report_ex(10905, 0); logs("TEST snow preview end"); } }
            if (tday == 40 && ntice) { int c0 = tice_c[0];   /* v26k38c: the test ice is let go - the rules' own thaw (an army on the ice keeps its cell until it steps off) */
                for (int i = 0; i < ntice; i++) { int c = tice_c[i]; cFrz[c >> 3] &= (u8)~(1 << (c & 7)); cFrzN[c >> 3] &= (u8)~(1 << (c & 7)); tmark[c] = 0; }
                char b[80], *p = b; p = pstr(p, "TEST: river ice let go, cells "); p = pint(p, ntice); *p = 0; logs(b); ntice = 0;
                test_line(d, c0, L"언 강이 녹기 시작 - 내일부터 빈 칸은 강으로 돌아가고, 얼음 위 부대는 내릴 때까지 그 칸 유지"); }
            if (tday == 31 || tday == 32) { for (int c = 0; c < NCELL; c++) { int x = c % 220, y = c / 220; if (x < 15 || x > 80 || y < 50 || y > 90 || !cl_land(c)) continue; rain[c] = 3; crack[c] = 0; dryd[c] = 0; wet[c] = 220; }   /* the test drought ends in a downpour */
                if (tday == 31) { rset(L"[시험] 가뭄 자리(장안·서량)에 폭우 - 다음 턴에 '가뭄이 풀렸습니다'"); report_ex(15468, 0); } } }
        if (test_rain_days > 0) { test_rain_days--; for (int k = 0; k < 10; k++) if (force_rain_r[k]) { force_snow_k = (k >= 4 && k <= 6) ? 1 : 0; force_rain_at(force_rain_cx[k], force_rain_cy[k], force_rain_r[k], force_rain_t[k]); } force_snow_k = 0; }
#endif
        weather_morale(d, day); city_weather_count(); fires_day(d, t); hydro_day(); floods_day(d, t); flood_entry(d); game_fires_day(d); if (weather_resend) weather_resend = 0; weather_write(); TE(t_day, "day weather", 0); }
#ifdef SEASON_TEST
    if (test_on && !test_done && sess_days >= 3 && !first_day) test_start(d, t);   /* v26: the forced disasters come on the third day after loading */
#endif
    spd_build();                               /* v26k28: the day's slow-downs into the table the speed hook reads */
    hold_day = held ? day : 0;
    if (npend && !held) { TB(t_fl); flush_reports(1); TE(t_fl, "news lines out (all of the day)", 1); }               /* v25.9b: the day's reports go out at once, on the day the disaster happens
                                                  (the game may stop at a new turn; waiting for the pictures held them a whole turn) */
}
typedef int (GAMEABI *TimeFn)(void);
static TimeFn time_orig;          /* v26k38a: the game's time step - its own entry (call-site hook) or our copy of its start (entry hook) */
static u8 *time_skip_ret;          /* v26k38a entry hook: the game's other caller of the time step (not the day loop) */
#ifdef SEASON_TEST
/* link test: how often the game calls its time step and the longest wait between two calls (the game's own work + drawing) */
static i64 th_prev; static u32 th_calls; static i64 th_maxgap; static int th_lines;
static int GAMEABI TimeHook(void) { int r = time_orig();
    if (!test_on) { on_time(); return r; }
    if (!q_freq) QueryPerformanceFrequency(&q_freq);
    i64 q0 = qnow(), gap = 0;
    if (th_prev && q_freq > 0) { gap = (q0 - th_prev) * 1000 / q_freq; if (gap > 60000) gap = 0; } th_calls++;
    u32 before = last_day; on_time(); i64 q1 = qnow();
    if (last_day != before) { u8 *d = db(); if (d && q_freq > 0) test_day(d, (q1 - q0) * 1000000 / q_freq);
        if (before && th_lines++ < 200) { char b[200], *p = b; p = pstr(p, "steps: day "); p = pint(p, before); p = pstr(p, " calls "); p = pint(p, th_calls); p = pstr(p, " longest gap ms "); p = pint(p, th_maxgap);
            { static u64 rc_prev; u64 rcd = rc_calls - rc_prev; rc_prev = rc_calls; p = pstr(p, " route steps "); p = pint(p, (int)(rcd > 2000000000ull ? 2000000000ull : rcd)); p = pstr(p, rc_off ? " (old cost)" : " (new cost)"); } *p = 0; logs(b); }
        th_calls = 0; th_maxgap = 0;
        int dd = 0; if (d) { u8 *date = *(u8 **)(d + DB_DATE); if (readable(date, 0x40)) dd = date[0x37]; }
        if (dd != 1 && dd != 11 && dd != 21) th_maxgap = gap; }   /* a new turn stops the game for the player: that wait is not a stall */
    else if (gap > th_maxgap) th_maxgap = gap;
    th_prev = qnow();
    return r; }
#else
#ifdef DIAG28
static int GAMEABI TimeHook(void) { int r = time_orig(); i64 t0 = hq_now(); on_time(); i64 d = hq_now() - t0; th_ticks += d; th_n++; if (d > th_maxt) th_maxt = d; return r; }
#else
static int GAMEABI TimeHook(void) { int r = time_orig(); on_time(); return r; }
#endif
#endif

/* ---- install ---- */
static int install_call(const char *id, u32 site, u32 target, void *dest)
{
    S14_Hook h; memset(&h, 0, sizeof(h));
    u8 *at = game + site; h.target = at; h.length = 5;
    h.expected[0] = 0xE8; i32 o = (i32)((i64)target - (i64)site - 5); memcpy(h.expected + 1, &o, 4);
    if (!readable(at, 5)) return 0; for (int i = 0; i < 5; i++) if (at[i] != h.expected[i]) return 0;
    u8 *relay = api->MakeRelay(at, dest); if (!relay) return 0;
    i64 dl = relay - (at + 5); if (dl < -0x7fffffffLL || dl > 0x7fffffffLL) { api->FreeRelay(relay); return 0; }
    h.replacement[0] = 0xE8; i32 dd = (i32)dl; memcpy(h.replacement + 1, &dd, 4);
    if (!api->InstallHooks(id, &h, 1)) { api->FreeRelay(relay); return 0; }
    return 1;
}
/* v26k38a: when another mod already holds the time step's call site (its own call there), the hook goes on the time step's own first
   bytes instead: mov rax,[rip+x] (7 bytes, copied with its address made absolute) then back into the game. The step has one other
   caller (game+0x4ba280 / rollback 0x4b97a0); from there it runs as before without our day (the call-site hook never saw it either). */
static int GAMEABI TimeHook(void);
static int GAMEABI TimeEntryHook(void)
{
    if (__builtin_return_address(0) == (void *)time_skip_ret) return time_orig();
    return TimeHook();
}
static int install_time_entry(void)
{
    u8 *at = game + RVA_TIME_FN; if (!readable(at, 16)) return 0;
    static const u8 h0[3] = { 0x48, 0x8b, 0x05 };                                       /* mov rax, [rip + disp32] */
    for (int i = 0; i < 3; i++) if (at[i] != h0[i]) return 0;
    if (at[7] != 0x48 || at[8] != 0x8b || at[9] != 0x88) return 0;                      /* mov rcx, [rax + disp32] as found after it */
    u64 src = (u64)(at + 7 + *(int *)(at + 3));
    u8 *tr = (u8 *)VirtualAlloc(0, 4096, 0x3000, 0x40); if (!tr) return 0;
    tr[0] = 0x48; tr[1] = 0xa1; *(u64 *)(tr + 2) = src;                                  /* mov rax, [abs64] */
    tr[10] = 0xff; tr[11] = 0x25; *(u32 *)(tr + 12) = 0; *(u64 *)(tr + 16) = (u64)(at + 7);   /* jmp back after the copied instruction */
    FlushInstructionCache(GetCurrentProcess(), tr, 32);
    time_orig = (TimeFn)tr; time_skip_ret = game + (gv_off ? GA(0x4ba285u) : 0x4b97a5u);
    u8 *relay = api->MakeRelay(at, (void *)TimeEntryHook); if (!relay) { VirtualFree(tr, 0, 0x8000); time_orig = 0; return 0; }
    i64 dl = relay - (at + 5); if (dl < -0x7fffffffLL || dl > 0x7fffffffLL) { api->FreeRelay(relay); VirtualFree(tr, 0, 0x8000); time_orig = 0; return 0; }
    S14_Hook h; memset(&h, 0, sizeof(h)); h.target = at; h.length = 7; memcpy(h.expected, at, 7);
    h.replacement[0] = 0xE9; i32 dd = (i32)dl; memcpy(h.replacement + 1, &dd, 4); h.replacement[5] = 0x90; h.replacement[6] = 0x90;
    if (!api->InstallHooks("SAN14CE.SeasonRules.TimeEntry", &h, 1)) { api->FreeRelay(relay); VirtualFree(tr, 0, 0x8000); time_orig = 0; return 0; }
    return 1;
}
static int install_speed(void)
{
    static const u8 head1[8] = { 0x40, 0x53, 0x56, 0x41, 0x55, 0x41, 0x56, 0 };       /* 1.0.1: push rbx (rex); push rsi; push r13; push r14 */
    static const u8 head5[8] = { 0x40, 0x57, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57 };    /* v26k41, 2026-11-05: push rdi (rex); push r13; push r14; push r15 */
    const u8 *head = gv_new ? head5 : head1; u32 hl = gv_new ? 8 : 7;
    u8 *at = game + RVA_SPEED_FN;
    if (!readable(at, 16)) return 0; for (u32 i = 0; i < hl; i++) if (at[i] != head[i]) return 0;
    u8 *tr = (u8 *)VirtualAlloc(0, 4096, 0x3000, 0x40); if (!tr) return 0;
    memcpy(tr, head, hl); tr[hl] = 0xFF; tr[hl + 1] = 0x25; *(u32 *)(tr + hl + 2) = 0; *(u64 *)(tr + hl + 6) = (u64)(at + hl);
    FlushInstructionCache(GetCurrentProcess(), tr, 32);
    speed_orig = (SpeedFn)tr;
    u8 *relay = api->MakeRelay(at, (void *)SpeedHook); if (!relay) { VirtualFree(tr, 0, 0x8000); return 0; }
    i64 dl = relay - (at + 5); if (dl < -0x7fffffffLL || dl > 0x7fffffffLL) { api->FreeRelay(relay); VirtualFree(tr, 0, 0x8000); return 0; }
    S14_Hook h; memset(&h, 0, sizeof(h)); h.target = at; h.length = hl; memcpy(h.expected, head, hl);
    h.replacement[0] = 0xE9; i32 dd = (i32)dl; memcpy(h.replacement + 1, &dd, 4); for (u32 i = 5; i < hl; i++) h.replacement[i] = 0x90;
    if (!api->InstallHooks("SAN14CE.SeasonRules.SnowSpeed", &h, 1)) { api->FreeRelay(relay); VirtualFree(tr, 0, 0x8000); speed_orig = 0; return 0; }
    return 1;
}
/* v26k38c: when another mod (the AI mod's SAN14_CombatAI.dll) already wraps the unit speed function with a jump at its start, the
   season slow-down goes in front of it instead of being left out: the jump is pointed at our hook, and our hook calls what that jump
   led to (the other mod's wrapper, which calls the game's own function as before). Both changes apply - theirs, then the season's. */
static u64 spd_chained_to;
static int install_speed_chain(void)
{
    u8 *at = game + RVA_SPEED_FN; if (!readable(at, 16)) return 0;
    u64 dst = 0;
    if (at[0] == 0xE9) dst = (u64)(at + 5 + *(i32 *)(at + 1));
    else if (at[0] == 0xFF && at[1] == 0x25 && *(i32 *)(at + 2) == 0) dst = *(u64 *)(at + 6);
    if (!dst || !readable((void *)dst, 16)) return 0;
    if (dst >= (u64)game + RVA_SPEED_FN && dst < (u64)game + RVA_SPEED_FN + 16) return 0;   /* a jump into itself: not a wrapper */
    u8 cur[5]; memcpy(cur, at, 5);
    speed_orig = (SpeedFn)dst;
    u8 *relay = api->MakeRelay(at, (void *)SpeedHook); if (!relay) { speed_orig = 0; return 0; }
    i64 dl = relay - (at + 5); if (dl < -0x7fffffffLL || dl > 0x7fffffffLL) { api->FreeRelay(relay); speed_orig = 0; return 0; }
    S14_Hook h; memset(&h, 0, sizeof(h)); h.target = at; h.length = 5; memcpy(h.expected, cur, 5);
    h.replacement[0] = 0xE9; i32 dd = (i32)dl; memcpy(h.replacement + 1, &dd, 4);
    if (!api->InstallHooks("SAN14CE.SeasonRules.SnowSpeedChain", &h, 1)) { api->FreeRelay(relay); speed_orig = 0; return 0; }
    spd_chained_to = dst; return 1;
}
/* v26k38c: the hook sits on the first bytes of the step-cost function itself (0x1864f0), not on its three call sites: the integrated
   patch already wraps the call at 0x1841af (it clamps a step at 0x4000 and flags the search) and its wrapper calls this function -
   so both work, whichever is installed first. */
static int install_routecost(void)
{
    static const u8 head[10] = { 0x48, 0x89, 0x5c, 0x24, 0x10, 0x48, 0x89, 0x6c, 0x24, 0x20 };   /* mov [rsp+10h],rbx; mov [rsp+20h],rbp */
    u32 r0 = GA(0x1864f0u), rs = GA(0x1865aau), rr = GA(0x1865afu); if (!r0 || !rs || !rr) return 0;
    u8 *at = game + r0;
    if (!readable(at, 16) || !readable(game + rs, 5)) return 0;
    for (int i = 0; i < 10; i++) if (at[i] != head[i]) return 0;
    if (game[rs] != 0xE8 || *(i32 *)(game + rs + 1) != (i32)(RVA_SPEED_FN - rr)) return 0;   /* its speed call returns where the speed hook looks */
    u8 *tr = (u8 *)VirtualAlloc(0, 4096, 0x3000, 0x40); if (!tr) return 0;
    memcpy(tr, head, 10); tr[10] = 0xFF; tr[11] = 0x25; *(u32 *)(tr + 12) = 0; *(u64 *)(tr + 16) = (u64)(at + 10);
    FlushInstructionCache(GetCurrentProcess(), tr, 32);
    rc_orig = (RouteCostFn)tr; rc_ret = game + rr;
    u8 *relay = api->MakeRelay(at, (void *)RouteCost); if (!relay) { VirtualFree(tr, 0, 0x8000); rc_orig = 0; return 0; }
    i64 dl = relay - (at + 5); if (dl < -0x7fffffffLL || dl > 0x7fffffffLL) { api->FreeRelay(relay); VirtualFree(tr, 0, 0x8000); rc_orig = 0; return 0; }
    S14_Hook h; memset(&h, 0, sizeof(h)); h.target = at; h.length = 10; memcpy(h.expected, head, 10);
    h.replacement[0] = 0xE9; i32 dd = (i32)dl; memcpy(h.replacement + 1, &dd, 4); for (int i = 5; i < 10; i++) h.replacement[i] = 0x90;
    if (!api->InstallHooks("SAN14CE.SeasonRules.RouteCost", &h, 1)) { api->FreeRelay(relay); VirtualFree(tr, 0, 0x8000); rc_orig = 0; return 0; }
    return 1;
}
/* v26k38c: the second place (see rc_scale): 8 bytes a little into the step-cost function, the base cost scaled there */
static int install_routecost_mid(void)
{
    static const u8 head[8] = { 0x49, 0x8b, 0xf9, 0x49, 0x8b, 0xd8, 0x8b, 0xe9 };   /* mov rdi,r9; mov rbx,r8; mov ebp,ecx */
    u32 r0 = GA(0x1864ffu), rs = GA(0x1865aau), rr = GA(0x1865afu); if (!r0 || !rs || !rr) return 0;
    u8 *at = game + r0;
    if (!readable(at, 16) || !readable(game + rs, 5)) return 0;
    for (int i = 0; i < 8; i++) if (at[i] != head[i]) return 0;
    if (game[rs] != 0xE8 || *(i32 *)(game + rs + 1) != (i32)(RVA_SPEED_FN - rr)) return 0;
    u8 *st = (u8 *)VirtualAlloc(0, 4096, 0x3000, 0x40); if (!st) return 0;
    static const u8 a[] = { 0x49, 0x8b, 0xf9, 0x49, 0x8b, 0xd8, 0x52, 0x41, 0x50, 0x41, 0x51, 0x48, 0x83, 0xec, 0x28, 0x48, 0xb8 };
    static const u8 b[] = { 0xff, 0xd0, 0x48, 0x83, 0xc4, 0x28, 0x41, 0x59, 0x41, 0x58, 0x5a, 0x8b, 0xe8, 0xff, 0x25, 0, 0, 0, 0 };
    int n = 0; memcpy(st + n, a, sizeof(a)); n += sizeof(a); *(u64 *)(st + n) = (u64)(void *)rc_scale; n += 8;
    memcpy(st + n, b, sizeof(b)); n += sizeof(b); *(u64 *)(st + n) = (u64)(at + 8); n += 8;
    FlushInstructionCache(GetCurrentProcess(), st, n);
    rc_ret = game + rr;
    u8 *relay = api->MakeRelay(at, (void *)st); if (!relay) { VirtualFree(st, 0, 0x8000); return 0; }
    i64 dl = relay - (at + 5); if (dl < -0x7fffffffLL || dl > 0x7fffffffLL) { api->FreeRelay(relay); VirtualFree(st, 0, 0x8000); return 0; }
    S14_Hook h; memset(&h, 0, sizeof(h)); h.target = at; h.length = 8; memcpy(h.expected, head, 8);
    h.replacement[0] = 0xE9; i32 dd = (i32)dl; memcpy(h.replacement + 1, &dd, 4); for (int i = 5; i < 8; i++) h.replacement[i] = 0x90;
    if (!api->InstallHooks("SAN14CE.SeasonRules.RouteCost2", &h, 1)) { api->FreeRelay(relay); VirtualFree(st, 0, 0x8000); return 0; }
    return 1;
}
/* v26k38c: the game's own per-unit route code (official 0x9b210) reads the last entry of a route list it filled without checking that
   the list has anything in it; with no route found that entry is empty and the game stops (SAN14CE_KO.exe+0x9bbbc, 10-28 / 10-29).
   Each of its three reads is given the missing check: empty = that unit's step ends at the code's own clean-up (0x9bc5d), as when it
   finds nothing to do. Installed only where the bytes are the game's own. */
static volatile long rg_caught;
static u8 *rg_stub(const u8 *chk, int nchk, const u8 *load, int nload, u8 *cont, u8 *skip)
{
    u8 *st = (u8 *)VirtualAlloc(0, 4096, 0x3000, 0x40); if (!st) return 0;
    int n = 0;
    memcpy(st + n, chk, nchk); n += nchk;                         /* test reg,reg */
    st[n++] = 0x74; int jz = n++;                                  /* jz skip */
    memcpy(st + n, load, nload); n += nload;                       /* the game's own load */
    st[n++] = 0x48; st[n++] = 0xb8; *(u64 *)(st + n) = (u64)(game + GA(0x22dda0u)); n += 8;   /* the game's own call */
    st[n++] = 0xff; st[n++] = 0xd0;
    st[n++] = 0xff; st[n++] = 0x25; *(u32 *)(st + n) = 0; n += 4; *(u64 *)(st + n) = (u64)cont; n += 8;
    st[jz] = (u8)(n - (jz + 1));
    st[n++] = 0x48; st[n++] = 0xb8; *(u64 *)(st + n) = (u64)(void *)&rg_caught; n += 8;
    st[n++] = 0xf0; st[n++] = 0xff; st[n++] = 0x00;               /* count it */
    st[n++] = 0xff; st[n++] = 0x25; *(u32 *)(st + n) = 0; n += 4; *(u64 *)(st + n) = (u64)skip; n += 8;
    FlushInstructionCache(GetCurrentProcess(), st, n);
    return st;
}
static int install_routeguard(void)
{
    /* 0x9bbbc / 0x9bbfe: mov rcx,[rcx]; call 0x22dda0 - 0x9bc43: mov rcx,[r13]; call 0x22dda0 (v26k41: the call's distance worked out for
       the running exe, the rest of the bytes as found in both) */
    u32 rf = GA(0x22dda0u), rb = GA(0x9bbb0u), rk = GA(0x9bc5du); if (!rf || !rb || !rk) return 0;
    u8 e1[8] = { 0x48, 0x8b, 0x09, 0xe8 }, e2[8] = { 0x48, 0x8b, 0x09, 0xe8 }, e3[9] = { 0x49, 0x8b, 0x4d, 0x00, 0xe8 };
    static const u8 trcx[3] = { 0x48, 0x85, 0xc9 }, tr13[3] = { 0x4d, 0x85, 0xed };
    struct { u32 rva; u8 *e; int len; const u8 *t; } P[3] = { { GA(0x9bbbcu), e1, 8, trcx }, { GA(0x9bbfeu), e2, 8, trcx }, { GA(0x9bc43u), e3, 9, tr13 } };
    for (int k = 0; k < 3; k++) { if (!P[k].rva) return 0; i32 o = (i32)((i64)rf - (i64)(P[k].rva + P[k].len)); memcpy(P[k].e + P[k].len - 4, &o, 4); }
    if (!readable(game + rb, 0xb0)) return 0;
    for (int k = 0; k < 3; k++) for (int i = 0; i < P[k].len; i++) if (game[P[k].rva + i] != P[k].e[i]) return 0;
    S14_Hook h[3]; u8 *rl[3] = { 0, 0, 0 };
    for (int k = 0; k < 3; k++) {
        u8 *at = game + P[k].rva;
        u8 *st = rg_stub(P[k].t, 3, P[k].e, P[k].len - 5, at + P[k].len, game + rk);
        if (st) rl[k] = api->MakeRelay(at, (void *)st);
        i64 dl = rl[k] ? rl[k] - (at + 5) : 0;
        if (!st || !rl[k] || dl < -0x7fffffffLL || dl > 0x7fffffffLL) { for (int j = 0; j <= k; j++) if (rl[j]) api->FreeRelay(rl[j]); return 0; }
        memset(&h[k], 0, sizeof(h[k])); h[k].target = at; h[k].length = P[k].len; memcpy(h[k].expected, P[k].e, P[k].len);
        h[k].replacement[0] = 0xE9; i32 dd = (i32)dl; memcpy(h[k].replacement + 1, &dd, 4); for (int i = 5; i < P[k].len; i++) h[k].replacement[i] = 0x90;
    }
    if (!api->InstallHooks("SAN14CE.SeasonRules.RouteGuard", h, 3)) { for (int k = 0; k < 3; k++) api->FreeRelay(rl[k]); return 0; }
    return 1;
}
#include "tileinfo_v26.inc"
#ifndef HOSTTEST
#include "pathfind_v26.inc"   /* v26k36: the game's unit route search without the long walks (same routes) */
#endif
#ifdef SEASON_MEASURE
#include "measure_v26.inc"
#endif
/* v26k18: which game version is running (its build stamp in the exe header); an unknown one = nothing is changed */
static int mod_inited;
static void use_version(int i)
{
    const GameVer *g = &GVER[i]; gver = g; gv_off = i >= 1; gv_new = i == 2; gv_image = i == 2 ? 0x2a02000 : 0x29ff000;
    RVA_GAMEDB = i == 2 ? 0x2780f60 : 0x277df18;
    VT_ARMY = g->vt_army; VT_FOOT = g->vt_foot; RVA_TIME_SITE = g->time_site; RVA_TIME_FN = g->time_fn; RVA_SPEED_FN = g->speed_fn; RVA_HISTORY = g->history;
    RVA_HISTMGR = g->histmgr; RVA_EMPTYSTR = g->emptystr; VT_FOOTHOLD = g->vt_foothold; VT_CITYDATA = g->vt_citydata; RVA_GETFOOD = g->getfood;
    RVA_FIRERATE = g->firerate; RVA_SETHEXFIRE = g->sethexfire; VT_LOGROW = g->vt_logrow;
}
/* v26k38a (asked for): when a part of the season rules cannot start (another mod holds its place, another game version, a damaged
   install), a message box says so at the start of the game with one short line to copy into a comment - which parts are off and who
   holds their place. Shown from its own thread, so the game goes on starting meanwhile. Never shown when everything is on. */
#ifndef HOSTTEST
static u16 sn_text[1400];
static DWORD WINAPI sn_thread(void *p) { (void)p; MessageBoxW(0, sn_text, L"계절 맵 v26k51", 0x30 | 0x10000 | 0x40000); return 0; }
static void start_notice(const char *code)
{
    u16 *w = sn_text; const u16 *head = L"계절 맵의 일부 기능이 꺼진 채로 시작했습니다.\r\n(다른 모드가 같은 자리를 쓰고 있거나, 게임 판이 다르거나, 설치가 덜 된 경우)\r\n계절 그림은 그대로 나옵니다.\r\n\r\n아래 한 줄을 그대로 댓글로 알려 주세요 (이 창에서 Ctrl+C 를 누르면 복사됩니다):\r\n\r\n";
    while (*head) *w++ = *head++;
    for (int i = 0; code[i] && w - sn_text < 1390; i++) *w++ = (u16)(u8)code[i];
    *w = 0;
    { char b[700], *p = b; p = pstr(p, "START NOTICE (message box): "); for (int i = 0; code[i] && p - b < 680; i++) *p++ = code[i]; *p = 0; logs(b); }
    HANDLE th = CreateThread(0, 0, (void *)sn_thread, 0, 0, 0); if (th) CloseHandle(th);
}
#else
static void start_notice(const char *code) { (void)code; }
#endif
static int pick_version(void)
{
    u32 stamp = 0;
    if (readable(game, 0x400)) { u32 pe = *(u32 *)(game + 0x3c); if (pe < 0x300 && readable(game + pe, 0x18) && *(u32 *)(game + pe) == 0x4550) stamp = *(u32 *)(game + pe + 8); }
    for (int i = 0; i < NGVER; i++) if (GVER[i].stamp == stamp) { use_version(i); char b[160], *p = b; p = pstr(p, "game version: "); p = pstr(p, GVER[i].name); *p = 0; logs(b); return 1; }
#ifdef HOSTTEST
    if (!gver) use_version(0); return 1;      /* the host test's stand-in image has no exe header: the table it chose (default rollback) */
#else
    { char b[120], *p = b; p = pstr(p, "SKIPPED: unknown game version (exe stamp "); p = phex(p, stamp); p = pstr(p, ") - nothing changed"); *p = 0; logs(b); }
    { char c[80], *p = c; p = pstr(p, "SM51 VER "); p = phex(p, stamp); *p = 0; start_notice(c); }
    return 0;
#endif
}
EXPORT u32 GAMEABI SAN14_ModInitV1(const S14_CoreAPI *a)
{
    if (!a || a->size < sizeof(S14_CoreAPI) || a->abi != 0x00010000u || !a->resolved || (a->resolved->image_size != 0x29ff000 && a->resolved->image_size != 0x2a02000) ||
        !a->Readable || !a->InstallHooks || !a->MakeRelay || !a->FreeRelay) return 0;
    api = a; game = a->resolved->image;
    {   /* paths next to this DLL */
        u16 p[600]; DWORD n = GetModuleFileNameW(self_module, p, 560); if (!n) return 0;
        while (n && p[n - 1] != '\\' && p[n - 1] != '/') n--; p[n] = 0;
        wcat(p, L"SAN14Mods\\SeasonRules\\");   /* the loader keeps module DLLs in the game folder; our files live here */
        logpath[0] = 0; wcat(logpath, p); wcat(logpath, L"SeasonRules.log");
        datpath[0] = 0; wcat(datpath, p); wcat(datpath, L"SeasonRules.dat");
    }
    logs("SeasonRules 1 starting");
#if defined(SEASON_TEST) && !defined(HOSTTEST)
    sr_crash_catcher();
#endif
    if (mod_inited) { logs("already started once in this game session - the second start is ignored"); return 1; }   /* v26k18: never twice (old loader + self-host) */
    if (!pick_version()) return 0;
#ifdef SEASON_TEST
    { u16 fp[600]; fp[0] = 0; wcat(fp, datpath); int n = wlen(fp); while (n && fp[n - 1] != '\\') n--; fp[n] = 0; wcat(fp, L"SeasonRules_test.flag");
      if (GetFileAttributesW(fp) != 0xFFFFFFFFu) DeleteFileW(fp); test_on = 1;   /* v26k30b (asked for): the test build forces everything in every session - no flag needed */
      logs("TEST BUILD: test flag found - every disaster, every weather and every ground state is forced on the third day after loading (every session)"); }
#endif
    if (!load_tables()) { logs("SKIPPED: SeasonRules.dat missing or damaged"); start_notice("SM51 DAT"); if (a->Log) a->Log("SeasonRules: data file missing; nothing changed."); return 0; }
    { char b[80], *p = b; p = pstr(p, "tables ok, freezable river cells "); p = pint(p, nf); *p = 0; logs(b); }
    te_load();                                 /* v26k42 */
    engine_load();
    { HANDLE th = CreateThread(0, 0, (void *)sender_thread, 0, 0, 0); if (th) CloseHandle(th); else logs("map link thread failed (the map will not show the disasters)"); }
    time_orig = (TimeFn)(game + RVA_TIME_FN);
#ifdef SEASON_TEST
    int t_ok = 0;   /* v26k38a test build: the call site is left alone on purpose, so the time step's entry hook (the fallback) runs in the game */
    logs("TEST BUILD: time call site skipped on purpose - the fallback (time step entry) is what runs in this test");
#else
    int t_ok = install_call("SAN14CE.SeasonRules.Time", RVA_TIME_SITE, RVA_TIME_FN, (void *)TimeHook);
#endif
    u8 time_site_seen[8]; for (int i = 0; i < 8; i++) time_site_seen[i] = readable(game + RVA_TIME_SITE, 8) ? game[RVA_TIME_SITE + i] : 0;
    if (!t_ok) { t_ok = install_time_entry() ? 2 : 0;
        logs(t_ok ? "time hook: the call site is taken by another mod - hooked at the time step's own start instead (same days)" : "time hook: call site taken and the time step's start changed too"); }
    int s_ok = install_speed();
    if (!s_ok && install_speed_chain()) { s_ok = 2;
        char b[200], *p = b; p = pstr(p, "snow speed hook: the speed function's start is another mod's wrapper (");
#ifndef HOSTTEST
        p = sr_where(p, spd_chained_to);
#endif
        p = pstr(p, ") - the season slow-down is put in front of it (both apply)"); *p = 0; logs(b); }
    rc_ok = gv_off ? install_routecost() : 0;   /* v26k38c: the route step cost per cell - the official exe only */
    if (!rc_ok && gv_off && install_routecost_mid()) rc_ok = 2;
    logs(rc_ok == 1 ? "route cost: installed at the step-cost function's start (each cell weighed by its weather / disaster; rain x1.3, downpour x1.6, at most x4) - the integrated patch's own wrapper of the call is left as it is"
       : rc_ok == 2 ? "route cost: the function's start is taken by another mod - installed a few bytes in instead (the same weights)"
                    : "route cost: SKIPPED (another game version or both places taken) - the old weighing");
    rg_ok = gv_off ? install_routeguard() : 0;
    logs(rg_ok ? "route guard: installed (the game's empty route-list read at 0x9bbbc is checked: that unit's step is skipped instead of a crash)"
               : "route guard: SKIPPED (another game version or the bytes are not the game's own)");
    int f_ok = v2 ? install_firerate() : 0;
    ti_fr_ok = f_ok; int ti_ok = install_tileinfo();   /* v26k30 */
#ifndef HOSTTEST
    int pf_ok = install_pathfind();
    logs(pf_ok ? "route search: installed (same routes, without the long list walks; self-check on)" : "route search: SKIPPED - the game's own route search unchanged");
#endif
    { static const u8 sh[12] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x57, 0x48, 0x83, 0xec, 0x20, 0x8b, 0xda }; u8 *q = game + RVA_SETHEXFIRE; sethexfire_ok = 1; if (!readable(q, 16)) sethexfire_ok = 0; else for (int i = 0; i < 12; i++) if (q[i] != sh[i]) sethexfire_ok = 0; }
    logs(t_ok ? "time hook: installed" : "time hook: SKIPPED (place taken or different game code) - rivers and disasters off");
    logs(s_ok ? "snow speed hook: installed" : "snow speed hook: SKIPPED (place taken or different game code) - snow slowdown off");
    logs(f_ok ? "fire rate hook: installed (rain weakens 화계)" : "fire rate hook: SKIPPED (place taken or different game code) - 화계 unchanged by rain");
    logs(sethexfire_ok ? "game fire setter: found (rain shortens the game's own fires)" : "game fire setter: not found (the game's own fires ignore the rain)");
    logs(v2 ? "data: v2 climate records loaded (living climate on)" : "data: v1 tables only (fixed-season rules)");
    logs(ti_ok ? "tile info: installed (the hex panel shows what is going on in the cell)" : "tile info: SKIPPED (place taken or different game code) - the hex panel unchanged");
    if (a->Log) a->Log(t_ok ? "SeasonRules: seasonal rivers/disasters on." : "SeasonRules: time hook skipped.");
    {   /* v26k38a: one line - T time step, I tile panel, S speed, F 화계, R route cost (official only); 1 on, 2 on another way, 0 off */
        char c[600], *p = c; p = pstr(p, "SM51 T"); p = pint(p, t_ok); p = pstr(p, " I"); p = pint(p, ti_ok && ti_calls_ok ? 1 : 0); p = pstr(p, " S"); p = pint(p, s_ok);
        p = pstr(p, " F"); p = pint(p, f_ok); p = pstr(p, " R"); p = pint(p, rc_ok); p = pstr(p, " G"); p = pint(p, rg_ok); p = pstr(p, gv_new ? " n5" : gv_off ? " off" : " rb");
        int bad = !t_ok || !(ti_ok && ti_calls_ok) || !s_ok || (gv_off && (!rc_ok || !rg_ok));
#ifndef HOSTTEST
        if (!t_ok) { p = pstr(p, " | T "); for (int i = 0; i < 5; i++) { p = sr_hex(p, time_site_seen[i] >> 4); p = sr_hex(p, time_site_seen[i] & 15); }
            u64 dst = sr_jump_dest(time_site_seen[0] == 0xe8 || time_site_seen[0] == 0xe9 ? game + RVA_TIME_SITE : game + RVA_TIME_FN); if (dst) { p = pstr(p, ">"); p = sr_where(p, dst); } }
        if (!(ti_ok && ti_calls_ok) && ti_bad) { p = pstr(p, " | I "); p = pstr(p, ti_bad);
            if (ti_bad_rva && !(ti_bad[0] == 's' && ti_bad[1] == 'l')) { u64 dst = sr_jump_dest(game + ti_bad_rva); if (!dst) dst = sr_jump_dest(game + ti_bad_rva + 9); if (dst) { p = pstr(p, ">"); p = sr_where(p, dst); } } }
        if (!s_ok) { p = pstr(p, " | S"); u64 dst = sr_jump_dest(game + RVA_SPEED_FN); if (dst) { p = pstr(p, ">"); p = sr_where(p, dst); } }
#endif
        *p = 0; logs(c); if (bad) start_notice(c);
    }
#ifdef SEASON_MEASURE
    measure_start();
#endif
    mod_inited = 1;
    return (t_ok || s_ok) ? 1u : 0u;
}
/* ---- v26k18: self-host. The season rules no longer need the shared mod loader (its resolver knows only the rollback exe, and
   whatever else it loads is not ours to switch on). SeasonSync (the graphics tool's companion, chain-loaded by 3DMigoto when the game
   creates its Direct3D device, before any campaign runs) calls SAN14_SeasonSelfHost once: the same start as through the loader, with
   a small hook installer of our own - every patch site is compared with the bytes it must have first, all or nothing. ---- */
static i32 GAMEABI sh_readable(const void *p, UPTR n)
{
    const u8 *a = (const u8 *)p, *e = a + n; if (!p) return 0;
    while (a < e) { MBI m; if (VirtualQuery(a, &m, sizeof(m)) != sizeof(m)) return 0;
        if (m.State != 0x1000 || (m.Protect & 0x101) || !(m.Protect & 0xEE)) return 0;     /* committed, not guard / no-access, readable */
        a = (const u8 *)m.Base + m.Size; }
    return 1;
}
static i32 GAMEABI sh_install(const char *id, const S14_Hook *h, u32 n)
{
    for (u32 i = 0; i < n; i++) { if (!h[i].target || h[i].length == 0 || h[i].length > 16 || !sh_readable(h[i].target, h[i].length)) return 0;
        for (u32 k = 0; k < h[i].length; k++) if (h[i].target[k] != h[i].expected[k]) { char b[160], *p = b; p = pstr(p, "self-host: "); p = pstr(p, id); p = pstr(p, " - the game code there is not what it should be (another mod or another version): skipped"); *p = 0; logs(b); return 0; } }
    for (u32 i = 0; i < n; i++) { DWORD old = 0; if (!VirtualProtect(h[i].target, h[i].length, 0x40, &old)) return 0;
        for (u32 k = 1; k < h[i].length; k++) h[i].target[k] = h[i].replacement[k];   /* the first byte last */
        h[i].target[0] = h[i].replacement[0];
        VirtualProtect(h[i].target, h[i].length, old, &old); FlushInstructionCache(GetCurrentProcess(), h[i].target, h[i].length); }
    { char b[120], *p = b; p = pstr(p, "self-host: "); p = pstr(p, id); p = pstr(p, " installed"); *p = 0; logs(b); }
    return 1;
}
static u8 *sh_page; static u32 sh_used;
static u8 *GAMEABI sh_relay(const void *near_at, const void *dest)
{
    if (!sh_page) {                          /* one page within reach of a 32-bit jump from the game code: just after the exe image */
        u8 *base = game + ((gv_image + 0xffffu) & ~0xffffu);
        for (u64 k = 0; k < 0x4000 && !sh_page; k++) { u8 *want = base + k * 0x10000; MBI m;
            if (VirtualQuery(want, &m, sizeof(m)) != sizeof(m) || m.State != 0x10000) continue;   /* free */
            sh_page = (u8 *)VirtualAlloc(want, 0x1000, 0x3000, 0x40); }
        if (!sh_page) return 0; }
    if (sh_used + 16 > 0x1000) return 0;
    i64 dl = sh_page - ((const u8 *)near_at + 5); if (dl < -0x7fff0000LL || dl > 0x7fff0000LL) return 0;
    u8 *r = sh_page + sh_used; sh_used += 16;
    r[0] = 0xFF; r[1] = 0x25; *(u32 *)(r + 2) = 0; *(u64 *)(r + 6) = (u64)dest; r[14] = 0xCC; r[15] = 0xCC;   /* jmp [rip+0]; dest */
    FlushInstructionCache(GetCurrentProcess(), r, 16);
    return r;
}
static void GAMEABI sh_free(void *p) { (void)p; }
static void GAMEABI sh_log(const char *s) { (void)s; }
static u32 selfhost_at(u8 *img)
{
    static S14_Resolved res; static S14_CoreAPI capi; static volatile long once;
    if (__sync_lock_test_and_set(&once, 1)) return mod_inited;
    if (!img) return 0;
    u32 pe = *(u32 *)(img + 0x3c); if (pe > 0x300 || *(u32 *)(img + pe) != 0x4550) return 0;
    res.size = sizeof(res); res.abi = 0x00010000; res.image = img; res.image_size = *(u32 *)(img + pe + 0x50);
    capi.size = sizeof(capi); capi.abi = 0x00010000; capi.resolved = &res;
    capi.Readable = sh_readable; capi.InstallHooks = sh_install; capi.MakeRelay = sh_relay; capi.FreeRelay = sh_free; capi.Log = sh_log;
    game = img;                                 /* for the relay page; SAN14_ModInitV1 sets it again */
    return SAN14_ModInitV1(&capi);
}
EXPORT u32 SAN14_SeasonSelfHost(void) { return selfhost_at((u8 *)GetModuleHandleW(0)); }
#ifdef SELFHOST_TEST
EXPORT u32 SAN14_SeasonSelfHostAt(u8 *img) { return selfhost_at(img); }   /* test only: a copy of the game exe mapped by the test program */
#endif
BOOL WINAPI DllMain(void *h, DWORD reason, void *r) { (void)r; if (reason == 1) self_module = h; return 1; }
