/* SAN14 SeasonSync v8 core.
 * Chain-loaded by 3DMigoto (proxy_d3d11). A low-priority thread finds the
 * in-game month and taps F13..F24 (month 1..12) so d3dx.ini can set $month.
 * Read-only: memory is only read with ReadProcessMemory (never faults).
 * Detection, in order of trust:
 *   1. text "217년 12월 상순" (UTF-16 / UTF-8 / CP949 / Japanese 年月旬)
 *   2. learned integer date fields (y,m pairs that advance like a calendar)
 *   3. text "12월 상순" without year (most frequent)
 */
typedef unsigned long long u64;
typedef long long i64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int BOOL;
typedef void *HANDLE;
typedef unsigned long DWORD;
typedef u64 SIZE_T;
#define WINAPI __stdcall
#define IMP __declspec(dllimport)

typedef struct {
    void *BaseAddress; void *AllocationBase; DWORD AllocationProtect; u16 PartitionId;
    SIZE_T RegionSize; DWORD State; DWORD Protect; DWORD Type;
} MBI;
typedef struct { u16 wVk, wScan; DWORD dwFlags, time; u64 dwExtraInfo; } KEYBDINPUT;
typedef struct { DWORD type; DWORD pad; KEYBDINPUT ki; u8 pad2[8]; } INPUT;
typedef struct { u16 y, mo, dow, d, h, mi, s, ms; } SYSTEMTIME;

