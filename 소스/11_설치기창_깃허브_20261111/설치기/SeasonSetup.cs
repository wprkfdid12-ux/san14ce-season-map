// 삼국지14 CE 계절 맵 설치기 (2026-11-11 첫 판)
//  - 사용자 PC 의 .NET Framework 4.x C# 컴파일러(csc.exe, C# 5)로 Build.ps1 이 매번 컴파일해서 실행한다.
//  - 설치·제거는 검증된 tools\Season.ps1 / tools\Terrain26.ps1 을 그대로 부른다(게임 폴더를 -GameDir 로 넘김).
//  - 판 번호는 game\SAN14Mods\SeasonRules\BUILD.txt 의 첫 낱말(v26k45 등)에서 읽는다. 새 판이면 창 제목·큰 제목·상태 줄이 저절로 바뀜.
//  - 깃허브 저장소 updates/index.json(판 목록)을 켤 때 한 번 읽어 새 판·되돌리기를 보여 준다(계속 읽지 않음).
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Text;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Net;
using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Web.Script.Serialization;
using System.Windows.Forms;
using Microsoft.Win32;

namespace San14Season {

static class Program {
 public static string GameArg = "", AfterUpdate = "";
 [STAThread] static void Main(string[] args) {
  for (int i = 0; i + 1 < args.Length; i++) {
   if (args[i] == "--game") GameArg = args[i + 1].Trim('"');
   else if (args[i] == "--after-update") AfterUpdate = args[i + 1].Trim('"');
  }
  bool first;
  using (var mutex = new Mutex(true, "Local\\SAN14CE_SeasonMap_Setup", out first)) {
   if (!first && AfterUpdate != "") { try { first = mutex.WaitOne(30000); } catch (AbandonedMutexException) { first = true; } }
   if (!first) { MessageBox.Show("계절 맵 설치기가 이미 열려 있습니다. 열려 있는 창을 닫고 다시 실행해 주세요.", "계절 맵 설치기"); return; }
   Application.EnableVisualStyles();
   Application.SetCompatibleTextRenderingDefault(false);
   try { Pkg.Load(); Application.Run(new MainForm()); }
   catch (Exception e) { ErrorBox.Show(null, "계절 맵 설치기", ErrorBox.Describe("설치기를 여는 중", null, e, "ZIP 을 새 폴더에 전부 풀었는지, 백신이 파일을 막지 않는지 확인해 주세요.")); }
  }
 }
}

// 글자를 복사할 수 있는 오류·안내 창 (댓글용)
static class ErrorBox {
 public static string Describe(string doing, string file, Exception e, string fix) {
  var b = new StringBuilder();
  b.AppendLine("[계절 맵 " + Pkg.Version + " 설치기]");
  if (!String.IsNullOrEmpty(doing)) b.AppendLine("무엇을 하다가: " + doing);
  if (!String.IsNullOrEmpty(file)) b.AppendLine("어떤 파일: " + file);
  if (e != null) {
   b.AppendLine("오류: " + e.Message);
   if (fix == null && e is UnauthorizedAccessException) fix = "쓰기 권한이 없습니다. 게임 폴더·설치기 폴더 권한을 확인하거나 설치.cmd 를 관리자 권한으로 실행해 주세요.";
   if (fix == null && e is WebException) fix = "인터넷 연결을 확인한 뒤 다시 눌러 주세요. 설치·제거는 인터넷 없이도 됩니다.";
  }
  if (!String.IsNullOrEmpty(fix)) b.AppendLine("해결: " + fix);
  b.AppendLine();
  b.AppendLine("이 글자를 그대로 복사해 댓글로 알려 주시면 원인을 확인할 수 있습니다.");
  return b.ToString();
 }
 public static void Show(IWin32Window owner, string title, string text) {
  using (var f = new Form()) {
   f.Text = title; f.ClientSize = new Size(720, 380); f.StartPosition = owner != null ? FormStartPosition.CenterParent : FormStartPosition.CenterScreen;
   f.MinimizeBox = false; f.MaximizeBox = false; f.ShowInTaskbar = owner == null; f.Font = Theme.Ui(10f); f.BackColor = Theme.Paper;
   var box = new TextBox(); box.Multiline = true; box.ReadOnly = true; box.ScrollBars = ScrollBars.Vertical; box.Dock = DockStyle.Fill; box.Text = text; box.BackColor = Theme.Note;
   var panel = new FlowLayoutPanel(); panel.Dock = DockStyle.Bottom; panel.Height = 48; panel.FlowDirection = FlowDirection.RightToLeft; panel.Padding = new Padding(6);
   var close = Theme.Btn("닫기", false); var copy = Theme.Btn("이 내용 복사", false);
   close.Click += delegate { f.Close(); };
   copy.Click += delegate { try { Clipboard.SetText(text); copy.Text = "복사했습니다"; } catch (Exception) { } };
   panel.Controls.Add(close); panel.Controls.Add(copy);
   f.Controls.Add(box); f.Controls.Add(panel); f.AcceptButton = close;
   if (owner != null) f.ShowDialog(owner); else f.ShowDialog();
  }
 }
 // 선택지 버튼이 있는 물음 창: 누른 버튼 번호(0~), 창을 닫으면 -1
 public static int Ask(IWin32Window owner, string title, string text, string[] choices) {
  int result = -1;
  using (var f = new Form()) {
   f.Text = title; f.ClientSize = new Size(640, 270); f.StartPosition = FormStartPosition.CenterParent;
   f.MinimizeBox = false; f.MaximizeBox = false; f.ShowInTaskbar = false; f.Font = Theme.Ui(10f); f.FormBorderStyle = FormBorderStyle.FixedDialog; f.BackColor = Theme.Paper;
   var label = new Label(); label.Text = text; label.Dock = DockStyle.Fill; label.Padding = new Padding(16); label.AutoSize = false; label.ForeColor = Theme.Ink;
   var panel = new FlowLayoutPanel(); panel.Dock = DockStyle.Bottom; panel.Height = 54; panel.FlowDirection = FlowDirection.RightToLeft; panel.Padding = new Padding(8);
   for (int i = choices.Length - 1; i >= 0; i--) {
    int k = i; var b = Theme.Btn(choices[i], i == 0);
    b.Click += delegate { result = k; f.Close(); };
    panel.Controls.Add(b);
   }
   f.Controls.Add(label); f.Controls.Add(panel);
   if (owner != null) f.ShowDialog(owner); else f.ShowDialog();
  }
  return result;
 }
}

// 포스터(소개글 1번)의 색·글꼴
static class Theme {
 public static readonly Color Paper = Color.FromArgb(243, 239, 228), Card = Color.FromArgb(251, 249, 243), Note = Color.FromArgb(255, 253, 245);
 public static readonly Color Ink = Color.FromArgb(29, 26, 22), Sub = Color.FromArgb(93, 87, 77), Mute = Color.FromArgb(106, 100, 88);
 public static readonly Color Line = Color.FromArgb(217, 210, 195), Line2 = Color.FromArgb(231, 225, 212), Head = Color.FromArgb(240, 235, 223);
 public static readonly Color Accent = Color.FromArgb(138, 58, 31), Accent2 = Color.FromArgb(180, 83, 42), Ok = Color.FromArgb(61, 107, 58);
 public static readonly Color Warn = Color.FromArgb(165, 101, 26), Bad = Color.FromArgb(170, 40, 30), Sel = Color.FromArgb(246, 233, 220), Foot = Color.FromArgb(239, 233, 220);
 public static readonly Color BtnLine = Color.FromArgb(143, 135, 120);
 static string uiName, serifName; static HashSet<string> fontNames;
 static bool Has(string name) {
  if (fontNames == null) {
   fontNames = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
   try { using (var c = new InstalledFontCollection()) { foreach (var f in c.Families) fontNames.Add(f.Name); } } catch (Exception) { }
  }
  return fontNames.Contains(name);
 }
 static string UiName { get { if (uiName == null) uiName = Has("Malgun Gothic") ? "Malgun Gothic" : (Has("맑은 고딕") ? "맑은 고딕" : SystemFonts.MessageBoxFont.Name); return uiName; } }
 static string SerifName {
  get {
   if (serifName == null) {
    serifName = UiName;
    foreach (string n in new[] { "Noto Serif KR", "Noto Serif CJK KR", "Batang", "바탕", "Gungsuh" }) if (Has(n)) { serifName = n; break; }
   }
   return serifName;
  }
 }
 public static Font Ui(float pt) { return new Font(UiName, pt); }
 public static Font UiB(float pt) { return new Font(UiName, pt, FontStyle.Bold); }
 public static Font Serif(float pt) { return new Font(SerifName, pt, FontStyle.Bold); }
 public static Button Btn(string text, bool primary) {
  var b = new Button();
  b.Text = text; b.AutoSize = true; b.AutoSizeMode = AutoSizeMode.GrowOnly; b.Height = 36; b.MinimumSize = new Size(0, 36);
  b.Padding = new Padding(10, 0, 10, 0); b.Margin = new Padding(0, 4, 8, 4);
  b.FlatStyle = FlatStyle.Flat; b.Font = UiB(10f); b.Cursor = Cursors.Hand; b.UseVisualStyleBackColor = false;
  SetKind(b, primary ? 1 : 0);
  return b;
 }
 // 0 = 보통, 1 = 검은 주 버튼, 2 = 갈색 강조(새 판)
 public static void SetKind(Button b, int kind) {
  if (kind == 1) { b.BackColor = Ink; b.ForeColor = Color.White; b.FlatAppearance.BorderColor = Ink; }
  else if (kind == 2) { b.BackColor = Accent; b.ForeColor = Color.White; b.FlatAppearance.BorderColor = Accent; }
  else { b.BackColor = Card; b.ForeColor = Ink; b.FlatAppearance.BorderColor = BtnLine; }
  b.FlatAppearance.BorderSize = 1;
 }
 public static GraphicsPath Round(Rectangle r, int rad) {
  var p = new GraphicsPath(); int d = rad * 2;
  if (rad <= 0) { p.AddRectangle(r); return p; }
  p.AddArc(r.X, r.Y, d, d, 180, 90); p.AddArc(r.Right - d, r.Y, d, d, 270, 90);
  p.AddArc(r.Right - d, r.Bottom - d, d, d, 0, 90); p.AddArc(r.X, r.Bottom - d, d, d, 90, 90); p.CloseFigure();
  return p;
 }
}

static class Sha {
 public static string Hex(byte[] h) { var s = new StringBuilder(h.Length * 2); foreach (byte x in h) s.Append(x.ToString("x2")); return s.ToString(); }
 public static string HashFile(string path) {
  using (var sha = SHA256.Create()) using (var f = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite, 1 << 16)) return Hex(sha.ComputeHash(f));
 }
 public static string HashPart(string path, long off, int len) {
  using (var sha = SHA256.Create()) using (var f = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite, 1 << 16)) {
   if (off + len > f.Length) return "";
   f.Position = off; byte[] buf = new byte[1 << 20]; int left = len;
   while (left > 0) {
    int k = f.Read(buf, 0, Math.Min(buf.Length, left)); if (k <= 0) return "";
    sha.TransformBlock(buf, 0, k, null, 0); left -= k;
   }
   sha.TransformFinalBlock(buf, 0, 0); return Hex(sha.Hash);
  }
 }
}

// 이 설치기 폴더(패키지)의 정보
static class Pkg {
 public const string GameLabel = "2026-11-05 업데이트판";
 public static string Root = "", Version = "?", Date = "";
 public static readonly Dictionary<string, string> Sums = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);   // 패키지 안 경로(/) → sha
 public static void Load() {
  string dir = AppDomain.CurrentDomain.BaseDirectory.TrimEnd('\\', '/');
  string root = Path.GetDirectoryName(Path.GetDirectoryName(dir));   // ...\support\installer → 패키지
  if (root == null || !File.Exists(Path.Combine(Path.Combine(root, "tools"), "Season.ps1")))
   throw new IOException("설치 자료(tools\\Season.ps1)를 찾지 못했습니다. ZIP 전체를 같은 폴더에 풀었는지 확인해 주세요.\r\n설치기 위치: " + dir);
  Root = root;
  string v, d; ReadBuild(Path.Combine(root, "game\\SAN14Mods\\SeasonRules\\BUILD.txt"), out v, out d);
  Version = v == "" ? "?" : v; Date = d;
  string sums = Path.Combine(root, "SHA256SUMS.txt");
  if (File.Exists(sums)) {
   foreach (string raw in File.ReadAllLines(sums, Encoding.UTF8)) {
    string line = raw.Trim(); if (line.Length < 66) continue;
    string h = line.Substring(0, 64).ToLowerInvariant(); string rel = line.Substring(64).Trim().TrimStart('*').Replace('\\', '/');
    if (rel != "") Sums[rel] = h;
   }
  }
 }
 // BUILD.txt 첫 낱말 = 판 번호(v26k45), 그 안의 날짜(yyyy-MM-dd)
 public static void ReadBuild(string path, out string version, out string date) {
  version = ""; date = "";
  try {
   if (!File.Exists(path)) return;
   string t = File.ReadAllText(path, Encoding.UTF8).Trim('\uFEFF', ' ', '\r', '\n', '\t');
   var m = Regex.Match(t, @"^(v\d+k\d+[a-z]?)"); if (m.Success) version = m.Groups[1].Value;
   var d = Regex.Match(t, @"(\d{4}-\d{2}-\d{2})"); if (d.Success) date = d.Groups[1].Value;
  } catch (Exception) { }
 }
}

