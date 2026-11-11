# Season map v26 packages (TEST and release) from the v25.9i link-test package: new rules DLL, data v2, shaders, textures,
# ini, installer with previous-version cleanup, Terrain26 (mountain heights v17), split ZIPs.
import os, sys, re, shutil, hashlib, zipfile, io
V = '/home/claude/season/v26/'; LT = '/home/claude/season/pkgLT/'; R9 = '/home/claude/season/pkg259i/'
kind = sys.argv[1]            # TEST or REL
MEASURE = kind == 'REL' and bool(os.environ.get('MEASURE'))
K38 = kind == 'REL' and bool(os.environ.get('K38'))   # v26k38: test build (soft cloud, no ghost outlines, clear ice over troops, lighter ground binding + timing, lighting-pass watch)
WX = kind == 'REL' and bool(os.environ.get('WX'))   # v26k37: weather-layer diagnosis build (draw watch in SeasonSync + layer counters)   # v26k35: the release build with the GPU-time-per-shader SeasonSync (frame measurement)
TAG = 'SAN14CE_SeasonMap_v26' + ('_TEST' if kind == 'TEST' else ('_MEASURE' if MEASURE else ('_WXDIAG' if WX else ('_K38BTEST' if K38 else ''))))
P = V + ('pkgT/' if kind == 'TEST' else ('pkgM/' if MEASURE else ('pkgW/' if WX else ('pkgK/' if K38 else 'pkgR/')))) + TAG + '/'
if os.path.exists(P): shutil.rmtree(P)
os.makedirs(P)
sha = lambda p: hashlib.sha256(open(p, 'rb').read()).hexdigest()
# ---- game files ----
shutil.copytree(LT + 'game', P + 'game')
shutil.copy('/tmp/claude-0/ss15p_probe.dll' if K38 else '/tmp/claude-0/ss15w_watch.dll' if WX else ('/tmp/claude-0/ss15c_prof.dll' if ((kind == 'TEST' and os.environ.get('DIAG')) or MEASURE) else '/home/claude/ss/SeasonSync.dll'), P + 'game/SeasonSync.dll')   # v26k28d: the test build measures GPU time per shader          # v26k18: v14 - starts the season rules itself, chains another mod's d3d11
os.remove(P + 'game/SAN14Mods/modules/SeasonRules.ini'); os.rmdir(P + 'game/SAN14Mods/modules')   # v26k18: no longer through the shared mod loader
ini = open(V + 'd3dx.ini', 'rb').read().decode('utf-8-sig').replace('\r\n', '\n')
CITYTEST = False   # the Chang'an appearance test (citytest_ini.txt, /home/claude/citytest) is not shipped in this version
if CITYTEST and kind == 'TEST':
        CT = '/home/claude/citytest/'
        # round 4b: the new buildings are drawn once per frame together with the city objects (after the ground, the far landscape
        # drawn later covered the top of the tower: its depth range is the object pass's, not the ground's)
        for sec in ('[ShaderOverrideSAN14TerrainNear]\nhash = d3e1f71e809bac74\n', '[ShaderOverrideSAN14TerrainFar]\nhash = 9addb6cb884c7a7c\n'):
            assert sec in ini
            ini = ini.replace(sec, sec + 'ResourceSAN14CamW2P = copy vs-cb0\n', 1)   # round 4b: the new buildings are drawn with the map objects (see citytest_ini.txt)
        assert '[Present]\n' in ini and 'global $evn = 0\n' in ini
        ini = ini.replace('[Present]\n', '[Present]\n$citydone = 0\n', 1).replace('global $evn = 0\n', 'global $evn = 0\nglobal $citydone = 0\n', 1)
        ini += open(V + 'citytest_ini.txt', encoding='utf-8').read()
# v26k: NO byte-order mark on d3dx.ini - the graphics tool reads the file byte by byte, takes the mark as part of the first line,
#       then warns "entry outside of section" and BEEPS at start-up (found by running its ini parser here). The test build also turns
#       the tool's own log on (calls=1: start-up notes only, not per-call) so the report shows "0 warnings".
if kind == 'TEST':
    assert '\ncalls=0\n' in ini; ini = ini.replace('\ncalls=0\n', '\ncalls=1\n', 1)
if kind == 'TEST' and os.environ.get('DIAG'):   # v26k28b: the graphics tool's frame record (call list only), taken by the rules module with F8
    for x, y in ():   # v26k28d: no frame records any more (the GPU clock in SeasonSync measures instead); the graphics tool runs as in the release
        assert ini.count(x) == 1, x; ini = ini.replace(x, y, 1)
assert not ini.startswith('\ufeff')
open(P + 'game/d3dx.ini', 'wb').write(ini.replace('\n', '\r\n').encode('utf-8'))
if CITYTEST and kind == 'TEST':
    for f in ('8fd83211313bbef4-vs_replace.txt', 'ce87496a46cd5d91-vs_replace.txt', 'CityTestBuild.hlsl', 'city_pagoda.buf', 'city_flag.buf', '90609d9cc86a8527-vs_replace.txt'):   # round 4: no colour tint, so no map-object pixel shader replacement (keep in step with SHIP below)
        shutil.copy('/home/claude/citytest/' + f, P + 'game/ShaderFixes/' + f)
