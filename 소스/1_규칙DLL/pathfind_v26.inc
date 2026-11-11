/* ==== v26k36: the game's unit route search, made fast - with the very same routes.
   What the measurements showed: when a day turns, one game thread runs the unit route search (official exe 0x183a30, an A* over the
   hex map) for up to 0.4-0.6 s while the screen waits for it. Most of that time goes into keeping its "cells still to look at" list:
   the game keeps it as a plain binary tree sorted by cost, and because new cells almost always cost more than the ones before, the
   tree grows into one long chain - every new cell walks the whole chain.
   What this does: the tree itself is left exactly as the game builds it (every link the same, node for node), but where in the tree a
   new cell goes, which cell is the cheapest, and where a cell sits when it is taken out are looked up in a side index (the cells
   grouped by cost, each group in arrival order; v26k36b - was a treap, 3-4x dearer) instead of walking the chain. Same tree, same order of cells, same route - only without the long walks.
   Safety:
   - the patch places are compared with the exact game code first (and the code around them), all or nothing;
   - the tree is checked when each search starts; anything unusual (a link left over from an earlier search, a node met twice, the
     order not as the game keeps it) = that search runs the game's own way (plain walks, written here exactly like the game code);
   - every operation: cheap checks of the place found (no left link at the cheapest, cost order at the insert place, the neighbour
     taken for a take-out is a rightmost cell) - v26k36c;
   - self-check: the first 20000 operations, then every 64th, are also done the game's own way and compared; one difference = the
     side index is switched off for the rest of the session (the game's own walks from then on) and it is written to the log. ==== */
#define PF_N 48400u                      /* hexes = nodes per search buffer (0xbd10) */
#define PF_NIL 0xFFFFFFFFu
#define PF_RVA_ENTRY GA(0x183a30u)
#define PF_RVA_POP GA(0x183d11u)
#define PF_RVA_POPRET GA(0x183d68u)
#define PF_RVA_RMCALL GA(0x1841f8u)
#define PF_RVA_RMFN GA(0x18ae60u)
#define PF_RVA_I1 GA(0x184228u)
#define PF_RVA_I1RET GA(0x1842e3u)
#define PF_RVA_I2 GA(0x184298u)
#define PF_RVA_I2RET GA(0x1842d9u)
#define PF_RVA_RCX GA(0x1343670u)
#define PF_RVA_BUF0 GA(0x1bb7b00u)
#define PF_BUFSTEP 0x1d8a80u
#define PF_STAMP_OFFICIAL 0x6a983c25u
#include "pathfind_spans.h"

#define PF_KCAP 8192u                    /* distinct costs in one open list at most (more = that search runs the game's own way) */
typedef struct { u32 call, idxcall, fast, n, nmax, nk;
    u32 *stamp, *parent, *nx, *pv, *stk; i32 *fk; i32 *kf; u32 *kh, *kt; } PFB;
static PFB pfb[3];
static u8 *pf_buf0;                      /* game + buffer 0 */
static volatile int pf_off;              /* 1 after a self-check difference: the game's own walks only */
static volatile u64 pf_n_calls, pf_n_fastcalls, pf_n_odd, pf_n_ops_fast, pf_n_ops_slow, pf_n_checks, pf_n_unknownbuf; static volatile u32 pf_maxn;
static u64 pf_next_log;
typedef void (GAMEABI *PFRemoveFn)(u8 **rootp, u8 *x);
static PFRemoveFn pf_orig_remove;
u64 pfk_ret_pop, pfk_ret_i1, pfk_ret_i2, pfk_ret_entry, pfk_rcx_const;

#define PL(n) (*(u8 **)(n))
#define PR(n) (*(u8 **)((u8 *)(n) + 8))
#define PF(n) (*(i32 *)((u8 *)(n) + 0x24))
#define PFLAG(n) (*(u8 *)((u8 *)(n) + 0x1c))