// 게임 폴더 찾기·기억
static class GameDir {
 static string SavePath { get { return Path.Combine(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "SAN14CE_SeasonMap"), "game_folder.txt"); } }
 public static bool Valid(string d) { return !String.IsNullOrEmpty(d) && File.Exists(Path.Combine(d, "SAN14CE_KO.exe")); }
 public static void Save(string d) { try { Directory.CreateDirectory(Path.GetDirectoryName(SavePath)); File.WriteAllText(SavePath, d, Encoding.UTF8); } catch (Exception) { } }
 public static string Find() {
  if (Valid(Program.GameArg)) return Program.GameArg.TrimEnd('\\');
  try { if (File.Exists(SavePath)) { string s = File.ReadAllText(SavePath, Encoding.UTF8).Trim(); if (Valid(s)) return s; } } catch (Exception) { }
  string here = Pkg.Root;
  for (int k = 0; k < 4 && !String.IsNullOrEmpty(here); k++) { if (Valid(here)) return here; here = Path.GetDirectoryName(here); }
  var roots = new List<string>();
  foreach (string key in new[] { @"HKEY_CURRENT_USER\Software\Valve\Steam|SteamPath", @"HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Valve\Steam|InstallPath", @"HKEY_LOCAL_MACHINE\SOFTWARE\Valve\Steam|InstallPath" }) {
   try { string[] a = key.Split('|'); object v = Registry.GetValue(a[0], a[1], null); if (v != null && v.ToString() != "") roots.Add(v.ToString().Replace('/', '\\')); } catch (Exception) { }
  }
  foreach (string sr in roots.Distinct(StringComparer.OrdinalIgnoreCase)) {
   try {
    if (!Directory.Exists(sr)) continue;
    var libs = new List<string>(); libs.Add(sr);
    string vdf = Path.Combine(sr, "steamapps\\libraryfolders.vdf");
    if (File.Exists(vdf)) foreach (string line in File.ReadAllLines(vdf)) { var m = Regex.Match(line, "\"path\"\\s+\"([^\"]+)\""); if (m.Success) libs.Add(m.Groups[1].Value.Replace("\\\\", "\\")); }
    foreach (string lib in libs.Distinct(StringComparer.OrdinalIgnoreCase)) {
     string common = Path.Combine(lib, "steamapps\\common"); if (!Directory.Exists(common)) continue;
     foreach (string d in Directory.GetDirectories(common)) if (Valid(d)) return d;
    }
   } catch (Exception) { }
  }
  return "";
 }
 public static bool Running() { try { return Process.GetProcessesByName("SAN14CE_KO").Length > 0; } catch (Exception) { return false; } }
}

// 게임 폴더 상태 확인 (구성 3가지 + 마지막 실행 기록 + 함께 쓰는 모드)
sealed class Part {
 public string Name = "", Desc = "", Status = "확인 중…", StatusSub = "", Applied = "", AppliedSub = "", Detail = "";
 public Color StatusColor = Theme.Mute, AppliedColor = Theme.Mute;
 public int Ok, Total, Kind;   // Kind: 0 없음, 1 설치됨, 2 일부, 3 확인 중
}
sealed class Snapshot {
 public string Game = "", InstalledVersion = "", Error = "";
 public bool Marker;
 public Part[] Parts = new Part[3];
 public readonly List<string[]> Coexist = new List<string[]>();   // {이름, 상태, 색(ok/mute)}
 public string LastRun = "";
 public int Overall;   // 0 설치 안 됨, 1 이 판 그대로, 2 다른 판, 3 일부만, 4 확인 불가
}
static class Checker {
 static bool InRules(string rel) { return rel.StartsWith("SAN14Mods/", StringComparison.OrdinalIgnoreCase) || rel.Equals("SAN14_SeasonRules.dll", StringComparison.OrdinalIgnoreCase) || rel.Equals("SeasonSync.dll", StringComparison.OrdinalIgnoreCase); }
 public static Snapshot Run(string game) {
  var s = new Snapshot(); s.Game = game;
  s.Parts[0] = new Part(); s.Parts[0].Name = "사계절 지도 그림"; s.Parts[0].Desc = "땅·풀·밭·단풍·눈·언 강·비·재해 그림 (셰이더)";
  s.Parts[1] = new Part(); s.Parts[1].Name = "실제 지형 산세"; s.Parts[1].Desc = "산 모양·산 사진 무늬 (칸 판정·길은 순정 그대로)";
  s.Parts[2] = new Part(); s.Parts[2].Name = "날씨·재해 규칙"; s.Parts[2].Desc = "비·눈 3단계, 산불·홍수·산사태·지진·가뭄, 수확";
  if (!GameDir.Valid(game)) { s.Overall = 4; s.Error = "게임 폴더를 찾지 못했습니다"; foreach (var p in s.Parts) { p.Status = "—"; p.Kind = 0; } return s; }
  string d; Pkg.ReadBuild(Path.Combine(game, "SAN14Mods\\SeasonRules\\BUILD.txt"), out s.InstalledVersion, out d);
  s.Marker = File.Exists(Path.Combine(game, "_SAN14_SeasonMap_installed.txt"));
  // 1·3: 패키지 game\ 아래 파일이 게임 폴더에 지문 그대로 있는가
  var miss = new List<string>[] { new List<string>(), null, new List<string>() };
  foreach (var kv in Pkg.Sums) {
   if (!kv.Key.StartsWith("game/", StringComparison.OrdinalIgnoreCase)) continue;
   string rel = kv.Key.Substring(5); int idx = InRules(rel) ? 2 : 0; var p = s.Parts[idx]; p.Total++;
   string path = Path.Combine(game, rel.Replace('/', '\\')); string why = null;
   try { if (!File.Exists(path)) why = "없음"; else if (Sha.HashFile(path) != kv.Value) why = "다른 파일"; } catch (Exception e) { why = "읽기 실패(" + e.Message + ")"; }
   if (why == null) p.Ok++; else miss[idx].Add(rel.Replace('/', '\\') + " — " + why);
  }
  foreach (int idx in new[] { 0, 2 }) {
   var p = s.Parts[idx]; var b = new StringBuilder();
   b.AppendLine("게임 폴더 파일 " + p.Ok + " / " + p.Total + " 개가 이 판(" + Pkg.Version + ")과 같습니다.");
   if (miss[idx].Count > 0) { b.AppendLine(); b.AppendLine("다른 것:"); foreach (string m in miss[idx].Take(14)) b.AppendLine("  " + m); if (miss[idx].Count > 14) b.AppendLine("  … 외 " + (miss[idx].Count - 14) + "개"); }
   p.Detail = b.ToString(); SetKind(p, p.Total == 0 ? 0 : p.Ok == p.Total ? 1 : p.Ok == 0 ? 0 : 2);
  }
  CheckTerrain(game, s.Parts[1]);
  ReadLastRun(game, s);
  // 함께 쓰는 모드 (설치 순서 상관없이 따로 작동)
  string rec = Path.Combine(game, "SAN14_기록\\설치기.log");
  if (File.Exists(rec)) { string tag = ""; try { var m = Regex.Matches(Tail(rec, 8192), @"R\d+\.\d+"); if (m.Count > 0) tag = " (" + m[m.Count - 1].Value + ")"; } catch (Exception) { } s.Coexist.Add(new[] { "통합패치", "있음" + tag + " · 따로 작동", "ok" }); }
  else s.Coexist.Add(new[] { "통합패치", "없음", "mute" });
  bool mapx = Directory.Exists(Path.Combine(game, ".san14ce_mapexpand")) || File.Exists(Path.Combine(game, "SAN14CE_MapExpand_helper.log"));
  s.Coexist.Add(mapx ? new[] { "맵 확장 (한반도)", "있음 · 따로 작동", "ok" } : new[] { "맵 확장 (한반도)", "없음", "mute" });
  Updates.MapExpand = mapx;
  if (File.Exists(Path.Combine(game, "San14_2D\\.installed.json")) || File.Exists(Path.Combine(game, "d3d11_chain.dll")))
   s.Coexist.Add(new[] { "2D 부대 그림", File.Exists(Path.Combine(game, "d3d11_chain.dll")) ? "있음 · 계절 맵 아래에서 같이 돎" : "있음 · [설치]하면 계절 맵 아래로 옮겨 같이 돎", "ok" });
  else s.Coexist.Add(new[] { "2D 부대 그림", "없음", "mute" });
  // 전체
  int n1 = s.Parts.Count(p => p.Kind == 1), n0 = s.Parts.Count(p => p.Kind == 0);
  if (n1 == 3 && s.InstalledVersion == Pkg.Version) s.Overall = 1;
  else if (n0 == 3 && !s.Marker) s.Overall = 0;
  else if (s.InstalledVersion != "" && s.InstalledVersion != Pkg.Version) s.Overall = 2;
  else s.Overall = 3;
  return s;
 }
 static void SetKind(Part p, int k) {
  p.Kind = k;
  if (k == 1) { p.Status = "설치됨"; p.StatusColor = Theme.Ok; }
  else if (k == 2) { p.Status = "일부 다름"; p.StatusColor = Theme.Warn; p.StatusSub = p.Ok + " / " + p.Total; }
  else { p.Status = "없음"; p.StatusColor = Theme.Mute; }
 }
 // 산세: tools\Terrain26.ps1 의 대상 목록(파일·위치·길이·순정/새 지문)으로 게임 0002 폴더 자료를 확인
 static void CheckTerrain(string game, Part p) {
  var b = new StringBuilder(); int nNew = 0, nOrig = 0, nOther = 0, n = 0; bool grown = false;
  try {
   string ps = File.ReadAllText(Path.Combine(Pkg.Root, "tools\\Terrain26.ps1"), Encoding.UTF8);
   var rx = new Regex(@"Name='([^']+)';File='([^']+)';FileLen=\[long\](\d+);Offset=\[long\](\d+);Len=\[int\](\d+);Orig='([0-9a-f]{64})';New='([0-9a-f]{64})'");
   foreach (Match m in rx.Matches(ps)) {
    string orig = m.Groups[6].Value, nw = m.Groups[7].Value; if (orig == nw) continue;
    n++; string path = Path.Combine(game, "0002\\" + m.Groups[2].Value); string h = "";
    long flen = Int64.Parse(m.Groups[3].Value), off = Int64.Parse(m.Groups[4].Value); int len = Int32.Parse(m.Groups[5].Value);
    // v26k47: the file may be longer than the game's own (another mod appended data after it) - only our own bytes are checked here
    try { if (File.Exists(path)) { long L = new FileInfo(path).Length; if (L >= off + len) h = Sha.HashPart(path, off, len); string nm = m.Groups[1].Value; if (L > flen && (nm.StartsWith("mhk_") || nm.StartsWith("tex_"))) grown = true; } } catch (Exception) { }
    if (h == nw) nNew++; else if (h == orig) nOrig++; else { nOther++; b.AppendLine("  " + m.Groups[2].Value + " [" + m.Groups[1].Value + "] — " + (h == "" ? "없음" : "순정도 새 산도 아님")); }
   }
  } catch (Exception e) { b.AppendLine("확인 실패: " + e.Message); }
  // v26k47: the bigger map another mod (map expand) appended and pointed the game at - the season mountains are put into it too
  int meAll = 0, meOk = 0; var mb = new StringBuilder();
  string mk = Path.Combine(game, "_SAN14_SeasonMap_terrain_me.txt");
  try {
   if (File.Exists(mk)) foreach (string ln in File.ReadAllLines(mk, Encoding.UTF8)) {
    string[] q = ln.Trim('\uFEFF', ' ').Split('|'); if (q.Length < 6) continue; meAll++;
    string path = Path.Combine(game, "0002\\" + q[0]); long o = Int64.Parse(q[1]); int len = Int32.Parse(q[2]); string h = "";
    // v26k48: by content - a few cells of the copy (written down at install) are read back; the copy may be re-packed by the other mod
    //         (map expand puts the current heights in again when it is applied) and still hold the season mountains
    int pst = -1; if (q.Length >= 7) { try { pst = ProbeState(path, o, len, q[6]); } catch (Exception) { pst = -1; } }
    if (pst == -1) { try { if (File.Exists(path) && new FileInfo(path).Length >= o + len) h = Sha.HashPart(path, o, len); } catch (Exception) { } }
    string what = q[5].StartsWith("mhk_") ? "높이(판정용)" : "높이(화면용)";
    if (pst == 1 || (pst == -1 && h == q[4])) { meOk++; mb.AppendLine("  " + what + " — 들어 있음"); }
    else if (pst == 2) mb.AppendLine("  " + what + " — 일부만 들어 있음");
    else mb.AppendLine("  " + what + " — 맵 확장이 다시 깔아서 빠짐");
   }
  } catch (Exception e) { mb.AppendLine("  확인 실패: " + e.Message); }
  bool meNeed = grown && (meAll == 0 || meOk < meAll);
  p.Total = n + (grown ? Math.Max(meAll, 1) : 0); p.Ok = nNew + (grown ? meOk : 0);
  string head = "게임 지형 자료 " + n + "곳 중 새 산 " + nNew + " · 순정 " + nOrig + (nOther > 0 ? " · 그 밖 " + nOther : "") + "\r\n(산세는 게임 자료에 직접 들어가서 게임 안 실행 기록은 따로 없습니다)";
  if (grown) head += "\r\n\r\n맵 확장(큰 지도)이 지형 자료를 새로 붙여 씀 — 그 안의 중원 땅에도 산세를 넣습니다 (맵 확장이 바꾼 칸은 그대로):\r\n" + (meAll > 0 ? mb.ToString() : "  아직 안 넣음 — [설치 / 다시 맞추기]를 누르세요\r\n");
  p.Detail = head + (b.Length > 0 ? "\r\n다른 것:\r\n" + b.ToString() : "");
  SetKind(p, n == 0 ? 0 : (nNew == n && !meNeed) ? 1 : nNew == 0 && nOther == 0 && !grown ? 0 : 2);
  if (p.Kind == 2 && meNeed && nNew == n) p.StatusSub = "맵 확장 쪽 다시 맞추기";
  p.Applied = p.Kind == 1 ? (grown ? "자료에 들어 있음 (맵 확장 포함)" : "자료에 들어 있음") : "—"; p.AppliedColor = p.Kind == 1 ? Theme.Ok : Theme.Mute;
 }
 // v26k48: probe cells "P<piece size>:<raw offset>=<season hex>/<original hex>,..." of an IDRK copy at (path, o, len): only the pieces
 // holding them are inflated. 1 = all season, 0 = all original, 2 = mixed / other, -1 = cannot tell
 static int ProbeState(string path, long o, int len, string pr) {
  if (pr == null || pr.Length < 4 || pr[0] != 'P') return -1;
  int colon = pr.IndexOf(':'); if (colon < 2) return -1; int CH = Int32.Parse(pr.Substring(1, colon - 1)); if (CH < 1024) return -1;
  string[] items = pr.Substring(colon + 1).Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries); if (items.Length == 0) return -1;
  var offs = new long[items.Length]; var sv = new uint[items.Length]; var ov = new uint[items.Length]; long maxK = 0;
  for (int i = 0; i < items.Length; i++) {
   int e = items[i].IndexOf('='), sl = items[i].IndexOf('/'); if (e < 1 || sl < e) return -1;
   offs[i] = Int64.Parse(items[i].Substring(0, e)); sv[i] = Convert.ToUInt32(items[i].Substring(e + 1, sl - e - 1), 16); ov[i] = Convert.ToUInt32(items[i].Substring(sl + 1), 16);
   if (offs[i] % CH + 4 > CH) return -1; maxK = Math.Max(maxK, offs[i] / CH);
  }
  if (!File.Exists(path)) return -1;
  using (var f = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite, 1 << 16)) {
   if (f.Length < o + len) return -1;
   var hb = new byte[48]; f.Position = o; if (f.Read(hb, 0, 48) != 48) return -1;
   if (hb[0] != 0x49 || hb[1] != 0x44 || hb[2] != 0x52 || hb[3] != 0x4b || BitConverter.ToUInt32(hb, 8) != (uint)len) return -1;
   long p = o + (long)BitConverter.ToUInt32(hb, 8) - (long)BitConverter.ToUInt32(hb, 16); var lb = new byte[4];
   var pos = new List<long>(); var lens = new List<int>();
   while (pos.Count <= maxK) {
    if (p + 4 > o + len) return -1; f.Position = p; if (f.Read(lb, 0, 4) != 4) return -1;
    int L = BitConverter.ToInt32(lb, 0); if (L <= 6 || L > (1 << 20)) return -1;
    pos.Add(p + 4); lens.Add(L); p += 4 + L;
   }
   var raw = new Dictionary<long, byte[]>(); int nS = 0, nO = 0;
   for (int i = 0; i < offs.Length; i++) {
    long k = offs[i] / CH; int w = (int)(offs[i] % CH); byte[] r;
    if (!raw.TryGetValue(k, out r)) {
     var zb = new byte[lens[(int)k]]; f.Position = pos[(int)k]; int got = 0, n; while (got < zb.Length && (n = f.Read(zb, got, zb.Length - got)) > 0) got += n; if (got != zb.Length) return -1;
     using (var ms = new MemoryStream(zb, 2, zb.Length - 6)) using (var ds = new DeflateStream(ms, CompressionMode.Decompress)) using (var outm = new MemoryStream()) { ds.CopyTo(outm); r = outm.ToArray(); }
     raw[k] = r;
    }
    if (w + 4 > r.Length) return -1;
    uint v = BitConverter.ToUInt32(r, w); if (v == sv[i]) nS++; else if (v == ov[i]) nO++;
   }
   return nS == offs.Length ? 1 : nO == offs.Length ? 0 : 2;
  }
 }
 public static string Tail(string path, int max) {
  using (var f = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite)) {
   long start = Math.Max(0, f.Length - max); f.Position = start; var buf = new byte[f.Length - start]; int got = 0, k;
   while (got < buf.Length && (k = f.Read(buf, got, buf.Length - got)) > 0) got += k;
   return Encoding.UTF8.GetString(buf, 0, got);
  }
 }
 // SeasonSync_log.txt: 게임을 켤 때마다 이어 씀(설치하면 지워짐). 마지막 'SeasonSync v14 started' 뒤가 마지막 실행.
 static void ReadLastRun(string game, Snapshot s) {
  var g = s.Parts[0]; var r = s.Parts[2];
  string log = Path.Combine(game, "SeasonSync_log.txt");
  if (!File.Exists(log)) {
   foreach (var p in new[] { g, r }) { p.Applied = p.Kind == 0 ? "—" : "아직 실행 기록 없음"; p.AppliedSub = p.Kind == 0 ? "" : "설치 뒤 게임을 켜면 생김"; p.AppliedColor = Theme.Mute; }
   return;
  }
  string when = File.GetLastWriteTime(log).ToString("MM-dd HH:mm"); s.LastRun = when;
  string[] lines; try { lines = Tail(log, 65536).Replace("\r\n", "\n").Split('\n'); } catch (Exception) { lines = new string[0]; }
  int start = -1; for (int i = 0; i < lines.Length; i++) if (lines[i].Contains("SeasonSync v14 started")) start = i;
  if (start < 0) { g.Applied = "기록 읽기 실패"; g.AppliedColor = Theme.Warn; r.Applied = "기록 읽기 실패"; r.AppliedColor = Theme.Warn; return; }
  var sess = lines.Skip(start).Where(x => x.Trim() != "").ToList();
  bool failed = sess.Any(x => x.Contains("FAILED"));
  g.Applied = (failed ? "● 문제 " : "● 켜짐 ") + when; g.AppliedColor = failed ? Theme.Bad : Theme.Ok; g.AppliedSub = failed ? "그래픽 연결 실패 — 설명 칸 확인" : "계절 그림 연결됨";
  string rl = sess.FirstOrDefault(x => x.Contains("season rules:"));
  if (rl == null) { r.Applied = "규칙 기록 없음"; r.AppliedColor = Theme.Warn; r.AppliedSub = "게임을 끝까지 켜면 생김"; }
  else if (rl.Contains(" - started")) { r.Applied = "● 켜짐 " + when; r.AppliedColor = Theme.Ok; r.AppliedSub = "규칙 " + (s.InstalledVersion != "" ? s.InstalledVersion : "?") + " 켜짐"; }
  else { r.Applied = "● 안 켜짐 " + when; r.AppliedColor = Theme.Bad; r.AppliedSub = rl.Contains("not found") ? "규칙 파일이 없음" : rl.Contains("load failed") ? "규칙 파일을 못 읽음" : rl.Contains("old rules") ? "옛 규칙 파일" : "게임판을 몰라 꺼진 채로 있음"; }
  string tailText = String.Join("\r\n", sess.Select(x => Korean(x)).Where(x => x != null).Distinct().Take(12).ToArray());
  g.Detail += "\r\n마지막 실행(" + when + ") 기록:\r\n" + tailText;
  r.Detail += "\r\n마지막 실행(" + when + ") 기록:\r\n" + tailText;
 }
 // 기록 한 줄 → 한글 요약 (모르는 줄은 null 로 빼서 영어가 보이지 않게)
 static string Korean(string line) {
  string t = line; var m = Regex.Match(line, @"^\[(\d\d:\d\d:\d\d)\]\s*(.*)$"); string tm = "";
  if (m.Success) { tm = m.Groups[1].Value + "  "; t = m.Groups[2].Value; }
  if (t.Contains("SeasonSync v14 started")) return tm + "계절 그림 도구 켜짐";
  if (t.Contains("season rules:")) return tm + (t.Contains(" - started") ? "날씨·재해 규칙 켜짐" : t.Contains("not found") ? "날씨·재해 규칙 파일이 없음" : t.Contains("load failed") ? "날씨·재해 규칙 파일을 못 읽음" : "날씨·재해 규칙 꺼진 채로 있음(게임판을 모름)");
  if (t.Contains("FAILED")) return tm + "그래픽 연결 실패";
  if (t.StartsWith("chain:")) return tm + (t.Contains("loaded") && !t.Contains("failed") ? "2D 부대 그림도 함께 켜짐" : "2D 부대 그림을 함께 켜지 못함");
  if (t.Contains("date readable")) return tm + "세이브 날짜 읽음 (계절 맞춤 시작)";
  if (t.Contains("date place found")) return tm + "게임 날짜 위치 찾음";

  return null;
 }
}