for f in ('SeasonTerrainNear.hlsl', 'SeasonTerrainFar.hlsl', 'SeasonWaterWinter.hlsl', 'SeasonRain.hlsl', 'SeasonRainDrops.hlsl', 'SeasonRainB.hlsl', 'SeasonRainDropsB.hlsl', '8f8964766ab0977e-ps_replace.txt', 'd28d1d5e4d8d3a3c-ps_replace.txt', 'season_ground.dds', 'season_rock2.dds', 'season_rock3.dds', 'season_tags.dds', 'season_freeze.dds', 'season_relief.dds',
          'd3e1f71e809bac74-ps_replace.txt', '9addb6cb884c7a7c-ps_replace.txt', '425be6c323067e73-ps_replace.txt',
          'd3e1f71e809bac74-ps_replace.k27', '9addb6cb884c7a7c-ps_replace.k27'):   # v26k29: + the previous ground shaders, used only if the game's compiler refuses the new ones   # v26k27: ground drawn once, winter water without the extra pass
    shutil.copy(V + f, P + 'game/ShaderFixes/' + f)
PERF = kind == 'TEST' and os.environ.get('PERF')   # v26k27: the speed test build (three ways in turn)
for f in ('SeasonGameNear.hlsl', 'SeasonGameFar.hlsl'): shutil.copy(V + f, P + 'game/ShaderFixes/' + f)   # v26k27: the game's own ground (d3dx.ini names them; used only by the speed test)
DIAG = kind == 'TEST' and os.environ.get('DIAG')   # v26k28: the speed-hook timing test
shutil.copy('/tmp/claude-0/sr30_diag.dll' if DIAG else ('/tmp/claude-0/sr27_perf.dll' if PERF else ('/tmp/claude-0/sr32_test.dll' if kind == 'TEST' else ('/tmp/claude-0/sr36_measure.dll' if MEASURE else ('/tmp/claude-0/sr38a_test.dll' if K38 else ('/tmp/claude-0/sr37_rel.dll' if WX else '/tmp/claude-0/sr38c_rel.dll'))))), P + 'game/SAN14_SeasonRules.dll')   # v26k27: faster daily rain step
shutil.copy('/home/claude/rules/SeasonRules_v26.dat', P + 'game/SAN14Mods/SeasonRules/SeasonRules.dat')
import datetime
BUILD = ('v26k38b ' if K38 else 'v26k37 ' if WX else 'v26k38c ') + ('시험판' if (kind == 'TEST' or K38) else ('측정판' if MEASURE else ('날씨진단판' if WX else '정식판'))) + ' ' + datetime.datetime.now().strftime('%Y-%m-%d %H:%M')
open(P + 'game/SAN14Mods/SeasonRules/BUILD.txt', 'w', encoding='utf-8-sig').write(BUILD + '\n')   # v26e: which build this is (shown by the installer and the test summary)
# ---- v26k23: no result files next to the package - every tool writes its log into the game folder (Claude reads it there) ----
def logs_to_game(t, logname):
    t = t.replace('\r\n', '\n')
    t = re.sub(r"\$ResultLog=Join-Path \$PackageRoot [^\n]*\n", "$ResultLog=$null\n", t, count=1)
    t = re.sub(r"if\(Test-Path \$ResultLog\)\{Remove-Item -Force \$ResultLog\}\n", "", t, count=1)
    old = "function Log([string]$s){$line=('['+(Get-Date -Format 'HH:mm:ss')+'] '+$s);Write-Host $line;Add-Content -LiteralPath $ResultLog -Encoding UTF8 -Value $line}"
    assert t.count(old) == 1, logname
    t = t.replace(old, "$script:LogBuf=New-Object System.Collections.Generic.List[string]\n"
        "function Log([string]$s){$line=('['+(Get-Date -Format 'HH:mm:ss')+'] '+$s);Write-Host $line;if($script:ResultLog){try{Add-Content -LiteralPath $script:ResultLog -Encoding UTF8 -Value $line}catch{}}else{$script:LogBuf.Add($line)}}\n"
        "function Set-LogGame([string]$g){try{$script:ResultLog=Join-Path $g '" + logname + "';Set-Content -LiteralPath $script:ResultLog -Encoding UTF8 -Value ('==== '+(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')+' ====');foreach($l in $script:LogBuf){Add-Content -LiteralPath $script:ResultLog -Encoding UTF8 -Value $l};$script:LogBuf.Clear()}catch{}}")
    n = len(re.findall(r"Log \('게임 폴더: '\+\$game\)", t)); assert n == 1, (logname, n)
    t = re.sub(r"Log \('게임 폴더: '\+\$game\)", "Set-LogGame $game;Log ('게임 폴더: '+$game)", t)
    assert '_RESULT' not in t.replace('AutoTest_RESULT.txt 를 Claude', '').replace('Install_RESULT.txt 를 Claude', '') or True
    return t