/* ---- the game's own way, written out instruction for instruction ---- */
static void pf_slow_insert(u8 **rootp, u8 *x)          /* 0x184228..0x18426b / 0x184298..0x1842d5 */
{
    u8 *a = *rootp; if (!a) { *rootp = x; return; }
    i32 f = PF(x);
    for (;;) { if (PF(a) <= f) { if (!PR(a)) { PR(a) = x; return; } a = PR(a); } else { if (!PL(a)) { PL(a) = x; return; } a = PL(a); } }
}
static u8 *pf_slow_pop(u8 **rootp)                      /* 0x183d11..0x183d64 */
{
    u8 *r15 = *rootp, *r12 = PL(r15);
    if (!r12) { r12 = r15; *rootp = PR(r15); PR(r12) = 0; PL(r12) = 0; return r12; }
    u8 *rcx = r15; while (PL(r12)) { rcx = r12; r12 = PL(r12); }
    PL(rcx) = PR(r12); PR(r12) = 0; PL(r12) = 0; return r12;
}
/* where the game's walk would put x (no change made): the node it hangs under and the side (1 = right) */
static u8 *pf_slow_where(u8 *root, i32 f, int *right)
{
    u8 *a = root;
    for (int g = 0; g < 200000; g++) { if (PF(a) <= f) { if (!PR(a)) { *right = 1; return a; } a = PR(a); } else { if (!PL(a)) { *right = 0; return a; } a = PL(a); } }
    *right = -1; return 0;
}

/* ---- the side index: the open cells grouped by cost. Each cost has a list of its cells in arrival order (a new cell always
   arrives last); the costs present are kept in a small sorted array. That is exactly the game tree's order: by cost, then arrival. ---- */
static PFB *pf_b(u8 *buf) { if (!pf_buf0) return 0; u64 d = (u64)(buf - pf_buf0); if (d % PF_BUFSTEP) return 0; d /= PF_BUFSTEP; return d < 3 && pfb[d].stamp ? &pfb[d] : 0; }
static u8 *pf_bufof(PFB *B) { return pf_buf0 + (u64)(B - pfb) * PF_BUFSTEP; }
#define PNODE(B, i) (pf_bufof(B) + (u64)(i) * 40)
static u32 pf_id(PFB *B, u8 *n) { u64 d = (u64)(n - pf_bufof(B)); if (d % 40) return PF_NIL; d /= 40; return d < PF_N ? (u32)d : PF_NIL; }
static int pf_kle(PFB *B, i32 f) { int lo = 0, hi = (int)B->nk - 1, r = -1; while (lo <= hi) { int m = (lo + hi) >> 1; if (B->kf[m] <= f) { r = m; lo = m + 1; } else hi = m - 1; } return r; }
static int pf_kins(PFB *B, u32 x, i32 f)            /* 0 = no room (too many different costs) */
{
    B->fk[x] = f; int k = pf_kle(B, f);
    if (k >= 0 && B->kf[k] == f) { u32 t = B->kt[k]; B->pv[x] = t; B->nx[x] = PF_NIL; B->nx[t] = x; B->kt[k] = x; }
    else { if (B->nk >= PF_KCAP) return 0; k++; for (int j = (int)B->nk; j > k; j--) { B->kf[j] = B->kf[j - 1]; B->kh[j] = B->kh[j - 1]; B->kt[j] = B->kt[j - 1]; }
        B->kf[k] = f; B->kh[k] = B->kt[k] = x; B->pv[x] = B->nx[x] = PF_NIL; B->nk++; }
    if (++B->n > B->nmax) B->nmax = B->n; return 1;
}
static void pf_kdel(PFB *B, u32 x)
{
    int k = pf_kle(B, B->fk[x]); u32 p = B->pv[x], q = B->nx[x];
    if (p != PF_NIL) B->nx[p] = q; else B->kh[k] = q;
    if (q != PF_NIL) B->pv[q] = p; else B->kt[k] = p;
    if (B->kh[k] == PF_NIL) { for (u32 j = (u32)k; j + 1 < B->nk; j++) { B->kf[j] = B->kf[j + 1]; B->kh[j] = B->kh[j + 1]; B->kt[j] = B->kt[j + 1]; } B->nk--; }
    B->n--;
}
static u32 pf_tmin(PFB *B) { return B->nk ? B->kh[0] : PF_NIL; }
static u32 pf_last_le(PFB *B, i32 f) { int k = pf_kle(B, f); return k >= 0 ? B->kt[k] : PF_NIL; }
static u32 pf_first_gt(PFB *B, i32 f) { int k = pf_kle(B, f) + 1; return k < (int)B->nk ? B->kh[k] : PF_NIL; }
static u32 pf_pred(PFB *B, u32 x) { if (B->pv[x] != PF_NIL) return B->pv[x]; int k = pf_kle(B, B->fk[x]); return k > 0 ? B->kt[k - 1] : PF_NIL; }
static int pf_member(PFB *B, u32 i) { return i != PF_NIL && B->stamp[i] == B->call; }