// ───────── 깃허브 판 목록·새 판 받기 (통합패치 R43.28 방식) ─────────
sealed class Release {
 public string Tag = "", Title = "", Notes = "", Date = "", AssetName = "", AssetUrl = "", AssetSha = "", Only = "";   // Only "mapexpand" = 맵 확장을 쓰는 PC 에만 필요한 판
 public long AssetSize; public int Major, Minor; public string Letter = "";
 public readonly List<Release> BaseParts = new List<Release>();
}
static class Updates {
 public const string Repository = "wprkfdid12-ux/san14ce-season-map";
 public const string PagesUrl = "https://github.com/" + Repository;
 const string Raw = "https://raw.githubusercontent.com/" + Repository + "/main/";
 const string Index = Raw + "updates/index.json";
 public static void ParseTag(string tag, out int major, out int minor, out string letter) {
  major = 0; minor = 0; letter = "";
  var m = Regex.Match(tag ?? "", @"^v(\d+)k(\d+)([a-z]?)$");
  if (m.Success) { major = Int32.Parse(m.Groups[1].Value); minor = Int32.Parse(m.Groups[2].Value); letter = m.Groups[3].Value; }
 }
 public static int Compare(string a, string b) {
  int a1, a2, b1, b2; string al, bl; ParseTag(a, out a1, out a2, out al); ParseTag(b, out b1, out b2, out bl);
  if (a1 != b1) return a1.CompareTo(b1); if (a2 != b2) return a2.CompareTo(b2); return String.CompareOrdinal(al, bl);
 }
 static HttpWebRequest Request(string url) {
  ServicePointManager.SecurityProtocol |= (SecurityProtocolType)3072;   // TLS 1.2
  var q = (HttpWebRequest)WebRequest.Create(url); q.UserAgent = "SAN14CE-SeasonMap-Installer"; q.Timeout = 20000; q.ReadWriteTimeout = 60000; q.AllowAutoRedirect = true;
  return q;
 }
 static string Str(Dictionary<string, object> d, string k) { object v; return d != null && d.TryGetValue(k, out v) && v != null ? v.ToString() : ""; }
 static long Num(Dictionary<string, object> d, string k) { long n; return Int64.TryParse(Str(d, k), out n) ? n : 0; }
 public static List<Release> Fetch() {
  string text;
  var q = Request(Index + "?t=" + DateTime.UtcNow.Ticks);
  using (var r = q.GetResponse()) using (var s = new StreamReader(r.GetResponseStream(), Encoding.UTF8)) text = s.ReadToEnd();
  return Parse(text);
 }
 // updates/index.json: {"format":1,"releases":[{"tag","date","title","notes","file","sha256","size"}…],"base":[{"file","sha256","size"}…]}
 public static List<Release> Parse(string json) {
  var list = new List<Release>();
  var js = new JavaScriptSerializer(); js.MaxJsonLength = 1 << 26;
  var root = js.DeserializeObject(json) as Dictionary<string, object>;
  if (root == null) throw new IOException("판 목록(index.json) 형식이 예상과 다릅니다.");
  var parts = new List<Release>(); object bo; root.TryGetValue("base", out bo);
  var bl = bo as object[];
  if (bl != null) foreach (object o in bl) {
   var b = o as Dictionary<string, object>; if (b == null) continue; string file = Str(b, "file"); if (!SafeFile(file)) continue;
   var part = new Release(); part.AssetName = Path.GetFileName(file); part.AssetUrl = Raw + file; part.AssetSha = Str(b, "sha256").ToLowerInvariant(); part.AssetSize = Num(b, "size");
   parts.Add(part);
  }
  parts.Sort(delegate(Release x, Release y) { return String.CompareOrdinal(x.AssetName, y.AssetName); });
  object ro; root.TryGetValue("releases", out ro);
  var rl = ro as object[];
  if (rl != null) foreach (object o in rl) {
   var d = o as Dictionary<string, object>; if (d == null) continue;
   var r = new Release(); r.Tag = Str(d, "tag"); r.Title = Str(d, "title"); r.Date = Str(d, "date"); r.Notes = Str(d, "notes").Replace("\r\n", "\n").Replace("\n", "\r\n"); r.Only = Str(d, "only");
   ParseTag(r.Tag, out r.Major, out r.Minor, out r.Letter); if (r.Major == 0) continue;
   string file = Str(d, "file"); if (!SafeFile(file)) continue;
   r.AssetName = Path.GetFileName(file); r.AssetUrl = Raw + file; r.AssetSha = Str(d, "sha256").ToLowerInvariant(); r.AssetSize = Num(d, "size");
   r.BaseParts.AddRange(parts);
   list.Add(r);
  }
  list.Sort(delegate(Release x, Release y) { return Compare(y.Tag, x.Tag); });
  return list;
 }
 static bool SafeFile(string f) { return f.StartsWith("updates/") && !f.Contains("..") && !f.Contains("\\") && f.EndsWith(".zip", StringComparison.OrdinalIgnoreCase); }
 // 맵 확장용 판(only = mapexpand)은 맵 확장이 없는 PC 에서는 '새 판'으로 치지 않는다 (받을 필요 없음). 고르면 받을 수는 있음
 public static bool MapExpand;
 public static bool Skip(Release r) { return r != null && r.Only == "mapexpand" && !MapExpand; }
 public static Release Latest(List<Release> all) { if (all == null) return null; foreach (var r in all) if (!Skip(r)) return r; return null; }
 static Dictionary<string, string> ReadSums(Stream s) {
  var d = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
  using (var r = new StreamReader(s, Encoding.UTF8)) {
   string line;
   while ((line = r.ReadLine()) != null) {
    line = line.Trim(); if (line.Length < 66) continue;
    string h = line.Substring(0, 64).ToLowerInvariant(); string rel = line.Substring(64).Trim().TrimStart('*').Replace('\\', '/');
    if (rel == "" || rel.Contains("..") || Path.IsPathRooted(rel)) throw new IOException("SHA256SUMS.txt 에 이상한 경로: " + rel);
    d[rel] = h;
   }
  }
  return d;
 }
 static string Native(string root, string rel) { return Path.Combine(root, rel.Replace('/', Path.DirectorySeparatorChar)); }
 static string Mb(long n) { return (n / 1048576.0).ToString("0.0") + "MB"; }
 static void Download(string url, string path, long size, string what, Action<string> say) {
  string part = path + ".part";
  for (int attempt = 1; ; attempt++) {
   try {
    var q = Request(url); q.Timeout = 60000; q.ReadWriteTimeout = 120000;
    using (var r = q.GetResponse()) using (var s = r.GetResponseStream()) using (var f = new FileStream(part, FileMode.Create, FileAccess.Write)) {
     byte[] buf = new byte[1 << 16]; long got = 0, shown = 0; int k;
     while ((k = s.Read(buf, 0, buf.Length)) > 0) { f.Write(buf, 0, k); got += k; if (got - shown >= (4L << 20)) { shown = got; say(what + " 받는 중 " + Mb(got) + (size > 0 ? " / " + Mb(size) : "")); } }
     if (size > 0 && got != size) throw new IOException("받은 크기가 다릅니다(" + got + " / " + size + " 바이트). 인터넷이 중간에 끊겼을 수 있습니다.");
    }
    if (File.Exists(path)) File.Delete(path); File.Move(part, path); return;
   } catch (WebException e) {
    try { if (File.Exists(part)) File.Delete(part); } catch (Exception) { }
    if (attempt >= 3) throw new IOException(what + " 받기 실패(3번 시도): " + e.Message, e);
    say(what + " 받기가 끊겨 다시 시도합니다(" + attempt + "/3): " + e.Message); Thread.Sleep(2000 * attempt);
   }
  }
 }
 static void Expect(string path, string sha, string what) {
  if (sha == "") return;
  string h = Sha.HashFile(path);
  if (h != sha) throw new IOException(what + " 지문(SHA-256)이 깃허브 값과 다릅니다. 받는 중 파일이 깨졌을 수 있습니다. 다시 눌러 주세요.\r\n받은 것: " + h + "\r\n깃허브: " + sha);
 }
 static void Extract(ZipArchiveEntry e, string dest, string sha) {
  Directory.CreateDirectory(Path.GetDirectoryName(dest));
  string tmp = dest + ".tmp"; e.ExtractToFile(tmp, true);
  string h = Sha.HashFile(tmp); if (h != sha) { File.Delete(tmp); throw new IOException("압축 안 파일 지문이 목록과 다릅니다: " + e.FullName); }
  if (File.Exists(dest)) File.Delete(dest); File.Move(tmp, dest);
 }
 // 새 판 폴더 만들기: 지금 설치기 폴더 옆 '계절 맵 <판>'. 지금 폴더·게임 폴더는 건드리지 않는다.
 public static string Build(Release r, string package, Action<string> say) {
  string pkg = Path.GetFullPath(package).TrimEnd(Path.DirectorySeparatorChar);
  string parent = Path.GetDirectoryName(pkg); if (String.IsNullOrEmpty(parent)) throw new IOException("지금 설치기 폴더의 위 폴더를 알 수 없습니다: " + pkg);
  string dest = Path.Combine(parent, "계절 맵 " + r.Tag);
  if (String.Equals(dest, pkg, StringComparison.OrdinalIgnoreCase)) throw new IOException("지금 쓰는 설치기가 이미 그 판 폴더입니다: " + dest);
  string work = dest + " (받는 중)";
  try { if (Directory.Exists(work)) Directory.Delete(work, true); Directory.CreateDirectory(work); }
  catch (Exception e) { throw new IOException("새 판 폴더를 만들 수 없습니다(쓰기 권한·백신 확인): " + work + "\r\n" + e.Message, e); }
  say("새 판 폴더: " + dest);
  string zip = Path.Combine(work, "_" + r.AssetName);
  Download(r.AssetUrl, zip, r.AssetSize, r.Tag + " 파일", say); Expect(zip, r.AssetSha, r.Tag + " 파일");
  var missing = new List<string>(); Dictionary<string, string> sums; int fromZip = 0, fromHere = 0;
  using (var z = ZipFile.OpenRead(zip)) {
   var se = z.GetEntry("SHA256SUMS.txt"); if (se == null) throw new IOException(r.AssetName + " 안에 SHA256SUMS.txt 가 없습니다(올린 파일이 잘못됨).");
   using (var s = se.Open()) sums = ReadSums(s);
   say("파일 " + sums.Count + "개 확인 중 (바뀐 파일은 받은 것에서, 나머지는 지금 폴더에서 복사)");
   foreach (var kv in sums) {
    string dst = Native(work, kv.Key); var e = z.GetEntry(kv.Key);
    if (e != null) { Extract(e, dst, kv.Value); fromZip++; continue; }
    string src = Native(pkg, kv.Key);
    if (File.Exists(src) && Sha.HashFile(src) == kv.Value) { Directory.CreateDirectory(Path.GetDirectoryName(dst)); File.Copy(src, dst, true); fromHere++; }
    else missing.Add(kv.Key);
   }
  }
  File.Delete(zip);
  if (missing.Count > 0) {
   if (r.BaseParts.Count == 0) throw new IOException("지금 폴더에 없는 기본 파일이 " + missing.Count + "개 있는데(예: " + missing[0] + "), 깃허브에 기본판 파일이 없습니다.");
   say("지금 폴더에 없는 기본 파일 " + missing.Count + "개 → 기본판 조각을 받습니다");
   var still = new List<string>(missing);
   foreach (var part in r.BaseParts) {
    if (still.Count == 0) break;
    string bz = Path.Combine(work, "_" + part.AssetName); Download(part.AssetUrl, bz, part.AssetSize, "기본판 " + part.AssetName, say); Expect(bz, part.AssetSha, "기본판 " + part.AssetName);
    using (var z = ZipFile.OpenRead(bz)) foreach (string rel in still.ToArray()) { var e = z.GetEntry(rel); if (e == null) continue; Extract(e, Native(work, rel), sums[rel]); still.Remove(rel); }
    File.Delete(bz);
   }
   if (still.Count > 0) throw new IOException("기본판에도 없는 파일이 " + still.Count + "개 있습니다: " + String.Join(", ", still.Take(5).ToArray()));
  }
  using (var w = new StreamWriter(Path.Combine(work, "SHA256SUMS.txt"), false, new UTF8Encoding(false)))
   foreach (var kv in sums.OrderBy(x => x.Key, StringComparer.Ordinal)) w.Write(kv.Value + "  " + kv.Key + "\n");
  foreach (var kv in sums) if (Sha.HashFile(Native(work, kv.Key)) != kv.Value) throw new IOException("새 판 폴더 확인 실패: " + kv.Key);
  if (Directory.Exists(dest)) { string keep = dest + " (옛 " + DateTime.Now.ToString("yyyyMMdd_HHmmss") + ")"; Directory.Move(dest, keep); say("같은 이름 폴더가 있어 옆으로 옮김: " + keep); }
  Directory.Move(work, dest);
  say("새 판 폴더 완성: 받은 파일 " + fromZip + "개 · 지금 폴더에서 복사 " + fromHere + "개" + (missing.Count > 0 ? " · 기본판에서 " + missing.Count + "개" : ""));
  return dest;
 }
 // 새 판 설치기 열기(그 폴더의 Build.ps1 이 컴파일해서 실행). 지금 설치기는 닫는다.
 public static void Launch(string dest, string game, string fromVersion) {
  string ps = Path.Combine(dest, "support\\installer\\Build.ps1");
  if (!File.Exists(ps)) throw new IOException("새 판 폴더에 설치기 자료가 없습니다: " + ps);
  string args = "-NoProfile -ExecutionPolicy Bypass -File \"" + ps + "\" -AfterUpdate \"" + fromVersion + "\"";
  if (!String.IsNullOrEmpty(game)) args += " -GameDirectory \"" + game.TrimEnd('\\') + "\"";
  var psi = new ProcessStartInfo("powershell.exe", args); psi.UseShellExecute = true; psi.WorkingDirectory = dest; psi.WindowStyle = ProcessWindowStyle.Hidden;
  Process.Start(psi);
 }
}

