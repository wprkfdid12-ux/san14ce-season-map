# package helper: refresh the installer's file list ($Files in tools\Season.ps1, sorted case-insensitively, KnownOld + the replaced
# files' old fingerprints) and SHA256SUMS.txt (every package file, sorted) after files in pkg\game changed.  usage: mkpkg.py <old_pkg> <new_pkg>
import os, re, sys, hashlib
old, new = sys.argv[1], sys.argv[2]
def sha(p): return hashlib.sha256(open(p, 'rb').read()).hexdigest()
def gamefiles(pkg):
    out = {}
    for root, ds, fs in os.walk(os.path.join(pkg, 'game')):
        for f in fs:
            p = os.path.join(root, f); out[os.path.relpath(p, os.path.join(pkg, 'game')).replace('/', '\\')] = sha(p)
    return out
og, ng = gamefiles(old), gamefiles(new)
ps = os.path.join(new, 'tools', 'Season.ps1'); s = open(ps, encoding='utf-8-sig', newline='').read()
nl = '\r\n' if '\r\n' in s else '\n'
m = re.search(r"\$Files=@\(" + re.escape(nl) + r"(.*?)" + re.escape(nl) + r"\)", s, re.S); assert m
lines = ["  @{Rel='%s';Sha='%s'}" % (k, ng[k]) for k in sorted(ng, key=str.lower)]
s = s[:m.start(1)] + (',' + nl).join(lines) + s[m.end(1):]
km = re.search(r"\$KnownOld=@\(([^)]*)\)", s); known = re.findall(r"'([0-9a-f]{64})'", km.group(1))
add = [og[k] for k in og if k in ng and og[k] != ng[k] and og[k] not in known]
if add: s = s[:km.end(1)] + ''.join(",'%s'" % h for h in add) + s[km.end(1):]
open(ps, 'w', encoding='utf-8-sig', newline='').write(s)
print('files', len(ng), 'changed', sorted(k for k in og if k in ng and og[k] != ng[k]), 'new', sorted(set(ng) - set(og)), 'gone', sorted(set(og) - set(ng)), 'knownold +', len(add))
allf = []
for root, ds, fs in os.walk(new):
    for f in fs:
        p = os.path.relpath(os.path.join(root, f), new).replace(os.sep, '/')
        if p != 'SHA256SUMS.txt': allf.append(p)
sums = ''.join('%s  %s\n' % (sha(os.path.join(new, p)), p) for p in sorted(allf))
raw0 = open(os.path.join(old, 'SHA256SUMS.txt'), 'rb').read()
bom = raw0.startswith(b'\xef\xbb\xbf'); crlf = b'\r\n' in raw0
if crlf: sums = sums.replace('\n', '\r\n')
open(os.path.join(new, 'SHA256SUMS.txt'), 'wb').write((b'\xef\xbb\xbf' if bom else b'') + sums.encode('utf-8'))
print('sums', len(allf), 'bom', bom, 'crlf', crlf)
