# v26k43: poison swamp (독늪) footprint -> green channel of season_forest.dds (1024x1024 RGBA8, DX10 header 148 bytes).
# Source: the game's own terrain data, FieldEditor4.rdb.bin14 tex section, G1T texture 1 (2049x2049 RGBA8), blue channel =
# ground class per height-map pixel; class 8 and 16 are the swamp ground (98% of them lie on cells of terrain code 7 = 독늪).
# The shaders (SeasonTerrainNear/Far and the merged d3e1f71e / 9addb6cb ground shaders) take the season ground off there again.
import numpy as np, struct, sys
g1t_raw, dds = sys.argv[1], sys.argv[2]          # decompressed tex section (G1T), season_forest.dds (changed in place)
d = open(g1t_raw, 'rb').read()
to = struct.unpack_from('<I', d, 12)[0]; offs = struct.unpack_from('<5I', d, to)
end1 = to + offs[2]; n = 2049 * 2049 * 4
cls = np.frombuffer(d[end1 - n:end1], np.uint8).reshape(2049, 2049, 4)[..., 2]
from scipy import ndimage as nd   # v26k45: softened at the height map's resolution first (sigma 3 px = 300 world units), no stair edges
fp = ((cls == 8) | (cls == 16)).astype(np.float32)
m = nd.gaussian_filter(fp, 3.0)[:2048, :2048].reshape(1024, 2, 1024, 2).mean((1, 3))
g = np.clip(np.round(m * 255), 0, 255).astype(np.uint8)
x = bytearray(open(dds, 'rb').read())
a = np.frombuffer(bytes(x[148:]), np.uint8).reshape(1024, 1024, 4).copy(); a[..., 1] = g; x[148:] = a.tobytes()
open(dds, 'wb').write(x)