static void pf_mismatch(const char *what, i32 a, i32 b)
{
    if (pf_off) return; pf_off = 1;
    char m[200], *p = m; p = pstr(p, "route search: self-check difference at "); p = pstr(p, what); p = pstr(p, " ("); p = pint(p, a); p = pstr(p, " / "); p = pint(p, b);
    p = pstr(p, ") - the fast index is off for this session, the game's own way from now on (routes stay the game's)"); *p = 0; logs(m);
}
static int pf_checknow(void) { u64 n = pf_n_ops_fast; return n < 20000 || (n < 2000000 ? (n & 63) == 0 : (n & 1023) == 0); }

/* the tree as the game left it when this search began (its start cells): read it into the index, or decide this search runs plain */
static int pf_ready(PFB *B, u8 **rootp)
{
    if (B->idxcall == B->call) return B->fast;
    B->idxcall = B->call; B->fast = 0; B->nk = 0; B->n = 0;
    if (pf_off) return 0;
    u8 *root = *rootp; u32 sp = 0, cnt = 0; i32 lastf = 0; int have = 0; u8 *cur = root; u32 prev = PF_NIL;
    /* in-order walk with an explicit stack (4096 deep at most), parents noted, every node checked */
    if (cur) { u32 ci = pf_id(B, cur); if (ci == PF_NIL) { pf_n_odd++; return 0; } B->parent[ci] = PF_NIL; }
    for (;;) {
        while (cur) { u32 ci = pf_id(B, cur); if (ci == PF_NIL || B->stamp[ci] == B->call || sp >= 4096 || !(PFLAG(cur) & 1)) { pf_n_odd++; return 0; }
            B->stamp[ci] = B->call; B->stk[sp++] = ci;
            u8 *l = PL(cur); if (l) { u32 li = pf_id(B, l); if (li == PF_NIL) { pf_n_odd++; return 0; } B->parent[li] = ci; } cur = l; }
        if (!sp) break;
        u32 ci = B->stk[--sp]; u8 *n = PNODE(B, ci); i32 f = PF(n);
        if (have && f < lastf) { pf_n_odd++; return 0; }                                    /* not in cost order */
        if (PL(n) && (prev == PF_NIL || PF(PNODE(B, prev)) >= f)) { pf_n_odd++; return 0; }  /* left side must cost strictly less */
        have = 1; lastf = f; if (!pf_kins(B, ci, f)) { pf_n_odd++; return 0; } prev = ci; cnt++;
        u8 *r = PR(n); if (r) { u32 ri = pf_id(B, r); if (ri == PF_NIL) { pf_n_odd++; return 0; } B->parent[ri] = ci; } cur = r; }
    B->fast = 1; pf_n_fastcalls++; return 1;
}