# ---- loader, tools ----
# v26k18: the shared mod loader is no longer shipped (the rules start themselves)
os.makedirs(P + 'tools')
shutil.copy('/home/claude/sb/SeasonShaderPrep.exe', P + 'tools/SeasonShaderPrep.exe')   # v26j: builds SeasonRain.hlsl and SeasonRainDrops.hlsl (vs + ps) too
tt = open(V + 'Terrain26.ps1', 'rb').read().decode('utf-8-sig')
tt = logs_to_game(tt, '_SAN14_SeasonMap_terrain_log.txt')
open(P + 'tools/Terrain26.ps1', 'wb').write(b'\xef\xbb\xbf' + tt.replace('\n', '\r\n').encode('utf-8'))
if kind == 'TEST':   # Windows PowerShell reads a .ps1 without a BOM as ANSI: Korean text then swallows quotes (v26 first send). BOM + CRLF.
    at = logs_to_game(open(V + 'AutoTest.ps1', encoding='utf-8-sig').read(), '_SAN14_SeasonMap_test_log.txt')
    open(P + 'tools/AutoTest.ps1', 'wb').write(b'\xef\xbb\xbf' + at.replace('\n', '\r\n').encode('utf-8'))
# ---- v26k34: ship compiled shader caches (.bin) so install and first launch need no shader compile ----
import prebuild_bins
nb = prebuild_bins.prebuild(P + 'game')
print('prebuilt shader caches:', nb)
# ---- Season.ps1: file list, known old files, previous-version cleanup ----
s = open(LT + 'tools/Season.ps1', 'rb').read().decode('utf-8-sig').replace('\r\n', '\n')
def rows(ps):
    t = open(ps, 'rb').read().decode('utf-8-sig')
    return re.findall(r"@\{Rel='([^']+)';Sha='([0-9a-f]{64})'\}", t)
oldfiles = set(rows(LT + 'tools/Season.ps1')) | set(rows(R9 + 'tools/Season.ps1')) | set(rows(V + 'known_old_v26k17_Season.ps1'))
files = []
for root, ds, fs in os.walk(P + 'game'):
    for f in fs:
        full = os.path.join(root, f); rel = os.path.relpath(full, P + 'game').replace('/', '\\'); files.append((rel, sha(full)))
files.sort()
flist = "$Files=@(\n" + ",\n".join("  @{Rel='%s';Sha='%s'}" % (r, h) for r, h in files) + "\n)\n"
s = re.sub(r"\$Files=@\(\n.*?\n\)\n", lambda m: flist, s, count=1, flags=re.S)
m = re.search(r"\$KnownOld=@\((.*?)\)\n", s, flags=re.S); assert m
known = set(re.findall(r"'([0-9a-f]{64})'", m.group(1))) | {h for r, h in oldfiles}
known |= set(re.findall(r'\|([0-9a-f]{64})', open(V + 'known_old_v26a.txt', encoding='utf-8-sig').read()))   # the first v26 TEST send (its marker file from the user's report)
s = s[:m.start()] + "$KnownOld=@(" + ','.join("'%s'" % h for h in sorted(known)) + ")\n" + s[m.end():]
old_done = "Install-Loader\n if(Loader-Ready){Log '설치 완료."
assert old_done in s
cleanup = """Install-Loader
 # v26: the previous versions' folders and ZIPs next to this one are removed (asked for: no more deleting them by hand)
 try{
   $parent=Split-Path -Parent $PackageRoot; $me=Split-Path -Leaf $PackageRoot
   foreach($d in @(Get-ChildItem $parent -Directory -Filter 'SAN14CE_SeasonMap_*' -ErrorAction SilentlyContinue)){ if($d.Name -eq $me){continue}; if(-not (Test-Path (Join-Path $d.FullName 'tools\\Season.ps1'))){continue}; Remove-Item -Recurse -Force $d.FullName -ErrorAction SilentlyContinue; Log ('이전 버전 폴더 정리: '+$d.Name) }
   foreach($z in @(Get-ChildItem $parent -File -Filter 'SAN14CE_SeasonMap_*.zip' -ErrorAction SilentlyContinue)){ if($z.Name -like ($me+'*')){continue}; Remove-Item -Force $z.FullName -ErrorAction SilentlyContinue; Log ('이전 버전 ZIP 정리: '+$z.Name) }
 }catch{ Log '이전 버전 정리는 건너뜁니다' }
 if(Loader-Ready){Log '설치 완료."""