// ───────── 화면 조각 ─────────
// 머리: 포스터 1번 모양(갈색 머리글·명조 큰 제목·설명 2줄·테두리 꼬리표·지도 그림)
sealed class HeaderPanel : Panel {
 Image map;
 public HeaderPanel() {
  DoubleBuffered = true; BackColor = Theme.Paper; Dock = DockStyle.Fill;
  try { string p = Path.Combine(Pkg.Root, "support\\installer\\header.jpg"); if (File.Exists(p)) using (var s = new MemoryStream(File.ReadAllBytes(p))) map = new Bitmap(Image.FromStream(s)); } catch (Exception) { map = null; }
 }
 protected override void OnResize(EventArgs e) { base.OnResize(e); Invalidate(); }
 protected override void OnPaint(PaintEventArgs e) {
  base.OnPaint(e);
  var g = e.Graphics; g.SmoothingMode = SmoothingMode.AntiAlias; g.TextRenderingHint = TextRenderingHint.ClearTypeGridFit; g.InterpolationMode = InterpolationMode.HighQualityBicubic;
  int x = 28, y = 14;
  int mapW = Math.Min(470, Math.Max(0, Width - 760)), mapH = Height - 26;
  using (var f = Theme.UiB(10.5f)) using (var br = new SolidBrush(Theme.Accent)) g.DrawString("삼국지14 CE · " + Pkg.GameLabel + " 대응", f, br, x, y);
  y += 24;
  string title = "계절 맵 " + Pkg.Version;
  using (var f = Theme.Serif(30f)) using (var br = new SolidBrush(Theme.Ink)) {
   g.DrawString(title, f, br, x - 4, y);
   var sz = g.MeasureString(title, f);
   using (var f2 = Theme.Serif(15f)) using (var br2 = new SolidBrush(Theme.Accent)) g.DrawString("설치기", f2, br2, x - 4 + sz.Width + 2, y + sz.Height - 38);
   y += (int)sz.Height - 2;
  }
  using (var f = Theme.Ui(10.5f)) using (var br = new SolidBrush(Theme.Sub)) {
   g.DrawString("땅·산·강·나무가 계절 따라 바뀌고, 날씨와 재해가 실제로 전쟁에 영향을 줍니다.", f, br, x, y); y += 22;
   g.DrawString("순정 칸 판정·길·거점은 그대로 두고 그림과 규칙만 더합니다.", f, br, x, y); y += 30;
  }
  using (var f = Theme.UiB(9.5f)) using (var pen = new Pen(Theme.Ink, 1.6f)) using (var br = new SolidBrush(Theme.Ink)) using (var bg = new SolidBrush(Color.FromArgb(120, 255, 255, 255))) {
   int px = x;
   foreach (string t in new[] { "사계절 지도", "실제 지형 산세", "비·눈 3단계", "산불·홍수·산사태·지진·가뭄", "언 강 건너기" }) {
    var sz = g.MeasureString(t, f); int w = (int)sz.Width + 18, h = 28;
    if (px + w > Width - mapW - 40) break;
    var r = new Rectangle(px, y, w, h);
    using (var path = Theme.Round(r, h / 2)) { g.FillPath(bg, path); g.DrawPath(pen, path); }
    g.DrawString(t, f, br, px + 9, y + (h - sz.Height) / 2 + 1); px += w + 7;
   }
  }
  if (map != null && mapW > 120) {
   var r = new Rectangle(Width - 28 - mapW, 14, mapW, mapH);
   using (var path = Theme.Round(r, 7)) {
    var keep = g.Clip; g.SetClip(path);
    float sc = Math.Max((float)r.Width / map.Width, (float)r.Height / map.Height);
    float w = map.Width * sc, h = map.Height * sc;
    g.DrawImage(map, r.X + (r.Width - w) / 2, r.Y + (r.Height - h) / 2, w, h);
    g.Clip = keep;
   }
   using (var f = Theme.UiB(9.5f)) {
    string badge = Pkg.Version + (Pkg.Date != "" ? " · " + Pkg.Date : "");
    var sz = g.MeasureString(badge, f); var br = new Rectangle(r.X + 10, r.Y + 10, (int)sz.Width + 16, (int)sz.Height + 8);
    using (var bp = Theme.Round(br, 4)) using (var b = new SolidBrush(Color.FromArgb(205, 29, 26, 22))) g.FillPath(b, bp);
    g.DrawString(badge, f, Brushes.White, br.X + 8, br.Y + 4);
   }
   using (var f = Theme.Ui(9f)) {
    string tip = "새 판은 [업데이트] 한 번으로 받습니다";
    var sz = g.MeasureString(tip, f); var br = new Rectangle(r.Right - (int)sz.Width - 26, r.Bottom - (int)sz.Height - 18, (int)sz.Width + 16, (int)sz.Height + 8);
    using (var bp = Theme.Round(br, 4)) using (var b = new SolidBrush(Color.FromArgb(230, 251, 249, 243))) g.FillPath(b, bp);
    using (var tb = new SolidBrush(Theme.Ink)) g.DrawString(tip, f, tb, br.X + 8, br.Y + 4);
   }
  }
 }
}