IMP HANDLE WINAPI CreateThread(void *, SIZE_T, DWORD (WINAPI *)(void *), void *, DWORD, DWORD *);
IMP void WINAPI Sleep(DWORD);
IMP SIZE_T WINAPI VirtualQuery(const void *, MBI *, SIZE_T);
IMP void *WINAPI VirtualAlloc(void *, SIZE_T, DWORD, DWORD);
IMP BOOL WINAPI ReadProcessMemory(HANDLE, const void *, void *, SIZE_T, SIZE_T *);
IMP HANDLE WINAPI GetCurrentProcess(void);
IMP DWORD WINAPI GetCurrentProcessId(void);
IMP BOOL WINAPI DisableThreadLibraryCalls(void *);
IMP DWORD WINAPI GetModuleFileNameW(void *, u16 *, DWORD);
IMP HANDLE WINAPI CreateFileW(const u16 *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
IMP BOOL WINAPI CloseHandle(HANDLE);
IMP DWORD WINAPI SetFilePointer(HANDLE, long, long *, DWORD);
IMP u64 WINAPI GetTickCount64(void);
IMP BOOL WINAPI SetThreadPriority(HANDLE, int);
IMP HANDLE WINAPI GetCurrentThread(void);
IMP void WINAPI GetLocalTime(SYSTEMTIME *);
IMP void *WINAPI GetForegroundWindow(void);
IMP DWORD WINAPI GetWindowThreadProcessId(void *, DWORD *);
IMP u32 WINAPI SendInput(u32, INPUT *, int);
IMP HANDLE WINAPI CreateMutexW(void *, BOOL, const u16 *);
IMP DWORD WINAPI WaitForSingleObject(HANDLE, DWORD);
IMP BOOL WINAPI ReleaseMutex(HANDLE);
/* v13: one shared lock with the SeasonRules module so month/turn keys and disaster-data keys never interleave */
static void rules_once(const char *why);
static HANDLE g_keymx;
static void keylock(void) { if (!g_keymx) g_keymx = CreateMutexW(0, 0, L"SAN14CE_SeasonKeys"); if (g_keymx) WaitForSingleObject(g_keymx, 15000); }
static void keyunlock(void) { if (g_keymx) ReleaseMutex(g_keymx); }
IMP void *WINAPI LoadLibraryW(const u16 *);
IMP void *WINAPI GetProcAddress(void *, const char *);
IMP u32 WINAPI GetSystemDirectoryW(u16 *, u32);
IMP BOOL WINAPI ReadFile(HANDLE, void *, DWORD, DWORD *, void *);
IMP void *WINAPI GetModuleHandleW(const u16 *);
IMP DWORD WINAPI GetFileAttributesW(const u16 *);

void *memset(void *d, int c, SIZE_T n) { u8 *p = (u8 *)d; while (n--) *p++ = (u8)c; return d; }
void *memcpy(void *d, const void *s, SIZE_T n) { u8 *p = (u8 *)d; const u8 *q = (const u8 *)s; while (n--) *p++ = *q++; return d; }
int _fltused = 0;

static u16 g_logpath[600];
static u16 g_userini[600];
static int g_lognum = 0;
static void lograw(const char *s, int n)
{
    if (g_lognum > 600) return;
    g_lognum++;
    HANDLE h = CreateFileW(g_logpath, 4, 3, 0, 4, 0x80, 0);
    if (h == (HANDLE)(i64)-1) return;
    SetFilePointer(h, 0, 0, 2);
    char buf[64]; int k = 0;
    SYSTEMTIME t; GetLocalTime(&t);
    int v[3] = { t.h, t.mi, t.s };
    buf[k++] = '[';
    for (int i = 0; i < 3; i++) { buf[k++] = '0' + v[i] / 10; buf[k++] = '0' + v[i] % 10; if (i < 2) buf[k++] = ':'; }
    buf[k++] = ']'; buf[k++] = ' ';
    DWORD w; WriteFile(h, buf, k, &w, 0); WriteFile(h, s, n, &w, 0); WriteFile(h, "\r\n", 2, &w, 0);
    CloseHandle(h);
}
static void logline(const char *s) { int n = 0; while (s[n]) n++; lograw(s, n); }
static char *put_int(char *p, i64 v) { char t[24]; int n = 0; if (v < 0) { *p++ = '-'; v = -v; } do { t[n++] = '0' + v % 10; v /= 10; } while (v); while (n) *p++ = t[--n]; return p; }
static char *put_str(char *p, const char *s) { while (*s) *p++ = *s++; return p; }
static char *put_hex(char *p, u64 v) { const char *h = "0123456789abcdef"; for (int i = 44; i >= 0; i -= 4) *p++ = h[(v >> i) & 15]; return p; }
static char *put_u16_utf8(char *p, const u16 *s, int n)
{
    for (int i = 0; i < n; i++) {
        u32 c = s[i];
        if (c < 0x20) c = '.';
        if (c < 0x80) *p++ = (char)c;
        else if (c < 0x800) { *p++ = (char)(0xC0 | (c >> 6)); *p++ = (char)(0x80 | (c & 63)); }
        else { *p++ = (char)(0xE0 | (c >> 12)); *p++ = (char)(0x80 | ((c >> 6) & 63)); *p++ = (char)(0x80 | (c & 63)); }
    }
    return p;
}

/* ---------------- text patterns ---------------- */
static int isdig(int c) { return c >= '0' && c <= '9'; }
static int isdigw(u32 c) { return (c >= '0' && c <= '9') || (c >= 0xFF10 && c <= 0xFF19); }
static int digv(u32 c) { return c >= 0xFF10 ? (int)(c - 0xFF10) : (int)(c - '0'); }
static int issepw(u32 c) { return c == 0x20 || c == 0xA0 || c == 0x3000 || (c >= 0x2000 && c <= 0x200B) || c == 0x09; }

/* UTF-16 at index i = '월'(0xC6D4) or '月'(0x6708). returns month in *mo, year (0 if none) in *yr, xun (-1 if none) */
static int parse16(const u16 *b, int i, int n, int *yr, int *mo, int *xu)
{
    int jp = b[i] == 0x6708;
    int j = i - 1;
    while (j >= 0 && issepw(b[j]) && i - j < 3) j--;
    int m = 0, mul = 1, nd = 0;
    while (j >= 0 && isdigw(b[j]) && nd < 2) { m += digv(b[j]) * mul; mul *= 10; j--; nd++; }
    if (!nd || m < 1 || m > 12) return 0;
    if (j >= 0 && isdigw(b[j])) return 0;          /* 3+ digits: not a month */
    int y = 0;
    int jj = j; while (jj >= 0 && issepw(b[jj]) && j - jj < 3) jj--;
    if (jj >= 0 && b[jj] == (jp ? 0x5E74 : 0xB144)) { /* 년 / 年 */
        int k = jj - 1; while (k >= 0 && issepw(b[k]) && jj - k < 3) k--;
        int yy = 0; mul = 1; nd = 0;
        while (k >= 0 && isdigw(b[k]) && nd < 4) { yy += digv(b[k]) * mul; mul *= 10; k--; nd++; }
        if (nd && yy >= 100 && yy <= 600) y = yy;
    }
    int x = -1;
    int k = i + 1; while (k < n && issepw(b[k]) && k - i < 4) k++;
    if (k + 1 < n) {
        if (!jp) { int c = b[k]; int t = c == 0xC0C1 ? 0 : c == 0xC911 ? 1 : c == 0xD558 ? 2 : -1; if (t >= 0 && b[k + 1] == 0xC21C) x = t; }
        else { int c = b[k]; int t = c == 0x4E0A ? 0 : c == 0x4E2D ? 1 : c == 0x4E0B ? 2 : -1; if (t >= 0 && b[k + 1] == 0x65EC) x = t; }
    }
    *yr = y; *mo = m; *xu = x;
    return 1;
}
static const u8 U8_WOL[3] = { 0xEC, 0x9B, 0x94 }, U8_NYEON[3] = { 0xEB, 0x85, 0x84 }, U8_SANG[3] = { 0xEC, 0x83, 0x81 },
                U8_JUNG[3] = { 0xEC, 0xA4, 0x91 }, U8_HA[3] = { 0xED, 0x95, 0x98 }, U8_SUN[3] = { 0xEC, 0x88, 0x9C };
static const u8 C9_WOL[2] = { 0xBF, 0xF9 }, C9_NYEON[2] = { 0xB3, 0xE2 }, C9_SANG[2] = { 0xBB, 0xF3 }, C9_JUNG[2] = { 0xC1, 0xDF },
                C9_HA[2] = { 0xC7, 0xCF }, C9_SUN[2] = { 0xBC, 0xF8 };
static int eqn(const u8 *a, const u8 *b, int n) { for (int i = 0; i < n; i++) if (a[i] != b[i]) return 0; return 1; }
static int parse8(const u8 *b, int i, int n, int enc, int *yr, int *mo, int *xu)
{
    int L = enc == 1 ? 3 : 2;
    const u8 *NY = enc == 1 ? U8_NYEON : C9_NYEON;
    int j = i - 1;
    if (j >= 0 && b[j] == 0x20) j--;
    int m = 0, mul = 1, nd = 0;
    while (j >= 0 && isdig(b[j]) && nd < 2) { m += (b[j] - '0') * mul; mul *= 10; j--; nd++; }
    if (!nd || m < 1 || m > 12) return 0;
    if (j >= 0 && isdig(b[j])) return 0;
    int y = 0;
    int jj = j; if (jj >= 0 && b[jj] == 0x20) jj--;
    if (jj - L + 1 >= 0 && eqn(b + jj - L + 1, NY, L)) {
        int k = jj - L; if (k >= 0 && b[k] == 0x20) k--;
        int yy = 0; mul = 1; nd = 0;
        while (k >= 0 && isdig(b[k]) && nd < 4) { yy += (b[k] - '0') * mul; mul *= 10; k--; nd++; }
        if (nd && yy >= 100 && yy <= 600) y = yy;
    }
    int x = -1;
    int k = i + L; if (k < n && b[k] == 0x20) k++;
    if (k + 2 * L <= n) {
        int t = -1;
        if (eqn(b + k, enc == 1 ? U8_SANG : C9_SANG, L)) t = 0;
        else if (eqn(b + k, enc == 1 ? U8_JUNG : C9_JUNG, L)) t = 1;
        else if (eqn(b + k, enc == 1 ? U8_HA : C9_HA, L)) t = 2;
        if (t >= 0 && eqn(b + k + L, enc == 1 ? U8_SUN : C9_SUN, L)) x = t;
    }
    *yr = y; *mo = m; *xu = x;
    return 1;
}

static void add_year(int y);
static void add_td(int y, int m);
/* per-scan statistics */
static long st_full, st_mx, st_mo;           /* full dates, month+xun, month only */
static int best_full;                         /* best y*36+(m-1)*3+x */
static long mx_count[13];                     /* month+xun hits per month */
static int g_samples;
static void sample16(const u16 *b, int i, int n, u64 addr)
{
    if (g_samples >= 25) return;
    g_samples++;
    int a = i - 10; if (a < 0) a = 0;
    int e = i + 8; if (e > n) e = n;
    char buf[300], *p = buf;
    p = put_str(p, "  utf16 @"); p = put_hex(p, addr); p = put_str(p, " : ");
    p = put_u16_utf8(p, b + a, e - a);
    lograw(buf, (int)(p - buf));
}
static int g_sampling;
#define MAXH 8192
static u64 cur_a[MAXH]; static u8 cur_m[MAXH]; static int n_cur;
#define HT 32768
static u64 prev_a[HT]; static u8 prev_m[HT]; static int have_prev;
static int ht_find(u64 a) { u32 h = (u32)((a >> 1) * 2654435761u) & (HT - 1); for (int k = 0; k < HT; k++) { u32 i = (h + k) & (HT - 1); if (prev_a[i] == a) return (int)i; if (!prev_a[i]) return -1; } return -1; }
static void ht_put(u64 a, u8 m) { u32 h = (u32)((a >> 1) * 2654435761u) & (HT - 1); for (int k = 0; k < HT; k++) { u32 i = (h + k) & (HT - 1); if (!prev_a[i] || prev_a[i] == a) { prev_a[i] = a; prev_m[i] = m; return; } } }
static u64 g_note_addr;
static void note(int y, int m, int x)
{
    if (y && x >= 0) { int v = y * 36 + (m - 1) * 3 + x; st_full++; if (v > best_full) best_full = v; add_year(y); add_td(y, m); }
    else if (x >= 0) { st_mx++; mx_count[m]++; if (n_cur < MAXH) { cur_a[n_cur] = g_note_addr; cur_m[n_cur] = (u8)m; n_cur++; } }
    else st_mo++;
}
static void scan_text(const u8 *buf, SIZE_T len, u64 base)
{
    const u16 *w = (const u16 *)buf; int nw = (int)(len / 2);
    for (int i = 4; i < nw - 2; i++) {
        u16 c = w[i];
        if (c == 0xC6D4 || c == 0x6708) {
            int y, m, x;
            if (parse16(w, i, nw, &y, &m, &x)) { g_note_addr = base + 2 * (u64)i; note(y, m, x); if (g_sampling && (x >= 0 || y)) sample16(w, i, nw, base + 2 * (u64)i); }
        }
    }
    for (SIZE_T i = 6; i + 8 < len; i++) {
        u8 c = buf[i];
        int y, m, x;
        if (c == 0xEC && buf[i + 1] == 0x9B && buf[i + 2] == 0x94) { if (parse8(buf, (int)i, (int)len, 1, &y, &m, &x)) { g_note_addr = base + i; note(y, m, x); } }
        else if (c == 0xBF && buf[i + 1] == 0xF9) { if (parse8(buf, (int)i, (int)len, 2, &y, &m, &x)) { g_note_addr = base + i; note(y, m, x); } }
    }
}


static int rd(u64 a, void *d, SIZE_T n)
{
    SIZE_T got = 0;
    return ReadProcessMemory(GetCurrentProcess(), (const void *)a, d, n, &got) && got == n;
}
#include "learn.h"

/* ---------------- memory walk ---------------- */
#define CHUNK (1u << 20)
static u8 *g_buf;
static u64 g_bytes, g_regions;
static int region_ok(MBI *mbi)
{
    DWORD pr = mbi->Protect;
    int rw = (pr & (0x04 | 0x08 | 0x40 | 0x80)) && !(pr & 0x100) && !(pr & 0x01);
    return mbi->State == 0x1000 && rw && mbi->RegionSize <= ((SIZE_T)768 << 20);
}
static void walk(int want_text, int want_ints)
{
    HANDLE proc = GetCurrentProcess();
    u8 *addr = (u8 *)0x10000;
    MBI mbi;
    while ((u64)addr < 0x7FFFFFFF0000ULL && VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        u8 *base = (u8 *)mbi.BaseAddress; SIZE_T size = mbi.RegionSize;
        if (size == 0) break;
        int ok = region_ok(&mbi);
        if (ok) g_regions++;
        if (ok && !(g_buf >= base && g_buf < base + size) && !((u8 *)g_c >= base && (u8 *)g_c < base + size)) {
            for (SIZE_T off = 0; off < size; off += CHUNK) {
                SIZE_T want = size - off; if (want > CHUNK + 64) want = CHUNK + 64;
                SIZE_T got = 0;
                if (!ReadProcessMemory(proc, base + off, g_buf, want, &got) || got < 16) continue;
                g_bytes += got;
                if (want_text) scan_text(g_buf, got, (u64)(base + off));
                if (want_ints) scan_ints(g_buf, got > CHUNK ? CHUNK : got, (u64)(base + off));
            }
        }
        addr = base + size;
    }
}
/* only region sizes, no reading: cheap */
static u64 commit_total(void)
{
    u64 sum = 0; u8 *addr = (u8 *)0x10000; MBI mbi;
    while ((u64)addr < 0x7FFFFFFF0000ULL && VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.RegionSize == 0) break;
        if (region_ok(&mbi)) sum += mbi.RegionSize;
        addr = (u8 *)mbi.BaseAddress + mbi.RegionSize;
    }
    return sum;
}
static void platform_build_walk(void)
{
    st_full = st_mx = st_mo = 0; best_full = 0; n_cur = 0; for (int i = 0; i < 13; i++) mx_count[i] = 0;
    g_sampling = 0; g_bytes = 0; g_regions = 0;
    walk(1, 1);
}


