# v26k38b: SeasonRainB.hlsl / SeasonRainDropsB.hlsl for the game's clustered-light lighting pass 6642ae48461b1425 - the same shaders
# with its layout: depth t5 (not t4), camera cb0[16] (not [13]), clip -> world cb0[26..29] (not [23..26]). Run after any edit of the two.
import re
V = '/home/claude/season/v26/'
for f, g in (('SeasonRain.hlsl', 'SeasonRainB.hlsl'), ('SeasonRainDrops.hlsl', 'SeasonRainDropsB.hlsl')):
    s = open(V + f, encoding='utf-8').read()
    assert s.count('register(t4)') == 1
    assert set(re.findall(r'cb0\[(\d+)\]', s)) <= {'13', '23', '24', '25', '26', '31'}, set(re.findall(r'cb0\[(\d+)\]', s))
    s = s.replace('register(t4)', 'register(t5)')
    s = re.sub(r'cb0\[(13|23|24|25|26)\]', lambda m: 'cb0[%d]' % {13: 16, 23: 26, 24: 27, 25: 28, 26: 29}[int(m.group(1))], s)
    s = s.replace('float4 cb0[31];', 'float4 cb0[34];')
    hdr = ('// v26k38b: LAYOUT B - made from %s by gen_rainB.py (the same picture) for the game\'s other lighting pass 6642ae48461b1425\n'
           '//   (used whenever lit points are on screen - towns, fires; the weather vanished there): its depth is t5, its constants\n'
           '//   carry the camera at [16] and clip -> world at [26..29] (three rows later than 70b6fa39daa0880e).\n' % f)
    open(V + g, 'w', encoding='utf-8').write(hdr + s)
print('ok')