// 상태 줄: 둥근 카드 안에 색이 다른 글자 조각
sealed class StatusPanel : Panel {
 public readonly List<object[]> Segs = new List<object[]>();   // {글자, 색, 굵게}
 public StatusPanel() { DoubleBuffered = true; BackColor = Theme.Paper; Dock = DockStyle.Fill; }
 public void Set(params object[] flat) { Segs.Clear(); for (int i = 0; i + 2 < flat.Length; i += 3) Segs.Add(new object[] { flat[i], flat[i + 1], flat[i + 2] }); Invalidate(); }
 protected override void OnResize(EventArgs e) { base.OnResize(e); Invalidate(); }
 protected override void OnPaint(PaintEventArgs e) {
  base.OnPaint(e);
  var g = e.Graphics; g.SmoothingMode = SmoothingMode.AntiAlias; g.TextRenderingHint = TextRenderingHint.ClearTypeGridFit;
  var r = new Rectangle(28, 3, Width - 57, Height - 8);
  using (var p = Theme.Round(r, 6)) using (var b = new SolidBrush(Theme.Card)) using (var pen = new Pen(Theme.Line)) { g.FillPath(b, p); g.DrawPath(pen, p); }
  float x = r.X + 14;
  using (var f = Theme.Ui(10.5f)) using (var fb = Theme.UiB(10.5f)) {
   foreach (var s in Segs) {
    string t = (string)s[0]; var font = (bool)s[2] ? fb : f;
    var sz = TextRenderer.MeasureText(g, t, font, new Size(9999, 99), TextFormatFlags.NoPadding);
    TextRenderer.DrawText(g, t, font, new Point((int)x, r.Y + (r.Height - sz.Height) / 2), (Color)s[1], TextFormatFlags.NoPadding);
    x += sz.Width;
   }
  }
 }
}

// 탭 줄 (구성 / 업데이트 / 작업 기록)
sealed class TabStrip : Panel {
 public string[] Names = new[] { "구성", "업데이트", "작업 기록" };
 public string[] Marks = new[] { "", "", "" };
 public int Current;
 public event Action<int> Changed;
 readonly List<Rectangle> hit = new List<Rectangle>();
 public TabStrip() { DoubleBuffered = true; BackColor = Theme.Paper; Dock = DockStyle.Fill; Cursor = Cursors.Hand; }
 public void Select(int i) { Current = i; Invalidate(); if (Changed != null) Changed(i); }
 protected override void OnMouseClick(MouseEventArgs e) { base.OnMouseClick(e); for (int i = 0; i < hit.Count; i++) if (hit[i].Contains(e.Location)) { Select(i); break; } }
 protected override void OnResize(EventArgs e) { base.OnResize(e); Invalidate(); }
 protected override void OnPaint(PaintEventArgs e) {
  base.OnPaint(e);
  var g = e.Graphics; g.SmoothingMode = SmoothingMode.AntiAlias; hit.Clear();
  using (var pen = new Pen(Theme.Line, 1.5f)) g.DrawLine(pen, 0, Height - 1, Width, Height - 1);
  int x = 28;
  using (var f = Theme.UiB(10.5f)) {
   for (int i = 0; i < Names.Length; i++) {
    var s1 = TextRenderer.MeasureText(g, Names[i], f, new Size(999, 99), TextFormatFlags.NoPadding);
    var s2 = Marks[i] == "" ? Size.Empty : TextRenderer.MeasureText(g, " " + Marks[i], f, new Size(999, 99), TextFormatFlags.NoPadding);
    int w = s1.Width + s2.Width + 36; var r = new Rectangle(x, 6, w, Height - 6);
    if (i == Current) {
     using (var b = new SolidBrush(Theme.Card)) g.FillRectangle(b, r);
     using (var pen = new Pen(Theme.Line, 1.5f)) { g.DrawLine(pen, r.X, r.Bottom, r.X, r.Y); g.DrawLine(pen, r.X, r.Y, r.Right, r.Y); g.DrawLine(pen, r.Right, r.Y, r.Right, r.Bottom); }
    }
    int ty = r.Y + (r.Height - s1.Height) / 2;
    TextRenderer.DrawText(g, Names[i], f, new Point(r.X + 18, ty), i == Current ? Theme.Ink : Theme.Mute, TextFormatFlags.NoPadding);
    if (Marks[i] != "") TextRenderer.DrawText(g, " " + Marks[i], f, new Point(r.X + 18 + s1.Width, ty), Theme.Accent2, TextFormatFlags.NoPadding);
    hit.Add(r); x += w + 2;
   }
  }
 }
}

// ───────── 본 창 ─────────
sealed class MainForm : Form {
 string game = "";
 Snapshot snap;
 List<Release> releases; string fetchError = "";
 bool busy, checking, gameOn;
 readonly HeaderPanel header = new HeaderPanel();
 readonly StatusPanel status = new StatusPanel();
 readonly TabStrip tabs = new TabStrip();
 readonly Label pathLabel = new Label();
 readonly Button updateBtn, rollbackBtn, browseBtn, refreshBtn, readmeBtn, githubBtn, installBtn, removeBtn, gameFolderBtn, pkgFolderBtn, logOpenBtn, copyBtn, relGoBtn, relReadmeBtn;
 readonly Panel host = new Panel();
 readonly Panel pageParts = new Panel(), pageRel = new Panel(), pageLog = new Panel();
 readonly DataGridView grid = new DataGridView();
 readonly TextBox desc = new TextBox(), detail = new TextBox(), logBox = new TextBox(), relNotes = new TextBox();
 readonly Label detailTitle = new Label(), relHead = new Label(), relTitle = new Label(), relMeta = new Label();
 readonly ListBox relList = new ListBox();
 readonly Panel coexist = new Panel();
 readonly System.Windows.Forms.Timer gameTimer = new System.Windows.Forms.Timer();
 readonly StringBuilder live = new StringBuilder();
 bool liveShown;

 static readonly string[] Descs = new[] {
  "1. 사계절 지도 그림\r\n코에이 계절(봄 1~3월 · 여름 4~6월 · 가을 7~9월 · 겨울 10~12월)과 실제 도시 기온을 따라 꽃·풀·밭·단풍·눈이 바뀝니다. 하북은 눈에 덮이고 강남은 푸르게 남습니다. 비·눈 구름, 젖은 땅, 언 강, 홍수 물, 산불 자국도 그립니다.\r\n\r\n게임 폴더의 ShaderFixes·d3d11.dll(그래픽 도구)·d3dx.ini 로 들어갑니다. 2D 부대 그림 모드와 같이 쓸 수 있습니다.",
  "2. 실제 지형 산세\r\n산·저산 칸의 산 모양과 바위·흙 사진 무늬를 실제 지형처럼 바꿉니다. 칸 판정·길·거점·길찾기는 순정과 똑같습니다.\r\n\r\n게임 0002 폴더 지형 자료의 해당 위치만 바꾸고, 원본은 같은 폴더 _TerrainVisual_Backup 에 백업합니다. [제거]하면 순정으로 돌아갑니다.",
  "3. 날씨·재해 규칙\r\n가랑비·비·폭우 / 진눈깨비·눈·눈보라 3단계, 산불·홍수·산사태·지진·가뭄, 수확이 해마다 새로 뽑힙니다. 이동·사기·병력·화계에 영향을 주고 AI 도 같은 규칙을 받습니다.\r\n\r\nSAN14_SeasonRules.dll·SeasonSync.dll·SAN14Mods\\SeasonRules 로 들어갑니다. 공용 모드 로더 없이 혼자 켜집니다.\r\n\r\n세 가지는 서로 맞물려 있어 따로 고르지 않고 [설치] 한 번에 함께 깔립니다."
 };

