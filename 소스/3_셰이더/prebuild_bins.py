# v26k34: ship 3DMigoto's compiled shader caches (.bin) so install and first launch need no shader compile.
# Each .bin is produced by SeasonShaderPrep.exe through the package's own d3dcompiler_46.dll (the real Microsoft
# compiler) under wine - the exact bytecode the user's game would make on first run. Keyed by source sha in bincache/,
# so a shader is only recompiled when its source actually changes. The installer re-pairs the bin's write-time to its
# source after copying (3DMigoto keeps a cache only on an exact timestamp match), so the shipped bins are used as-is.
import os, hashlib, shutil, subprocess, sys
CACHE = '/home/claude/season/v26/bincache/'
WINE = '/tmp/claude-0/winedl/wine-10.0-amd64/bin/wine64'
WENV = dict(os.environ, PATH='/tmp/claude-0/winedl/wine-10.0-amd64/bin:' + os.environ.get('PATH', ''),
            WINEPREFIX='/tmp/claude-0/winedl/prefix', WINEDEBUG='-all', WINEDLLOVERRIDES='mscoree,mshtml=', DISPLAY='')
sha = lambda p: hashlib.sha256(open(p, 'rb').read()).hexdigest()

def _pairs(sfd):
    # source file -> the bin name(s) SeasonShaderPrep.exe makes from it (only sources present in this package)
    p = {}
    for base in ('SeasonTerrainNear', 'SeasonTerrainFar', 'SeasonWaterWinter', 'SeasonRain', 'SeasonRainB', 'SeasonGameNear', 'SeasonGameFar'):
        if os.path.exists(sfd + base + '.hlsl'): p[base + '.hlsl'] = [base + '.ps_5_0.8000.bin']
    if os.path.exists(sfd + 'SeasonRainDrops.hlsl'):
        p['SeasonRainDrops.hlsl'] = ['SeasonRainDrops.vs_5_0.8000.bin', 'SeasonRainDrops.ps_5_0.8000.bin']
    if os.path.exists(sfd + 'SeasonRainDropsB.hlsl'):
        p['SeasonRainDropsB.hlsl'] = ['SeasonRainDropsB.vs_5_0.8000.bin', 'SeasonRainDropsB.ps_5_0.8000.bin']
    for f in os.listdir(sfd):
        if f.endswith('-ps_replace.txt'): p[f] = [f[:-4] + '.bin']
        elif f.endswith('-vs_replace.txt'): p[f] = [f[:-4] + '.bin']
    return p

def prebuild(game_dir):
    sfd = os.path.join(game_dir, 'ShaderFixes') + '/'
    pairs = _pairs(sfd)
    want = {}                      # bin name -> source sha (for cache keys)
    miss = False
    for src, bins in pairs.items():
        s = sha(sfd + src)
        for b in bins:
            want[b] = s
            if not os.path.exists(CACHE + s + '__' + b): miss = True
    if miss:
        # a source changed (or first time): compile the whole set once through the real d3dcompiler_46 under wine, cache it
        if not os.path.exists(WINE): raise SystemExit('prebuild: wine missing and bincache incomplete - cannot ship .bin')
        subprocess.run([WINE, os.path.join(game_dir, '..', 'tools', 'SeasonShaderPrep.exe'),
                        'Z:' + os.path.abspath(game_dir).replace('/', '\\')], env=WENV, check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1800)
        for b, s in want.items():
            if os.path.exists(sfd + b): shutil.copy(sfd + b, CACHE + s + '__' + b)
    # place every wanted bin from the cache into the package (prep run leaves them too, but the cache is the source of truth)
    n = 0
    for b, s in want.items():
        c = CACHE + s + '__' + b
        if os.path.exists(c): shutil.copy(c, sfd + b); n += 1
        else: raise SystemExit('prebuild: missing cache for ' + b)
    # clean any stale .bin the prep produced that we are not shipping (e.g. citytest), keep only the wanted set
    for f in os.listdir(sfd):
        if f.endswith('.bin') and f not in want: os.remove(sfd + f)
    return n

if __name__ == '__main__':
    print('prebuilt', prebuild(sys.argv[1]), 'bins')
