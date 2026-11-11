/* ---- v26k32: tile info (asked for). When the cursor rests on a cell, the game's own hex info panel (CInfoSummaryHex) shows two lines:
   line 1 the terrain, the 화계 and movement marks; line 2 the game's own free text (box 0xe) and the morale penalty (box 0xf, 사기감소).
   Those two lines are the game's and are never written. What is going on in the cell NOW goes on new lines BELOW them, each line its
   own text box made by the game's own panel code (see ti_rows_make), in the look of box 0xe (its style block copied):
     line 3   what is there and what it does to moving and to 화계:   깊은 눈   눈보라   이속 -37%   화계 불가
              (names in a fixed order: ground - 언 강 / 얕은 눈 / 깊은 눈 / 진흙 / 잿더미 / 가뭄 / 가뭄 근처, weather, fire, flood,
              quake, landslide; then 이속 -N% (everything together, the number the speed hook uses; 통행 불가 on a blocked landslide cell)
              and 화계 -20% / -50% / 불가)
     line 4   what it does every turn and the raised risks:   사기 -1/일   병력·사기 -10~15/턴 (70%)   해빙 병력·사기 -15 (25%)
              산사태 위험 ×3   산불 위험 ×1.5
   Every kind always on its own line. A line that would not fit is drawn a little smaller (down to 80%); only when line 3 would need
   less than 85% (four things at once in one cell - a quake on a burning dry slope in drizzle, say) its names go on a line of their own
   and 이속·화계 one line lower. The panel moves up by the new lines (it sits at the bottom of the screen), its backing grows with them.
   A cell with nothing going on leaves the panel exactly as the game made it.
   k30d/k31 wrote everything into box 0xe and asked it for several lines: it draws one line only (the line breaks are dropped) - so the
   words ran together and over the morale box. ---- */
typedef void (GAMEABI *TI_SetSize)(void *node, int w, int h);
typedef void (GAMEABI *TI_SetPos)(void *node, int x, int y);
typedef u64 (GAMEABI *TI_Fill)(void *panel);
typedef u8 *(GAMEABI *TI_Create)(void *owner, const char *name, int type, void *wh, int z);
typedef void (GAMEABI *TI_Attach)(void *parent, void *node, int yes, void *z);
typedef void (GAMEABI *TI_NSetText)(void *node, const u16 *s);
typedef void (GAMEABI *TI_FontSz)(void *node, int sz);
typedef int (GAMEABI *TI_VCount)(void *box);
typedef void (GAMEABI *TI_VReserve)(void *box, i64 n);
typedef i64 (GAMEABI *TI_VRoom)(void *box);
typedef struct { u32 stamp, fill, slot, setsize, setpos, create, attach, nsettext, fontsz; } TIVer;
static const TIVer TIV[3] = {
    { 0x6a6d9f72u, 0x928190, 0x14b8920, 0x92f870, 0x92f4c0, 0xad0240, 0xac5bf0, 0x9302f0, 0x930710 },   /* rollback: found from the official ones (whole functions, relative fields masked) */
    { 0x6a983c25u, 0x928ce0, 0x14b8980, 0x9303b0, 0x930000, 0xad0e20, 0xac64e0, 0x930e30, 0x931250 },   /* official: CInfoSummaryHex vtable slot 49 = its fill(hex at +0x178) */
    { 0x6ad06f53u, 0x92b190, 0x14bba10, 0x932940, 0x932590, 0xad3900, 0xac8d90, 0x9333c0, 0x9337e0 } }; /* v26k41: 2026-11-05 update (all the same functions, moved) */
/* the first bytes of the game functions called (the same in both builds) - no call unless the code is as found */
static const u8 TI_HEAD_CREATE[12] = { 0x4c, 0x8b, 0xdc, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48 };
static const u8 TI_HEAD_ATTACH[12] = { 0x48, 0x8b, 0xc4, 0x48, 0x89, 0x70, 0x18, 0x48, 0x89, 0x78, 0x20, 0x55 };
static const u8 TI_HEAD_NSET[12] = { 0x48, 0x85, 0xc9, 0x0f, 0x84, 0xec, 0x00, 0x00, 0x00, 0x4c, 0x8b, 0xdc };
static const u8 TI_HEAD_FONT[12] = { 0x48, 0x85, 0xc9, 0x0f, 0x84, 0xb2, 0x00, 0x00, 0x00, 0x48, 0x89, 0x5c };
static const u8 TI_HEAD_SETPOS[10] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x8b, 0xda, 0x4c, 0x8b, 0xd9 };
static const TIVer *tiv;
static TI_Fill ti_fill_orig;
static int ti_on, ti_ready, ti_dumped, ti_nlog, ti_fr_ok, ti_calls_ok;
#define TI_LEFT 0xe
#define TI_RIGHT 0xf
#define TI_ROWS 3

static u16 *ti_w(u16 *p, const u16 *s) { while (*s) *p++ = *s++; *p = 0; return p; }
static u16 *ti_n(u16 *p, int v) { u16 t[12]; int n = 0; if (v < 0) { *p++ = '-'; v = -v; } do { t[n++] = (u16)('0' + v % 10); v /= 10; } while (v); while (n) *p++ = t[--n]; *p = 0; return p; }