/* ---------------- direct date reading (v8) ----------------
   The game keeps the current date in one place: [[exe+G] + O] + 0x34 :
   int16 year, int8 month (1..12), int8 day (1..30). G and O are taken from the game's own
   code (the event condition "game date"), found by its byte pattern, so a different
   exe build still works if that code is unchanged.
     48 8B 05 <G>   48 8D 4C 24 60   48 8B 90 <O>   48 83 C2 34 */
static u64 g_dglobal; static u32 g_doff; static int g_direct;
static int find_direct(void)
{
    u8 *base = (u8 *)GetModuleHandleW(0);
    static u8 hdr[0x1000];
    if (!base || !rd((u64)base, hdr, sizeof(hdr))) return 0;
    u32 pe = *(u32 *)(hdr + 0x3c); if (pe > 0xE00) return 0;
    int nsec = *(u16 *)(hdr + pe + 6), opt = *(u16 *)(hdr + pe + 20);
    for (int i = 0; i < nsec && i < 16; i++) {
        u8 *sh = hdr + pe + 24 + opt + 40 * i;
        if (sh + 40 > hdr + sizeof(hdr)) break;
        u32 va = *(u32 *)(sh + 12), vs = *(u32 *)(sh + 8), ch = *(u32 *)(sh + 36);
        if (!(ch & 0x20000000)) continue;            /* executable sections only */
        for (u32 off = 0; off < vs; off += CHUNK) {
            SIZE_T want = vs - off; if (want > CHUNK + 32) want = CHUNK + 32;
            SIZE_T got = 0;
            if (!ReadProcessMemory(GetCurrentProcess(), base + va + off, g_buf, want, &got) || got < 24) continue;
            for (SIZE_T k = 0; k + 24 <= got && k < CHUNK; k++) {
                const u8 *p = g_buf + k;
                if (p[0] == 0x48 && p[1] == 0x8B && p[2] == 0x05 && p[7] == 0x48 && p[8] == 0x8D && p[9] == 0x4C && p[10] == 0x24 && p[11] == 0x60 &&
                    p[12] == 0x48 && p[13] == 0x8B && p[14] == 0x90 && p[19] == 0x48 && p[20] == 0x83 && p[21] == 0xC2 && p[22] == 0x34) {
                    int disp = *(const int *)(p + 3);
                    g_dglobal = (u64)(base + va + off + k + 7) + (i64)disp;
                    g_doff = *(const u32 *)(p + 15);
                    return 1;
                }
            }
        }
    }
    return 0;
}
/* 1 = valid date read */
static int read_direct(int *y, int *m, int *d)
{
    u64 p1 = 0, p2 = 0; u8 t[4];
    if (!rd(g_dglobal, &p1, 8) || !p1) return 0;
    if (!rd(p1 + g_doff, &p2, 8) || !p2) return 0;
    if (!rd(p2 + 0x34, t, 4)) return 0;
    *y = *(short *)t; *m = (signed char)t[2]; *d = (signed char)t[3];
    return *y >= 100 && *y <= 999 && *m >= 1 && *m <= 12 && *d >= 1 && *d <= 31;
}

/* month numbering learned in an earlier session (SeasonSync_numbering.txt: "kind zb") */
static u16 g_prefpath[600];
static void load_pref(void)
{
    HANDLE h = CreateFileW(g_prefpath, 0x80000000, 3, 0, 3, 0x80, 0);
    if (h == (HANDLE)(i64)-1) return;
    char b[32]; DWORD n = 0; ReadFile(h, b, 31, &n, 0); CloseHandle(h); b[n] = 0;
    if (n >= 3 && b[0] >= '0' && b[0] <= '4' && b[1] == ' ' && (b[2] == '0' || b[2] == '1')) { g_pref_kind = b[0] - '0'; g_pref_zb = b[2] - '0'; }
}
static void save_pref(int kind, int zb)
{
    HANDLE h = CreateFileW(g_prefpath, 0x40000000, 1, 0, 2 /*CREATE_ALWAYS*/, 0x80, 0);
    if (h == (HANDLE)(i64)-1) return;
    char b[4] = { (char)('0' + kind), ' ', (char)('0' + zb), '\n' }; DWORD w;
    WriteFile(h, b, 4, &w, 0); CloseHandle(h);
}

/* month that 3DMigoto remembered from last time ($month in d3dx_user.ini), 0 if none */
static int persisted_month(void)
{
    HANDLE h = CreateFileW(g_userini, 0x80000000 /*GENERIC_READ*/, 3, 0, 3 /*OPEN_EXISTING*/, 0x80, 0);
    if (h == (HANDLE)(i64)-1) return 0;
    char buf[2048]; DWORD n = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &n, 0); CloseHandle(h);
    buf[n] = 0;
    for (DWORD i = 0; i + 5 < n; i++) {
        if (buf[i] == 'm' && buf[i + 1] == 'o' && buf[i + 2] == 'n' && buf[i + 3] == 't' && buf[i + 4] == 'h') {
            DWORD k = i + 5; while (k < n && (buf[k] == ' ' || buf[k] == '=')) k++;
            int v = 0, nd = 0; while (k < n && buf[k] >= '0' && buf[k] <= '9' && nd < 3) { v = v * 10 + buf[k] - '0'; k++; nd++; }
            if (nd && v >= 1 && v <= 12) return v;
        }
    }
    return 0;
}

static void send_vk(u16 vk)
{
    INPUT in[2];
    memset(in, 0, sizeof(in));
    in[0].type = 1; in[0].ki.wVk = vk;
    in[1].type = 1; in[1].ki.wVk = vk; in[1].ki.dwFlags = 2;
    keylock();
    SendInput(1, &in[0], sizeof(INPUT));
    Sleep(180);
    SendInput(1, &in[1], sizeof(INPUT));
    Sleep(60);
    keyunlock();
}
/* v9: turn within the month (0 = first ten days, 1 = middle, 2 = last) -> ATTN / CRSEL / EXSEL */
static void send_turn(int x) { send_vk((u16)(0xF6 + x)); }
static void send_month(int m) { send_vk((u16)(0x7C + m - 1)); }
static int game_foreground(void)
{
    DWORD pid = 0; void *w = GetForegroundWindow();
    if (!w) return 0;
    GetWindowThreadProcessId(w, &pid);
    return pid == GetCurrentProcessId();
}


/* ---------------- v9: one-time memory survey for the test build ----------------
   Runs only when SeasonSync_survey.flag exists in the game folder (the test kit creates it) and a save is loaded.
   Reads memory only (never writes to the game). Output: SeasonSync_survey.bin, then SeasonSync_survey.done */
