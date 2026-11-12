using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Security.Cryptography;
// v26k47: put the season mountain heights into a copy of the height data that another mod (the map-expand mod) appended
// to the same game file and pointed the game at. Only cells where that copy still holds the original game value are changed,
// the copy keeps its exact byte size (re-compressed 16 KB pieces, padded with empty deflate blocks, no piece above 16000 bytes).
public static class SeasonME {
 public static List<string> Log = new List<string>();
 public static string LastProbes = ""; public static int LastApplied, LastAlready, LastTheirs;   // v26k48: a few cells of the copy and their season / original values (the installer checks them without inflating it all)
 const int CAP = 16000;
 static uint U32(byte[] b, int o) { return (uint)(b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24)); }
 static void P32(byte[] b, int o, uint v) { b[o] = (byte)v; b[o + 1] = (byte)(v >> 8); b[o + 2] = (byte)(v >> 16); b[o + 3] = (byte)(v >> 24); }
 public static string Sha(byte[] b) { using (var s = SHA256.Create()) { byte[] h = s.ComputeHash(b); var c = new char[h.Length * 2]; const string x = "0123456789abcdef"; for (int i = 0; i < h.Length; i++) { c[i * 2] = x[h[i] >> 4]; c[i * 2 + 1] = x[h[i] & 15]; } return new string(c); } }
 public static string AssetId(byte[] cont) { if (cont.Length < 48 || cont[0] != 0x49 || cont[1] != 0x44 || cont[2] != 0x52 || cont[3] != 0x4b) return ""; return BitConverter.ToString(cont, 36, 8); }

 // ---- container: IDRK header, then [u32 length][zlib piece]... ended by a zero length ----
 sealed class Cont {
  public byte[] Bytes; public int Hdr, End; public List<int> Pos = new List<int>(); public List<int> Len = new List<int>(); public int CH;
  public Dictionary<int, byte[]> Raw = new Dictionary<int, byte[]>();
  public Cont(byte[] b) {
   Bytes = b; Hdr = (int)(U32(b, 8) - U32(b, 16)); int p = Hdr;
   while (true) { int L = (int)U32(b, p); if (L == 0) break; Pos.Add(p + 4); Len.Add(L); p += 4 + L; if (p > b.Length - 4) throw new InvalidDataException("자료 조각이 끝을 넘음"); }
   End = p; CH = Piece(0).Length;
  }
  public byte[] Piece(int k) { byte[] r; if (Raw.TryGetValue(k, out r)) return r; r = Inflate(Bytes, Pos[k], Len[k]); Raw[k] = r; return r; }
  public long RawLen { get { return U32(Bytes, 24); } }
  // read n bytes of the raw (uncompressed) data at raw offset o
  public byte[] ReadRaw(long o, int n) { var outb = new byte[n]; int got = 0; while (got < n) { long q = o + got; int k = (int)(q / CH); int w = (int)(q % CH); byte[] pc = Piece(k); int c = Math.Min(n - got, pc.Length - w); if (c <= 0) throw new InvalidDataException("자료 끝을 넘어 읽음"); Buffer.BlockCopy(pc, w, outb, got, c); got += c; } return outb; }
 }
 static byte[] Inflate(byte[] b, int off, int len) {
  using (var ms = new MemoryStream(b, off + 2, len - 6)) using (var ds = new DeflateStream(ms, CompressionMode.Decompress)) using (var o = new MemoryStream()) { ds.CopyTo(o); return o.ToArray(); }
 }
 static byte[] RawDeflate(byte[] d, int off, int n) {
  using (var o = new MemoryStream()) { using (var ds = new DeflateStream(o, CompressionLevel.Optimal, true)) { ds.Write(d, off, n); } return o.ToArray(); }
 }
 static uint Adler(byte[] d) { uint a = 1, b = 0; for (int i = 0; i < d.Length; i++) { a = (a + d[i]) % 65521; b = (b + a) % 65521; } return (b << 16) | a; }
 // zlib piece of data with exactly 'size' bytes (null when impossible): [78 9C][empty stored blocks][stored block of the first m bytes][deflate of the rest][adler32]
 static byte[] Exact(byte[] data, int size) {
  for (int m = 0; m <= 80 && m < data.Length; m++) {
   byte[] df = RawDeflate(data, m, data.Length - m);
   int fixedLen = 2 + (m > 0 ? 5 + m : 0) + df.Length + 4;
   int rest = size - fixedLen;
   if (rest < 0) { if (m == 0) return null; continue; }
   if (rest % 5 != 0) continue;
   var o = new byte[size]; int p = 0; o[p++] = 0x78; o[p++] = 0x9C;
   for (int e = 0; e < rest / 5; e++) { o[p++] = 0; o[p++] = 0; o[p++] = 0; o[p++] = 0xFF; o[p++] = 0xFF; }
   if (m > 0) { o[p++] = 0; o[p++] = (byte)m; o[p++] = (byte)(m >> 8); o[p++] = (byte)~m; o[p++] = (byte)((~m) >> 8); Buffer.BlockCopy(data, 0, o, p, m); p += m; }
   Buffer.BlockCopy(df, 0, o, p, df.Length); p += df.Length;
   uint ad = Adler(data); o[p++] = (byte)(ad >> 24); o[p++] = (byte)(ad >> 16); o[p++] = (byte)(ad >> 8); o[p++] = (byte)ad;
   return o;
  }
  return null;
 }
 static byte[] Zlib(byte[] data) { byte[] df = RawDeflate(data, 0, data.Length); var o = new byte[df.Length + 6]; o[0] = 0x78; o[1] = 0x9C; Buffer.BlockCopy(df, 0, o, 2, df.Length); uint ad = Adler(data); int p = o.Length - 4; o[p] = (byte)(ad >> 24); o[p + 1] = (byte)(ad >> 16); o[p + 2] = (byte)(ad >> 8); o[p + 3] = (byte)ad; return o; }

 // where the height grid starts in the raw data and its width: kind 0 = MHK heights (u32, 32-byte head), 1 = G1T texture 0 (float, 28-byte head)
 static void Grid(Cont c, int kind, out long start, out int width) {
  if (kind == 0) { start = 32; width = (int)Math.Round(Math.Sqrt((c.RawLen - 32) / 4.0)); }
  else {
   byte[] h = c.ReadRaw(0, 64); uint to = U32(h, 0xc); byte[] t = c.ReadRaw(to, 8);
   long o0 = U32(t, 0) + to, o1 = U32(t, 4) + to; start = o0 + 28; width = (int)Math.Round(Math.Sqrt((o1 - o0 - 28) / 4.0));
  }
  if ((long)width * width * 4 + start > c.RawLen) throw new InvalidDataException("높이 판이 자료보다 큼");
 }
 static uint[] ReadGrid(Cont c, long start, int w) { byte[] b = c.ReadRaw(start, w * w * 4); var g = new uint[w * w]; Buffer.BlockCopy(b, 0, g, 0, b.Length); return g; }
 static uint Round64(uint bits) { float f = BitConverter.ToSingle(BitConverter.GetBytes(bits), 0); float r = (float)(Math.Round(f * 64.0) / 64.0); return BitConverter.ToUInt32(BitConverter.GetBytes(r), 0); }

 // mode 1 = install (orig -> season), 2 = check only, 3 = take the season back out (season -> orig: uninstall when the other mod's copy got
 // the season cells some other way, with no backup of ours). Returns the new container bytes (same length) or null when nothing to do / not possible.
 public static byte[] Merge(byte[] meCont, byte[] origCont, byte[] newCont, int kind, int mode, out string report) {
  report = ""; LastProbes = ""; LastApplied = 0; LastAlready = 0; LastTheirs = 0;
  var me = new Cont(meCont); var co = new Cont(origCont); var cn = new Cont(newCont);
  long so, sn, sm; int wo, wn, wm;
  Grid(co, kind, out so, out wo); Grid(cn, kind, out sn, out wn); Grid(me, kind, out sm, out wm);
  if (wo != wn) throw new InvalidDataException("순정·계절 높이 판 크기가 다름");
  uint[] go = ReadGrid(co, so, wo), gn = ReadGrid(cn, sn, wn);
  uint[] gr = (uint[])gn.Clone();   // v26k48: the season values as the season file has them (before rounding)
  if (kind == 1) for (int i = 0; i < gn.Length; i++) if (gn[i] != go[i]) gn[i] = Round64(gn[i]);
  // find where the original map sits inside the bigger grid: look for a 64-value run of the original
  int dr = -1, dc = -1;
  // probes: 64-value runs the season never changed (so they look the same before and after this merge) and that are not flat
  var probes = new List<int[]>();
  for (int r = wo / 7; r < wo - 1 && probes.Count < 8; r += 97) for (int c = wo / 9; c + 64 < wo && probes.Count < 8; c += 89) {
   int b0 = r * wo + c; bool keep = true, flat = true;
   for (int j = 0; j < 64; j++) { if (go[b0 + j] != gn[b0 + j]) { keep = false; break; } if (go[b0 + j] != go[b0]) flat = false; }
   if (keep && !flat) probes.Add(new int[] { r, c });
  }
  uint[] gm = ReadGrid(me, sm, wm);
  foreach (int[] pr in probes) {
   int r0 = pr[0], c0 = pr[1]; int baseI = r0 * wo + c0;
   for (int i = 0; i + 64 <= gm.Length && dr < 0; i++) {
    if (gm[i] != go[baseI]) continue; bool ok = true;
    for (int j = 1; j < 64; j++) if (gm[i + j] != go[baseI + j]) { ok = false; break; }
    if (ok) { int rr = i / wm - r0, cc = i % wm - c0; if (rr >= 0 && cc >= 0 && rr + wo <= wm && cc + wo <= wm) { dr = rr; dc = cc; } }
   }
   if (dr >= 0) break;
  }
  if (dr < 0) { report = "큰 지도 안에서 순정 땅 자리를 못 찾아 그대로 둠"; return null; }
  int same = 0, tot = 0; for (int r = 0; r < wo; r += 37) for (int cc2 = 0; cc2 < wo; cc2 += 41) { tot++; if (gm[(r + dr) * wm + cc2 + dc] == go[r * wo + cc2]) same++; }
  if (same * 3 < tot) { report = "큰 지도가 순정 땅과 " + same + "/" + tot + " 곳만 같아 그대로 둠"; return null; }
  int applied = 0, already = 0, theirs = 0, nseas = 0; var touched = new SortedSet<int>();
  var pb = new List<long>(); var ps = new List<uint>(); var po = new List<uint>();   // every 4096th cell that ends as season: probe candidates
  for (int r = 0; r < wo; r++) for (int c = 0; c < wo; c++) {
   int i = r * wo + c; if (go[i] == gn[i]) continue;
   int j = (r + dr) * wm + c + dc; uint v = gm[j]; uint tv;
   long bp = sm + 4L * j;
   if (mode == 3) {
    if (v == go[i]) { already++; continue; }
    if (v != gn[i] && v != gr[i]) { theirs++; continue; }
    tv = go[i];
   } else {
    if (v != gn[i] && v != go[i]) { theirs++; continue; }
    if ((nseas++ & 4095) == 0 && (bp % me.CH) + 4 <= me.CH) { pb.Add(bp); ps.Add(gn[i]); po.Add(go[i]); }
    if (v == gn[i]) { already++; continue; }
    tv = gn[i];
   }
   applied++;
   int k = (int)(bp / me.CH); int w = (int)(bp % me.CH);
   if (mode == 1 || mode == 3) {
    byte[] pc = me.Piece(k);
    if (w + 4 <= pc.Length) { P32(pc, w, tv); touched.Add(k); }
    else { // value split over two pieces
     byte[] nb = BitConverter.GetBytes(tv); for (int q = 0; q < 4; q++) { long bq = bp + q; int kq = (int)(bq / me.CH), wq = (int)(bq % me.CH); me.Piece(kq)[wq] = nb[q]; touched.Add(kq); }
    }
   }
  }
  if (pb.Count > 0) {   // 24 of them, evenly spread: "P<piece size>:<raw offset>=<season hex>/<original hex>,..."
   var sbp = new System.Text.StringBuilder(); sbp.Append("P").Append(me.CH).Append(":"); int want = Math.Min(24, pb.Count);
   for (int t = 0; t < want; t++) { int q = (int)((long)t * pb.Count / want); if (t > 0) sbp.Append(","); sbp.Append(pb[q]).Append("=").Append(ps[q].ToString("x8")).Append("/").Append(po[q].ToString("x8")); }
   LastProbes = sbp.ToString();
  }
  LastApplied = applied; LastAlready = already; LastTheirs = theirs;
  if (mode == 3) report = "순정 땅 자리 행 +" + dr + " 열 +" + dc + " (" + wm + "판) | 산세 뺌 " + applied + "칸, 이미 순정 " + already + "칸, 맵 확장이 바꾼 칸 " + theirs + "칸은 그대로";
  else report = "순정 땅 자리 행 +" + dr + " 열 +" + dc + " (" + wm + "판) | 산세 넣음 " + applied + "칸, 이미 들어 있음 " + already + "칸, 맵 확장이 바꾼 칸 " + theirs + "칸은 그대로";
  if (mode == 2 || applied == 0) return null;
  // re-compress the touched pieces, then pad back to the exact old size
  int n = me.Len.Count; var pieces = new byte[n][]; long oldSum = 0, newSum = 0;
  for (int k = 0; k < n; k++) { oldSum += me.Len[k]; if (touched.Contains(k)) { pieces[k] = Zlib(me.Raw[k]); if (me.Raw[k].Length != me.CH && k != n - 1) throw new InvalidDataException("조각 크기가 고르지 않음"); } newSum += touched.Contains(k) ? pieces[k].Length : me.Len[k]; }
  long slack = oldSum - newSum;
  if (slack < 0) {
   // squeeze untouched pieces (largest first) - only helps if the other mod packed them loosely
   var order = new List<int>(); for (int k = 0; k < n; k++) if (!touched.Contains(k)) order.Add(k);
   order.Sort(delegate(int a, int b) { return me.Len[b].CompareTo(me.Len[a]); });
   foreach (int k in order) { if (slack >= 0) break; byte[] z = Zlib(me.Piece(k)); if (z.Length < me.Len[k]) { slack += me.Len[k] - z.Length; pieces[k] = z; touched.Add(k); } }
   if (slack < 0) { report += " | 같은 크기 안에 다시 묶이지 않아(" + slack + "바이트) 그대로 둠"; return null; }
  }
  foreach (int k in new List<int>(touched)) {
   if (slack == 0) break;
   int cur = pieces[k].Length; int room = CAP - cur; if (room <= 0) continue;
   int add = (int)Math.Min(slack, room);
   byte[] ex = null; for (int a = add; a >= Math.Max(0, add - 12) && ex == null; a--) { if (a == 0) break; ex = Exact(me.Raw[k], cur + a); }
   if (ex == null) continue; slack -= ex.Length - cur; pieces[k] = ex;
  }
  if (slack != 0) { report += " | 크기를 정확히 맞추지 못해(" + slack + ") 그대로 둠"; return null; }
  var outb = new byte[meCont.Length];
  Buffer.BlockCopy(meCont, 0, outb, 0, me.Hdr); int p = me.Hdr;
  for (int k = 0; k < n; k++) {
   if (pieces[k] == null) { Buffer.BlockCopy(meCont, me.Pos[k] - 4, outb, p, 4 + me.Len[k]); p += 4 + me.Len[k]; }
   else { if (pieces[k].Length > CAP) throw new InvalidDataException("조각이 16000바이트를 넘음"); P32(outb, p, (uint)pieces[k].Length); Buffer.BlockCopy(pieces[k], 0, outb, p + 4, pieces[k].Length); p += 4 + pieces[k].Length; }
  }
  if (p != me.End) throw new InvalidDataException("다시 묶은 길이가 다름");
  Buffer.BlockCopy(meCont, me.End, outb, p, meCont.Length - me.End);
  // verify: every rewritten piece inflates back to the intended data, the rest is untouched
  var chk = new Cont(outb);
  if (chk.Len.Count != n) throw new InvalidDataException("조각 수가 달라짐");
  foreach (int k in touched) { byte[] a = Inflate(outb, chk.Pos[k], chk.Len[k]); byte[] b = me.Raw[k]; if (a.Length != b.Length) throw new InvalidDataException("확인 실패(길이)"); for (int q = 0; q < a.Length; q++) if (a[q] != b[q]) throw new InvalidDataException("확인 실패(내용)"); }
  report += " | 조각 " + touched.Count + "개 다시 묶음, 크기 그대로";
  return outb;
 }
}

