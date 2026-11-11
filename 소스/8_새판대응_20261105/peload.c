/* peload: loads the real SAN14_SeasonRules.dll (Windows x64 PE) on Linux, with the real game exe mapped at its own base,
   and starts the rules the way SeasonSync does (SAN14_SeasonSelfHostAt). Checks every hook is placed on the real game bytes.
   usage: peload <rules.dll> <unpacked game exe> <root dir> [sim flags: r43 ai] */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/syscall.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef uint64_t u64; typedef int32_t i32; typedef int64_t i64;
#define MS __attribute__((ms_abi))
static const char *ROOT; static u8 *GAME; static u32 GAMESZ; static u8 *DLL; static u32 DLLSZ;
static void w2a(const u16 *w, char *o) { char *s = o; strcpy(o, ROOT); o += strlen(o);
    if (w[0] && w[1] == ':') w += 2;
    while (*w) { u16 c = *w++; *o++ = c == '\\' ? '/' : c < 128 ? (char)c : '_'; } *o = 0; (void)s; }
static void w2u8(const u16 *w, char *o, int n) { int k = 0; while (*w && k < n - 4) { u32 c = *w++;
    if (c < 0x80) o[k++] = c; else if (c < 0x800) { o[k++] = 0xc0 | c >> 6; o[k++] = 0x80 | (c & 63); } else { o[k++] = 0xe0 | c >> 12; o[k++] = 0x80 | ((c >> 6) & 63); o[k++] = 0x80 | (c & 63); } } o[k] = 0; }
/* ---- kernel32 ---- */
static MS void *VirtualAlloc_(void *a, u64 n, u32 t, u32 p) { (void)t; (void)p; n = (n + 4095) & ~4095ull;
    void *m = mmap(a, n, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | (a ? MAP_FIXED_NOREPLACE : 0), -1, 0);
    if (m == MAP_FAILED) return 0; if (a && m != a) { munmap(m, n); return 0; } return m; }
static MS int VirtualFree_(void *a, u64 n, u32 t) { (void)a; (void)n; (void)t; return 1; }
static MS int VirtualProtect_(void *a, u64 n, u32 np, u32 *op) { (void)a; (void)n; (void)np; if (op) *op = 0x40; return 1; }
typedef struct { u64 Base, AllocBase; u32 AllocProt; u32 pad0; u64 Size; u32 State, Protect, Type, pad1; } MBI;
static MS u64 VirtualQuery_(const void *addr, MBI *m, u64 n) { (void)n; u64 a = (u64)addr & ~4095ull;
    FILE *f = fopen("/proc/self/maps", "r"); char l[512]; u64 nxt = ~0ull; memset(m, 0, sizeof(*m)); m->Base = a;
    while (fgets(l, sizeof l, f)) { u64 s, e; char pr[8]; if (sscanf(l, "%lx-%lx %7s", &s, &e, pr) != 3) continue;
        if (a >= s && a < e) { fclose(f); m->AllocBase = s; m->Size = e - a; m->State = 0x1000;
            m->Protect = pr[0] != 'r' ? 1 : pr[2] == 'x' ? (pr[1] == 'w' ? 0x40 : 0x20) : (pr[1] == 'w' ? 4 : 2);
            m->Type = (a >= (u64)GAME && a < (u64)GAME + GAMESZ) || (a >= (u64)DLL && a < (u64)DLL + DLLSZ) ? 0x1000000 : 0x20000; return 48; }
        if (s > a && s < nxt) nxt = s; }
    fclose(f); m->State = 0x10000; m->Protect = 1; m->Size = (nxt == ~0ull ? 0x100000000ull : nxt - a); return 48; }
static MS int ReadProcessMemory_(void *h, const void *src, void *dst, u64 n, u64 *got) { (void)h;
    struct iovec li = { dst, n }, ri = { (void *)src, n }; ssize_t r = process_vm_readv(getpid(), &li, 1, &ri, 1, 0);
    if (got) *got = r > 0 ? (u64)r : 0; return r == (ssize_t)n; }