static u16 g_flagpath[600], g_surveypath[600], g_donepath[600];
static HANDLE g_sf; static u64 g_sbytes; static int g_sblocks;
static void sw(const void *p, DWORD n) { DWORD w = 0; WriteFile(g_sf, p, n, &w, 0); g_sbytes += n; }
static int mem_ok(u64 a, MBI *m)
{
    if (VirtualQuery((void *)a, m, sizeof(*m)) != sizeof(*m)) return 0;
    DWORD pr = m->Protect;
    return m->State == 0x1000 && (pr & (0x02 | 0x04 | 0x08 | 0x20 | 0x40 | 0x80)) && !(pr & 0x100) && !(pr & 0x01);
}
/* record: "BLK1" u32 kind u64 addr u32 len, data */
static void dump_block(u64 addr, u32 len, u32 kind)
{
    MBI m; if (!mem_ok(addr, &m)) return;
    u64 end = (u64)m.BaseAddress + m.RegionSize;
    if (addr + len > end) len = (u32)(end - addr);
    if (!len || g_sbytes > ((u64)200 << 20)) return;
    u8 hdr[20] = { 'B', 'L', 'K', '1' };
    *(u32 *)(hdr + 4) = kind; *(u64 *)(hdr + 8) = addr; *(u32 *)(hdr + 16) = len;
    sw(hdr, 20);
    u32 done = 0;
    while (done < len) {
        u32 n = len - done; if (n > CHUNK) n = CHUNK;
        SIZE_T got = 0;
        if (!ReadProcessMemory(GetCurrentProcess(), (void *)(addr + done), g_buf, n, &got) || got < n) { memset(g_buf + got, 0, n - got); }
        sw(g_buf, n); done += n;
    }
    g_sblocks++;
}
static int heap_ptr(u64 v)
{
    if (v < 0x10000 || v > 0x7FFFFFFF0000ULL || (v & 7)) return 0;
    MBI m; if (!mem_ok(v, &m)) return 0;
    return m.Type == 0x20000;   /* MEM_PRIVATE */
}
static u64 g_ptrs[2048]; static int g_np;
static void collect_ptrs(u64 base, u32 len)
{
    static u8 tmp[0x40000];
    if (len > sizeof(tmp)) len = sizeof(tmp);
    if (!rd(base, tmp, len)) return;
    for (u32 o = 0; o + 8 <= len && g_np < 2048; o += 8) {
        u64 v = *(u64 *)(tmp + o);
        if (!heap_ptr(v)) continue;
        int dup = 0; for (int i = 0; i < g_np; i++) if (g_ptrs[i] == v) { dup = 1; break; }
        if (!dup) g_ptrs[g_np++] = v;
    }
}
/* long runs of small numbers (map cell grids: terrain kind, owner, ...) for element sizes 1, 2, 4 */
static int g_ncand; static u64 g_candbytes;
static u64 g_ca[60]; static u32 g_cl[60], g_ce[60];
static void grid_scan(void)
{
    HANDLE proc = GetCurrentProcess();
    u8 *addr = (u8 *)0x10000; MBI mbi;
    while ((u64)addr < 0x7FFFFFFF0000ULL && VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi) && g_ncand < 60) {
        u8 *base = (u8 *)mbi.BaseAddress; SIZE_T size = mbi.RegionSize;
        if (size == 0) break;
        if (region_ok(&mbi) && mbi.Type == 0x20000 && size >= 40000 && !(g_buf >= base && g_buf < base + size)) {
            for (int es = 1; es <= 4 && g_ncand < 60; es *= 2) {
                u64 runStart = 0; u64 runLen = 0; u32 hist[64]; for (int i = 0; i < 64; i++) hist[i] = 0;
                for (SIZE_T off = 0; off < size; off += CHUNK) {
                    SIZE_T want = size - off; if (want > CHUNK) want = CHUNK;
                    SIZE_T got = 0;
                    if (!ReadProcessMemory(proc, base + off, g_buf, want, &got)) got = 0;
                    for (SIZE_T k = 0; k + es <= got; k += es) {
                        u32 v = es == 1 ? g_buf[k] : es == 2 ? *(u16 *)(g_buf + k) : *(u32 *)(g_buf + k);
                        int small = v < 64;
                        if (small) { if (!runLen) runStart = (u64)(base + off + k); runLen++; hist[v]++; }
                        if (!small || (off + k + es >= size)) {
                            if (runLen >= 40000) {
                                int distinct = 0; for (int i = 0; i < 64; i++) if (hist[i]) distinct++;
                                if (distinct >= 4 && hist[0] < runLen * 95 / 100 && g_ncand < 60 && g_candbytes < ((u64)60 << 20)) {
                                    u32 len = (u32)(runLen * es); if (len > 400000) len = 400000;
                                    g_ca[g_ncand] = runStart; g_cl[g_ncand] = len; g_ce[g_ncand] = (u32)es; g_ncand++; g_candbytes += len;
                                }
                            }
                            runLen = 0; for (int i = 0; i < 64; i++) hist[i] = 0;
                        }
                    }
                }
            }
        }
        addr = base + size;
    }
}