 public MainForm() {
  Text = "삼국지14 CE 계절 맵 " + Pkg.Version + " (공식 " + Pkg.GameLabel + " 전용)";
  Font = Theme.Ui(10f); BackColor = Theme.Paper; ClientSize = new Size(1360, 880); MinimumSize = new Size(1120, 760);
  AutoScaleMode = AutoScaleMode.Font; StartPosition = FormStartPosition.CenterScreen;
  try { Icon = Icon.ExtractAssociatedIcon(Application.ExecutablePath); } catch (Exception) { }

  updateBtn = Theme.Btn("업데이트", false); rollbackBtn = Theme.Btn("버전 되돌리기…", false); browseBtn = Theme.Btn("게임 폴더 변경…", false); refreshBtn = Theme.Btn("상태 새로고침", false);
  readmeBtn = Theme.Btn("설명서 열기", false); githubBtn = Theme.Btn("깃허브 열기", false);
  installBtn = Theme.Btn("설치 / 다시 맞추기", true); removeBtn = Theme.Btn("제거 (게임 원본으로)", false);
  gameFolderBtn = Theme.Btn("게임 폴더 열기", false); pkgFolderBtn = Theme.Btn("설치기 폴더 열기", false);
  logOpenBtn = Theme.Btn("기록 열기", false); copyBtn = Theme.Btn("이 내용 복사", false);
  relGoBtn = Theme.Btn("이 판으로 업데이트", true); relReadmeBtn = Theme.Btn("깃허브에서 보기", false);

  var root = new TableLayoutPanel(); root.Dock = DockStyle.Fill; root.ColumnCount = 1; root.RowCount = 7; root.BackColor = Theme.Paper; root.Margin = Padding.Empty; root.Padding = Padding.Empty;
  root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
  foreach (float h in new float[] { 200, 52, 48, 26, 46, 100, 62 }) root.RowStyles.Add(h == 100 ? new RowStyle(SizeType.Percent, 100) : new RowStyle(SizeType.Absolute, h));
  root.Controls.Add(header, 0, 0);

  var bar = new Panel(); bar.Dock = DockStyle.Fill; bar.Padding = new Padding(28, 6, 28, 0);
  var barL = new FlowLayoutPanel(); barL.Dock = DockStyle.Fill; barL.WrapContents = false; barL.AutoSize = false;
  var barR = new FlowLayoutPanel(); barR.Dock = DockStyle.Right; barR.WrapContents = false; barR.AutoSize = true; barR.FlowDirection = FlowDirection.LeftToRight;
  foreach (var b in new[] { updateBtn, rollbackBtn, browseBtn, refreshBtn }) barL.Controls.Add(b);
  foreach (var b in new[] { readmeBtn, githubBtn }) barR.Controls.Add(b);
  bar.Controls.Add(barL); bar.Controls.Add(barR);
  root.Controls.Add(bar, 0, 1);
  root.Controls.Add(status, 0, 2);
  pathLabel.Dock = DockStyle.Fill; pathLabel.Padding = new Padding(30, 2, 28, 0); pathLabel.ForeColor = Theme.Mute; pathLabel.Font = Theme.Ui(9f); pathLabel.AutoEllipsis = true;
  root.Controls.Add(pathLabel, 0, 3);
  root.Controls.Add(tabs, 0, 4);
  host.Dock = DockStyle.Fill; host.Padding = new Padding(28, 12, 28, 10); host.BackColor = Theme.Paper;
  root.Controls.Add(host, 0, 5);
  var foot = new Panel(); foot.Dock = DockStyle.Fill; foot.BackColor = Theme.Foot; foot.Padding = new Padding(28, 10, 20, 8);
  var footL = new FlowLayoutPanel(); footL.Dock = DockStyle.Fill; footL.WrapContents = false;
  var footR = new FlowLayoutPanel(); footR.Dock = DockStyle.Right; footR.WrapContents = false; footR.AutoSize = true;
  var footLabel = new Label(); footLabel.Text = "작업:"; footLabel.AutoSize = true; footLabel.Font = Theme.UiB(10f); footLabel.ForeColor = Theme.Sub; footLabel.Margin = new Padding(0, 13, 8, 0);
  footL.Controls.Add(footLabel); footL.Controls.Add(installBtn); footL.Controls.Add(removeBtn);
  footR.Controls.Add(gameFolderBtn); footR.Controls.Add(pkgFolderBtn);
  foot.Controls.Add(footL); foot.Controls.Add(footR);
  foot.Paint += delegate(object s, PaintEventArgs e) { using (var pen = new Pen(Theme.Line)) e.Graphics.DrawLine(pen, 0, 0, foot.Width, 0); };
  root.Controls.Add(foot, 0, 6);
  Controls.Add(root);

  BuildPartsPage(); BuildRelPage(); BuildLogPage();
  host.Controls.Add(pageParts); host.Controls.Add(pageRel); host.Controls.Add(pageLog);
  tabs.Changed += ShowTab; ShowTab(0);

  updateBtn.Click += delegate { DoUpdate(null); };
  rollbackBtn.Click += delegate { tabs.Select(1); var older = releases == null ? null : releases.FirstOrDefault(r => Updates.Compare(r.Tag, Pkg.Version) < 0); if (older != null) relList.SelectedItem = older; };
  browseBtn.Click += delegate { Browse(); };
  refreshBtn.Click += delegate { Refresh2(); };
  readmeBtn.Click += delegate { Open(Path.Combine(Pkg.Root, "README_KO.txt")); };
  githubBtn.Click += delegate { Open(Updates.PagesUrl); };
  relReadmeBtn.Click += delegate { Open(Updates.PagesUrl); };
  installBtn.Click += delegate { DoInstall(false); };
  removeBtn.Click += delegate { DoInstall(true); };
  gameFolderBtn.Click += delegate { if (GameDir.Valid(game)) Open(game); };
  pkgFolderBtn.Click += delegate { Open(Pkg.Root); };
  logOpenBtn.Click += delegate { string p = Path.Combine(game, "SeasonSync_log.txt"); if (!File.Exists(p)) p = Path.Combine(game, "_SAN14_SeasonMap_install_log.txt"); if (File.Exists(p)) Open(p); else MessageBox.Show(this, "아직 기록 파일이 없습니다.", "기록 열기"); };
  copyBtn.Click += delegate { try { Clipboard.SetText(desc.Text + "\r\n\r\n" + detailTitle.Text + "\r\n" + detail.Text); copyBtn.Text = "복사했습니다"; } catch (Exception) { } };
  relGoBtn.Click += delegate { var r = relList.SelectedItem as Release; if (r != null) DoUpdate(r); };

  gameTimer.Interval = 2000; gameTimer.Tick += delegate { bool on = GameDir.Running(); if (on != gameOn) { gameOn = on; ShowStatus(); } };
  Shown += delegate { OnShownFirst(); };
  FormClosing += delegate(object s, FormClosingEventArgs e) { if (busy && ErrorBox.Ask(this, "닫기", "설치·제거가 아직 진행 중입니다. 지금 닫으면 중간에 멈출 수 있습니다. 그래도 닫을까요?", new[] { "기다리기", "닫기" }) != 1) e.Cancel = true; };
 }

