# Terrain data patch for the season map v26 (k23): the v13 rock tiles / cliffs (unchanged) + mountain heights v24 (ways and side ways at the original heights, rolling where lifted, no gutter along the walls) (v17 + tilted fault-block ranges, hill re-form, wide flank gullies by regional level 1-10 cross-checked against each range).
import re, hashlib, pickle, struct, zlib, os, sys
sys.path.insert(0, '/home/claude/tv')
B = '/home/claude/season/v10/build/'
OUT = '/home/claude/season/v26/'
sha = lambda b: hashlib.sha256(b).hexdigest()
def rows_of(ps1):
    s = open(ps1, 'rb').read().decode('utf-8-sig'); out = []
    for r in re.findall(r"@\{Kind=.*?\}", s):
        d = {}
        for k, v in re.findall(r"(\w+)=(?:\[\w+\])?('[^']*'|\$\w+|-?\d+)", r): d[k] = v.strip("'")
        out.append(d)
    return out
old = rows_of(B + 'Terrain13.ps1')
pay13 = b''.join(open(B + 'payload13/season13.part%d' % i, 'rb').read() for i in (1, 2, 3))
blob = bytearray(); index = {}
def add(b):
    h = sha(b)
    if h not in index: index[h] = (len(blob), len(b)); blob.extend(b)
    return index[h]
def segs_from_payload(segstr, pay):
    out = []
    for sg in segstr.split(';'):
        q = sg.split(',')
        if q[0] == 'O': out.append(('O', int(q[1]), int(q[2])))
        else: out.append(('P', pay[int(q[1]):int(q[1]) + int(q[2])]))
    return out
def segstr(sg):
    parts = []
    for s in sg:
        if s[0] == 'O': parts.append('O,%d,%d' % (s[1], s[2]))
        else: o, n = add(s[1]); parts.append('P,%d,%d' % (o, n))
    return ';'.join(parts)
mhk = pickle.load(open('/home/claude/tv/v24mhk_fit.pkl', 'rb')); tex = pickle.load(open('/home/claude/tv/v24tex_fit.pkl', 'rb'))
mhk22 = pickle.load(open('/home/claude/tv/v22mhk_fit.pkl', 'rb')); tex22 = pickle.load(open('/home/claude/tv/v22tex_fit.pkl', 'rb'))
key22 = {'mhk_948': mhk22['948'], 'mhk_613': mhk22['613'], 'mhk_613b2': mhk22['613'],
       'tex_bin14_0': tex22['bin14_0'], 'tex_bin14': tex22['bin14'], 'tex_bin': tex22['bin'], 'tex_bin2': tex22['bin']}
mhk21 = pickle.load(open('/home/claude/tv/v21mhk_fit.pkl', 'rb')); tex21 = pickle.load(open('/home/claude/tv/v21tex_fit.pkl', 'rb'))
key21 = {'mhk_948': mhk21['948'], 'mhk_613': mhk21['613'], 'mhk_613b2': mhk21['613'],
       'tex_bin14_0': tex21['bin14_0'], 'tex_bin14': tex21['bin14'], 'tex_bin': tex21['bin'], 'tex_bin2': tex21['bin']}
mhk19 = pickle.load(open('/home/claude/tv/v19mhk_fit.pkl', 'rb')); tex19 = pickle.load(open('/home/claude/tv/v19tex_fit.pkl', 'rb'))
key19 = {'mhk_948': mhk19['948'], 'mhk_613': mhk19['613'], 'mhk_613b2': mhk19['613'],
       'tex_bin14_0': tex19['bin14_0'], 'tex_bin14': tex19['bin14'], 'tex_bin': tex19['bin'], 'tex_bin2': tex19['bin']}
mhk18 = pickle.load(open('/home/claude/tv/v18mhk_fit.pkl', 'rb')); tex18 = pickle.load(open('/home/claude/tv/v18tex_fit.pkl', 'rb'))
key18 = {'mhk_948': mhk18['948'], 'mhk_613': mhk18['613'], 'mhk_613b2': mhk18['613'],
       'tex_bin14_0': tex18['bin14_0'], 'tex_bin14': tex18['bin14'], 'tex_bin': tex18['bin'], 'tex_bin2': tex18['bin']}
mhk17 = pickle.load(open('/home/claude/tv/v17mhk_fit.pkl', 'rb')); tex17 = pickle.load(open('/home/claude/tv/v17tex_fit.pkl', 'rb'))
key17 = {'mhk_948': mhk17['948'], 'mhk_613': mhk17['613'], 'mhk_613b2': mhk17['613'],
       'tex_bin14_0': tex17['bin14_0'], 'tex_bin14': tex17['bin14'], 'tex_bin': tex17['bin'], 'tex_bin2': tex17['bin']}
key = {'mhk_948': mhk['948'], 'mhk_613': mhk['613'], 'mhk_613b2': mhk['613'],
       'tex_bin14_0': tex['bin14_0'], 'tex_bin14': tex['bin14'], 'tex_bin': tex['bin'], 'tex_bin2': tex['bin']}
T = []
for r in old:
    if r['Name'] in key:
        a, b, sg = key[r['Name']]
        assert a == r['Orig'], r['Name']
        prev = [p for p in r['Prev'].split('|') if p] + [r['New'], key17[r['Name']][1], key18[r['Name']][1], key19[r['Name']][1], key21[r['Name']][1], key22[r['Name']][1]]   # v13 and v17 (season map v26 up to k18) heights are known previous states
        T.append(dict(r, New=b, Prev='|'.join(prev), Segs=segstr(sg)))
    else:
        T.append(dict(r, Segs=segstr(segs_from_payload(r['Segs'], pay13))))   # tiles and cliffs: as in v13