/* ---------------- survey v10 ---------------- */
static u64 g_selfbase;
static int skip_region(MBI *m)
{
    if ((u64)m->AllocationBase == g_selfbase) return 1;
    if (g_buf >= (u8 *)m->BaseAddress && g_buf < (u8 *)m->BaseAddress + m->RegionSize) return 1;
    return 0;
}
#define MAXT 4096
static u64 t_lo[MAXT], t_hi[MAXT]; static u32 t_id[MAXT]; static u16 t_cnt[MAXT]; static int g_nt;
static void add_target(u64 lo, u64 hi, u32 id) { if (g_nt < MAXT) { t_lo[g_nt] = lo; t_hi[g_nt] = hi; t_id[g_nt] = id; t_cnt[g_nt] = 0; g_nt++; } }
static void sort_targets(void)
{
    for (int i = 1; i < g_nt; i++) { u64 a = t_lo[i], b = t_hi[i]; u32 c = t_id[i]; int j = i - 1;
        while (j >= 0 && t_lo[j] > a) { t_lo[j + 1] = t_lo[j]; t_hi[j + 1] = t_hi[j]; t_id[j + 1] = t_id[j]; j--; }
        t_lo[j + 1] = a; t_hi[j + 1] = b; t_id[j + 1] = c; }
    for (int i = 0; i < g_nt; i++) t_cnt[i] = 0;
}
#define MAXH2 6000
static u64 h_a[MAXH2], h_v[MAXH2]; static u32 h_t[MAXH2]; static int g_nh;
/* one pass over all readable writable memory: qwords that point into any target range */
static void ptr_pass(u32 tag)
{
    if (!g_nt) return;
    u64 mn = t_lo[0], mx = 0; for (int i = 0; i < g_nt; i++) if (t_hi[i] > mx) mx = t_hi[i];
    HANDLE proc = GetCurrentProcess(); u8 *addr = (u8 *)0x10000; MBI mbi; g_nh = 0;
    while ((u64)addr < 0x7FFFFFFF0000ULL && VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        u8 *base = (u8 *)mbi.BaseAddress; SIZE_T size = mbi.RegionSize; if (size == 0) break;
        if (region_ok(&mbi) && !skip_region(&mbi)) {
            for (SIZE_T off = 0; off < size; off += CHUNK) {
                SIZE_T want = size - off; if (want > CHUNK) want = CHUNK; SIZE_T got = 0;
                if (!ReadProcessMemory(proc, base + off, g_buf, want, &got)) continue;
                u64 *q = (u64 *)g_buf; SIZE_T nq = got / 8;
                for (SIZE_T k = 0; k < nq; k++) {
                    u64 v = q[k]; if (v < mn || v >= mx) continue;
                    int lo = 0, hi = g_nt - 1, f = -1;
                    while (lo <= hi) { int mid = (lo + hi) >> 1; if (t_lo[mid] <= v) { f = mid; lo = mid + 1; } else hi = mid - 1; }
                    for (int j = f; j >= 0 && j > f - 8; j--) {
                        if (v >= t_lo[j] && v < t_hi[j]) {
                            if (t_cnt[j] < 48 && g_nh < MAXH2) { t_cnt[j]++; h_a[g_nh] = (u64)(base + off) + k * 8; h_v[g_nh] = v; h_t[g_nh] = t_id[j]; g_nh++;
                                u8 r[24] = { 'P', 'T', 'R', '0' }; r[3] = (u8)tag; *(u32 *)(r + 4) = t_id[j]; *(u64 *)(r + 8) = h_a[g_nh - 1]; *(u64 *)(r + 16) = v; sw(r, 24); }
                            break;
                        }
                    }
                }
            }
        }
        addr = base + size;
    }
}
/* strings (city names) and number pairs (faction gold / food from the test save) */
static const u8 *s_pat[16]; static int s_len[16], s_n, s_cnt[16];
static u64 s_hit[16][24];
static u8 P16[8][4], P8[8][6];
static void add_name(u16 a, u16 b)
{
    u8 *p = P16[s_n / 2]; p[0] = (u8)a; p[1] = (u8)(a >> 8); p[2] = (u8)b; p[3] = (u8)(b >> 8);
    s_pat[s_n] = p; s_len[s_n] = 4; s_n++;
    u8 *q = P8[s_n / 2]; u16 c[2] = { a, b }; for (int i = 0; i < 2; i++) { q[i * 3] = (u8)(0xE0 | (c[i] >> 12)); q[i * 3 + 1] = (u8)(0x80 | ((c[i] >> 6) & 63)); q[i * 3 + 2] = (u8)(0x80 | (c[i] & 63)); }
    s_pat[s_n] = q; s_len[s_n] = 6; s_n++;
}
static u32 n_pair[4][2] = { { 56854, 313531 }, { 238833, 1110340 }, { 119101, 313531 }, { 355310, 1110340 } };
static u64 n_hit[4][16]; static int n_cnt[4];
static u8 s_first[256];
static void find_pass(void)
{
    for (int i = 0; i < s_n; i++) s_first[s_pat[i][0]] = 1;
    HANDLE proc = GetCurrentProcess(); u8 *addr = (u8 *)0x10000; MBI mbi;
    while ((u64)addr < 0x7FFFFFFF0000ULL && VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        u8 *base = (u8 *)mbi.BaseAddress; SIZE_T size = mbi.RegionSize; if (size == 0) break;
        if (region_ok(&mbi) && !skip_region(&mbi)) {
            for (SIZE_T off = 0; off < size; off += CHUNK) {
                SIZE_T want = size - off; if (want > CHUNK + 64) want = CHUNK + 64; SIZE_T got = 0;
                if (!ReadProcessMemory(proc, base + off, g_buf, want, &got) || got < 16) continue;
                SIZE_T lim = got > CHUNK ? CHUNK : got;
                for (SIZE_T k = 0; k + 8 <= lim; k++) {
                    u8 c0 = g_buf[k];
                    if (s_first[c0]) for (int i = 0; i < s_n; i++) {
                        if (s_pat[i][0] != c0 || s_cnt[i] >= 24 || k + s_len[i] > got) continue;
                        int ok = 1; for (int j = 1; j < s_len[i]; j++) if (g_buf[k + j] != s_pat[i][j]) { ok = 0; break; }
                        if (ok) s_hit[i][s_cnt[i]++] = (u64)(base + off + k);
                    }
                    if (!(k & 3) && k + 4 <= lim) {
                        u32 v = *(u32 *)(g_buf + k);
                        for (int i = 0; i < 4; i++) if (v == n_pair[i][0] && n_cnt[i] < 16) {
                            for (SIZE_T j = k + 4; j < k + 96 && j + 4 <= got; j += 4) if (*(u32 *)(g_buf + j) == n_pair[i][1]) { n_hit[i][n_cnt[i]++] = (u64)(base + off + k); break; }
                        }
                    }
                }
            }
        }
        addr = base + size;
    }
}
static void dump_region_of(u64 a, u32 kind, u32 cap)
{
    MBI m; if (VirtualQuery((void *)a, &m, sizeof(m)) != sizeof(m)) return;
    u64 b = (u64)m.BaseAddress; u32 n = m.RegionSize > cap ? cap : (u32)m.RegionSize;
    if (a - b > n / 2 && m.RegionSize > cap) b = a - n / 2;
    dump_block(b, n, kind);
    MBI p; if (VirtualQuery((void *)(b - 1), &p, sizeof(p)) == sizeof(p) && mem_ok(b - 0x10000, &p)) dump_block(b - 0x10000, 0x10000, kind | 0x80);
}
/* v12: snapshot of the game's data: the main game record and every memory page it points to (two levels) */
#define MAXPG 12000
static u64 pg[MAXPG]; static int npg;
static int pg_has(u64 p) { int lo = 0, hi = npg - 1; while (lo <= hi) { int m = (lo + hi) >> 1; if (pg[m] == p) return 1; if (pg[m] < p) lo = m + 1; else hi = m - 1; } return 0; }
static void pg_sort(void) { for (int i = 1; i < npg; i++) { u64 v = pg[i]; int j = i - 1; while (j >= 0 && pg[j] > v) { pg[j + 1] = pg[j]; j--; } pg[j + 1] = v; } }
static u64 newp[MAXPG]; static int nnew;
static void scan_ptrs(const u8 *buf, u32 len)
{
    for (u32 o = 0; o + 8 <= len; o += 8) {
        u64 v = *(const u64 *)(buf + o);
        if (v < 0x10000 || v > 0x7FFFFFFF0000ULL) continue;
        u64 p = v & ~0xFFFULL;
        if (pg_has(p)) continue;
        int dup = 0; for (int i = nnew - 1; i >= 0 && i > nnew - 64; i--) if (newp[i] == p) { dup = 1; break; }
        if (dup || nnew >= MAXPG) continue;
        MBI m; if (!mem_ok(p, &m) || m.Type != 0x20000) continue;
        newp[nnew++] = p;
    }
}
static void run_imagedump(void)
{
    g_sf = CreateFileW(g_surveypath, 0x40000000, 1, 0, 2, 0x80, 0);
    if (g_sf == (HANDLE)(i64)-1) return;
    logline("survey v12 (game data snapshot): start");
    u8 *exe = (u8 *)GetModuleHandleW(0);
    u64 G = 0; rd(g_dglobal, &G, 8);
    {
        u8 h[36] = { 'R', 'O', 'O', 'T' }; u64 p2 = 0; if (G) rd(G + g_doff, &p2, 8);
        *(u64 *)(h + 4) = (u64)exe; *(u64 *)(h + 12) = g_dglobal - (u64)exe; *(u32 *)(h + 20) = g_doff; *(u64 *)(h + 24) = G; *(u32 *)(h + 32) = (u32)(p2 & 0xffffffff);
        sw(h, 36); sw(&p2, 8);
    }
    if (G) {
        static u8 gb[0x100000];
        u32 glen = 0x100000; MBI m; if (VirtualQuery((void *)G, &m, sizeof(m))) { u64 end = (u64)m.BaseAddress + m.RegionSize; if (G + glen > end) glen = (u32)(end - G); }
        if (!rd(G, gb, glen)) { glen = 0x80000; rd(G, gb, glen); }
        dump_block(G, glen, 1);
        npg = 0; nnew = 0; scan_ptrs(gb, glen);
        for (int lev = 1; lev <= 2 && nnew; lev++) {
            int start = npg;
            for (int i = 1; i < nnew; i++) { u64 v = newp[i]; int j = i - 1; while (j >= 0 && newp[j] > v) { newp[j + 1] = newp[j]; j--; } newp[j + 1] = v; }
            for (int i = 0; i < nnew && npg < MAXPG; i++) if (i == 0 || newp[i] != newp[i - 1]) pg[npg++] = newp[i];
            int end = npg; nnew = 0;
            for (int i = start; i < end; i++) { dump_block(pg[i], 0x1000, 0x70 + lev); }
            pg_sort();
            if (lev == 1) {
                static u8 tmp[0x1000];
                for (int i = 0; i < npg; i++) if (rd(pg[i], tmp, 0x1000)) scan_ptrs(tmp, 0x1000);
            }
            { char b[80], *p = b; p = put_str(p, "survey: pages "); p = put_int(p, npg); *p = 0; logline(b); }
        }
        /* the hex records (48400 x 32 bytes) in one piece */
        u64 h0 = 0; rd(G + 0xe270, &h0, 8);
        if (h0) dump_block(h0 - 0x20, 48402 * 32, 0x80);
    }
    { char b[120], *p = b; p = put_str(p, "survey: done, blocks "); p = put_int(p, g_sblocks); p = put_str(p, ", MB "); p = put_int(p, (int)(g_sbytes >> 20)); *p = 0; logline(b); }
    CloseHandle(g_sf);
    HANDLE d = CreateFileW(g_donepath, 0x40000000, 1, 0, 2, 0x80, 0);
    if (d != (HANDLE)(i64)-1) { DWORD w; WriteFile(d, "done\r\n", 6, &w, 0); CloseHandle(d); }
}
static void run_survey(void)
{
    g_sf = CreateFileW(g_surveypath, 0x40000000, 1, 0, 2, 0x80, 0);
    if (g_sf == (HANDLE)(i64)-1) return;
    logline("survey v10: start");
    { MBI m; VirtualQuery((void *)&g_nt, &m, sizeof(m)); g_selfbase = (u64)m.AllocationBase; }
    u8 *exe = (u8 *)GetModuleHandleW(0);
    {
        u8 *addr = (u8 *)0x10000; MBI mbi; u16 path[300];
        while ((u64)addr < 0x7FFFFFFF0000ULL && VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if (mbi.RegionSize == 0) break;
            if (mbi.Type == 0x1000000 && mbi.AllocationBase == mbi.BaseAddress) {
                DWORD n = GetModuleFileNameW(mbi.AllocationBase, path, 299);
                if (n) { u8 h[20] = { 'M', 'O', 'D', '1' }; *(u32 *)(h + 4) = n * 2; *(u64 *)(h + 8) = (u64)mbi.AllocationBase; *(u32 *)(h + 16) = 0; sw(h, 20); sw(path, n * 2); }
            }
            addr = (u8 *)mbi.BaseAddress + mbi.RegionSize;
        }
    }
    u64 p1 = 0, p2 = 0;
    rd(g_dglobal, &p1, 8); if (p1) rd(p1 + g_doff, &p2, 8);
    {
        u8 h[36] = { 'R', 'O', 'O', 'T' };
        *(u64 *)(h + 4) = (u64)exe; *(u64 *)(h + 12) = g_dglobal - (u64)exe; *(u32 *)(h + 20) = g_doff; *(u64 *)(h + 24) = p1; *(u32 *)(h + 32) = (u32)(p2 & 0xffffffff);
        sw(h, 36); sw(&p2, 8);
    }
    if (p1) dump_block(p1, 0x10000, 1);
    if (p2) dump_block(p2, 0x10000, 2);
    /* the exe image: every writable piece (globals), up to 64 MB in all */
    {
        u8 *addr = exe; MBI mbi; u64 tot = 0;
        while (VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi) && mbi.AllocationBase == (void *)exe && mbi.RegionSize) {
            DWORD pr = mbi.Protect;
            if (mbi.State == 0x1000 && (pr & (0x04 | 0x08 | 0x40 | 0x80)) && !(pr & 0x100) && tot < ((u64)64 << 20)) {
                u32 n = mbi.RegionSize > (16u << 20) ? (16u << 20) : (u32)mbi.RegionSize; dump_block((u64)mbi.BaseAddress, n, 4); tot += n;
            }
            addr = (u8 *)mbi.BaseAddress + mbi.RegionSize;
        }
    }
    logline("survey: map grids");
    grid_scan();
    for (int i = 0; i < g_ncand; i++) {
        int dup = 0; MBI m; VirtualQuery((void *)g_ca[i], &m, sizeof(m));
        for (int j = 0; j < i; j++) { MBI n; VirtualQuery((void *)g_ca[j], &n, sizeof(n)); if (n.BaseAddress == m.BaseAddress) dup = 1; }
        if (dup) continue;
        dump_region_of(g_ca[i], 0x20 + g_ce[i], 8u << 20);
        add_target((u64)m.BaseAddress, (u64)m.BaseAddress + m.RegionSize, 0x100 + i);
    }
    logline("survey: names and numbers");
    add_name(0xD5C8, 0xCC3D);   /* 허창 */
    add_name(0xAC74, 0xC5C5);   /* 건업 */
    add_name(0xC131, 0xB3C4);   /* 성도 */
    add_name(0xB099, 0xC591);   /* 낙양 */
    add_name(0xC7A5, 0xC548);   /* 장안 */
    add_name(0xC591, 0xC591);   /* 양양 */
    find_pass();
    for (int i = 0; i < s_n; i++) for (int k = 0; k < s_cnt[i]; k++) {
        dump_block(s_hit[i][k] - 0x400, 0x800, 0x40 + i);
        add_target(s_hit[i][k] - 0x20, s_hit[i][k] + 0x8, 0x200 + i * 32 + k);
    }
    for (int i = 0; i < 4; i++) for (int k = 0; k < n_cnt[i]; k++) {
        dump_block(n_hit[i][k] - 0x1000, 0x2000, 0x50 + i);
        add_target(n_hit[i][k] - 0x800, n_hit[i][k] + 0x200, 0x400 + i * 16 + k);
    }
    { char b[160], *p = b; p = put_str(p, "survey: name hits");
      for (int i = 0; i < s_n; i++) { *p++ = ' '; p = put_int(p, s_cnt[i]); }
      p = put_str(p, ", number hits"); for (int i = 0; i < 4; i++) { *p++ = ' '; p = put_int(p, n_cnt[i]); } *p = 0; logline(b); }
    logline("survey: pointers 1");
    sort_targets(); ptr_pass(1);
    { char b[80], *p = b; p = put_str(p, "survey: pointer hits "); p = put_int(p, g_nh); *p = 0; logline(b); }
    static u64 lp[MAXH2];
    for (int lev = 2; lev <= 3; lev++) {
        int n1 = g_nh; for (int i = 0; i < n1; i++) lp[i] = h_a[i];
        int dumped = 0;
        for (int i = 0; i < n1 && dumped < 300; i++) { dump_block(lp[i] - 0x200, 0x400, 0x5F + lev); dumped++; }
        g_nt = 0;
        for (int i = 0; i < n1 && g_nt < MAXT; i++) {
            MBI m; if (!VirtualQuery((void *)lp[i], &m, sizeof(m))) continue;
            if ((u64)m.AllocationBase == (u64)exe) continue;      /* already a fixed place in the exe */
            add_target(lp[i] - 0x800, lp[i] + 0x8, (u32)(lev << 12) + i);
        }
        { char b[80], *p = b; p = put_str(p, "survey: pointers "); p = put_int(p, lev); *p = 0; logline(b); }
        sort_targets(); ptr_pass((u32)lev);
        { char b[80], *p = b; p = put_str(p, "survey: pointer hits "); p = put_int(p, g_nh); *p = 0; logline(b); }
    }
    for (int i = 0, dumped = 0; i < g_nh && dumped < 300; i++) { dump_block(h_a[i] - 0x200, 0x400, 0x63); dumped++; }
    { char b[120], *p = b; p = put_str(p, "survey: done, blocks "); p = put_int(p, g_sblocks); p = put_str(p, ", grids "); p = put_int(p, g_ncand);
      p = put_str(p, ", MB "); p = put_int(p, (int)(g_sbytes >> 20)); *p = 0; logline(b); }
    CloseHandle(g_sf);
    HANDLE d = CreateFileW(g_donepath, 0x40000000, 1, 0, 2, 0x80, 0);
    if (d != (HANDLE)(i64)-1) { DWORD w; WriteFile(d, "done\r\n", 6, &w, 0); CloseHandle(d); }
}

