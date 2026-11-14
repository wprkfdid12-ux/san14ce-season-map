# test build (ground speed measurement): ground shader variants made from the release ground shaders by exact cuts
#   A = the same picture without the reuse store (no UAV in the shader -> the GPU's early depth test works again)
#   B = A without the rarely shown parts (disasters, the weather marks on the ground, the northwest winter): measures how much
#       the size of the shader itself (registers) costs. Its picture differs - only for its 3-second turns in the speed test.
import re, sys
SF = sys.argv[1]
def one(name, tag):
    s = open('%s/%s-ps_replace.txt' % (SF, name), encoding='utf-8').read()
    n0 = s.count('RWTexture2DArray<uint> SeasonCache : register(u5);'); assert n0 == 1
    s = s.replace('RWTexture2DArray<uint> SeasonCache : register(u5);', '// (test build A: no reuse store)')
    c = '[branch] if (sanIn && (pm98.x < 0.5 || pm98.x > 2.5)) {'; assert s.count(c) == 1
    s = s.replace(c, '[branch] if (sanIn) {   // test build: always the season picture (the speed test picks this shader by itself)')
    m = re.search(r'float3 A, M;\n\s*// v26k29: the season colours.*?(season\(uv, [^;]*\);).*?(o0\.rgb = A \+ saturate\(o0\.rgb\) \* M;)', s, re.S); assert m
    call = m.group(1)
    s = s[:m.start()] + 'float3 A, M; float dynT;\n            ' + call + '\n            ' + m.group(2) + s[m.end():]
    assert 'SeasonCache' not in s and 'op164' not in s
    a = '// SAN14 season map - TEST BUILD ground variant A (%s): the release ground shader without the reuse store\n' % tag + s
    # B: cut the northwest-winter block and the disasters + weather marks block out of season()
    b = s
    i = b.index('    float wf = nwZ * smoothstep(5.0, -3.0, T);'); j = b.index('    // ---- above the treeline: bare rock in every season')
    b = b[:i] + '    // (test build B: northwest winter left out)\n\n' + b[j:]
    i = b.index('    float4 ip2 = IniParams.Load(int2(2, 0));'); j = b.index('    // ---- v26k19: rock relief on the mountain flanks')
    b = b[:i] + '    // (test build B: disasters and the weather marks on the ground left out)\n\n' + b[j:]
    b = '// SAN14 season map - TEST BUILD ground variant B (%s): A without the disasters, the weather marks and the northwest winter\n' % tag + b
    return a, b
out = sys.argv[2]
for name, tag in (('d3e1f71e809bac74', 'Near'), ('9addb6cb884c7a7c', 'Far')):
    a, b = one(name, tag)
    open('%s/SeasonTest%sA.hlsl' % (out, tag), 'w', encoding='utf-8').write(a)
    open('%s/SeasonTest%sB.hlsl' % (out, tag), 'w', encoding='utf-8').write(b)
    print(tag, 'A', len(a.split('\n')), 'B', len(b.split('\n')))