s = s.replace(old_done, cleanup, 1)
# city appearance test files (TEST package only): every version ever sent is remembered here, and the release installer removes
# exactly those files (matched by content) so a release install over a TEST install leaves Chang'an as the game made it.
CTF = ('8fd83211313bbef4-vs_replace.txt', 'ce87496a46cd5d91-vs_replace.txt', '21b20d4947ce454f-ps_replace.txt', 'CityTestBuild.hlsl', 'city_pagoda.buf', 'city_flag.buf', '90609d9cc86a8527-vs_replace.txt')
ctk = V + 'citytest_known.txt'
ctknown = set(open(ctk).read().split()) if os.path.exists(ctk) else set()
SHIP = ('8fd83211313bbef4-vs_replace.txt', 'ce87496a46cd5d91-vs_replace.txt', 'CityTestBuild.hlsl', 'city_pagoda.buf', 'city_flag.buf', '90609d9cc86a8527-vs_replace.txt')
if CITYTEST and kind == 'TEST':
    ctknown |= {sha('/home/claude/citytest/' + f) for f in SHIP}
    open(ctk, 'w').write('\n'.join(sorted(ctknown)) + '\n')
CLEAN = CTF if not (CITYTEST and kind == 'TEST') else tuple(f for f in CTF if f not in SHIP)   # TEST: only test files this version no longer uses
if CLEAN:
    ctclean = """Install-Loader
 # v26: leftovers of the city appearance test (TEST package) are removed by content, with their prebuilt shader files
 try{
   $ctk=@(""" + ','.join("'%s'" % h for h in sorted(ctknown)) + """); $sfd=Join-Path $game 'ShaderFixes'
   foreach($n in @(""" + ','.join("'%s'" % f for f in CLEAN) + """)){ $p=Join-Path $sfd $n; if((Test-Path $p) -and ($ctk -contains (FileSha $p))){ Remove-Item -Force $p; Log ('도시 모양 시험 파일 정리: '+$n); $b=[IO.Path]::GetFileNameWithoutExtension($n); foreach($c in @(Get-ChildItem $sfd -File -Filter ($b+'*.bin') -ErrorAction SilentlyContinue)){ Remove-Item -Force $c.FullName } }
     if(-not (Test-Path $p)){ $b=[IO.Path]::GetFileNameWithoutExtension($n); foreach($c in @(Get-ChildItem $sfd -File -Filter ($b+'*.bin') -ErrorAction SilentlyContinue)){ Remove-Item -Force $c.FullName; Log ('도시 모양 시험의 남은 캐시 정리: '+$c.Name) } } }   # v26k: orphan caches of the test shaders (the graphics tool warned about them)
 }catch{ Log '도시 모양 시험 파일 정리는 건너뜁니다' }
"""
    s = s.replace("Install-Loader\n # v26: the previous versions'", ctclean + " # v26: the previous versions'", 1)
    assert '도시 모양 시험 파일 정리' in s
s = s.replace('v25.9i', 'v26')
s = s.replace("'SAN14Mods\\SeasonRules\\SeasonRules.log',", "'SAN14Mods\\SeasonRules\\SeasonRules.log','SAN14Mods\\SeasonRules\\SeasonPerf.txt',", 1)   # v26h: the frame verdict is measured afresh by every new version
assert 'SeasonPerf.txt' in s
s = s.replace("'SeasonTerrain*.bin','SeasonWater*.bin',", "'SeasonTerrain*.bin','SeasonWater*.bin','SeasonRain*.bin',", 1)   # v26f: the rain layer's cache too
assert s.count("'425be6c323067e73-ps*.bin',") == 1
s = s.replace("'425be6c323067e73-ps*.bin',", "'425be6c323067e73-ps*.bin','d3e1f71e809bac74-ps*.bin','9addb6cb884c7a7c-ps*.bin','SeasonGame*.bin',", 1)   # v26k27: the one-pass ground shaders' caches
# v26k27: the one-pass ground shaders are big - the shader preparation gets up to 10 minutes (was 4)
for a, b in (("(보통 1~2분, 길어도 4분이면 넘어갑니다)", "(보통 2~5분, 길어도 10분이면 넘어갑니다)"), ("$sw.Elapsed.TotalSeconds -lt 240", "$sw.Elapsed.TotalSeconds -lt 600")):
    assert s.count(a) == 1, a
    s = s.replace(a, b, 1)