static DWORD WINAPI worker(void *arg)
{
    (void)arg;
    SetThreadPriority(GetCurrentThread(), -1);
    g_buf = (u8 *)VirtualAlloc(0, CHUNK + 4096, 0x3000, 0x04);
    g_c = (CAND *)VirtualAlloc(0, sizeof(CAND) * (SIZE_T)MAXC, 0x3000, 0x04);
    if (!g_buf || !g_c) return 0;
    logline("SeasonSync v14 started");
    Sleep(8000); rules_once("worker, 8 s after start");   /* v14: if the game never asked for a device through us, start the season rules from here */
    load_pref();
    Sleep(5000);
    g_direct = find_direct();
    if (!g_direct) Sleep(15000);
    int persisted = persisted_month();
    {
        char b[120], *p = b; p = put_str(p, "remembered month = "); p = put_int(p, persisted);
        if (g_pref_kind >= 0) { p = put_str(p, ", numbering kind="); p = put_int(p, g_pref_kind); p = put_str(p, g_pref_zb ? " (0..11)" : " (1..12)"); }
        *p = 0; logline(b);
    }
    int sent = 0, resend = 0, last_logged = -1;
    {
        char b[160], *p = b;
        if (g_direct) { p = put_str(p, "date place found in game code: global @"); p = put_hex(p, g_dglobal - (u64)GetModuleHandleW(0)); p = put_str(p, " +"); p = put_hex(p, g_doff); }
        else p = put_str(p, "date place not found in game code (other game version?) -> learning mode");
        *p = 0; logline(b);
    }
    if (g_direct) {
        int last_ok = -1, sent_x = -1, xresend = 0, last_x = -1, surveyed = 0;
        for (;;) {
            int y, m, d;
            int ok = read_direct(&y, &m, &d);
            if (ok != last_ok) { logline(ok ? "date readable (save loaded)" : "no date yet (title / loading)"); last_ok = ok; }
            if (ok && m != last_logged) {
                char b[120], *p = b; p = put_str(p, "month = "); p = put_int(p, m); p = put_str(p, "  (game date "); p = put_int(p, y);
                p = put_str(p, "/"); p = put_int(p, m); p = put_str(p, "/"); p = put_int(p, d); p = put_str(p, ")");
                *p = 0; logline(b); last_logged = m;
            }
            if (ok && (m != sent || resend) && game_foreground()) {
                send_month(m);
                resend = (m != sent); sent = m;
                sent_x = -1;
            }
            if (ok && !surveyed && GetFileAttributesW(g_flagpath) != 0xFFFFFFFF && GetFileAttributesW(g_donepath) == 0xFFFFFFFF) {
                surveyed = 1; Sleep(3000); run_imagedump();
            }
            if (ok && sent == m) {
                int x = d <= 10 ? 0 : (d <= 20 ? 1 : 2);
                if ((x != sent_x || xresend) && game_foreground()) {
                    Sleep(250);
                    send_turn(x);
                    xresend = (x != sent_x); sent_x = x;
                    if (x != last_x) { char b[60], *p = b; p = put_str(p, "turn = "); p = put_int(p, x + 1); *p = 0; logline(b); last_x = x; }
                }
            }
            Sleep(1000);
        }
    }
    for (;;) {
        u64 now = GetTickCount64();
        learn_tick(now);
        int month = g_int_month;
        if (month && month != last_logged) {
            char b[160], *p = b; p = put_str(p, "month = "); p = put_int(p, month);
            p = put_str(p, " (turn steps "); p = put_int(p, g_int_best); p = put_str(p, ", fields agreeing "); p = put_int(p, g_int_votes); p = put_str(p, ")");
            *p = 0; logline(b);
            log_top_cands();
            last_logged = month;
        }
        if (month && (month != sent || resend) && game_foreground()) {
            send_month(month);
            resend = (month != sent); sent = month;
        }
        Sleep(g_alive > 1000000 ? 5000 : 2500);
    }
    return 0;
}