 void BuildPartsPage() {
  pageParts.Dock = DockStyle.Fill; pageParts.BackColor = Theme.Paper;
  var t = new TableLayoutPanel(); t.Dock = DockStyle.Fill; t.ColumnCount = 2; t.RowCount = 1;
  t.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 60)); t.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 40));
  var left = new TableLayoutPanel(); left.Dock = DockStyle.Fill; left.ColumnCount = 1; left.RowCount = 2; left.Margin = new Padding(0, 0, 14, 0);
  left.RowStyles.Add(new RowStyle(SizeType.Percent, 100)); left.RowStyles.Add(new RowStyle(SizeType.Absolute, 136));
  // 표
  grid.Dock = DockStyle.Fill; grid.BackgroundColor = Theme.Card; grid.BorderStyle = BorderStyle.FixedSingle; grid.GridColor = Theme.Line2;
  grid.RowHeadersVisible = false; grid.AllowUserToAddRows = false; grid.AllowUserToDeleteRows = false; grid.AllowUserToResizeRows = false; grid.AllowUserToResizeColumns = false;
  grid.ReadOnly = true; grid.MultiSelect = false; grid.SelectionMode = DataGridViewSelectionMode.FullRowSelect; grid.EnableHeadersVisualStyles = false;
  grid.CellBorderStyle = DataGridViewCellBorderStyle.SingleHorizontal; grid.ColumnHeadersBorderStyle = DataGridViewHeaderBorderStyle.Single;
  grid.ColumnHeadersHeightSizeMode = DataGridViewColumnHeadersHeightSizeMode.DisableResizing; grid.ColumnHeadersHeight = 36;
  grid.ColumnHeadersDefaultCellStyle.BackColor = Theme.Head; grid.ColumnHeadersDefaultCellStyle.ForeColor = Theme.Mute; grid.ColumnHeadersDefaultCellStyle.Font = Theme.UiB(9.5f);
  grid.ColumnHeadersDefaultCellStyle.SelectionBackColor = Theme.Head; grid.ColumnHeadersDefaultCellStyle.Padding = new Padding(6, 0, 0, 0);
  grid.DefaultCellStyle.BackColor = Theme.Card; grid.DefaultCellStyle.ForeColor = Theme.Ink; grid.DefaultCellStyle.SelectionBackColor = Theme.Sel; grid.DefaultCellStyle.SelectionForeColor = Theme.Ink;
  grid.RowTemplate.Height = 70; grid.ScrollBars = ScrollBars.None;
  grid.Columns.Add(new DataGridViewTextBoxColumn { Name = "No", HeaderText = "번호", Width = 56, SortMode = DataGridViewColumnSortMode.NotSortable });
  grid.Columns.Add(new DataGridViewTextBoxColumn { Name = "Item", HeaderText = "구성", AutoSizeMode = DataGridViewAutoSizeColumnMode.Fill, SortMode = DataGridViewColumnSortMode.NotSortable });
  grid.Columns.Add(new DataGridViewTextBoxColumn { Name = "State", HeaderText = "현재 상태", Width = 110, SortMode = DataGridViewColumnSortMode.NotSortable });
  grid.Columns.Add(new DataGridViewTextBoxColumn { Name = "Applied", HeaderText = "게임 적용 (마지막 실행)", Width = 230, SortMode = DataGridViewColumnSortMode.NotSortable });
  for (int i = 0; i < 3; i++) grid.Rows.Add((i + 1).ToString(), "", "", "");
  grid.CellPainting += PaintCell;
  grid.SelectionChanged += delegate { ShowPartDetail(); };
  left.Controls.Add(grid, 0, 0);
  coexist.Dock = DockStyle.Fill; coexist.Margin = new Padding(0, 12, 0, 0); coexist.BackColor = Theme.Paper;
  coexist.Paint += PaintCoexist;
  left.Controls.Add(coexist, 0, 1);
  t.Controls.Add(left, 0, 0);
  var right = new TableLayoutPanel(); right.Dock = DockStyle.Fill; right.ColumnCount = 1; right.RowCount = 5; right.Margin = Padding.Empty;
  right.RowStyles.Add(new RowStyle(SizeType.Absolute, 30)); right.RowStyles.Add(new RowStyle(SizeType.Percent, 52)); right.RowStyles.Add(new RowStyle(SizeType.Absolute, 34)); right.RowStyles.Add(new RowStyle(SizeType.Percent, 48)); right.RowStyles.Add(new RowStyle(SizeType.Absolute, 48));
  var descTitle = new Label(); descTitle.Text = "설명"; descTitle.Font = Theme.UiB(10f); descTitle.Dock = DockStyle.Fill; descTitle.TextAlign = ContentAlignment.MiddleLeft; descTitle.ForeColor = Theme.Ink;
  foreach (var box in new[] { desc, detail }) { box.Multiline = true; box.ReadOnly = true; box.ScrollBars = ScrollBars.Vertical; box.Dock = DockStyle.Fill; box.BackColor = Theme.Note; box.BorderStyle = BorderStyle.FixedSingle; box.ForeColor = Theme.Ink; }
  desc.Font = Theme.Ui(10f); detail.Font = Theme.Ui(9.5f);
  detailTitle.Text = "게임 적용 확인"; detailTitle.Font = Theme.UiB(10f); detailTitle.Dock = DockStyle.Fill; detailTitle.TextAlign = ContentAlignment.BottomLeft; detailTitle.ForeColor = Theme.Ink;
  var btns = new FlowLayoutPanel(); btns.Dock = DockStyle.Fill; btns.Padding = new Padding(0, 4, 0, 0); btns.Controls.Add(logOpenBtn); btns.Controls.Add(copyBtn);
  right.Controls.Add(descTitle, 0, 0); right.Controls.Add(desc, 0, 1); right.Controls.Add(detailTitle, 0, 2); right.Controls.Add(detail, 0, 3); right.Controls.Add(btns, 0, 4);
  t.Controls.Add(right, 1, 0);
  pageParts.Controls.Add(t);
 }

 void PaintCell(object sender, DataGridViewCellPaintingEventArgs e) {
  if (e.RowIndex < 0 || e.RowIndex > 2) return;
  e.PaintBackground(e.CellBounds, true);
  var g = e.Graphics; var b = e.CellBounds; Part p = snap == null ? null : snap.Parts[e.RowIndex];
  int x = b.X + 10, y = b.Y + 12;
  if (e.ColumnIndex == 0) { using (var f = Theme.Ui(10.5f)) TextRenderer.DrawText(g, (e.RowIndex + 1).ToString(), f, new Point(x + 4, y), Theme.Ink, TextFormatFlags.NoPadding); }
  else if (e.ColumnIndex == 1) {
   string name = p != null ? p.Name : new[] { "사계절 지도 그림", "실제 지형 산세", "날씨·재해 규칙" }[e.RowIndex];
   string d = p != null ? p.Desc : "";
   using (var f = Theme.UiB(11f)) TextRenderer.DrawText(g, name, f, new Point(x, y), Theme.Ink, TextFormatFlags.NoPadding);
   using (var f = Theme.Ui(9.5f)) TextRenderer.DrawText(g, d, f, new Rectangle(x, y + 26, b.Width - 16, 22), Theme.Mute, TextFormatFlags.NoPadding | TextFormatFlags.EndEllipsis);
  } else if (e.ColumnIndex == 2) {
   string s1 = p == null || checking ? "확인 중…" : p.Status; Color c = p == null || checking ? Theme.Mute : p.StatusColor;
   using (var f = Theme.UiB(10.5f)) TextRenderer.DrawText(g, s1, f, new Point(x, y), c, TextFormatFlags.NoPadding);
   if (p != null && !checking && p.StatusSub != "") using (var f = Theme.Ui(9.5f)) TextRenderer.DrawText(g, p.StatusSub, f, new Point(x, y + 26), Theme.Mute, TextFormatFlags.NoPadding);
  } else if (e.ColumnIndex == 3) {
   if (p != null && !checking) {
    using (var f = Theme.UiB(10.5f)) TextRenderer.DrawText(g, p.Applied, f, new Rectangle(x, y, b.Width - 14, 24), p.AppliedColor, TextFormatFlags.NoPadding | TextFormatFlags.EndEllipsis);
    if (p.AppliedSub != "") using (var f = Theme.Ui(9.5f)) TextRenderer.DrawText(g, p.AppliedSub, f, new Rectangle(x, y + 26, b.Width - 14, 22), Theme.Mute, TextFormatFlags.NoPadding | TextFormatFlags.EndEllipsis);
   }
  }
  e.Handled = true;
 }

 void PaintCoexist(object sender, PaintEventArgs e) {
  var g = e.Graphics; g.SmoothingMode = SmoothingMode.AntiAlias;
  var r = new Rectangle(0, 0, coexist.Width - 1, coexist.Height - 1);
  using (var p = Theme.Round(r, 6)) using (var b = new SolidBrush(Theme.Card)) using (var pen = new Pen(Theme.Line)) { g.FillPath(b, p); g.DrawPath(pen, p); }
  using (var f = Theme.UiB(9.5f)) TextRenderer.DrawText(g, "함께 쓰는 모드 (설치 순서 상관없이 따로 작동)", f, new Point(14, 10), Theme.Mute, TextFormatFlags.NoPadding);
  if (snap == null) return;
  int y = 38;
  using (var f = Theme.Ui(10f)) using (var fb = Theme.UiB(10f)) {
   foreach (var row in snap.Coexist) {
    TextRenderer.DrawText(g, row[0], f, new Point(14, y), Theme.Sub, TextFormatFlags.NoPadding);
    TextRenderer.DrawText(g, row[1], row[2] == "ok" ? fb : f, new Point(210, y), row[2] == "ok" ? Theme.Ok : Theme.Mute, TextFormatFlags.NoPadding);
    y += 28;
   }
  }
 }

 void BuildRelPage() {
  pageRel.Dock = DockStyle.Fill; pageRel.BackColor = Theme.Paper;
  var t = new TableLayoutPanel(); t.Dock = DockStyle.Fill; t.ColumnCount = 2; t.RowCount = 2;
  t.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 320)); t.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
  t.RowStyles.Add(new RowStyle(SizeType.Absolute, 38)); t.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
  relHead.Dock = DockStyle.Fill; relHead.ForeColor = Theme.Sub; relHead.Font = Theme.Ui(10f); relHead.TextAlign = ContentAlignment.MiddleLeft;
  t.Controls.Add(relHead, 0, 0); t.SetColumnSpan(relHead, 2);
  relList.Dock = DockStyle.Fill; relList.IntegralHeight = false; relList.DrawMode = DrawMode.OwnerDrawFixed; relList.ItemHeight = 62; relList.BorderStyle = BorderStyle.FixedSingle; relList.BackColor = Theme.Card; relList.Margin = new Padding(0, 0, 14, 0);
  relList.DrawItem += DrawRel;
  relList.SelectedIndexChanged += delegate { ShowRel(); };
  t.Controls.Add(relList, 0, 1);
  var notes = new TableLayoutPanel(); notes.Dock = DockStyle.Fill; notes.ColumnCount = 1; notes.RowCount = 4; notes.BackColor = Theme.Note; notes.Padding = new Padding(20, 14, 20, 10); notes.Margin = Padding.Empty;
  notes.RowStyles.Add(new RowStyle(SizeType.Absolute, 46)); notes.RowStyles.Add(new RowStyle(SizeType.Absolute, 26)); notes.RowStyles.Add(new RowStyle(SizeType.Percent, 100)); notes.RowStyles.Add(new RowStyle(SizeType.Absolute, 50));
  notes.Paint += delegate(object s, PaintEventArgs e) { using (var pen = new Pen(Theme.Line)) e.Graphics.DrawRectangle(pen, 0, 0, notes.Width - 1, notes.Height - 1); };
  relTitle.Dock = DockStyle.Fill; relTitle.Font = Theme.Serif(17f); relTitle.ForeColor = Theme.Ink; relTitle.AutoEllipsis = true;
  relMeta.Dock = DockStyle.Fill; relMeta.Font = Theme.Ui(9.5f); relMeta.ForeColor = Theme.Mute;
  relNotes.Dock = DockStyle.Fill; relNotes.Multiline = true; relNotes.ReadOnly = true; relNotes.ScrollBars = ScrollBars.Vertical; relNotes.BorderStyle = BorderStyle.None; relNotes.BackColor = Theme.Note; relNotes.Font = Theme.Ui(10.5f); relNotes.ForeColor = Theme.Ink;
  var rb = new FlowLayoutPanel(); rb.Dock = DockStyle.Fill; rb.Padding = new Padding(0, 6, 0, 0); rb.Controls.Add(relGoBtn); rb.Controls.Add(relReadmeBtn);
  notes.Controls.Add(relTitle, 0, 0); notes.Controls.Add(relMeta, 0, 1); notes.Controls.Add(relNotes, 0, 2); notes.Controls.Add(rb, 0, 3);
  t.Controls.Add(notes, 1, 1);
  pageRel.Controls.Add(t);
 }

 void DrawRel(object sender, DrawItemEventArgs e) {
  if (e.Index < 0) return;
  var r = relList.Items[e.Index] as Release; if (r == null) return;
  bool sel = (e.State & DrawItemState.Selected) != 0; var g = e.Graphics; var b = e.Bounds;
  using (var br = new SolidBrush(sel ? Theme.Sel : Theme.Card)) g.FillRectangle(br, b);
  if (sel) using (var br = new SolidBrush(Theme.Accent)) g.FillRectangle(br, b.X, b.Y, 4, b.Height);
  using (var pen = new Pen(Theme.Line2)) g.DrawLine(pen, b.X, b.Bottom - 1, b.Right, b.Bottom - 1);
  g.TextRenderingHint = TextRenderingHint.ClearTypeGridFit;
  using (var f = Theme.Serif(13f)) TextRenderer.DrawText(g, r.Tag, f, new Point(b.X + 16, b.Y + 8), Theme.Ink, TextFormatFlags.NoPadding);
  string sub = r.Date + (r.Title != "" ? " · " + ShortTitle(r.Title) : "");
  using (var f = Theme.Ui(9f)) TextRenderer.DrawText(g, sub, f, new Rectangle(b.X + 16, b.Y + 36, b.Width - 24, 20), Theme.Mute, TextFormatFlags.NoPadding | TextFormatFlags.EndEllipsis);
  string mark = r.Tag == Pkg.Version ? "지금 판" : Updates.Compare(r.Tag, Pkg.Version) > 0 ? (Updates.Skip(r) ? "맵 확장용 · 안 받아도 됨" : "새 판") : (Updates.Skip(r) ? "맵 확장용" : "");
  if (mark != "") using (var f = Theme.UiB(9f)) { var sz = TextRenderer.MeasureText(g, mark, f); TextRenderer.DrawText(g, mark, f, new Point(b.Right - sz.Width - 12, b.Y + 10), mark == "새 판" ? Theme.Accent2 : mark == "지금 판" ? Theme.Ok : Theme.Mute, TextFormatFlags.NoPadding); }
 }
 static string ShortTitle(string t) { int i = t.IndexOf('—'); return i >= 0 ? t.Substring(i + 1).Trim() : t; }

 void BuildLogPage() {
  pageLog.Dock = DockStyle.Fill; pageLog.BackColor = Theme.Paper;
  logBox.Dock = DockStyle.Fill; logBox.Multiline = true; logBox.ReadOnly = true; logBox.ScrollBars = ScrollBars.Both; logBox.WordWrap = false;
  logBox.BackColor = Theme.Note; logBox.BorderStyle = BorderStyle.FixedSingle; logBox.Font = Theme.Ui(9.5f); logBox.ForeColor = Theme.Ink;
  pageLog.Controls.Add(logBox);
 }

 void ShowTab(int i) {
  pageParts.Visible = i == 0; pageRel.Visible = i == 1; pageLog.Visible = i == 2;
  if (i == 2 && !liveShown) LoadRecords();
 }

 static void Open(string what) { try { Process.Start(new ProcessStartInfo(what) { UseShellExecute = true }); } catch (Exception) { } }

 void OnShownFirst() {
  game = GameDir.Find(); if (GameDir.Valid(game)) GameDir.Save(game);
  SetPath(); ShowStatus(); gameTimer.Start(); gameOn = GameDir.Running();
  Refresh2();
  var th = new Thread(delegate() {
   List<Release> got = null; string err = "";
   try { got = Updates.Fetch(); } catch (Exception e) { err = e.Message; }
   try { BeginInvoke(new Action(delegate { releases = got; fetchError = err; ShowReleases(); ShowStatus(); })); } catch (Exception) { }
  });
  th.IsBackground = true; th.Start();
  if (Program.AfterUpdate != "") {
   if (!GameDir.Valid(game)) { MessageBox.Show(this, "새 판(" + Pkg.Version + ") 설치기입니다. 게임 폴더를 찾지 못했으니 [게임 폴더 변경…]으로 고른 뒤 [설치]를 눌러 주세요.", "새 판"); return; }
   int k = ErrorBox.Ask(this, "새 판 " + Pkg.Version, "새 판(" + Pkg.Version + ") 폴더를 만들었습니다. (이전 설치기: " + Program.AfterUpdate + ")\r\n\r\n지금 게임에 적용할까요? 게임은 꺼져 있어야 합니다.\r\n앞으로는 이 새 폴더의 설치.cmd 를 쓰세요.", new[] { "지금 적용", "나중에" });
   if (k == 0) DoInstall(false);
  }
 }

 void SetPath() {
  pathLabel.Text = "게임 폴더: " + (GameDir.Valid(game) ? game : "(찾지 못함 — [게임 폴더 변경…]으로 고르기)") + "     ·     이 설치기: " + Pkg.Root;
 }

 void ShowStatus() {
  var segs = new List<object>();
  Action<string, Color, bool> add = delegate(string t, Color c, bool b) { segs.Add(t); segs.Add(c); segs.Add(b); };
  add("상태: ", Theme.Ink, true);
  if (!GameDir.Valid(game)) add("게임 폴더를 찾지 못함 — [게임 폴더 변경…]", Theme.Bad, true);
  else if (snap == null || checking) add("확인 중…", Theme.Mute, false);
  else if (snap.Overall == 0) add("설치 안 됨", Theme.Mute, true);
  else if (snap.Overall == 1) { add("설치됨 ", Theme.Ink, false); add(Pkg.Version, Theme.Ink, true); add(" (3/3)", Theme.Ink, false); }
  else if (snap.Overall == 2) { add(snap.InstalledVersion + " 설치됨", Theme.Warn, true); add(" — [설치]를 누르면 " + Pkg.Version + "(으)로 바뀝니다", Theme.Sub, false); }
  else add("일부만 맞음 — [설치 / 다시 맞추기]로 맞추세요", Theme.Warn, true);
  add("      공개 최신: ", Theme.Ink, false);
  var latest = Updates.Latest(releases);
  bool newer = latest != null && Updates.Compare(latest.Tag, Pkg.Version) > 0;
  if (releases == null && fetchError == "") add("확인 중…", Theme.Mute, false);
  else if (releases == null) add("확인 불가 (인터넷)", Theme.Mute, false);
  else if (latest == null) add("아직 없음", Theme.Mute, false);
  else if (newer) add(latest.Tag + " ● 업데이트 가능", Theme.Accent2, true);
  else add(latest.Tag + (latest.Tag == Pkg.Version ? " (지금 판이 최신)" : ""), Theme.Ok, latest.Tag == Pkg.Version);
  if (releases != null) { Release me = null; foreach (var x in releases) if (Updates.Skip(x) && Updates.Compare(x.Tag, Pkg.Version) > 0) { me = x; break; }
   if (me != null) add("  (맵 확장용 " + me.Tag + " 은 맵 확장을 안 쓰면 안 받아도 됨)", Theme.Mute, false); }
  add("      게임: ", Theme.Ink, false);
  add(gameOn ? "켜져 있음 (설치·제거 전에 끄세요)" : "꺼져 있음", gameOn ? Theme.Warn : Theme.Ok, true);
  status.Set(segs.ToArray());
  updateBtn.Text = newer ? "업데이트  ● 새 판 " + latest.Tag : "업데이트";
  Theme.SetKind(updateBtn, newer ? 2 : 0);
  tabs.Marks[0] = snap == null || checking ? "" : snap.Parts.Count(p => p.Kind == 1) + "/3";
  tabs.Marks[1] = newer ? "●" : ""; tabs.Invalidate(); relList.Invalidate();
  bool idle = !busy;
  installBtn.Enabled = idle && GameDir.Valid(game); removeBtn.Enabled = idle && GameDir.Valid(game); updateBtn.Enabled = idle; rollbackBtn.Enabled = idle; browseBtn.Enabled = idle; refreshBtn.Enabled = idle && !checking; relGoBtn.Enabled = idle && relList.SelectedItem != null && ((Release)relList.SelectedItem).Tag != Pkg.Version;
 }

 void Refresh2() {
  if (checking) return;
  checking = true; grid.Invalidate(); ShowStatus();
  string g = game;
  var th = new Thread(delegate() {
   Snapshot s; try { s = Checker.Run(g); } catch (Exception e) { s = new Snapshot(); s.Overall = 4; s.Error = e.Message; for (int i = 0; i < 3; i++) { s.Parts[i] = new Part(); s.Parts[i].Status = "확인 실패"; } }
   try { BeginInvoke(new Action(delegate { snap = s; checking = false; grid.Invalidate(); coexist.Invalidate(); ShowPartDetail(); ShowStatus(); })); } catch (Exception) { }
  });
  th.IsBackground = true; th.Start();
 }

 void ShowPartDetail() {
  int i = grid.CurrentRow == null ? 0 : grid.CurrentRow.Index; if (i < 0 || i > 2) i = 0;
  desc.Text = Descs[i] + (i == 0 ? "\r\n\r\n이 판: " + Pkg.Version + (Pkg.Date != "" ? " (" + Pkg.Date + ")" : "") + " — 바뀐 내용은 [업데이트] 탭에 있습니다." : "");
  detailTitle.Text = "게임 적용 확인" + (snap != null && snap.LastRun != "" ? " (마지막 실행: " + snap.LastRun + ")" : "");
  detail.Text = snap == null || checking ? "확인 중…" : snap.Overall == 4 ? (snap.Error != "" ? snap.Error : "게임 폴더를 먼저 골라 주세요.") : snap.Parts[i].Detail;
 }

 void ShowReleases() {
  var keep = relList.SelectedItem as Release; relList.Items.Clear();
  var latest = Updates.Latest(releases);
  relHead.Text = "지금 설치기: " + Pkg.Version + "     ·     " + (releases == null ? "공개판 확인 불가 — " + fetchError : latest == null ? "공개판 아직 없음" : "공개 최신: " + latest.Tag) + "     ·     깃허브: " + Updates.PagesUrl.Replace("https://", "");
  if (releases == null) { relTitle.Text = "판 목록을 읽지 못했습니다"; relMeta.Text = ""; relNotes.Text = "인터넷 연결을 확인한 뒤 설치기를 다시 열어 주세요. 설치·제거는 인터넷 없이도 됩니다.\r\n\r\n" + Updates.PagesUrl; ShowStatus(); return; }
  foreach (var r in releases) relList.Items.Add(r);
  Release pick = null;
  if (keep != null) pick = releases.FirstOrDefault(r => r.Tag == keep.Tag);
  if (pick == null) pick = releases.FirstOrDefault(r => r.Tag == Pkg.Version);
  if (pick == null && releases.Count > 0) pick = releases[0];
  if (pick != null) relList.SelectedItem = pick; else { relTitle.Text = "깃허브에 올라온 판이 아직 없습니다"; relNotes.Text = ""; }
  ShowStatus();
 }

 void ShowRel() {
  var r = relList.SelectedItem as Release; if (r == null) return;
  relTitle.Text = r.Title != "" ? r.Title : "계절 맵 " + r.Tag;
  relMeta.Text = r.Date + "   ·   " + (r.AssetSize > 0 ? "받을 크기 " + (r.AssetSize / 1048576.0).ToString("0.0") + "MB (바뀐 파일만)" : "") + "   ·   받은 파일은 지문으로 하나하나 확인";
  relNotes.Text = r.Notes;
  int c = Updates.Compare(r.Tag, Pkg.Version);
  bool skip = Updates.Skip(r);
  if (skip) relMeta.Text += "   ·   맵 확장용 판 — 이 PC 에는 맵 확장이 없어 안 받아도 됩니다";
  relGoBtn.Text = c > 0 ? (skip ? "이 판으로 업데이트 (맵 확장용)" : "이 판으로 업데이트") : c < 0 ? "이 판으로 되돌리기" : "지금 쓰는 판";
  Theme.SetKind(relGoBtn, c > 0 ? (skip ? 0 : 2) : c < 0 ? 1 : 0);
  ShowStatus();
 }

 void Browse() {
  using (var d = new OpenFileDialog()) {
   d.Title = "삼국지14 CE 게임 폴더의 SAN14CE_KO.exe 를 골라 주세요"; d.Filter = "SAN14CE_KO.exe|SAN14CE_KO.exe";
   if (GameDir.Valid(game)) d.InitialDirectory = game;
   if (d.ShowDialog(this) != DialogResult.OK) return;
   game = Path.GetDirectoryName(d.FileName); GameDir.Save(game); SetPath(); Refresh2();
  }
 }

 void LoadRecords() {
  var b = new StringBuilder();
  if (!GameDir.Valid(game)) { logBox.Text = "게임 폴더를 먼저 골라 주세요."; return; }
  foreach (var item in new[] { new[] { "_SAN14_SeasonMap_install_log.txt", "마지막 설치·제거 기록 (계절 맵)" }, new[] { "_SAN14_SeasonMap_terrain_log.txt", "마지막 설치·제거 기록 (산세)" } }) {
   string p = Path.Combine(game, item[0]);
   b.AppendLine("■ " + item[1] + (File.Exists(p) ? "  —  " + File.GetLastWriteTime(p).ToString("yyyy-MM-dd HH:mm") : ""));
   b.AppendLine("   " + p);
   if (File.Exists(p)) { try { b.AppendLine(File.ReadAllText(p, Encoding.UTF8).Replace("\r\n", "\n").Replace("\n", "\r\n")); } catch (Exception e) { b.AppendLine("읽기 실패: " + e.Message); } }
   else b.AppendLine("   (아직 없음 — [설치]·[제거]를 하면 생깁니다)");
   b.AppendLine();
  }
  logBox.Text = b.ToString();
 }

 void Say(string line) {
  string t = "[" + DateTime.Now.ToString("HH:mm:ss") + "] " + line;
  if (InvokeRequired) { try { BeginInvoke(new Action(delegate { AppendLive(t); })); } catch (Exception) { } } else AppendLive(t);
 }
 void AppendLive(string t) { live.AppendLine(t); logBox.AppendText(t + "\r\n"); }

 // 설치·제거: tools 의 검증된 스크립트를 그대로 실행 (INSTALL.cmd / UNINSTALL.cmd 와 같은 순서)
 void DoInstall(bool remove) {
  if (busy) return;
  if (!GameDir.Valid(game)) { MessageBox.Show(this, "게임 폴더를 먼저 골라 주세요([게임 폴더 변경…]).", "계절 맵"); return; }
  if (GameDir.Running()) { MessageBox.Show(this, "게임이 켜져 있습니다. 게임을 끈 뒤 다시 눌러 주세요.", "계절 맵"); return; }
  if (remove && ErrorBox.Ask(this, "제거", "계절 맵을 게임에서 빼고 원래대로 되돌립니다.\r\n(계절 그림·규칙 파일 삭제, 산세 자료는 백업한 순정으로 복원)\r\n\r\n세이브·다른 모드 파일은 건드리지 않습니다. 계속할까요?", new[] { "제거", "취소" }) != 0) return;
  busy = true; liveShown = true; logBox.Clear(); live.Clear(); tabs.Select(2); ShowStatus();
  string g = game; string tools = Path.Combine(Pkg.Root, "tools");
  Say((remove ? "제거" : "설치") + " 시작 — 계절 맵 " + Pkg.Version + " / 게임 폴더: " + g);
  var th = new Thread(delegate() {
   string fail = null;
   try {
    if (!remove) {
     int t = RunPs(Path.Combine(tools, "Terrain26.ps1"), "-GameDir " + Q(g));
     if (t != 0) Say("=== 산 모양은 건너뜁니다 (게임 파일은 그대로). 계절 맵 설치를 계속합니다 ===");
     int s = RunPs(Path.Combine(tools, "Season.ps1"), "-Mode Install -GameDir " + Q(g));
     if (s != 0) fail = "계절 맵 설치가 중간에 멈췄습니다(이번에 복사한 파일은 되돌렸습니다).";
    } else {
     int s = RunPs(Path.Combine(tools, "Season.ps1"), "-Mode Uninstall -GameDir " + Q(g));
     int t = RunPs(Path.Combine(tools, "Terrain26.ps1"), "-Uninstall -GameDir " + Q(g));
     if (s != 0 || t != 0) fail = "제거 중 일부가 멈췄습니다.";
    }
   } catch (Exception e) { fail = e.Message; }
   try {
    BeginInvoke(new Action(delegate {
     busy = false;
     Say(fail == null ? (remove ? "=== 제거가 모두 끝났습니다 ===" : "=== 설치가 모두 끝났습니다. 게임을 켜면 바로 적용됩니다 ===") : "=== " + fail + " ===");
     ShowStatus(); Refresh2();
     if (fail != null) ErrorBox.Show(this, "계절 맵 " + (remove ? "제거" : "설치"), ErrorBox.Describe(remove ? "제거하는 중" : "설치하는 중", null, null, fail + "\r\n\r\n작업 기록:\r\n" + LastLines(live.ToString(), 25)));
    }));
   } catch (Exception) { }
  });
  th.IsBackground = true; th.Start();
 }
 static string LastLines(string s, int n) { var a = s.Replace("\r\n", "\n").Split('\n').Where(x => x.Trim() != "").ToArray(); return String.Join("\r\n", a.Skip(Math.Max(0, a.Length - n)).ToArray()); }
 static string Q(string s) { return "'" + s.Replace("'", "''") + "'"; }
 // PowerShell 스크립트 실행(출력은 UTF-8 로 받아 작업 기록에 한 줄씩). 끝난 코드 반환
 int RunPs(string script, string args) {
  if (!File.Exists(script)) throw new IOException("설치 자료가 없습니다: " + script);
  string cmd = "[Console]::OutputEncoding=[Text.Encoding]::UTF8; $global:LASTEXITCODE=0; & " + Q(script) + " " + args + "; exit $LASTEXITCODE";
  var psi = new ProcessStartInfo("powershell.exe", "-NoProfile -ExecutionPolicy Bypass -Command \"" + cmd.Replace("\"", "\\\"") + "\"");
  psi.UseShellExecute = false; psi.CreateNoWindow = true; psi.RedirectStandardOutput = true; psi.RedirectStandardError = true;
  psi.StandardOutputEncoding = Encoding.UTF8; psi.StandardErrorEncoding = Encoding.UTF8; psi.WorkingDirectory = Pkg.Root;
  using (var p = new Process()) {
   p.StartInfo = psi;
   p.OutputDataReceived += delegate(object s, DataReceivedEventArgs e) { if (e.Data != null && e.Data.Trim() != "") Say(e.Data.TrimStart('[').Contains("] ") && e.Data.StartsWith("[") ? e.Data.Substring(e.Data.IndexOf("] ") + 2) : e.Data); };
   p.ErrorDataReceived += delegate(object s, DataReceivedEventArgs e) { if (e.Data != null && e.Data.Trim() != "") Say("(오류 출력) " + e.Data); };
   p.Start(); p.BeginOutputReadLine(); p.BeginErrorReadLine(); p.WaitForExit();
   return p.ExitCode;
  }
 }

 // 새 판 받기 / 되돌리기: 옆에 새 폴더를 만들고 그 판 설치기를 연다
 void DoUpdate(Release pick) {
  if (busy) return;
  if (releases == null) {
   try { releases = Updates.Fetch(); fetchError = ""; ShowReleases(); }
   catch (Exception e) { fetchError = e.Message; ErrorBox.Show(this, "업데이트", ErrorBox.Describe("깃허브에서 판 목록을 읽는 중", Updates.PagesUrl, e, null)); return; }
  }
  var r = pick ?? Updates.Latest(releases);
  if (r == null) { MessageBox.Show(this, "깃허브에 올라온 판이 아직 없습니다.", "업데이트"); return; }
  int c = Updates.Compare(r.Tag, Pkg.Version);
  if (pick == null && c <= 0) { MessageBox.Show(this, "지금 설치기(" + Pkg.Version + ")가 공개 최신판입니다.", "업데이트"); return; }
  if (c == 0) return;
  string head = (c > 0 ? "새 판 " : "옛 판 ") + r.Tag + " 을(를) 받아 이 설치기 폴더 옆에 '계절 맵 " + r.Tag + "' 폴더를 만들고, 그 판 설치기를 엽니다.\r\n지금 폴더·게임 폴더는 바로 바뀌지 않습니다(새 설치기에서 적용).";
  if (ErrorBox.Ask(this, c > 0 ? "업데이트" : "버전 되돌리기", head + "\r\n\r\n" + (r.Title != "" ? r.Title : r.Tag), new[] { c > 0 ? "받기" : "되돌리기", "취소" }) != 0) return;
  busy = true; liveShown = true; logBox.Clear(); live.Clear(); tabs.Select(2); ShowStatus();
  string g = game; string from = Pkg.Version;
  var th = new Thread(delegate() {
   string dest = null; Exception err = null;
   try { dest = Updates.Build(r, Pkg.Root, Say); } catch (Exception e) { err = e; }
   try {
    BeginInvoke(new Action(delegate {
     busy = false; ShowStatus();
     if (err != null) { Say("실패: " + err.Message); ErrorBox.Show(this, "새 판 받기", ErrorBox.Describe("새 판(" + r.Tag + ") 폴더를 만드는 중", null, err, null)); return; }
     try { Updates.Launch(dest, g, from); Say("새 판 설치기를 엽니다: " + dest); Close(); }
     catch (Exception e) { ErrorBox.Show(this, "새 판 받기", ErrorBox.Describe("새 판 설치기를 여는 중", dest, e, "새 폴더의 설치.cmd 를 직접 실행해 주세요.")); }
    }));
   } catch (Exception) { }
  });
  th.IsBackground = true; th.Start();
 }
}
}