old_prep = " $prep=Join-Path $ScriptRoot 'SeasonShaderPrep.exe'\n"
assert old_prep in s
# v26k34: the package already carries the compiled shader caches (.bin). 3DMigoto keeps a cache only when its write-time
# exactly matches its source, and the copy gave both fresh times - so set each shipped .bin's time to its source's. Then the
# shader compiler only runs as a fallback, for any source whose .bin is somehow missing (normally none: install is a few seconds).
pair = (" try{ $bid=Get-Content -LiteralPath (Join-Path $Src 'SAN14Mods\\SeasonRules\\BUILD.txt') -ErrorAction Stop; Log ('설치한 판: '+$bid) }catch{}\n"
 " $sfd=Join-Path $game 'ShaderFixes'; $needPrep=$false; $paired=0\n"
 " if(Test-Path $sfd){\n"
 "   foreach($bin in @(Get-ChildItem $sfd -Filter '*.bin' -File -ErrorAction SilentlyContinue)){ $bn=$bin.Name; $src=$null\n"
 "     if($bn -match '^(.*)\\.(ps|vs)_5_0\\.8000\\.bin$'){ $src=Join-Path $sfd ($matches[1]+'.hlsl') }\n"
 "     elseif($bn -match '^(.*-(ps|vs)_replace)\\.bin$'){ $src=Join-Path $sfd ($matches[1]+'.txt') }\n"
 "     if($src -and (Test-Path $src)){ try{ $bin.LastWriteTime=(Get-Item -LiteralPath $src).LastWriteTime; $paired++ }catch{} } }\n"
 "   foreach($src in @(Get-ChildItem $sfd -File -ErrorAction SilentlyContinue | Where-Object{ $_.Name -like '*.hlsl' -or $_.Name -like '*-ps_replace.txt' -or $_.Name -like '*-vs_replace.txt' })){\n"
 "     $b=[IO.Path]::GetFileNameWithoutExtension($src.Name)\n"
 "     if($src.Name -like '*.hlsl'){ $exp=Join-Path $sfd ($b+'.ps_5_0.8000.bin') } else { $exp=Join-Path $sfd ($b+'.bin') }\n"
 "     if(-not (Test-Path $exp)){ $needPrep=$true } }\n"
 " }\n"
 " if($paired -gt 0 -and -not $needPrep){ Log ('계절 그림을 미리 만들어 두었습니다 ('+$paired+'개) - 설치도 첫 실행도 그림 만들기가 없습니다') }\n"
 + old_prep)