/* ---- forwarding stubs to the system d3d11.dll ---- */
void *p_D3DKMTCloseAdapter;
__attribute__((naked)) void D3DKMTCloseAdapter(void) { __asm__ volatile("jmp *p_D3DKMTCloseAdapter(%rip)"); }
void *p_D3DKMTDestroyAllocation;
__attribute__((naked)) void D3DKMTDestroyAllocation(void) { __asm__ volatile("jmp *p_D3DKMTDestroyAllocation(%rip)"); }
void *p_D3DKMTDestroyContext;
__attribute__((naked)) void D3DKMTDestroyContext(void) { __asm__ volatile("jmp *p_D3DKMTDestroyContext(%rip)"); }
void *p_D3DKMTDestroyDevice;
__attribute__((naked)) void D3DKMTDestroyDevice(void) { __asm__ volatile("jmp *p_D3DKMTDestroyDevice(%rip)"); }
void *p_D3DKMTDestroySynchronizationObject;
__attribute__((naked)) void D3DKMTDestroySynchronizationObject(void) { __asm__ volatile("jmp *p_D3DKMTDestroySynchronizationObject(%rip)"); }
void *p_D3DKMTQueryAdapterInfo;
__attribute__((naked)) void D3DKMTQueryAdapterInfo(void) { __asm__ volatile("jmp *p_D3DKMTQueryAdapterInfo(%rip)"); }
void *p_D3DKMTSetDisplayPrivateDriverFormat;
__attribute__((naked)) void D3DKMTSetDisplayPrivateDriverFormat(void) { __asm__ volatile("jmp *p_D3DKMTSetDisplayPrivateDriverFormat(%rip)"); }
void *p_D3DKMTSignalSynchronizationObject;
__attribute__((naked)) void D3DKMTSignalSynchronizationObject(void) { __asm__ volatile("jmp *p_D3DKMTSignalSynchronizationObject(%rip)"); }
void *p_D3DKMTUnlock;
__attribute__((naked)) void D3DKMTUnlock(void) { __asm__ volatile("jmp *p_D3DKMTUnlock(%rip)"); }
void *p_D3DKMTWaitForSynchronizationObject;
__attribute__((naked)) void D3DKMTWaitForSynchronizationObject(void) { __asm__ volatile("jmp *p_D3DKMTWaitForSynchronizationObject(%rip)"); }
void *p_OpenAdapter10;
__attribute__((naked)) void OpenAdapter10(void) { __asm__ volatile("jmp *p_OpenAdapter10(%rip)"); }
void *p_OpenAdapter10_2;
__attribute__((naked)) void OpenAdapter10_2(void) { __asm__ volatile("jmp *p_OpenAdapter10_2(%rip)"); }
void *p_D3D11CoreCreateDevice;
__attribute__((naked)) void D3D11CoreCreateDevice(void) { __asm__ volatile("jmp *p_D3D11CoreCreateDevice(%rip)"); }
void *p_D3D11CoreCreateLayeredDevice;
__attribute__((naked)) void D3D11CoreCreateLayeredDevice(void) { __asm__ volatile("jmp *p_D3D11CoreCreateLayeredDevice(%rip)"); }
void *p_D3D11CoreGetLayeredDeviceSize;
__attribute__((naked)) void D3D11CoreGetLayeredDeviceSize(void) { __asm__ volatile("jmp *p_D3D11CoreGetLayeredDeviceSize(%rip)"); }
void *p_D3D11CoreRegisterLayers;
__attribute__((naked)) void D3D11CoreRegisterLayers(void) { __asm__ volatile("jmp *p_D3D11CoreRegisterLayers(%rip)"); }
void *p_D3D11CreateDevice;
void *p_D3D11CreateDeviceAndSwapChain;
/* v14: the season rules start here (SAN14_SeasonRules.dll next to the game, self-hosted: no shared mod loader needed, so it works on
   game versions that loader does not know). Once per game session, on the first device the game creates (start-up, before any campaign). */
