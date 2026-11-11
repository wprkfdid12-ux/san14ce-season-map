#!/usr/bin/env python3
# 계절 맵 깃허브 판 올리기 도구 (2026-11-11)
#  사용: season_release.py <패키지 폴더> <저장소 폴더> <날짜 yyyy-mm-dd> <제목> <바뀐내용.txt>
#  - 패키지 폴더에 SHA256SUMS.txt 를 새로 쓴다 (설치기 exe·BUILD_ERROR.txt·SHA256SUMS.txt 자신은 빼고)
#  - 저장소에 기본판(updates/base/*)이 없으면 이 패키지 전체로 기본판 조각(48MB 미만)을 만든다
#  - updates/<판>/SAN14CE_SeasonMap_<판>.zip = 기본판과 다른 파일 + SHA256SUMS.txt
#  - updates/index.json 의 releases 맨 앞에 이 판을 넣는다 (같은 판이 있으면 바꿈)
#  판 번호는 패키지 game/SAN14Mods/SeasonRules/BUILD.txt 첫 낱말.
import hashlib, json, os, re, sys, zipfile

SKIP = {'SHA256SUMS.txt', 'support/installer/SeasonSetup.exe', 'support/installer/BUILD_ERROR.txt'}
PART_LIMIT = 48 * 1024 * 1024

def sha(p):
    h = hashlib.sha256()
    with open(p, 'rb') as f:
        for b in iter(lambda: f.read(1 << 20), b''): h.update(b)
    return h.hexdigest()

def walk(pkg):
    out = {}
    for d, _, fs in os.walk(pkg):
        for f in fs:
            full = os.path.join(d, f); rel = os.path.relpath(full, pkg).replace(os.sep, '/')
            if rel in SKIP or rel.endswith('.part') or rel.endswith('.tmp'): continue
            out[rel] = sha(full)
    return dict(sorted(out.items()))

def zip_files(path, pkg, rels, extra=None):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for rel in rels:
            z.write(os.path.join(pkg, rel.replace('/', os.sep)), rel)
        if extra:
            for name, data in extra.items(): z.writestr(name, data)

def main():
    pkg, repo, date, title, notes_file = sys.argv[1:6]
    notes = open(notes_file, encoding='utf-8').read().strip()
    build = open(os.path.join(pkg, 'game/SAN14Mods/SeasonRules/BUILD.txt'), encoding='utf-8-sig').read()
    tag = re.match(r'(v\d+k\d+[a-z]?)', build.strip()).group(1)
    sums = walk(pkg)
    sums_text = ''.join('%s  %s\n' % (h, r) for r, h in sums.items())
    open(os.path.join(pkg, 'SHA256SUMS.txt'), 'w', encoding='utf-8', newline='\n').write(sums_text)
    upd = os.path.join(repo, 'updates'); os.makedirs(upd, exist_ok=True)
    idx_path = os.path.join(upd, 'index.json')
    idx = json.load(open(idx_path, encoding='utf-8')) if os.path.exists(idx_path) else {'format': 1, 'releases': [], 'base': []}
    base_sums_path = os.path.join(upd, 'base', 'BASE_SHA256SUMS.txt')
    if not idx.get('base'):
        # 기본판 조각: 큰 파일부터 채워 48MB 미만씩
        files = sorted(sums, key=lambda r: -os.path.getsize(os.path.join(pkg, r)))
        groups = []
        for r in files:
            sz = os.path.getsize(os.path.join(pkg, r))
            for g in groups:
                if g[0] + sz <= PART_LIMIT: g[0] += sz; g[1].append(r); break
            else: groups.append([sz, [r]])
        n = len(groups); idx['base'] = []
        for i, g in enumerate(groups, 1):
            name = 'updates/base/SAN14CE_SeasonMap_base_%s_%dof%d.zip' % (tag, i, n)
            p = os.path.join(repo, name); zip_files(p, pkg, sorted(g[1]))
            if os.path.getsize(p) >= 95 * 1024 * 1024: raise SystemExit('기본판 조각이 너무 큼: ' + name)
            idx['base'].append({'file': name, 'sha256': sha(p), 'size': os.path.getsize(p)})
        open(base_sums_path, 'w', encoding='utf-8', newline='\n').write(sums_text)
    base = {}
    for line in open(base_sums_path, encoding='utf-8'):
        line = line.strip()
        if len(line) >= 66: base[line[64:].strip()] = line[:64]
    changed = [r for r, h in sums.items() if base.get(r) != h]
    name = 'updates/%s/SAN14CE_SeasonMap_%s.zip' % (tag, tag)
    p = os.path.join(repo, name); zip_files(p, pkg, changed, {'SHA256SUMS.txt': sums_text})
    if os.path.getsize(p) >= 95 * 1024 * 1024: raise SystemExit('판 파일이 너무 큼 — 기본판을 새로 만들어야 함')
    rel = {'tag': tag, 'date': date, 'title': title, 'notes': notes, 'file': name, 'sha256': sha(p), 'size': os.path.getsize(p)}
    idx['releases'] = [rel] + [r for r in idx.get('releases', []) if r.get('tag') != tag]
    json.dump(idx, open(idx_path, 'w', encoding='utf-8', newline='\n'), ensure_ascii=False, indent=1)
    print('판', tag, '바뀐 파일', len(changed), '개, 판 파일', os.path.getsize(p), '바이트, 기본판 조각', len(idx['base']), '개')

if __name__ == '__main__':
    main()