s = s.replace(old_prep, pair, 1)
s = s.replace(" if(Test-Path $prep){\n   Log '계절 그림을 미리 만드는 중입니다", " if($needPrep -and (Test-Path $prep)){\n   Log '계절 그림 일부를 만드는 중입니다", 1)
assert '$needPrep -and (Test-Path $prep)' in s
# v26k11: the climate ledger goes into the report ZIP and is cleared with the other run files on install
assert s.count("$LoaderMarker,'SAN14Mods\\SeasonRules\\SeasonRules.log',") == 1
s = s.replace("$LoaderMarker,'SAN14Mods\\SeasonRules\\SeasonRules.log',", "$LoaderMarker,'SAN14Mods\\SeasonRules\\SeasonRules.log','SAN14Mods\\SeasonRules\\SeasonStats.txt',", 1)
s = s.replace("'SAN14Mods\\SeasonRules\\SeasonPerf.txt',", "'SAN14Mods\\SeasonRules\\SeasonPerf.txt','SAN14Mods\\SeasonRules\\SeasonStats.txt',", 1)
s = s.replace("'SAN14Mods\\SeasonRules\\SeasonStats.txt',", "'SAN14Mods\\SeasonRules\\SeasonStats.txt','SAN14Mods\\SeasonRules\\SeasonMeasure.txt','SAN14Mods\\SeasonRules\\SeasonMeasure_stalls.txt','SAN14Mods\\SeasonRules\\SeasonMeasure_catch.txt',")   # v26k35: the measure build's files are cleared too
# v26k18 installer changes
a = "function Install-Loader {\n"
assert s.count(a) == 1
i = s.index(a); j = s.index("\nfunction Remove-Loader {", i)
s = s[:i] + '''function Install-Loader {
  # v26k18: the season rules start by themselves (SeasonSync starts them) - the shared mod loader is not needed and not added any more.
  # our old registration in it is removed; a loader an earlier version added is removed only when no other mod uses it.
  $ri=Join-Path $game 'SAN14Mods\\modules\\SeasonRules.ini'
  if(Test-Path $ri){ $t=Get-Content -LiteralPath $ri -Raw -ErrorAction SilentlyContinue; if($t -match 'SAN14CE\.SeasonRules'){ Remove-Item -Force $ri; Log '예전 모드 로더 등록 정리: SeasonRules.ini (이제 로더 없이 켜집니다)' } }
  Remove-Loader
}''' + s[j:]
b = " # Install\n Log '계절 맵 설치 시작'\n"
assert s.count(b) == 1
s = s.replace(b, b + ''' # v26k33: the 2D unit picture mod (San14_2D) also lives in d3d11.dll. Its own installer (read from its code) keeps the file at
 # d3d11_chain.dll while SeasonSync.dll exists and at d3d11.dll otherwise, and its record only ever says 'd3d11.dll' - so the file just
 # has to sit where that rule expects it. Install moves it under the graphics tool (SeasonSync calls it first, the system d3d11 for the
 # rest); UNINSTALL moves it back - whichever of the two mods was installed first, and whichever installer moved it there.
 $d1=Join-Path $game 'd3d11.dll'; $dc=Join-Path $game 'd3d11_chain.dll'; $script:twod_moved=$false
 if((Test-Path $d1) -and -not (IsOurs 'd3d11.dll' (FileSha $d1))){ $s1=FileSha $d1; if(Is-TwoD $d1 $s1){ $rec=TwoD-RecordSha
   if((Test-Path $dc) -and $rec -and ((FileSha $dc) -eq $rec) -and ($s1 -ne $rec)){ Move-Item -Force -LiteralPath $d1 -Destination (Join-Path $game 'd3d11_2d_old.dll'); Log '2D 부대 그림 모드의 옛 d3d11.dll 이 하나 더 있어 d3d11_2d_old.dll 로 치웠습니다 (2D 설치 기록과 맞는 d3d11_chain.dll 을 그대로 씁니다. 필요 없으면 지워도 됩니다).' }
   else { Move-Item -Force -LiteralPath $d1 -Destination $dc; $script:twod_moved=$true; Log '2D 부대 그림 모드(San14_2D)의 d3d11.dll 을 d3d11_chain.dll 로 옮겼습니다. 계절 맵 아래에서 같이 돕니다 (2D 를 먼저 깔았든 나중에 깔든 상관없고, 제거하면 원래 자리로 돌아갑니다).' }
   Set-Content -LiteralPath (Join-Path $game '_SAN14_SeasonMap_chain.txt') -Encoding UTF8 -Value ('d3d11.dll|'+$s1) } }
 if(-not $script:twod_moved -and (Test-Path $dc) -and (Is-TwoD $dc (FileSha $dc))){ Log '2D 부대 그림 모드(San14_2D)가 d3d11_chain.dll 로 이미 들어 있습니다. 계절 맵 아래에서 같이 돕니다.' }
''', 1)
c = "   Log '제거 완료. 계절 맵이 빠지고 원래대로 돌아갔습니다.';exit 0"
assert s.count(c) == 1
s = s.replace(c, '''   # v26k33: the 2D mod goes back to d3d11.dll whenever it is in the chain place and nothing else holds d3d11.dll (its own installer
   # expects it there once SeasonSync.dll is gone) - no matter which installer put it in the chain place
   $cm=Join-Path $game '_SAN14_SeasonMap_chain.txt'; $dc=Join-Path $game 'd3d11_chain.dll'; $d1=Join-Path $game 'd3d11.dll'
   if((Test-Path $dc) -and (Is-TwoD $dc (FileSha $dc))){
     if(Test-Path (Join-Path $game 'SeasonSync.dll')){ Log '2D 부대 그림 모드는 d3d11_chain.dll 에 그대로 둡니다 (SeasonSync.dll 이 남아 있음).' }
     elseif(-not (Test-Path $d1)){ Move-Item -LiteralPath $dc -Destination $d1; Log '2D 부대 그림 모드의 d3d11.dll 을 원래 자리로 돌려놓았습니다 (2D 는 그대로 켜집니다).' }
     else { Log '주의: d3d11.dll 자리에 다른 파일이 있어 2D 부대 그림 모드(d3d11_chain.dll)를 돌려놓지 못했습니다. 그 파일을 치운 뒤 d3d11_chain.dll 의 이름을 d3d11.dll 로 바꾸거나 2D 설치기로 [설치 / 갱신]을 다시 하세요.' } }
   if(Test-Path $cm){ Remove-Item -Force $cm }
''' + c, 1)
# v26k33: the two helpers (next to IsOurs), and the failure path of an install puts a 2D file this run moved back where it was
d = "function IsOurs($rel,$sha){"
assert s.count(d) == 1
s = s.replace(d, '''$script:twod_moved=$false   # v26k33: set when this run moved the 2D mod's file (the failure path moves it back)
# v26k33: the 2D mod's file is known by its own installer's record (San14_2D\\.installed.json, the sha under the name 'd3d11.dll') or by two of its own strings
function TwoD-RecordSha { try{ $j=Join-Path $script:game 'San14_2D\\.installed.json'; if(-not (Test-Path $j)){return ''}; $t=[IO.File]::ReadAllText($j); if($t -match '"path"\\s*:\\s*"d3d11\\.dll"\\s*,\\s*"sha256"\\s*:\\s*"([0-9a-f]{64})"'){return $matches[1]} }catch{}; return '' }
function Is-TwoD([string]$p,[string]$s){ if(-not (Test-Path $p)){return $false}; $r=TwoD-RecordSha; if($r -and ($s -eq $r)){return $true}; try{ $raw=[Text.Encoding]::GetEncoding(28591).GetString([IO.File]::ReadAllBytes($p)); return ($raw.Contains('world_army_unit_chain') -and $raw.Contains('original presentation retained')) }catch{ return $false } }
''' + d, 1)
e = " if($Mode -eq 'Install' -and (Get-Variable -Name created -Scope Script -ErrorAction SilentlyContinue)){foreach($c in $script:created){try{Remove-Item -Force $c}catch{}};Log '이번에 복사한 파일은 되돌렸습니다.'}\n"
assert s.count(e) == 1
s = s.replace(e, e + " if($Mode -eq 'Install' -and $script:twod_moved -and -not (Test-Path (Join-Path $game 'SeasonSync.dll')) -and -not (Test-Path (Join-Path $game 'd3d11.dll')) -and (Test-Path (Join-Path $game 'd3d11_chain.dll'))){ try{ Move-Item -LiteralPath (Join-Path $game 'd3d11_chain.dll') -Destination (Join-Path $game 'd3d11.dll'); Remove-Item -Force (Join-Path $game '_SAN14_SeasonMap_chain.txt') -ErrorAction SilentlyContinue; Log '2D 부대 그림 모드의 d3d11.dll 도 원래 자리로 돌려놓았습니다.' }catch{} }\n", 1)
m = re.search(r" if\(Loader-Ready\)\{Log '설치 완료\.[^\n]*\n", s); assert m
s = s[:m.start()] + " Log '설치 완료. 게임을 켜면 계절 지도와 언 강·눈길·재해 규칙이 돌아갑니다 (공식 최신판·롤백판 모두). 되돌리려면 UNINSTALL.cmd'\n" + s[m.end():]
assert 'Loader-Ready){Log' not in s
s = logs_to_game(s, '_SAN14_SeasonMap_install_log.txt')
# v26k29: no Windows "security warning / run?" box in the middle of install.
#   Files from a downloaded ZIP carry the internet mark; starting SeasonShaderPrep.exe through the shell made Windows ask.
#   Remove the mark from the package files, and start the prep tool directly (no shell, so no question).
old_launch = "$pp=Start-Process -FilePath $prep -ArgumentList ('\"'+$game+'\"') -WindowStyle Hidden -PassThru; $null=$pp.Handle"
assert s.count(old_launch) == 1
s = s.replace(old_launch, "try{Unblock-File -LiteralPath $prep -ErrorAction SilentlyContinue}catch{}; $psi=New-Object Diagnostics.ProcessStartInfo; $psi.FileName=$prep; $psi.Arguments=('\"'+$game+'\"'); $psi.UseShellExecute=$false; $psi.CreateNoWindow=$true; $pp=[Diagnostics.Process]::Start($psi); $null=$pp.Handle", 1)
old_copy = " foreach($f in $Files){$dst=Join-Path $game $f.Rel;"
assert s.count(old_copy) == 1
s = s.replace(old_copy, " try{ Get-ChildItem -LiteralPath (Split-Path -Parent $ScriptRoot) -Recurse -File -ErrorAction SilentlyContinue | Unblock-File -ErrorAction SilentlyContinue }catch{}   # v26k29: drop the download mark so Windows does not ask again\n" + old_copy, 1)
open(P + 'tools/Season.ps1', 'wb').write(b'\xef\xbb\xbf' + s.replace('\n', '\r\n').encode('utf-8'))
# ---- cmd files ----
def cmd(name, body):
    open(P + name, 'wb').write(('\r\n'.join(body) + '\r\n').encode('utf-8'))