static MS u32 GetModuleFileNameW_(void *h, u16 *p, u32 n) { const char *s = h == 0 ? "C:\\game\\SAN14CE_KO.exe" : "C:\\game\\SAN14_SeasonRules.dll"; u32 i = 0; for (; s[i] && i < n - 1; i++) p[i] = s[i]; p[i] = 0; return i; }
static MS void *GetModuleHandleW_(const u16 *n) { if (!n) return GAME; return 0; }
static MS int GetModuleHandleExW_(u32 fl, const void *a, void **h) { (void)fl; (void)a; *h = DLL; return 1; }
static MS i64 CreateFileW_(const u16 *w, u32 acc, u32 sh, void *sa, u32 disp, u32 fl, void *t) { (void)sh; (void)sa; (void)fl; (void)t; char p[1200]; w2a(w, p);
    int o = (acc & 0x40000000) || (acc & 4) ? ((acc & 0x80000000) ? O_RDWR : O_WRONLY) : O_RDONLY;
    if (disp == 2) o |= O_CREAT | O_TRUNC; else if (disp == 4) o |= O_CREAT; else if (disp == 1) o |= O_CREAT | O_EXCL;
    if (acc & 4) o |= O_APPEND; int fd = open(p, o, 0644); return fd < 0 ? -1 : fd + 1000; }
static MS int ReadFile_(i64 h, void *b, u32 n, u32 *r, void *ov) { (void)ov; ssize_t k = read((int)h - 1000, b, n); *r = k > 0 ? k : 0; return k >= 0; }
static MS int WriteFile_(i64 h, const void *b, u32 n, u32 *w, void *ov) { (void)ov; ssize_t k = write((int)h - 1000, b, n); if (w) *w = k > 0 ? k : 0; return k >= 0; }
static MS int CloseHandle_(i64 h) { if (h >= 1000) close((int)h - 1000); return 1; }
static MS u32 SetFilePointer_(i64 h, i32 d, i32 *hi, u32 m) { (void)hi; return (u32)lseek((int)h - 1000, d, m == 2 ? SEEK_END : m == 1 ? SEEK_CUR : SEEK_SET); }
static MS u64 GetTickCount64_(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (u64)t.tv_sec * 1000 + t.tv_nsec / 1000000; }
static MS u32 GetFileAttributesW_(const u16 *w) { char p[1200]; w2a(w, p); struct stat st; if (stat(p, &st)) return 0xFFFFFFFF; return S_ISDIR(st.st_mode) ? 0x10 : 0x80; }
static MS int DeleteFileW_(const u16 *w) { char p[1200]; w2a(w, p); return unlink(p) == 0; }
static MS int FlushInstructionCache_(void *a, const void *b, u64 n) { (void)a; (void)b; (void)n; return 1; }
static MS i64 GetCurrentProcess_(void) { return -1; }
static MS u32 GetCurrentThreadId_(void) { return (u32)syscall(SYS_gettid); }
static MS u32 GetCurrentProcessId_(void) { return (u32)getpid(); }
static MS void GetLocalTime_(u16 *t) { time_t x = time(0); struct tm m; localtime_r(&x, &m); t[0] = m.tm_year + 1900; t[1] = m.tm_mon + 1; t[2] = m.tm_wday; t[3] = m.tm_mday; t[4] = m.tm_hour; t[5] = m.tm_min; t[6] = m.tm_sec; t[7] = 0; }
typedef MS u32 (*ThFn)(void *); typedef struct { ThFn f; void *a; } ThArg;
static void *th_tramp(void *p) { ThArg a = *(ThArg *)p; free(p); a.f(a.a); return 0; }
static MS i64 CreateThread_(void *sa, u64 st, ThFn f, void *arg, u32 fl, u32 *id) { (void)sa; (void)st; (void)fl; if (id) *id = 9; ThArg *a = malloc(sizeof *a); a->f = f; a->a = arg; pthread_t t; pthread_create(&t, 0, th_tramp, a); pthread_detach(t); return 1; }
static MS void Sleep_(u32 ms) { usleep(ms * 1000); }
static MS i64 CreateMutexW_(void *a, int b, const u16 *n) { (void)a; (void)b; (void)n; return 2; }
static MS u32 WaitForSingleObject_(i64 h, u32 ms) { (void)h; (void)ms; return 0; }
static MS int ReleaseMutex_(i64 h) { (void)h; return 1; }
static MS int QueryPerformanceCounter_(i64 *c) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); *c = (i64)t.tv_sec * 1000000000 + t.tv_nsec; return 1; }
static MS int QueryPerformanceFrequency_(i64 *c) { *c = 1000000000; return 1; }
static MS void *AddVectoredExceptionHandler_(u32 f, void *h) { (void)f; (void)h; return (void *)1; }
/* ---- user32 ---- */
static MS void *GetForegroundWindow_(void) { return 0; }
static MS u32 GetWindowThreadProcessId_(void *w, u32 *p) { (void)w; if (p) *p = 0; return 0; }
static MS u32 SendInput_(u32 n, void *i, int s) { (void)n; (void)i; (void)s; return 0; }
static int nbox; static char boxtext[4000];
static MS int MessageBoxW_(void *w, const u16 *t, const u16 *c, u32 f) { (void)w; (void)f; char a[4000], b[200]; w2u8(t, a, sizeof a); w2u8(c, b, sizeof b); nbox++; strcpy(boxtext, a); printf("MESSAGEBOX [%s]\n%s\n", b, a); return 1; }
static const struct { const char *n; void *f; } IMPS[] = {
#define I(x) { #x, (void *)x##_ }
    I(VirtualAlloc), I(VirtualFree), I(VirtualProtect), I(VirtualQuery), I(ReadProcessMemory), I(GetModuleFileNameW), I(GetModuleHandleW), I(GetModuleHandleExW),
    I(CreateFileW), I(ReadFile), I(WriteFile), I(CloseHandle), I(SetFilePointer), I(GetTickCount64), I(GetFileAttributesW), I(DeleteFileW), I(FlushInstructionCache),
    I(GetCurrentProcess), I(GetCurrentThreadId), I(GetCurrentProcessId), I(GetLocalTime), I(CreateThread), I(Sleep), I(CreateMutexW), I(WaitForSingleObject),
    I(ReleaseMutex), I(QueryPerformanceCounter), I(QueryPerformanceFrequency), I(AddVectoredExceptionHandler),
    I(GetForegroundWindow), I(GetWindowThreadProcessId), I(SendInput), I(MessageBoxW), { 0, 0 } };