typedef struct { u16 nm[8][12]; int nn; u16 spd[16], fr[16]; u16 r4[6][40]; int n4; } TileTxt;
/* what is going on at cell c right now (game thread: the rules write these arrays there too). Every rule that acts on an army standing in
   the cell, or on 화계 there, has its line; one-off blows (a flood rising, a quake, a landslide or a fire coming down on an army) are told
   in the news when they strike. */
static int tile_text(int c, TileTxt *o)
{
    memset(o, 0, sizeof(*o)); if (c < 0 || c >= NCELL || !v2 || g_turn < 0 || !dat) return 0;
    int t = g_turn; u8 rt = rain[c] <= 3 ? rain[c] : 3; int sn = snowing[c] ? 1 : 0;
    /* where a new fire can start at all (fires_turn), on cracked dry ground or beside it: chance x1.5 */
    int fr15 = 0; if (fuelT && fuelT[c] && cCR[c] > 0 && !cSnow[c] && !ash[c] && !burning[c] && !flooded[c]) { fr15 = crack[c]; for (int k = 0; k < 6 && !fr15; k++) { int q = nb6(c, k); if (q >= 0 && crack[q]) fr15 = 1; } }
    /* the names: the ground (under flood water nothing of it shows - as the map draws it), the weather, the disasters */
    if (!flooded[c]) {
        if (mine[c] == 1) ti_w(o->nm[o->nn++], L"언 강");
        else { int cl = cover_lvl(c, t); if (cl) ti_w(o->nm[o->nn++], cl >= 2 ? L"깊은 눈" : L"얕은 눈"); }   /* the value the speed and the map use */
        if (mud[c]) ti_w(o->nm[o->nn++], L"진흙");
        if (ash[c]) ti_w(o->nm[o->nn++], L"잿더미");
        if (crack[c]) ti_w(o->nm[o->nn++], L"가뭄");
        else if (fr15) ti_w(o->nm[o->nn++], L"가뭄 근처");
    }
    static const u16 *RN[2][3] = { { L"가랑비", L"비", L"폭우" }, { L"진눈깨비", L"눈", L"눈보라" } };
    if (rt) ti_w(o->nm[o->nn++], RN[sn][rt - 1]);
    int fn = 0; if (burning[c]) { for (int i = 0; i < 12 && !fn; i++) { Fire *F = &fires[i]; if (!F->active) continue; for (int j = 0; j < F->n; j++) if (F->cells[j] == c) { fn = F->n; break; } } if (fn) ti_w(o->nm[o->nn++], fire_name(fn)); }
    if (flooded[c]) { static const u16 *FN[3] = { L"홍수", L"큰 홍수", L"대홍수" }; ti_w(o->nm[o->nn++], FN[(flooded[c] > 3 ? 3 : flooded[c]) - 1]); }
    if (quaked[c] & 3) { static const u16 *QN[3] = { L"지진", L"강진", L"대지진" }; ti_w(o->nm[o->nn++], QN[(quaked[c] & 3) - 1]); }
    if (mine[c] == 2) ti_w(o->nm[o->nn++], L"산사태");
    /* moving and 화계: the movement factor exactly as the speed hook applies it, the 화계 rate as FireRateHook does (rain and snow alike) */
    if (mine[c] == 2) ti_w(o->spd, L"통행 불가");                                       /* the landslide closed the cell */
    else if (speed_orig) { float f = spd_cell(c, t); int pct = (int)((1.0f - f) * 100.0f + 0.5f); if (pct > 0) { u16 *p = ti_w(o->spd, L"이속 -"); p = ti_n(p, pct); ti_w(p, L"%"); } }
    if (ti_fr_ok && rt) ti_w(o->fr, rt == 1 ? L"화계 -20%" : rt == 2 ? L"화계 -50%" : L"화계 불가");
    /* every turn: the rain's / snow's morale (rain_morale), the fire's blows (fire_hurt: 70% of the turns), the thaw under an army */
    /* v26k38k: the weather's morale as weather_morale does it (k38c): rain / snow -1 every second day, a downpour / snowstorm -1 a day,
       never below 50 (the old 사기 -1/턴, -3/턴 was the k5 rule) */
    if (rt >= 2) ti_w(o->r4[o->n4++], rt >= 3 ? L"사기 -1/일" : L"사기 -1/2일");
    /* v26k38k: a flooded cell - flood_entry: an army stepping in from dry ground (or caught by the rising water) loses once
       20 / 30 / 40% of its troops and morale; moving or staying in the water costs nothing more */
    if (flooded[c]) { int L = flooded[c] > 3 ? 3 : flooded[c]; u16 *p = ti_w(o->r4[o->n4], L"진입 시 병력·사기 -"); p = ti_n(p, L >= 3 ? 40 : L == 2 ? 30 : 20); ti_w(p, L"%"); o->n4++; }
    if (fn) { int tier = fire_tier(fn), lo = tier == 0 ? 5 : tier == 1 ? 10 : 15; u16 *p = ti_w(o->r4[o->n4], L"병력·사기 -"); p = ti_n(p, lo); p = ti_w(p, L"~"); p = ti_n(p, lo + 5); ti_w(p, L"/턴 (70%)"); o->n4++; }
    if (mine[c] == 1 && vterr[c] == T_GREATRIVER) {                                    /* rivers(): an army on the ice when it should melt - once a year, 25%: 15 / 15 */
        int thaw = (v2 && cur_turn == t) ? (!frz_cur(c) || !frz_next(c)) : (!frozen_wanted(t, c) || !frozen_wanted(t == 35 ? 0 : t + 1, c));
        if (thaw) ti_w(o->r4[o->n4++], L"해빙 병력·사기 -15 (25%)"); }
    /* the raised risks: burnt slope on a landslide road x3 (slides_day), dry ground where a fire can start x1.5 (fires_turn) */
    if (ash[c] && slidez && slidez[c] && !near_gate[c]) ti_w(o->r4[o->n4++], L"산사태 위험 ×3");
    if (fr15) ti_w(o->r4[o->n4++], L"산불 위험 ×1.5");
    return o->nn || o->spd[0] || o->fr[0] || o->n4;
}
/* the width of a line in the panel's units (1920 x 1080), the game's font as measured on its own screen: a Hangul syllable 16, a space 5,
   anything else 8 */