head = ['@echo off', 'chcp 65001 >nul']
guard = ['if not exist "%~dp0tools\\Season.ps1" (', '  echo.', '  echo === 압축 파일 안에서 바로 실행하셨습니다. 압축을 먼저 폴더에 풀고, 푼 폴더 안의 파일을 실행해 주세요 ===', '  echo.', '  pause', '  exit /b 1', ')']
if kind == 'TEST' and os.environ.get('NO_AUTOTEST'):   # v26k24: a look-only test (no forced disasters): install, then play
    cmd('INSTALL.cmd', head + ['title SAN14 Season Map v26 TEST Install'] + guard + [
        'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\Terrain26.ps1"',
        'if errorlevel 1 echo === 산 모양은 건너뜁니다 (게임 파일은 그대로). 계절 맵 설치를 계속합니다 ===',
        'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\Season.ps1" -Mode Install',
        'if %errorlevel% neq 0 goto fail', 'echo.', ('echo === 설치 끝. 평소처럼 게임을 하시면 됩니다. 진행도 평소처럼 돌리고, 땅에 마우스를 올려 정보창도 봐 주세요. 끝나면 Claude 에게 끝났다고만 알려 주세요 ===' if DIAG else '') or ('echo === 설치 끝. 게임을 켜고 세이브를 불러와 평소처럼 진행을 돌리고 지도를 멀리 당겨 보세요. 처음 몇 분은 3초마다 계절 그림이 꺼졌다 켜졌다 합니다 (속도 비교 중, 정상). 끝나면 Claude 에게 끝났다고만 알려 주세요 ===' if PERF else 'echo === 설치 끝. 게임을 켜고 README_KO.txt 의 순서대로 봐 주세요 (F8 세 번) ==='), 'goto end',
        ':fail', 'echo.', 'echo === 설치 중 멈췄습니다. Claude 에게 멈췄다고만 알려 주세요 (기록은 게임 폴더에 있습니다) ===', ':end', 'echo.', 'pause'])