/* ---- the three tree operations, called from the patch places ---- */
__attribute__((used)) u8 *pfk_pop(u8 *frame, u8 **rootp)
{
    PFB *B = pf_b(*(u8 **)(frame + 0x6f));
    if (B && pf_ready(B, rootp)) {
        u8 *root = *rootp; u32 m = pf_tmin(B);
        if (m == PF_NIL) { B->fast = 0; pf_n_odd++; goto slow; }
        u8 *mn = PNODE(B, m);
        if (pf_checknow()) { pf_n_checks++; u8 *w = root; while (PL(w)) w = PL(w); if (w != mn) { pf_mismatch("cheapest cell", (i32)m, (i32)pf_id(B, w)); B->fast = 0; goto slow; } }
        if (PL(mn) || PF(mn) != B->fk[m]) { pf_mismatch("cheapest cell has a left link", (i32)m, 0); B->fast = 0; goto slow; }   /* always (cheap) */
        if (mn == root) { u8 *r = PR(mn); *rootp = r; if (r) B->parent[pf_id(B, r)] = PF_NIL; }
        else { u32 p = B->parent[m]; u8 *pn = PNODE(B, p); if (p == PF_NIL || PL(pn) != mn) { pf_mismatch("cheapest cell parent", (i32)m, (i32)p); B->fast = 0; goto slow; }
            u8 *r = PR(mn); PL(pn) = r; if (r) B->parent[pf_id(B, r)] = p; }
        PR(mn) = 0; PL(mn) = 0; pf_kdel(B, m); B->stamp[m] = 0; pf_n_ops_fast++; return mn;
    }
    if (!B) pf_n_unknownbuf++;
slow:
    pf_n_ops_slow++; return pf_slow_pop(rootp);
}
__attribute__((used)) void pfk_insert(u8 *frame, u8 **rootp, u8 *x)
{
    PFB *B = pf_b(*(u8 **)(frame + 0x6f));
    if (B && pf_ready(B, rootp)) {
        u32 xi = pf_id(B, x);
        if (xi == PF_NIL || PL(x) || PR(x) || pf_member(B, xi)) { B->fast = 0; pf_n_odd++; goto slow; }   /* a left-over link: the game's way */
        u8 *root = *rootp; i32 f = PF(x);
        if (!root) { *rootp = x; B->parent[xi] = PF_NIL; }
        else {
            u8 *at; int right; u32 p = pf_last_le(B, f), ai;
            if (p != PF_NIL && !PR(PNODE(B, p))) { ai = p; right = 1; }
            else { u32 s = pf_first_gt(B, f); if (s == PF_NIL || PL(PNODE(B, s))) { pf_mismatch("insert place", (i32)xi, f); B->fast = 0; goto slow; } ai = s; right = 0; }
            at = PNODE(B, ai);
            if (right ? !(PF(at) <= f) : !(PF(at) > f)) { pf_mismatch("insert place cost", (i32)ai, f); B->fast = 0; goto slow; }   /* always (cheap) */
            if (pf_checknow()) { pf_n_checks++; int r2; u8 *w = pf_slow_where(root, f, &r2); if (w != at || r2 != right) { pf_mismatch("insert place", (i32)ai, (i32)pf_id(B, w)); B->fast = 0; goto slow; } }
            if (right) PR(at) = x; else PL(at) = x; B->parent[xi] = ai; }
        if (!pf_kins(B, xi, f)) { B->fast = 0; pf_n_odd++; }   /* too many different costs: the rest of this search the game's way */ B->stamp[xi] = B->call; pf_n_ops_fast++; return;
    }
    if (!B) pf_n_unknownbuf++;
slow:
    pf_n_ops_slow++; pf_slow_insert(rootp, x);
}
__attribute__((used)) void pfk_remove(u8 **rootp, u8 *x, u8 *frame)
{
    PFB *B = pf_b(*(u8 **)(frame + 0x6f));
    if (B && pf_ready(B, rootp)) {
        u32 xi = pf_id(B, x);
        if (!pf_member(B, xi)) { B->fast = 0; pf_n_odd++; goto slow; }           /* not in this search's tree: the game's own code decides */
        u8 *root = *rootp, *l = PL(x), *r = PR(x); u32 Ri = PF_NIL;
        if (l) { Ri = pf_pred(B, xi); if (Ri == PF_NIL || PR(PNODE(B, Ri)) || PF(PNODE(B, Ri)) > PF(x)) { pf_mismatch("take-out neighbour", (i32)xi, 0); B->fast = 0; goto slow; } }   /* always (cheap) */
        if (pf_checknow() && l) { pf_n_checks++; u8 *w = l; while (PR(w)) w = PR(w); if (w != PNODE(B, Ri)) { pf_mismatch("take-out neighbour", (i32)Ri, (i32)pf_id(B, w)); B->fast = 0; goto slow; } }
        if (root == x) {
            if (l) { *rootp = l; B->parent[pf_id(B, l)] = PF_NIL; u8 *Rn = PNODE(B, Ri); PR(Rn) = r; if (r) B->parent[pf_id(B, r)] = Ri; }
            else { *rootp = r; if (r) B->parent[pf_id(B, r)] = PF_NIL; }
        } else {
            u32 p = B->parent[xi]; if (p == PF_NIL) { pf_mismatch("take-out parent", (i32)xi, 0); B->fast = 0; goto slow; }
            u8 *pn = PNODE(B, p); int right = PF(pn) <= PF(x);                    /* the game picks the side by cost, like this */
            if ((right ? PR(pn) : PL(pn)) != x) { pf_mismatch("take-out side", (i32)xi, (i32)p); B->fast = 0; goto slow; }
            if (pf_checknow()) { pf_n_checks++; u8 *a = root, *q = 0; i32 f = PF(x);       /* the game's search for the parent */
                for (int g = 0; a && a != x && g < 200000; g++) { q = a; a = PF(a) <= f ? PR(a) : PL(a); }
                if (a != x || q != pn) { pf_mismatch("take-out parent", (i32)p, (i32)(q ? pf_id(B, q) : PF_NIL)); B->fast = 0; goto slow; } }
            u8 *sub = l ? l : r;
            if (right) PR(pn) = sub; else PL(pn) = sub;
            if (sub) B->parent[pf_id(B, sub)] = p;
            if (l) { u8 *Rn = PNODE(B, Ri); PR(Rn) = r; if (r) B->parent[pf_id(B, r)] = Ri; }
        }
        PL(x) = 0; PR(x) = 0; pf_kdel(B, xi); B->stamp[xi] = 0; pf_n_ops_fast++; return;
    }
    if (!B) pf_n_unknownbuf++;
slow:
    pf_n_ops_slow++; pf_orig_remove(rootp, x);
}
static void pf_log(void)
{
    char b[400], *p = b; p = pstr(p, "route search: searches "); p = pint(p, (i64)pf_n_calls); p = pstr(p, ", fast "); p = pint(p, (i64)pf_n_fastcalls);
    p = pstr(p, ", game's own way "); p = pint(p, (i64)pf_n_odd); p = pstr(p, " | operations fast "); p = pint(p, (i64)pf_n_ops_fast); p = pstr(p, " plain "); p = pint(p, (i64)pf_n_ops_slow);
    p = pstr(p, " | self-checks "); p = pint(p, (i64)pf_n_checks); p = pstr(p, pf_off ? " - DIFFERENCE FOUND, index off" : " all equal");
    p = pstr(p, " | largest open list "); { u32 mx = 0; for (int i = 0; i < 3; i++) if (pfb[i].nmax > mx) mx = pfb[i].nmax; p = pint(p, mx); }
    if (pf_n_unknownbuf) { p = pstr(p, " | other buffers "); p = pint(p, (i64)pf_n_unknownbuf); }
    p = pstr(p, " | route cost steps "); p = pint(p, (i64)rc_calls); p = pstr(p, " weighed "); p = pint(p, (i64)rc_mulled);
    p = pstr(p, " | empty route lists caught "); p = pint(p, (i64)rg_caught);
    *p = 0; logs(b);
}
__attribute__((used)) void pfk_entry(u8 *buf)
{
    pf_n_calls++;
    PFB *B = pf_b(buf); if (B) { B->call++; if (!B->call) B->call = 1; }
    u64 now = GetTickCount64(); if (now >= pf_next_log) { int first = pf_next_log == 0; pf_next_log = now + 60000; if (!first) pf_log_due = 1; }
}
/* the glue: entered by a jump from the patch place (the game's stack as it is there), back by a jump to the game code after */
__asm__(
    ".intel_syntax noprefix\n"
    ".text\n"
    ".globl pfk_stub_entry\n"
    "pfk_stub_entry:\n"                       /* at the function's first instruction: rsp = entry, args in rcx rdx r8 r9 + stack */
    "  push rcx\n  push rdx\n  push r8\n  push r9\n  sub rsp, 0x28\n"
    "  mov rcx, [rsp + 0x70]\n"                /* 5th argument (the node buffer): entry rsp + 0x28 */
    "  call pfk_entry\n"
    "  add rsp, 0x28\n  pop r9\n  pop r8\n  pop rdx\n  pop rcx\n"
    "  mov rax, rsp\n  mov dword ptr [rax + 0x20], r9d\n"   /* the two instructions the patch took */
    "  jmp qword ptr [rip + pfk_ret_entry]\n"
    ".globl pfk_stub_pop\n"
    "pfk_stub_pop:\n"                         /* in place of 0x183d11..0x183d64: r15 = root (not empty) */
    "  sub rsp, 0x20\n  mov rcx, rbp\n  lea rdx, [rsp + 0x68]\n  call pfk_pop\n  add rsp, 0x20\n"
    "  mov r12, rax\n  mov r15, [rsp + 0x48]\n  xor edi, edi\n  mov rcx, [rip + pfk_rcx_const]\n"
    "  jmp qword ptr [rip + pfk_ret_pop]\n"
    ".globl pfk_stub_i1\n"
    "pfk_stub_i1:\n"                          /* in place of 0x184228..0x18426b: rdi = node, cost already written */
    "  sub rsp, 0x20\n  mov rcx, rbp\n  lea rdx, [rsp + 0x68]\n  mov r8, rdi\n  call pfk_insert\n  add rsp, 0x20\n"
    "  mov r15, [rsp + 0x48]\n"
    "  jmp qword ptr [rip + pfk_ret_i1]\n"
    ".globl pfk_stub_i2\n"
    "pfk_stub_i2:\n"                          /* in place of 0x184298..0x1842d5 */
    "  sub rsp, 0x20\n  mov rcx, rbp\n  lea rdx, [rsp + 0x68]\n  mov r8, rdi\n  call pfk_insert\n  add rsp, 0x20\n"
    "  mov r15, [rsp + 0x48]\n"
    "  jmp qword ptr [rip + pfk_ret_i2]\n"
    ".globl pfk_stub_rm\n"
    "pfk_stub_rm:\n"                          /* the call at 0x1841f8 (rcx = &root, rdx = node): same call, with the frame added */
    "  mov r8, rbp\n"
    "  jmp pfk_remove\n"
    ".att_syntax prefix\n");