static int ti_est(const u16 *s) { int w = 0; for (; *s; s++) w += (*s >= 0xAC00 && *s <= 0xD7A3) ? 16 : *s == ' ' ? 5 : 8; return w; }
/* the lines (fixed kinds): 0..2 lines, each with its size factor */
static int ti_lines(TileTxt *T, u16 L[TI_ROWS][160], float sc[TI_ROWS], int W)
{
    static u16 names[160], ef[64], l3[224]; u16 *p = names; *p = 0;
    for (int i = 0; i < T->nn; i++) { if (i) p = ti_w(p, L"   "); p = ti_w(p, T->nm[i]); }
    p = ef; *p = 0; if (T->spd[0]) p = ti_w(p, T->spd); if (T->fr[0]) { if (p != ef) p = ti_w(p, L"   "); p = ti_w(p, T->fr); }
    p = l3; *p = 0; p = ti_w(p, names); if (ef[0]) { if (names[0]) p = ti_w(p, L"   "); p = ti_w(p, ef); }
    int n = 0;
    if (l3[0]) {
        if (names[0] && ef[0] && (float)ti_est(l3) * 0.85f > (float)W) { ti_w(L[n++], names); ti_w(L[n++], ef); }   /* too many things at once: the names alone */
        else ti_w(L[n++], l3); }
    if (T->n4 && n < TI_ROWS) { p = L[n]; *p = 0; for (int i = 0; i < T->n4; i++) { if (i) p = ti_w(p, L"   "); p = ti_w(p, T->r4[i]); } n++; }
    for (int i = 0; i < n; i++) { int e = ti_est(L[i]); float f = e > W ? (float)W / (float)e : 1.0f; sc[i] = f < 0.80f ? 0.80f : f; }
    return n;
}

/* ---- the panel's boxes (from the game's own record in SeasonRules.log, official exe): layout = panel+0x130 (+8 node array, +0x10
   count). 0 the root, 1 the backing (640,996 300x72), 2/6/10 the three badges (their children 3-5, 7-9, 11-13 sit in them), 14 = 0xe
   the free text of line 2 (654,1036 240x24), 15 = 0xf the morale penalty (782,1036 144x24). A node: +0x10 references, +0x88 flags,
   +0xd0 its parent, +0xe8 transform (+0x30 x, +0x34 y up), +0xf0 w / +0xf2 h, +0xf4/+0xf8 scale, +0x10c pivot, +0x112 kind (2 text),
   +0x104 flags, +0x248 its text style object (vtable 0x141689d28, the style block at +8, 0x78 bytes of plain values: colours, size
   factors +0x10/+0x14, alignment, draw way +0x2c ... - 0x140aec7d0 fills it). SetSize keeps the top-left corner, SetPos takes it. ---- */
static u8 *ti_node(u8 *panel, int i)
{
    u8 *L = panel + 0x130; if (!readable(L, 0x18)) return 0; int n = *(int *)(L + 0x10); u8 **a = *(u8 ***)(L + 8);
    if (i < 0 || i >= n || n > 400 || !readable(a + i, 8)) return 0; u8 *nd = a[i]; if (!readable(nd, 0x250)) return 0;
    if (!readable(*(u8 **)(nd + 0xe8), 0x40)) return 0; return nd;
}
static int ti_count(u8 *panel) { u8 *L = panel + 0x130; if (!readable(L, 0x18)) return 0; int n = *(int *)(L + 0x10); return n > 0 && n <= 400 ? n : 0; }
typedef struct { int l, t, w, h; } TIRect;
static TIRect ti_rect(u8 *nd)
{
    TIRect r; int w = *(short *)(nd + 0xf0), h = *(short *)(nd + 0xf2); u8 pv = nd[0x10c]; float sx = *(float *)(nd + 0xf4), sy = *(float *)(nd + 0xf8);
    u8 *tf = *(u8 **)(nd + 0xe8); float tx = *(float *)(tf + 0x30), ty = *(float *)(tf + 0x34);
    int ox = (int)((float)((pv >> 4) * w) * sx * 0.5f), oy = (int)((float)((pv & 15) * h) * sy * 0.5f);
    r.l = (int)tx - ox; r.t = -((int)ty + oy); r.w = w; r.h = h; return r;
}
static int ti_same(TIRect a, TIRect b) { return a.l == b.l && a.t == b.t && a.w == b.w && a.h == b.h; }
static void ti_set_rect(u8 *nd, TIRect want)
{
    TIRect cur = ti_rect(nd);
    if (cur.w != want.w || cur.h != want.h) ((TI_SetSize)(game + tiv->setsize))(nd, want.w, want.h);
    cur = ti_rect(nd); if (cur.l != want.l || cur.t != want.t) ((TI_SetPos)(game + tiv->setpos))(nd, want.l, want.t);
}
#define TI_MAXN 24
typedef struct { u8 *panel; int n, nmove; int mv[TI_MAXN]; TIRect base[TI_MAXN], set[TI_MAXN]; int applied, bg, badge;
                 u8 *row[TI_ROWS], *rparent, *rn14; int rfail, rshown[TI_ROWS]; float sx0, sy0; int rw, rh; } TIPanel;