static u8 *rdfile(const char *p, u64 *n) { FILE *f = fopen(p, "rb"); if (!f) { perror(p); exit(2); } fseek(f, 0, 2); *n = ftell(f); fseek(f, 0, 0); u8 *b = malloc(*n); fread(b, 1, *n, f); fclose(f); return b; }
static u8 *map_pe(const char *path, u64 want, u32 *size, int reloc)
{
    u64 n; u8 *f = rdfile(path, &n); u32 pe = *(u32 *)(f + 0x3c); u16 ns = *(u16 *)(f + pe + 6), opt = *(u16 *)(f + pe + 20);
    u8 *oh = f + pe + 24; u64 ib = *(u64 *)(oh + 24); u32 is = *(u32 *)(oh + 56), hs = *(u32 *)(oh + 60);
    u8 *m = mmap((void *)(want ? want : 0), is, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | (want ? MAP_FIXED_NOREPLACE : 0), -1, 0);
    if (m == MAP_FAILED || (want && (u64)m != want)) { printf("map %s at %lx failed\n", path, want); exit(2); }
    memcpy(m, f, hs);
    for (int i = 0; i < ns; i++) { u8 *s = f + pe + 24 + opt + 40 * i; u32 vs = *(u32 *)(s + 8), va = *(u32 *)(s + 12), rs = *(u32 *)(s + 16), ra = *(u32 *)(s + 20);
        if (rs) memcpy(m + va, f + ra, rs < vs || !vs ? rs : vs); }
    if (reloc && (u64)m != ib) { u32 rva = *(u32 *)(oh + 112 + 5 * 8), rsz = *(u32 *)(oh + 112 + 5 * 8 + 4); i64 d = (i64)((u64)m - ib);
        for (u32 o = 0; o < rsz;) { u32 pg = *(u32 *)(m + rva + o), bs = *(u32 *)(m + rva + o + 4); if (!bs) break;
            for (u32 k = 8; k < bs; k += 2) { u16 e = *(u16 *)(m + rva + o + k); if ((e >> 12) == 10) *(u64 *)(m + pg + (e & 0xfff)) += d; else if ((e >> 12) && (e >> 12) != 0) { printf("reloc type %d\n", e >> 12); exit(2); } }
            o += bs; } }
    if (reloc) { u32 irva = *(u32 *)(oh + 112 + 8);
        for (u8 *d = m + irva; *(u32 *)(d + 12); d += 20) { const char *dn = (const char *)m + *(u32 *)(d + 12); u64 *th = (u64 *)(m + *(u32 *)(d + 16));
            for (; *th; th++) { const char *fn = (const char *)m + (*th & 0x7fffffff) + 2; void *fp = 0;
                for (int k = 0; IMPS[k].n; k++) if (!strcmp(IMPS[k].n, fn)) fp = IMPS[k].f;
                if (!fp) { printf("missing import %s!%s\n", dn, fn); exit(2); } *th = (u64)fp; } } }
    *size = is; free(f); return m;
}
static u8 *find_export(u8 *m, const char *name)
{
    u32 pe = *(u32 *)(m + 0x3c); u8 *oh = m + pe + 24; u32 erva = *(u32 *)(oh + 112); u8 *e = m + erva;
    u32 nn = *(u32 *)(e + 24); u32 *fa = (u32 *)(m + *(u32 *)(e + 28)), *na = (u32 *)(m + *(u32 *)(e + 32)); u16 *oa = (u16 *)(m + *(u32 *)(e + 36));
    for (u32 i = 0; i < nn; i++) if (!strcmp((char *)m + na[i], name)) return m + fa[oa[i]];
    return 0;
}
static void jmp_to(u8 *at, u8 *to, int len) { at[0] = 0xE9; *(i32 *)(at + 1) = (i32)(to - (at + 5)); for (int i = 5; i < len; i++) at[i] = 0x90; }
int main(int argc, char **argv)
{
    setvbuf(stdout, 0, _IONBF, 0);
    if (argc < 4) return 1; ROOT = argv[3];
    int r43 = 0, ai = 0; for (int i = 4; i < argc; i++) { if (!strcmp(argv[i], "r43")) r43 = 1; if (!strcmp(argv[i], "ai")) ai = 1; }
    GAME = map_pe(argv[2], 0x140000000ull, &GAMESZ, 0);
    u32 pe = *(u32 *)(GAME + 0x3c); u32 stamp = *(u32 *)(GAME + pe + 8);
    printf("game image %x stamp %08x\n", GAMESZ, stamp);
    u8 *orig = malloc(GAMESZ); memcpy(orig, GAME, GAMESZ);
    u8 *other = mmap((void *)0x13f000000ull, 0x10000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    memset(other, 0xC3, 0x10000);
    if (r43) {   /* what the integrated patch R43 changes at the season map's neighbours on this exe (StatusColor H6, IC route-cost call wrap) */
        if (stamp == 0x6ad06f53u) { jmp_to(GAME + 0x9333c9, other + 0x100, 12); GAME[0x1854df] = 0xE8; *(i32 *)(GAME + 0x1854e0) = (i32)((other + 0x200) - (GAME + 0x1854e4)); }
        else { jmp_to(GAME + 0x930e39, other + 0x100, 12); GAME[0x1841af] = 0xE8; *(i32 *)(GAME + 0x1841b0) = (i32)((other + 0x200) - (GAME + 0x1841b4)); }
        printf("simulated: integrated patch jumps at the text setter +9 and the route-cost call\n"); }
    if (ai) { u32 sp = stamp == 0x6ad06f53u ? 0x2b2550 : 0x2b0290; jmp_to(GAME + sp, other + 0x300, 5); printf("simulated: AI mod jump at the unit speed function\n"); }
    u8 *snap = malloc(GAMESZ); memcpy(snap, GAME, GAMESZ);
    DLL = map_pe(argv[1], 0, &DLLSZ, 1);
    u32 dpe = *(u32 *)(DLL + 0x3c); u32 ep = *(u32 *)(DLL + dpe + 24 + 16);
    if (ep) { typedef MS int (*Main)(void *, u32, void *); ((Main)(DLL + ep))(DLL, 1, 0); }
    typedef MS u32 (*Start)(void); Start st = (Start)find_export(DLL, "SAN14_SeasonSelfHost");
    if (!st) { printf("no SAN14_SeasonSelfHost\n"); return 2; }
    u32 r = st(); printf("self-host start -> %u\n", r);
    int nch = 0; u32 last = 0xffffffff;
    for (u32 i = 0x1000; i < GAMESZ; i++) if (GAME[i] != snap[i]) { if (i != last + 1) { nch++; printf("changed game+0x%x:", i); for (int k = 0; k < 16 && i + k < GAMESZ; k++) printf(" %02x", GAME[i + k]); printf("\n"); } last = i; }
    printf("changed places %d\n", nch);
    sleep(argc > 0 && getenv("WAIT") ? atoi(getenv("WAIT")) : 3);
    printf("message boxes %d\n", nbox);
    fflush(stdout); _exit(0);
}
