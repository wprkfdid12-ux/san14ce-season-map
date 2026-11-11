/* maps the game exe (unpacked) at a free address, loads the season rules DLL and runs its self-host on that copy */
typedef unsigned long long u64; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8; typedef void *HANDLE; typedef unsigned long DWORD; typedef int BOOL;
#define IMP __declspec(dllimport)
IMP HANDLE __stdcall CreateFileW(const u16 *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
IMP BOOL __stdcall ReadFile(HANDLE, void *, DWORD, DWORD *, void *);
IMP BOOL __stdcall CloseHandle(HANDLE);
IMP void *__stdcall VirtualAlloc(void *, u64, DWORD, DWORD);
IMP void *__stdcall LoadLibraryW(const u16 *);
IMP void *__stdcall GetProcAddress(void *, const char *);
IMP HANDLE __stdcall GetStdHandle(DWORD);
IMP BOOL __stdcall WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
IMP void __stdcall Sleep(DWORD);
IMP void __stdcall ExitProcess(u32);
IMP DWORD __stdcall GetFileSize(HANDLE, DWORD *);
static void out(const char *s) { DWORD w; int n = 0; while (s[n]) n++; WriteFile(GetStdHandle((DWORD)-11), s, n, &w, 0); }
static void hex(const char *t, u64 v) { char b[64]; int i = 0; while (*t) b[i++] = *t++; for (int k = 60; k >= 0; k -= 4) { int d = (v >> k) & 15; b[i++] = d < 10 ? '0' + d : 'a' + d - 10; } b[i++] = '\n'; b[i] = 0; out(b); }
static void bytes(const char *t, const u8 *p, int n) { char b[128]; int i = 0; while (*t) b[i++] = *t++; for (int k = 0; k < n; k++) { int h = p[k] >> 4, l = p[k] & 15; b[i++] = h < 10 ? '0' + h : 'a' + h - 10; b[i++] = l < 10 ? '0' + l : 'a' + l - 10; b[i++] = ' '; } b[i++] = '\n'; b[i] = 0; out(b); }
void *memset(void *d, int c, u64 n) { u8 *p = d; while (n--) *p++ = (u8)c; return d; }
void *memcpy(void *d, const void *s, u64 n) { u8 *p = d; const u8 *q = s; while (n--) *p++ = *q++; return d; }
int _fltused;
void start(void)
{
    static const u16 exe[] = L"game_copy.exe", dll[] = L"SAN14_SeasonRules.dll";
    HANDLE f = CreateFileW(exe, 0x80000000, 1, 0, 3, 0x80, 0); if (f == (HANDLE)(long long)-1) { out("no exe\n"); ExitProcess(1); }
    DWORD sz = GetFileSize(f, 0); u8 *raw = VirtualAlloc(0, sz, 0x3000, 4); DWORD got = 0, r = 0; while (got < sz && ReadFile(f, raw + got, sz - got, &r, 0) && r) got += r; CloseHandle(f);
    u32 pe = *(u32 *)(raw + 0x3c); u32 simg = *(u32 *)(raw + pe + 0x50); u16 ns = *(u16 *)(raw + pe + 6), opt = *(u16 *)(raw + pe + 20);
    u8 *img = VirtualAlloc((void *)0x7f0000000ULL, simg, 0x3000, 0x40); if (!img) img = VirtualAlloc(0, simg, 0x3000, 0x40);
    memcpy(img, raw, 0x1000);
    for (int i = 0; i < ns; i++) { u8 *sh = raw + pe + 24 + opt + 40 * i; u32 vs = *(u32 *)(sh + 8), va = *(u32 *)(sh + 12), rs = *(u32 *)(sh + 16), ro = *(u32 *)(sh + 20); memcpy(img + va, raw + ro, rs < vs ? rs : vs); }
    { /* apply base relocations so absolute pointers (vtables) match the mapped copy */
      u64 pref = *(u64 *)(raw + pe + 24 + 24); long long delta = (long long)((u64)img - pref);
      u32 rva = *(u32 *)(raw + pe + 24 + 112 + 5 * 8), rsz = *(u32 *)(raw + pe + 24 + 112 + 5 * 8 + 4); u32 off = 0; int nfix = 0;
      while (off + 8 <= rsz) { u8 *blk = img + rva + off; u32 page = *(u32 *)blk, bs = *(u32 *)(blk + 4); if (bs < 8) break;
        for (u32 k = 8; k + 2 <= bs; k += 2) { u16 e = *(u16 *)(blk + k); if ((e >> 12) == 10) { *(u64 *)(img + page + (e & 0xfff)) += delta; nfix++; } }
        off += bs; }
      hex("relocs applied ", nfix); }
    hex("image at ", (u64)img); hex("stamp ", *(u32 *)(img + pe + 8));
    static u8 keep[0x2d00]; memcpy(keep, img + 0x183a30, 0x2d00);   /* the route-search code 0x183a30..0x186730 */
    static u8 keep2[0x200]; memcpy(keep2, img + 0x9b200, 0x200);
    void *h = LoadLibraryW(dll); if (!h) { out("dll load failed\n"); ExitProcess(2); }
    u32 (*sh)(u8 *) = (u32 (*)(u8 *))GetProcAddress(h, "SAN14_SeasonSelfHostAt"); if (!sh) { out("no test export\n"); ExitProcess(3); }
    u32 ok = sh(img); hex("self-host result ", ok);
    u32 again = sh(img); hex("second call (must not install twice) ", again);
    u32 site = *(u32 *)(img + 0x200), spd, fr; (void)site;
    int ver = *(u32 *)(img + pe + 8) == 0x6a983c25 ? 1 : 0;
    u32 TS = ver ? 0x44c590 : 0x44b5e0, SP = ver ? 0x2b0290 : 0x2af6a0, FR = ver ? 0x268210 : 0x267630;
    bytes("time site now: ", img + TS, 5); bytes("speed fn now:  ", img + SP, 7); bytes("fire rate now: ", img + FR, 7);
    int rel = *(int *)(img + TS + 1); u8 *relay = img + TS + 5 + rel; bytes("time relay:    ", relay, 14); hex("relay goes to ", *(u64 *)(relay + 6));
    { int d = 0; for (int i = 0; i < 0x2d00; i++) if (keep[i] != img[0x183a30 + i]) d++; hex("route-search bytes changed ", d); }
    { int d = 0; for (int i = 0; i < 0x200; i++) if (keep2[i] != img[0x9b200 + i]) d++; hex("crash-site bytes changed ", d); }
    { static const u32 R[4] = { 0x9bbbc, 0x9bbfe, 0x9bc43, 0x1864ff };
      for (int k = 0; k < 4; k++) { u8 *a = img + R[k]; if (a[0] != 0xe9) { bytes("site not patched: ", a, 8); continue; }
        u8 *r = a + 5 + *(int *)(a + 1); u8 *st = r; if (r[0] == 0xff && r[1] == 0x25) st = *(u8 **)(r + 6);
        hex("site ", R[k]); bytes("  stub a: ", st, 40); bytes("  stub b: ", st + 40, 40); } }
    Sleep(1500);
    ExitProcess(0);
}