static TIPanel tip[4];
static void ti_log_node(u8 *panel, int i, const char *why)
{
    u8 *nd = ti_node(panel, i); char b[300], *p = b;
    p = pstr(p, "tileinfo node "); p = pint(p, i); p = pstr(p, why);
    if (!nd) { p = pstr(p, " (none)"); *p = 0; logs(b); return; }
    TIRect r = ti_rect(nd);
    p = pstr(p, " kind "); p = pint(p, nd[0x112]); p = pstr(p, " left "); p = pint(p, r.l); p = pstr(p, " top "); p = pint(p, r.t); p = pstr(p, " w "); p = pint(p, r.w); p = pstr(p, " h "); p = pint(p, r.h);
    p = pstr(p, " flags "); p = phex(p, *(u32 *)(nd + 0x104)); p = pstr(p, " parent "); p = phex(p, (u32)(u64)*(u8 **)(nd + 0xd0));
    if (nd[0x112] == 2) { p = pstr(p, " style "); p = phex(p, (u32)(u64)*(u8 **)(nd + 0x248)); p = pstr(p, " node "); p = phex(p, (u32)(u64)nd); }
    *p = 0; logs(b);
}
static TIPanel *ti_panel(u8 *panel)
{
    for (int i = 0; i < 4; i++) if (tip[i].panel == panel) return &tip[i];
    TIPanel *P = 0; for (int i = 0; i < 4 && !P; i++) if (!tip[i].panel) P = &tip[i]; if (!P) return 0;
    memset(P, 0, sizeof(*P)); P->panel = panel; P->bg = -1; P->badge = -1;
    int n = ti_count(panel); if (n < 16) { P->panel = 0; return 0; } P->n = n;
    u8 *nl = ti_node(panel, TI_LEFT), *nr = ti_node(panel, TI_RIGHT); if (!nl || !nr) { P->panel = 0; return 0; }
    TIRect L0 = ti_rect(nl), R0 = ti_rect(nr);
    /* the boxes placed on the panel itself (screen-sized numbers; the badges' children have small local ones): they move up together */
    for (int i = 1; i < n && P->nmove < TI_MAXN; i++) { u8 *nd = ti_node(panel, i); if (!nd) continue; TIRect r = ti_rect(nd);
        if (r.t < L0.t - 200 || r.t > L0.t + 200 || r.l < L0.l - 200 || r.l > R0.l + R0.w + 200) continue;
        P->mv[P->nmove] = i; P->base[P->nmove] = r; P->nmove++;
        if (nd[0x112] != 2 && r.l <= L0.l && r.l + r.w >= R0.l + R0.w && r.t <= L0.t && r.t + r.h >= L0.t + L0.h && (P->bg < 0 || r.w * r.h > ti_rect(ti_node(panel, P->bg)).w * ti_rect(ti_node(panel, P->bg)).h)) P->bg = i;
        if (nd[0x112] != 2 && r.t + r.h <= L0.t && r.l <= L0.l + 20 && r.l + r.w > L0.l && r.l + r.w < R0.l && r.h < 60) P->badge = i; }
    if (!ti_dumped) { ti_dumped = 1; char b[200], *p = b; p = pstr(p, "tileinfo: panel boxes "); p = pint(p, n); p = pstr(p, ", moved together "); p = pint(p, P->nmove); p = pstr(p, ", backing "); p = pint(p, P->bg); p = pstr(p, ", terrain badge "); p = pint(p, P->badge);
        p = pstr(p, ", panel root (+0xa0) "); p = phex(p, readable(panel + 0xa0, 8) ? (u32)(u64)*(u8 **)(panel + 0xa0) : 0); *p = 0; logs(b);
        for (int i = 0; i < n && i < 40; i++) ti_log_node(panel, i, ""); }
    return P;
}
/* the stored geometry is the game's own unless the game itself changed a box since (then that is the new base) */
static void ti_rebase(TIPanel *P)
{
    for (int k = 0; k < P->nmove; k++) { u8 *nd = ti_node(P->panel, P->mv[k]); if (!nd) continue; TIRect cur = ti_rect(nd);
        if (!P->applied || !ti_same(cur, P->set[k])) P->base[k] = cur; }
}
static int ti_kidx(TIPanel *P, int node) { for (int k = 0; k < P->nmove; k++) if (P->mv[k] == node) return k; return -1; }