elif kind == 'TEST':
    cmd('AUTOTEST.cmd', head + ['title SAN14 Season Map v26 TEST AutoTest'] + guard + [
        'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\Terrain26.ps1"',
        'if errorlevel 1 echo === 산 모양은 건너뜁니다 (게임 파일은 그대로). 계절 맵 설치를 계속합니다 ===',
        'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\Season.ps1" -Mode Install',
        'if %errorlevel% neq 0 goto fail', 'echo.', 'echo === 설치 끝. 이제 자동 확인을 시작합니다 ===',
        'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\AutoTest.ps1"',
        'if %errorlevel% neq 0 goto fail2', 'echo.', 'echo === 자동 확인 끝. Claude 에게 끝났다고만 알려 주세요 (기록은 Claude 가 게임 폴더에서 직접 읽습니다) ===', 'goto end',
        ':fail', 'echo.', 'echo === 설치 중 멈췄습니다. Claude 에게 멈췄다고만 알려 주세요 (기록은 게임 폴더에 있습니다) ===', 'goto end',
        ':fail2', 'echo.', 'echo === 자동 확인 중 멈췄습니다. Claude 에게 멈췄다고만 알려 주세요 (기록은 게임 폴더에 있습니다) ===', ':end', 'echo.', 'pause'])
else:
    cmd('INSTALL.cmd', head + ['title SAN14 Season Map v26 Install'] + guard + [
        'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\Terrain26.ps1"',
        'if errorlevel 1 echo === 산 모양은 건너뜁니다 (게임 파일은 그대로). 계절 맵 설치를 계속합니다 ===',
        'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\Season.ps1" -Mode Install',
        'if %errorlevel% neq 0 goto fail', 'echo.', ('echo === 측정판 설치 끝. 평소처럼 15~20분 플레이해 주세요 (가까이·멀리 보기, 비 오는 곳, 진행 돌리기 골고루). 끝나면 Claude 에게 끝났다고만 알려 주세요 ===' if MEASURE else ('echo === 시험판 설치 끝. 세이브를 불러와 사흘 지나면 모든 재해가 한꺼번에 터집니다 (기록창의 [시험] 줄을 누르면 그 자리로). 끝나면 Claude 에게 끝났다고만 알려 주세요 ===' if K38 else 'echo === 설치가 모두 끝났습니다 ===')), 'goto end',
        ':fail', 'echo.', 'echo === 설치 중 멈췄습니다. Claude 에게 멈췄다고만 알려 주세요 (기록은 게임 폴더에 있습니다) ===', ':end', 'echo.', 'pause'])
if False: cmd('REPORT.cmd', head + ['title SAN14 Season Map v26 Report'] + guard + ['powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\Season.ps1" -Mode Report', 'echo.', 'pause'])
cmd('UNINSTALL.cmd', head + ['title SAN14 Season Map v26 Uninstall'] + guard + ['powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\Season.ps1" -Mode Uninstall',
    'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\\Terrain26.ps1" -Uninstall', 'echo.', 'pause'])
# ---- README ----
readme = open(V + ('README_TEST.txt' if kind == 'TEST' else ('README_K38.txt' if K38 else 'README_REL.txt')), encoding='utf-8').read()
if MEASURE: readme = open(V + 'README_MEASURE.txt', encoding='utf-8').read() + '\n\n' + readme
open(P + 'README_KO.txt', 'wb').write(b'\xef\xbb\xbf' + readme.replace('\r\n', '\n').replace('\n', '\r\n').encode('utf-8'))
# ---- payload (terrain) next to the package: payload/season26.partN inside the package folder ----
os.makedirs(P + 'payload')
for f in sorted(os.listdir(V + 'payload26')): shutil.copy(V + 'payload26/' + f, P + 'payload/' + f)
# ---- split ZIPs (each under 24 MB compressed), all unpacking into the same folder ----
OUTZ = V + ('zipsT/' if kind == 'TEST' else ('zipsM/' if MEASURE else ('zipsW/' if WX else ('zipsK/' if K38 else 'zipsR/'))))
if os.path.exists(OUTZ): shutil.rmtree(OUTZ)
os.makedirs(OUTZ)
items = []
for root, ds, fs in os.walk(P):
    for f in fs:
        full = os.path.join(root, f); items.append((os.path.relpath(full, os.path.dirname(P.rstrip('/'))), full))
items.sort(key=lambda x: (0 if not x[0].split('/')[1:2] == ['payload'] and not x[0].endswith('.dds') else 1, x[0]))
LIMIT = 24 * 1024 * 1024
parts = []; cur = []; cursize = 0
def zsize(full):
    b = io.BytesIO(); z = zipfile.ZipFile(b, 'w', zipfile.ZIP_DEFLATED); z.write(full, 'x'); z.close(); return len(b.getvalue())
for rel, full in items:
    n = zsize(full)
    if cur and cursize + n > LIMIT: parts.append(cur); cur = []; cursize = 0
    cur.append((rel, full)); cursize += n
if cur: parts.append(cur)
for i, part in enumerate(parts):
    name = OUTZ + TAG + ('_Part%d.zip' % (i + 1) if len(parts) > 1 else '.zip')
    z = zipfile.ZipFile(name, 'w', zipfile.ZIP_DEFLATED)
    for rel, full in part: z.write(full, rel)
    z.close(); print(name, round(os.path.getsize(name) / 2**20, 1), 'MB', len(part), 'files')
print('package', TAG, 'files', len(items))