void pfk_stub_entry(void); void pfk_stub_pop(void); void pfk_stub_i1(void); void pfk_stub_i2(void); void pfk_stub_rm(void);

static int pf_span_ok(u32 rva, const u8 *want, u32 n, const char *what)
{
    u8 *a = game + rva; if (!readable(a, n)) return 0;
    for (u32 i = 0; i < n; i++) if (a[i] != want[i]) { char b[160], *p = b; p = pstr(p, "route search: game code at "); p = pstr(p, what); p = pstr(p, " is not as expected (another mod or version) - left as it is"); *p = 0; logs(b); return 0; }
    return 1;
}
static int install_pathfind(void)
{
    if (!gver || !gv_off) { logs("route search: not this game version - left as it is"); return 0; }
    /* v26k41: the 2026-11-05 update has the same code at other places; its bytes differ only in the relative fields (3 data, 4 calls) */
    const u8 *sp = gv_new ? PF_SPAN_POP_N : PF_SPAN_POP, *si = gv_new ? PF_SPAN_INS_N : PF_SPAN_INS, *sr = gv_new ? PF_SPAN_RM_N : PF_SPAN_RM, *se = gv_new ? PF_SPAN_ENTRY_N : PF_SPAN_ENTRY;
    if (!GA(0x183d0a) || !GA(0x1841b4) || !PF_RVA_RMFN || !PF_RVA_ENTRY || !PF_RVA_POP || !PF_RVA_POPRET || !PF_RVA_RMCALL || !PF_RVA_I1 || !PF_RVA_I1RET || !PF_RVA_I2 || !PF_RVA_I2RET || !PF_RVA_RCX || !PF_RVA_BUF0)
        { logs("route search: a place is missing from the version table - left as it is"); return 0; }
    if (!pf_span_ok(GA(0x183d0a), sp, sizeof(PF_SPAN_POP), "the take-cheapest part") || !pf_span_ok(GA(0x1841b4), si, sizeof(PF_SPAN_INS), "the insert part")
        || !pf_span_ok(PF_RVA_RMFN, sr, sizeof(PF_SPAN_RM), "the take-out function") || !pf_span_ok(PF_RVA_ENTRY, se, sizeof(PF_SPAN_ENTRY), "the start")) return 0;
    for (int i = 0; i < 3; i++) {
        u32 *m = (u32 *)VirtualAlloc(0, (u64)PF_N * 4 * 5 + 4096 * 4 + PF_KCAP * 4 * 3, 0x3000, 0x04); if (!m) { logs("route search: no memory - left as it is"); return 0; }
        PFB *B = &pfb[i]; B->stamp = m; B->parent = m + PF_N; B->nx = m + PF_N * 2; B->pv = m + PF_N * 3; B->fk = (i32 *)(m + PF_N * 4); B->stk = m + PF_N * 5;
        B->kf = (i32 *)(m + PF_N * 5 + 4096); B->kh = m + PF_N * 5 + 4096 + PF_KCAP; B->kt = m + PF_N * 5 + 4096 + PF_KCAP * 2;
        B->call = 1; B->idxcall = 0; B->nk = 0; }
    pf_buf0 = game + PF_RVA_BUF0; pf_orig_remove = (PFRemoveFn)(game + PF_RVA_RMFN);
    pfk_ret_entry = (u64)(game + PF_RVA_ENTRY + 7); pfk_ret_pop = (u64)(game + PF_RVA_POPRET); pfk_ret_i1 = (u64)(game + PF_RVA_I1RET); pfk_ret_i2 = (u64)(game + PF_RVA_I2RET);
    pfk_rcx_const = (u64)(game + PF_RVA_RCX);
    const struct { u32 rva, len; u8 op; void *dest; } P[5] = {
        { PF_RVA_ENTRY, 7, 0xE9, (void *)pfk_stub_entry }, { PF_RVA_POP, 6, 0xE9, (void *)pfk_stub_pop }, { PF_RVA_RMCALL, 5, 0xE8, (void *)pfk_stub_rm },
        { PF_RVA_I1, 5, 0xE9, (void *)pfk_stub_i1 }, { PF_RVA_I2, 5, 0xE9, (void *)pfk_stub_i2 } };
    S14_Hook h[5]; memset(h, 0, sizeof(h)); u8 *rl[5] = { 0 };
    for (int i = 0; i < 5; i++) { u8 *at = game + P[i].rva; h[i].target = at; h[i].length = P[i].len; memcpy(h[i].expected, at, P[i].len);
        rl[i] = api->MakeRelay(at, P[i].dest); if (!rl[i]) { for (int k = 0; k < i; k++) api->FreeRelay(rl[k]); logs("route search: no room for the jumps - left as it is"); return 0; }
        i64 dl = rl[i] - (at + 5); if (dl < -0x7fffffffLL || dl > 0x7fffffffLL) { for (int k = 0; k <= i; k++) api->FreeRelay(rl[k]); return 0; }
        h[i].replacement[0] = P[i].op; i32 dd = (i32)dl; memcpy(h[i].replacement + 1, &dd, 4); for (u32 k = 5; k < P[i].len; k++) h[i].replacement[k] = 0x90; }
    if (!api->InstallHooks("SAN14CE.SeasonRules.RouteSearch", h, 5)) { for (int k = 0; k < 5; k++) api->FreeRelay(rl[k]); return 0; }
    return 1;
}