/* ---- the new lines: text boxes made the way the game's panel code makes them (official 0x14086f920, used by many windows to put a
   text on a panel): owner = [[panel+0x98]+0x10] (its node list at +0x18: count = vt+0x18, room = vt+0x60 as 0x14086f7c0 does), name =
   the next number, node = 0x140ad0e20(owner, name, 2 = text, {w,h}, 0); 0x140ac64e0(parent, node, 1, 0) hangs it under the parent (the
   parent's list +0x18, node+0xd0 = parent, one more reference); node+0x88 bit 0 off; 0x140930e30 its text; 0x140931250(node, 18) its
   size; 0x140930000 its place. Our boxes hang under the parent of box 0xe, so they use its numbers; the style block of 0xe is copied
   into theirs (the same look: font size, colour, outline). We keep the reference the maker hands back (the game frees nothing we
   hold); a panel made again (another 0xe box or parent) gets new boxes, the old ones are left alone. ---- */
static int ti_head(u32 rva, const u8 *h, int n)
{
#ifdef HOSTTEST
    (void)rva; (void)h; (void)n; return 1;            /* the host test puts its stand-ins there */
#else
    u8 *at = game + rva; if (!readable(at, n)) return 0; for (int i = 0; i < n; i++) if (at[i] != h[i]) return 0; return 1;
#endif
}
/* v26k38i: the text setter is called through a private copy of its first instructions, never through the game's own entry.
   The integrated patch (StatusColor R41) puts a jump 9 bytes into that function (game+0x930e39) for its table colours; going
   through it fed our lines into its colour logic, and the changed bytes made the season map switch its lines off. The copy runs
   the function's own first 21 bytes (as in the game files, both builds) and jumps back into the game right after them (+0x15),
   so whatever another mod does at +0x9..+0x14 is never touched - and the season map never touches it either. */
static TI_NSetText ti_nset;
static int ti_nset_make(void)
{
#ifdef HOSTTEST
    ti_nset = (TI_NSetText)(game + tiv->nsettext); return 1;
#else
    static const u8 O[21] = { 0x48, 0x85, 0xc9, 0x0f, 0x84, 0xec, 0x00, 0x00, 0x00, 0x4c, 0x8b, 0xdc, 0x53, 0x56, 0x48, 0x81, 0xec, 0x38, 0x08, 0x00, 0x00 };
    u8 *f = game + tiv->nsettext; if (!readable(f, 32)) return 0;
    for (int i = 0; i < 9; i++) if (f[i] != O[i]) return 0;                            /* test rcx,rcx; je +0xec: as found */
    if (f[21] != 0x48 || f[22] != 0x8b || f[23] != 0x05) return 0;                      /* where the copy comes back: mov rax,[rip+..] as found */
    int clean = 1; for (int i = 9; i < 21; i++) if (f[i] != O[i]) clean = 0;
    if (!clean && f[9] != 0xe9) return 0;                                               /* changed by something else than a plain jump: leave it */
    u8 *t = (u8 *)VirtualAlloc(0, 4096, 0x3000, 0x40); if (!t) return 0;
    memcpy(t, O, 21);                                                                   /* 0: the 21 bytes */
    *(i32 *)(t + 5) = 40 - 9;                                                           /* je -> the far jump at 40 */
    t[21] = 0xff; t[22] = 0x25; *(u32 *)(t + 23) = 0; *(u64 *)(t + 27) = (u64)(f + 21); /* jmp game+fn+0x15 */
    t[40] = 0xff; t[41] = 0x25; *(u32 *)(t + 42) = 0; *(u64 *)(t + 46) = (u64)(f + 9 + 0xec);   /* the je's target: game+fn+0xf5 */
    FlushInstructionCache(GetCurrentProcess(), t, 64);
    ti_nset = (TI_NSetText)t; return clean ? 1 : 2;
#endif
}
static int ti_in_game(void *p) { return (u8 *)p >= game && (u8 *)p < game + gv_image; }
/* a text box's own style block (the authoritative one - 0x140931250 writes the size there; the object at +0x248 is the drawing copy the
   game keeps from it - both are written): the box's own accessor, virtual +0x80 (text node 0x140accd10: node+0x198) */