static volatile long g_rules_once;
static void rules_once(const char *why)
{
    if (__sync_lock_test_and_set(&g_rules_once, 1)) return;
    u16 path[600]; int k = 0; while (g_logpath[k]) k++; while (k > 0 && g_logpath[k - 1] != '\\') k--;
    for (int j = 0; j < k; j++) path[j] = g_logpath[j];
    const char *nm = "SAN14_SeasonRules.dll"; int i = 0; while (nm[i]) { path[k + i] = (u16)nm[i]; i++; } path[k + i] = 0;
    char b[160], *p = b; p = put_str(p, "season rules: "); p = put_str(p, why);
    if (GetFileAttributesW(path) == 0xFFFFFFFF) { p = put_str(p, " - SAN14_SeasonRules.dll not found"); *p = 0; logline(b); return; }
    void *h = LoadLibraryW(path);
    u32 (*start)(void) = h ? (u32 (*)(void))GetProcAddress(h, "SAN14_SeasonSelfHost") : 0;
    u32 r = start ? start() : 0;
    p = put_str(p, !h ? " - load failed" : !start ? " - old rules file (no self-host)" : r ? " - started" : " - not started (see SeasonRules.log)"); *p = 0; logline(b);
}
long D3D11CreateDevice(void *a, int b, void *c, unsigned d, const void *e, unsigned f, unsigned g, void **h, void *i, void **j)
{
    rules_once("first device");
    return ((long (*)(void *, int, void *, unsigned, const void *, unsigned, unsigned, void **, void *, void **))p_D3D11CreateDevice)(a, b, c, d, e, f, g, h, i, j);
}
long D3D11CreateDeviceAndSwapChain(void *a, int b, void *c, unsigned d, const void *e, unsigned f, unsigned g, const void *sd, void **sc, void **dev, void *fl, void **ctx)
{
    rules_once("first device");
    return ((long (*)(void *, int, void *, unsigned, const void *, unsigned, unsigned, const void *, void **, void **, void *, void **))p_D3D11CreateDeviceAndSwapChain)(a, b, c, d, e, f, g, sd, sc, dev, fl, ctx);
}
void *p_D3DKMTCreateAllocation;
__attribute__((naked)) void D3DKMTCreateAllocation(void) { __asm__ volatile("jmp *p_D3DKMTCreateAllocation(%rip)"); }
void *p_D3DKMTCreateContext;
__attribute__((naked)) void D3DKMTCreateContext(void) { __asm__ volatile("jmp *p_D3DKMTCreateContext(%rip)"); }
void *p_D3DKMTCreateDevice;
__attribute__((naked)) void D3DKMTCreateDevice(void) { __asm__ volatile("jmp *p_D3DKMTCreateDevice(%rip)"); }
void *p_D3DKMTCreateSynchronizationObject;
__attribute__((naked)) void D3DKMTCreateSynchronizationObject(void) { __asm__ volatile("jmp *p_D3DKMTCreateSynchronizationObject(%rip)"); }
void *p_D3DKMTEscape;
__attribute__((naked)) void D3DKMTEscape(void) { __asm__ volatile("jmp *p_D3DKMTEscape(%rip)"); }
void *p_D3DKMTGetContextSchedulingPriority;
__attribute__((naked)) void D3DKMTGetContextSchedulingPriority(void) { __asm__ volatile("jmp *p_D3DKMTGetContextSchedulingPriority(%rip)"); }
void *p_D3DKMTGetDeviceState;
__attribute__((naked)) void D3DKMTGetDeviceState(void) { __asm__ volatile("jmp *p_D3DKMTGetDeviceState(%rip)"); }
void *p_D3DKMTGetDisplayModeList;
__attribute__((naked)) void D3DKMTGetDisplayModeList(void) { __asm__ volatile("jmp *p_D3DKMTGetDisplayModeList(%rip)"); }
void *p_D3DKMTGetMultisampleMethodList;
__attribute__((naked)) void D3DKMTGetMultisampleMethodList(void) { __asm__ volatile("jmp *p_D3DKMTGetMultisampleMethodList(%rip)"); }
void *p_D3DKMTGetRuntimeData;
__attribute__((naked)) void D3DKMTGetRuntimeData(void) { __asm__ volatile("jmp *p_D3DKMTGetRuntimeData(%rip)"); }
void *p_D3DKMTGetSharedPrimaryHandle;
__attribute__((naked)) void D3DKMTGetSharedPrimaryHandle(void) { __asm__ volatile("jmp *p_D3DKMTGetSharedPrimaryHandle(%rip)"); }
void *p_D3DKMTLock;
__attribute__((naked)) void D3DKMTLock(void) { __asm__ volatile("jmp *p_D3DKMTLock(%rip)"); }
void *p_D3DKMTOpenAdapterFromHdc;
__attribute__((naked)) void D3DKMTOpenAdapterFromHdc(void) { __asm__ volatile("jmp *p_D3DKMTOpenAdapterFromHdc(%rip)"); }
void *p_D3DKMTOpenResource;
__attribute__((naked)) void D3DKMTOpenResource(void) { __asm__ volatile("jmp *p_D3DKMTOpenResource(%rip)"); }
void *p_D3DKMTPresent;
__attribute__((naked)) void D3DKMTPresent(void) { __asm__ volatile("jmp *p_D3DKMTPresent(%rip)"); }
void *p_D3DKMTQueryAllocationResidency;
__attribute__((naked)) void D3DKMTQueryAllocationResidency(void) { __asm__ volatile("jmp *p_D3DKMTQueryAllocationResidency(%rip)"); }
void *p_D3DKMTQueryResourceInfo;
__attribute__((naked)) void D3DKMTQueryResourceInfo(void) { __asm__ volatile("jmp *p_D3DKMTQueryResourceInfo(%rip)"); }
void *p_D3DKMTRender;
__attribute__((naked)) void D3DKMTRender(void) { __asm__ volatile("jmp *p_D3DKMTRender(%rip)"); }
void *p_D3DKMTSetAllocationPriority;
__attribute__((naked)) void D3DKMTSetAllocationPriority(void) { __asm__ volatile("jmp *p_D3DKMTSetAllocationPriority(%rip)"); }
void *p_D3DKMTSetContextSchedulingPriority;
__attribute__((naked)) void D3DKMTSetContextSchedulingPriority(void) { __asm__ volatile("jmp *p_D3DKMTSetContextSchedulingPriority(%rip)"); }
void *p_D3DKMTSetDisplayMode;
__attribute__((naked)) void D3DKMTSetDisplayMode(void) { __asm__ volatile("jmp *p_D3DKMTSetDisplayMode(%rip)"); }
void *p_D3DKMTSetGammaRamp;
__attribute__((naked)) void D3DKMTSetGammaRamp(void) { __asm__ volatile("jmp *p_D3DKMTSetGammaRamp(%rip)"); }
void *p_D3DKMTSetVidPnSourceOwner;
__attribute__((naked)) void D3DKMTSetVidPnSourceOwner(void) { __asm__ volatile("jmp *p_D3DKMTSetVidPnSourceOwner(%rip)"); }
void *p_D3DKMTWaitForVerticalBlankEvent;
__attribute__((naked)) void D3DKMTWaitForVerticalBlankEvent(void) { __asm__ volatile("jmp *p_D3DKMTWaitForVerticalBlankEvent(%rip)"); }
void *p_D3D11On12CreateDevice;
__attribute__((naked)) void D3D11On12CreateDevice(void) { __asm__ volatile("jmp *p_D3D11On12CreateDevice(%rip)"); }
static void *g_chain, *g_sys;
static void *pick(const char *n) { void *f = g_chain ? GetProcAddress(g_chain, n) : 0; return f ? f : GetProcAddress(g_sys, n); }
static void resolve_all(void)
{
    u16 path[600]; u32 n = GetSystemDirectoryW(path, 520); const char *t = "\\d3d11.dll"; int i = 0;
    while (t[i]) { path[n + i] = (u16)t[i]; i++; } path[n + i] = 0;
    void *hs = LoadLibraryW(path);
    if (!hs) { logline("system d3d11 load FAILED"); return; }
    void *hc = 0;
    { u16 cp[600]; int k = 0; while (g_logpath[k]) k++; while (k > 0 && g_logpath[k - 1] != '\\') k--;
      for (int j = 0; j < k; j++) cp[j] = g_logpath[j];
      const char *nm = "d3d11_chain.dll"; int q = 0; while (nm[q]) { cp[k + q] = (u16)nm[q]; q++; } cp[k + q] = 0;
      if (GetFileAttributesW(cp) != 0xFFFFFFFF) { hc = LoadLibraryW(cp); logline(hc ? "chain: d3d11_chain.dll (another mod's d3d11) loaded under the graphics tool" : "chain: d3d11_chain.dll present but failed to load - system d3d11 only"); } }
    g_chain = hc; g_sys = hs;
    p_D3DKMTCloseAdapter = pick("D3DKMTCloseAdapter");
    p_D3DKMTDestroyAllocation = pick("D3DKMTDestroyAllocation");
    p_D3DKMTDestroyContext = pick("D3DKMTDestroyContext");
    p_D3DKMTDestroyDevice = pick("D3DKMTDestroyDevice");
    p_D3DKMTDestroySynchronizationObject = pick("D3DKMTDestroySynchronizationObject");
    p_D3DKMTQueryAdapterInfo = pick("D3DKMTQueryAdapterInfo");
    p_D3DKMTSetDisplayPrivateDriverFormat = pick("D3DKMTSetDisplayPrivateDriverFormat");
    p_D3DKMTSignalSynchronizationObject = pick("D3DKMTSignalSynchronizationObject");
    p_D3DKMTUnlock = pick("D3DKMTUnlock");
    p_D3DKMTWaitForSynchronizationObject = pick("D3DKMTWaitForSynchronizationObject");
    p_OpenAdapter10 = pick("OpenAdapter10");
    p_OpenAdapter10_2 = pick("OpenAdapter10_2");
    p_D3D11CoreCreateDevice = pick("D3D11CoreCreateDevice");
    p_D3D11CoreCreateLayeredDevice = pick("D3D11CoreCreateLayeredDevice");
    p_D3D11CoreGetLayeredDeviceSize = pick("D3D11CoreGetLayeredDeviceSize");
    p_D3D11CoreRegisterLayers = pick("D3D11CoreRegisterLayers");
    p_D3D11CreateDevice = pick("D3D11CreateDevice");
    p_D3D11CreateDeviceAndSwapChain = pick("D3D11CreateDeviceAndSwapChain");
    p_D3DKMTCreateAllocation = pick("D3DKMTCreateAllocation");
    p_D3DKMTCreateContext = pick("D3DKMTCreateContext");
    p_D3DKMTCreateDevice = pick("D3DKMTCreateDevice");
    p_D3DKMTCreateSynchronizationObject = pick("D3DKMTCreateSynchronizationObject");
    p_D3DKMTEscape = pick("D3DKMTEscape");
    p_D3DKMTGetContextSchedulingPriority = pick("D3DKMTGetContextSchedulingPriority");
    p_D3DKMTGetDeviceState = pick("D3DKMTGetDeviceState");
    p_D3DKMTGetDisplayModeList = pick("D3DKMTGetDisplayModeList");
    p_D3DKMTGetMultisampleMethodList = pick("D3DKMTGetMultisampleMethodList");
    p_D3DKMTGetRuntimeData = pick("D3DKMTGetRuntimeData");
    p_D3DKMTGetSharedPrimaryHandle = pick("D3DKMTGetSharedPrimaryHandle");
    p_D3DKMTLock = pick("D3DKMTLock");
    p_D3DKMTOpenAdapterFromHdc = pick("D3DKMTOpenAdapterFromHdc");
    p_D3DKMTOpenResource = pick("D3DKMTOpenResource");
    p_D3DKMTPresent = pick("D3DKMTPresent");
    p_D3DKMTQueryAllocationResidency = pick("D3DKMTQueryAllocationResidency");
    p_D3DKMTQueryResourceInfo = pick("D3DKMTQueryResourceInfo");
    p_D3DKMTRender = pick("D3DKMTRender");
    p_D3DKMTSetAllocationPriority = pick("D3DKMTSetAllocationPriority");
    p_D3DKMTSetContextSchedulingPriority = pick("D3DKMTSetContextSchedulingPriority");
    p_D3DKMTSetDisplayMode = pick("D3DKMTSetDisplayMode");
    p_D3DKMTSetGammaRamp = pick("D3DKMTSetGammaRamp");
    p_D3DKMTSetVidPnSourceOwner = pick("D3DKMTSetVidPnSourceOwner");
    p_D3DKMTWaitForVerticalBlankEvent = pick("D3DKMTWaitForVerticalBlankEvent");
    p_D3D11On12CreateDevice = pick("D3D11On12CreateDevice");
}

BOOL WINAPI DllMain(void *inst, DWORD reason, void *res)
{
    (void)res;
    if (reason == 1) {
        DisableThreadLibraryCalls(inst);
        DWORD n = GetModuleFileNameW(inst, g_logpath, 520);
        int k = (int)n; while (k > 0 && g_logpath[k - 1] != '\\') k--;
        const char *nm = "SeasonSync_log.txt", *un = "d3dx_user.ini", *pf = "SeasonSync_numbering.txt";
        for (int j = 0; j < k; j++) { g_userini[j] = g_logpath[j]; g_prefpath[j] = g_logpath[j]; }
        int i = 0; while (nm[i]) { g_logpath[k + i] = (u16)nm[i]; i++; } g_logpath[k + i] = 0;
        i = 0; while (un[i]) { g_userini[k + i] = (u16)un[i]; i++; } g_userini[k + i] = 0;
        i = 0; while (pf[i]) { g_prefpath[k + i] = (u16)pf[i]; i++; } g_prefpath[k + i] = 0;
        { const char *a = "SeasonSync_survey.flag", *b2 = "SeasonSync_survey.bin", *c = "SeasonSync_survey.done";
          for (int j = 0; j < k; j++) { g_flagpath[j] = g_logpath[j]; g_surveypath[j] = g_logpath[j]; g_donepath[j] = g_logpath[j]; }
          i = 0; while (a[i]) { g_flagpath[k + i] = (u16)a[i]; i++; } g_flagpath[k + i] = 0;
          i = 0; while (b2[i]) { g_surveypath[k + i] = (u16)b2[i]; i++; } g_surveypath[k + i] = 0;
          i = 0; while (c[i]) { g_donepath[k + i] = (u16)c[i]; i++; } g_donepath[k + i] = 0; }
        resolve_all();
        CreateThread(0, 0, worker, 0, 0, 0);
    }
    return 1;
}