SIM = {'FieldEditor4.rdb.bin': '/mnt/user-data/uploads/FieldEditor4_rdb.bin', 'FieldEditor4.rdb.bin2': '/mnt/user-data/uploads/FieldEditor4_rdb.bin',
       'FieldEditor4.rdb.bin14': '/mnt/user-data/uploads/FieldEditor4_rdb.bin14', 'FieldEditor4.rdb.bin14_0': '/mnt/user-data/uploads/FieldEditor4_rdb.bin14_0'}
for t in T:
    f = open(SIM[t['File']], 'rb'); f.seek(int(t['Offset'])); og = f.read(int(t['Len']))
    assert sha(og) == t['Orig'], t['Name']
    nb = bytearray()
    for sg in t['Segs'].split(';'):
        q = sg.split(','); a, n = int(q[1]), int(q[2])
        nb += og[a:a + n] if q[0] == 'O' else blob[a:a + n]
    assert len(nb) == int(t['Len']) and sha(bytes(nb)) == t['New'], t['Name']
    p = int.from_bytes(nb[8:12], 'little') - int.from_bytes(nb[16:20], 'little')
    while True:
        m, = struct.unpack_from('<I', nb, p); p += 4
        if m == 0: break
        dz = zlib.decompressobj(); dz.decompress(bytes(nb[p:p + m])); assert dz.eof and not dz.unused_data; p += m
    print('verified', t['Name'])
PART = 23 * 1024 * 1024
parts = [bytes(blob[i:i + PART]) for i in range(0, len(blob), PART)]
os.makedirs(OUT + 'payload26', exist_ok=True)
names = []
for i, p in enumerate(parts):
    nm = 'season26.part%d' % (i + 1); names.append(nm); open(OUT + 'payload26/' + nm, 'wb').write(p)
print('payload MB', round(len(blob) / 2**20, 1), 'parts', len(parts))
rows = []
for t in T:
    rows.append("@{Kind='TEX';Name='%s';File='%s';FileLen=[long]%s;Offset=[long]%s;Len=[int]%s;Orig='%s';New='%s';Prev='%s';Segs='%s';Optional=%s}" % (
        t['Name'], t['File'], t['FileLen'], t['Offset'], t['Len'], t['Orig'], t['New'], t['Prev'], t['Segs'], t['Optional']))
s = open(B + 'Terrain13.ps1', 'rb').read().decode('utf-8-sig').replace('\r\n', '\n')
s = re.sub(r"\$Targets=@\(\n.*?\n\)\n", lambda m: "$Targets=@(\n" + ',\n'.join(' ' + r for r in rows) + "\n)\n", s, flags=re.S)
s = re.sub(r"(\$PaySha\['season13\.part\d'\]=.*?\n)+", lambda m: ''.join("$PaySha['%s']='%s'\n" % (n, sha(p)) for n, p in zip(names, parts)), s, flags=re.S)
s = s.replace("@('season13.part1','season13.part2','season13.part3')", "@(" + ','.join("'%s'" % n for n in names) + ")")
s = s.replace("이미 v13 그대로라 건드리지 않습니다", "이미 v26 그대로라 건드리지 않습니다")
s = s.replace("(계절 맵 v15 의 Part2·Part3·Part4 ZIP 을 이 폴더와 같은 위치에 풀어 주세요)", "(계절 맵 v26 의 Part2·Part3 ZIP 을 이 폴더와 같은 위치에 풀어 주세요)")
assert 'season13' not in s and 'season26.part1' in s
# v26k21: a previous season-map mountain state with its original backup gone (found on the user's PC): the new data is built from the
#         installed state itself - every part it keeps is untouched there (checked for v17/v18/v19) and the result is checked against
#         the expected fingerprint before anything is written; no backup can be made then, so UNINSTALL needs Steam's file check
a1 = "if($null -eq $orig){throw ($t.File+' ['+$t.Name+'] 이전 산 패치 상태인데 원본 백업을 찾지 못했습니다. 이전 패치의 UNINSTALL 후 다시 실행해 주세요.')};Log ($t.File+' ['+$t.Name+']: 이전 산 패치 상태 -> 원본 백업에서 순정 데이터를 가져옴')"
assert s.count(a1) == 1
s = s.replace(a1, "if($null -eq $orig){$orig=$cur;Log ($t.File+' ['+$t.Name+']: 이전 산 패치 상태, 원본 백업 없음 -> 지금 상태에서 바로 새 산으로 바꿈 (확인 후 적용)')}else{Log ($t.File+' ['+$t.Name+']: 이전 산 패치 상태 -> 원본 백업에서 순정 데이터를 가져옴')}")
a2 = "foreach($x in $plan){$t=$x[0];$bf=Join-Path $bk ($t.Name+'.original.idrk');if(-not(Test-Path $bf))"
assert s.count(a2) == 1
s = s.replace(a2, "foreach($x in $plan){$t=$x[0];$bf=Join-Path $bk ($t.Name+'.original.idrk');if([MS4]::Sha($x[3]) -ne $t.Orig){Log ($t.File+' ['+$t.Name+']: 원본 백업이 없어 백업은 건너뜀 (되돌릴 때는 Steam 의 게임 파일 무결성 검사)');continue};if(-not(Test-Path $bf))")
open(OUT + 'Terrain26.ps1', 'wb').write(b'\xef\xbb\xbf' + s.replace('\n', '\r\n').encode('utf-8'))
print('script written', len(rows), 'targets', names)