typedef u8 *(GAMEABI *TI_Inner)(void *node, int which);
static u8 *ti_inner(u8 *nd)
{
    if (!nd || !readable(nd, 8)) return 0; void **vt = *(void ***)nd; if (!readable(vt + 16, 8) || !ti_in_game(vt[16])) return 0;
    u8 *s = ((TI_Inner)vt[16])(nd, 0); return s && readable(s, 0x78) ? s : 0;
}
static int ti_rows_ok(TIPanel *P, u8 *n14)
{
    if (!P->row[0] || P->rn14 != n14 || !readable(n14 + 0xd0, 8) || *(u8 **)(n14 + 0xd0) != P->rparent) return 0;
    for (int k = 0; k < TI_ROWS; k++) { u8 *r = P->row[k]; if (!r || !readable(r, 0x250) || *(void **)r != *(void **)n14 || *(u8 **)(r + 0xd0) != P->rparent) return 0; }
    return 1;
}
static int ti_rows_make(TIPanel *P, u8 *n14, int W, int H)
{
    u8 *panel = P->panel; char b[300], *p = b; p = pstr(p, "tileinfo: new lines ");
    memset(P->row, 0, sizeof(P->row)); memset(P->rshown, 0, sizeof(P->rshown)); P->rn14 = 0; P->rparent = 0;
    u8 *o98 = readable(panel + 0x98, 8) ? *(u8 **)(panel + 0x98) : 0; u8 *owner = o98 && readable(o98 + 0x10, 8) ? *(u8 **)(o98 + 0x10) : 0;
    u8 *parent = readable(n14 + 0xd0, 8) ? *(u8 **)(n14 + 0xd0) : 0;
    if (!owner || !readable(owner, 0x360) || !ti_in_game(*(void **)owner) || !parent || !readable(parent, 0x250) || !ti_in_game(*(void **)parent)) {
        p = pstr(p, "NOT made: no owner / parent"); *p = 0; logs(b); return 0; }
    u8 *box = owner + 0x18; void **bvt = readable(box, 8) ? *(void ***)box : 0;
    if (!bvt || !readable(bvt + 13, 8) || !ti_in_game(bvt[3]) || !ti_in_game(bvt[4]) || !ti_in_game(bvt[12])) { p = pstr(p, "NOT made: the owner's list is not as found"); *p = 0; logs(b); return 0; }
    u8 *f14 = readable(n14 + 0x248, 8) ? *(u8 **)(n14 + 0x248) : 0; if (!f14 || !readable(f14, 0x80) || !ti_inner(n14)) { p = pstr(p, "NOT made: box 0xe has no style"); *p = 0; logs(b); return 0; }
    int copied = 0;
    for (int k = 0; k < TI_ROWS; k++) {
        int cnt = ((TI_VCount)bvt[3])(box); i64 room = ((TI_VRoom)bvt[4])(box); if ((i64)cnt + 1 > room) ((TI_VReserve)bvt[12])(box, 8);   /* room for one more (vt+0x60 adds room), as 0x14086f7c0 does */
        char name[16], *q = name; q = pint(q, cnt + 1); *q = 0;
        struct { u16 w, h; } wh = { (u16)W, (u16)H };
        u8 *nd = ((TI_Create)(game + tiv->create))(owner, name, 2, &wh, 0);
        if (!nd || !readable(nd, 0x250)) { p = pstr(p, "NOT made: the game made no box (line "); p = pint(p, k + 3); p = pstr(p, ")"); *p = 0; logs(b); return 0; }
        ((TI_Attach)(game + tiv->attach))(parent, nd, 1, 0);
        *(u32 *)(nd + 0x88) &= ~1u;
        ti_nset(nd, L"");
        ((TI_FontSz)(game + tiv->fontsz))(nd, 0x12);
        u8 *src = ti_inner(n14), *dst = ti_inner(nd);
        if (src && dst) { memcpy(dst, src, 0x78); copied++;                                /* the look of box 0xe (the node's own block) */
            u8 *fo = readable(nd + 0x248, 8) ? *(u8 **)(nd + 0x248) : 0; if (fo && readable(fo, 0x80) && *(void **)fo == *(void **)f14) memcpy(fo + 8, src, 0x78); }   /* and its drawing copy
               (no 'changed' flag 0x2000000: that one makes the game apply the box's animation track, +0x320 - our boxes have none) */
        P->row[k] = nd; }
    { u8 *src = ti_inner(n14); P->sx0 = src ? *(float *)(src + 0x10) : 0.0f; P->sy0 = src ? *(float *)(src + 0x14) : 0.0f; }
    P->rn14 = n14; P->rparent = parent; P->rw = W; P->rh = H;
    p = pstr(p, "made "); p = pint(p, TI_ROWS); p = pstr(p, " (owner "); p = phex(p, (u32)(u64)owner); p = pstr(p, ", parent of 0xe "); p = phex(p, (u32)(u64)parent);
    p = pstr(p, ", style of 0xe copied "); p = pint(p, copied); p = pstr(p, "/"); p = pint(p, TI_ROWS); p = pstr(p, ", size "); p = pint(p, W); p = pstr(p, "x"); p = pint(p, H); p = pstr(p, ")"); *p = 0; logs(b);
    for (int k = 0; k < TI_ROWS; k++) { char c2[120], *s = c2; s = pstr(s, "tileinfo: line box "); s = pint(s, k + 3); s = pstr(s, " node "); s = phex(s, (u32)(u64)P->row[k]); s = pstr(s, " parent "); s = phex(s, (u32)(u64)*(u8 **)(P->row[k] + 0xd0));
        s = pstr(s, " style "); s = phex(s, (u32)(u64)*(u8 **)(P->row[k] + 0x248)); *s = 0; logs(c2); }
    return 1;
}
static u16 ti_rtxt[4][TI_ROWS][160]; static float ti_rsc[4][TI_ROWS]; static u8 *ti_rnd[4][TI_ROWS]; static u64 ti_rat[4][TI_ROWS];
static void ti_row_text(TIPanel *P, int k, const u16 *s, float scale)
{
    u8 *nd = P->row[k]; if (!nd) return;
    {   /* v26k35: the same text at the same size on the same box: nothing to do (the game was asked to lay it out again every frame) */
        int pi = (int)(P - tip); if (pi >= 0 && pi < 4) { u16 *o = ti_rtxt[pi][k]; u64 tnow = GetTickCount64(); int same = ti_rnd[pi][k] == nd && ti_rsc[pi][k] == scale && tnow - ti_rat[pi][k] < 1000; int i = 0;   /* written again once a second anyway */
            if (same) { for (; i < 159 && s[i]; i++) if (o[i] != s[i]) { same = 0; break; } if (same && o[i] != 0) same = 0; }
            if (same) { P->rshown[k] = s[0] != 0; return; }
            for (i = 0; i < 159 && s[i]; i++) o[i] = s[i]; o[i] = 0; ti_rnd[pi][k] = nd; ti_rsc[pi][k] = scale; ti_rat[pi][k] = tnow; } }
    u8 *in = ti_inner(nd), *fo = readable(nd + 0x248, 8) ? *(u8 **)(nd + 0x248) : 0;
    if (in && P->sx0 > 0.0f && P->sy0 > 0.0f) { float sx = P->sx0 * scale, sy = P->sy0 * scale;
        if (*(float *)(in + 0x10) != sx || *(float *)(in + 0x14) != sy) { *(float *)(in + 0x10) = sx; *(float *)(in + 0x14) = sy;
            if (fo && readable(fo, 0x80)) { *(float *)(fo + 8 + 0x10) = sx; *(float *)(fo + 8 + 0x14) = sy; } } }
    ti_nset(nd, s); P->rshown[k] = s[0] != 0;
}
static void ti_restore(TIPanel *P)
{
    if (!P->applied) return;
    for (int k = 0; k < P->nmove; k++) { u8 *nd = ti_node(P->panel, P->mv[k]); if (nd) ti_set_rect(nd, P->base[k]); }
    u8 *n14 = ti_node(P->panel, TI_LEFT);
    if (n14 && ti_rows_ok(P, n14)) for (int k = 0; k < TI_ROWS; k++) if (P->rshown[k]) ti_row_text(P, k, L"", 1.0f);
    P->applied = 0;
}
static TileTxt ti_tt; static int ti_last_c = -2, ti_nchg;
static void ti_apply(u8 *panel)
{
    TIPanel *P = ti_panel(panel); if (!P) return;
    u8 *nl = ti_node(panel, TI_LEFT); if (!nl) return;
    ti_rebase(P);
    int kL = ti_kidx(P, TI_LEFT), kR = ti_kidx(P, TI_RIGHT), kB = P->bg >= 0 ? ti_kidx(P, P->bg) : -1;
    if (kL < 0 || kR < 0) return;
    TIRect L0 = P->base[kL], R0 = P->base[kR];
    u8 *hex = *(u8 **)(panel + 0x178); u8 *d = *dbp(); u8 *h0 = d ? *(u8 **)(d + DB_HEX) : 0; int c = -1;
    if (hex && h0) { i64 o = hex - h0; if (o >= 0 && o < (i64)NCELL * 32 && !(o & 31)) c = (int)(o >> 5); }
    /* a save just loaded and its first day not run yet: what we hold is the old game's (the rules forget it and roll again on that day) */
    int fresh = 0; { u8 *date = d ? *(u8 **)(d + DB_DATE) : 0; if (date && readable(date + 0x450, 4)) { u32 now = *(u32 *)(date + 0x450); fresh = !last_day || now < last_day || now > last_day + 40; } }
    TileTxt *T = &ti_tt; if (c < 0 || fresh || !tile_text(c, T)) memset(T, 0, sizeof(*T));   /* cheap (the cell's own numbers); the costly part was the memory checks and the game calls */
    /* a line's room: from box 0xe's left edge to the backing's right edge less the same margin */
    int one = L0.h >= 12 && L0.h <= 80 ? L0.h : 24;
    int right = kB >= 0 ? P->base[kB].l + P->base[kB].w - (L0.l - P->base[kB].l) : R0.l + R0.w; int W = right - L0.l; if (W < 100 || W > 600) W = R0.l + R0.w - L0.l;
    static u16 L[TI_ROWS][160]; float sc[TI_ROWS]; int n = ti_lines(T, L, sc, W);
    if (c != ti_last_c) { ti_last_c = c;              /* every cell change with something to show (first 80 of a play) */
        if (n && ti_nchg < 80) { ti_nchg++; char b[700], *s2 = b; s2 = pstr(s2, "tileinfo hover cell "); s2 = pint(s2, c); s2 = pstr(s2, " lines "); s2 = pint(s2, n); s2 = pstr(s2, " |");
            for (int i = 0; i < n && s2 - b < 600; i++) { s2 = pstr(s2, " "); s2 = putf8(s2, L[i], 60); if (sc[i] < 1.0f) { s2 = pstr(s2, " (size "); s2 = pint(s2, (int)(sc[i] * 100.0f + 0.5f)); s2 = pstr(s2, "%)"); } s2 = pstr(s2, " /"); } *s2 = 0; logs(b); } }
    if (!n) { ti_restore(P); return; }
    if (!ti_rows_ok(P, nl)) {
        if (P->rfail >= 3) return;                    /* the game would not make them: the panel stays as the game made it */
        if (!ti_calls_ok || !ti_rows_make(P, nl, W, one)) { P->rfail++; return; } }
    /* the geometry: everything of the panel moves up by the new lines, the backing grows down to where it was (its bottom stays) */
    int add = one * n;
    for (int k = 0; k < P->nmove; k++) { u8 *nd = ti_node(panel, P->mv[k]); if (!nd) continue; TIRect w = P->base[k]; w.t -= add;
        if (k == kB) w.h = P->base[k].h + add;
        ti_set_rect(nd, w); P->set[k] = ti_rect(nd); }
    for (int k = 0; k < TI_ROWS; k++) {
        if (k < n) { TIRect rr = ti_rect(P->row[k]); int wl = L0.l, wt = L0.t - add + one * (k + 1); if (rr.l != wl || rr.t != wt) ((TI_SetPos)(game + tiv->setpos))(P->row[k], wl, wt); ti_row_text(P, k, L[k], sc[k]); }
        else if (P->rshown[k]) ti_row_text(P, k, L"", 1.0f); }
    P->applied = 1;
    static int lc = -2, ln = -1; if (c == lc && n == ln) return; lc = c; ln = n;   /* logged when the cell or the number of lines changes */
    if (ti_nlog < 20) { ti_nlog++; char b[1500], *s = b; s = pstr(s, "tileinfo cell "); s = pint(s, c); s = pstr(s, " lines "); s = pint(s, n); s = pstr(s, " up "); s = pint(s, add); s = pstr(s, " |");
        for (int i = 0; i < n && s - b < 1300; i++) { TIRect r = ti_rect(P->row[i]); s = pstr(s, " ["); s = pint(s, r.l); s = pstr(s, ","); s = pint(s, r.t); s = pstr(s, " "); s = pint(s, r.w); s = pstr(s, "x"); s = pint(s, r.h); s = pstr(s, "] "); s = putf8(s, L[i], 60); }
        *s = 0; logs(b);
        if (ti_nlog <= 2) { ti_log_node(panel, TI_LEFT, " after"); if (P->bg >= 0) ti_log_node(panel, P->bg, " after"); } }
}
static u64 GAMEABI TI_FillHook(void *panel)
{
    u8 *pn = (u8 *)panel;
    u64 r = ti_fill_orig(panel);
    if (!ti_on || !ti_ready || !readable(pn, 0x190)) return r;
    ti_apply(pn);
    return r;
}
/* v26k38a: what kept the panel lines off (for the start notice): the piece and its place in the game */
static const char *ti_bad; static u32 ti_bad_rva;
static int install_tileinfo(void)
{
    tiv = 0; for (int i = 0; i < 3; i++) if (gver && TIV[i].stamp == gver->stamp) tiv = &TIV[i];
    if (!tiv) { ti_bad = "ver"; return 0; }
    u8 *fill = game + tiv->fill; static const u8 fh[4] = { 0x40, 0x53, 0x41, 0x54 };   /* push rbx (rex); push r12 */
    ti_bad = "fill"; ti_bad_rva = tiv->fill;
    if (!readable(fill, 8)) return 0; for (int i = 0; i < 4; i++) if (fill[i] != fh[i]) return 0;
    u8 *slot = game + tiv->slot; ti_bad = "slot"; ti_bad_rva = tiv->slot; if (!readable(slot, 8) || *(u8 **)slot != fill) return 0;
    ti_bad = 0;
    /* the game functions the new lines are made with: called only where their code is as found */
    int hc = ti_head(tiv->create, TI_HEAD_CREATE, 12), ha = ti_head(tiv->attach, TI_HEAD_ATTACH, 12), hn = ti_nset_make(),
        hf = ti_head(tiv->fontsz, TI_HEAD_FONT, 12), hp = ti_head(tiv->setpos, TI_HEAD_SETPOS, 10);
    ti_calls_ok = hc && ha && hn && hf && hp;
    if (!hc) { ti_bad = "create"; ti_bad_rva = tiv->create; } else if (!ha) { ti_bad = "attach"; ti_bad_rva = tiv->attach; } else if (!hn) { ti_bad = "text"; ti_bad_rva = tiv->nsettext; }
    else if (!hf) { ti_bad = "font"; ti_bad_rva = tiv->fontsz; } else if (!hp) { ti_bad = "place"; ti_bad_rva = tiv->setpos; }
    { char b[200], *p = b; p = pstr(p, "tileinfo: function starts (1 as found, 2 another mod's jump inside - our own copy of the start used, 0 refused) create "); p = pint(p, hc); p = pstr(p, " attach "); p = pint(p, ha);
      p = pstr(p, " text "); p = pint(p, hn); p = pstr(p, " font "); p = pint(p, hf); p = pstr(p, " place "); p = pint(p, hp); *p = 0; logs(b); }
    ti_fill_orig = (TI_Fill)fill;
    S14_Hook h[1]; memset(h, 0, sizeof(h));
    h[0].target = slot; h[0].length = 8; { u64 o = (u64)fill, nw = (u64)(void *)TI_FillHook; memcpy(h[0].expected, &o, 8); memcpy(h[0].replacement, &nw, 8); }
    if (!api->InstallHooks("SAN14CE.SeasonRules.TileInfo", h, 1)) { ti_bad = "slot"; ti_bad_rva = tiv->slot; return 0; }
    { char b[120], *p = b; p = pstr(p, "tileinfo: game functions for the new lines "); p = pstr(p, ti_calls_ok ? "as found - lines below the morale line" : "NOT as found - the panel stays as the game made it"); *p = 0; logs(b); }
    ti_ready = 1; ti_on = 1; return 1;
}
