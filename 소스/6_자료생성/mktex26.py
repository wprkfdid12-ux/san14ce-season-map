# v26 textures: ground slices 14-17 (cracked ground, wet mud, cracked ice, clear ice), two more rock photo packs,
# the named-site tag map, and the freeze mask with the Liaodong west coast added.
import sys, numpy as np, struct
sys.path.insert(0, '/home/claude/season/v24'); sys.path.insert(0, '/home/claude/tv')
from mmlib import load, array, chain
from PIL import Image
from scipy import ndimage as nd
Image.MAX_IMAGE_PIXELS = None
T = '/home/claude/tex_in/'
OUT = '/home/claude/season/v26/'
def tint(img, target):
    lin = (img / 255.0) ** 2.2; lin = lin * (np.array(target) / lin.reshape(-1, 3).mean(0)); return np.clip(lin ** (1 / 2.2) * 255, 0, 255)
def seamless(a, N=1024):
    sh = np.roll(np.roll(a, N // 2, 0), N // 2, 1); y, x = np.mgrid[0:N, 0:N] / N
    m = np.minimum(np.minimum(x, 1 - x), np.minimum(y, 1 - y)); w = np.clip(m / 0.22, 0, 1)[..., None]
    return a * w + sh * (1 - w)
# ---- ground slices: the v24 set (read back from the shipped dds is not possible - rebuild from the same sources) ----
B = '/home/claude/ph/'
ground = [load(B + 't4/dirt_aerial_02_4k_blend/textures/dirt_aerial_02_diff_4k.jpg'),
    load(B + 't4/dry_ground_01_4k_blend/textures/dry_ground_01_diff_4k.jpg'),
    load(B + 't4/brown_mud_02_4k_blend/textures/brown_mud_02_diff_4k.jpg', gain=1.35),
    load(B + 't3/red_laterite_soil_stones_4k_blend/textures/red_laterite_soil_stones_diff_4k.jpg'),
    load(B + 't4/Grass004/Grass004_4K-JPG_Color.jpg'),
    load(B + 't4/Ground003/Ground003_4K-JPG_Color.jpg'),
    tint(load(B + 't5/ground_0018_color_4k.jpg'), (0.17, 0.125, 0.065)),
    tint(load(B + 't5/aerial_grass_rock_diff_4k.jpg'), (0.12, 0.12, 0.045)),
    tint(load(B + 't5/forest_leaves_02_diffuse_4k.jpg'), (0.12, 0.085, 0.045)),
    load(B + 't6/Snow015_4K-JPG_Color.jpg'),
    tint(seamless(np.asarray(Image.open(B + 't7/burnt_ground.jpg').convert('RGB').resize((1024, 1024), Image.LANCZOS)).astype(np.float64)), (0.035, 0.032, 0.030)),
    tint(load(B + 't9/Ground087_4K-JPG_Color.jpg'), (0.30, 0.22, 0.12)),
    tint(load(B + 't9/Ground110_4K-JPG_Color.jpg'), (0.20, 0.165, 0.12)),
    tint(load(B + 't9/Ground079S_4K-JPG_Color.jpg'), (0.28, 0.21, 0.13)),
    # v26: 14 cracked dry ground (drought), 15 wet dark mud (rain / receding flood), 16 cracked ice, 17 clear ice
    tint(load(T + '1791362114464_mud_cracked_dry_riverbed_002_4k_blend/textures/mud_cracked_dry_riverbed_002_diff_4k.jpg'), (0.30, 0.235, 0.15)),
    tint(load(T + '1791362114464_ground_0034_2k_KQt6Gw/ground_0034_color_2k.jpg'), (0.115, 0.095, 0.072)),
    tint(load(T + '1791362114464_ice_0002_2k_AuS5d3/ice_0002_color_2k.jpg'), (0.52, 0.56, 0.62)),
    tint(load(T + '17ff8e19-1791362114465_Ice002_2K-JPG/Ice002_2K-JPG_Color.jpg'), (0.42, 0.50, 0.58))]
array(OUT + 'season_ground.dds', ground)
# ---- rock photo packs: brightness only, mean 0.5, 4 photos per texture (RGBA8 + mips, like season_rock.dds) ----
def lum_pack(files, path, N=1024):
    chans = []
    for f in files:
        g = load(f, N); lin = (g / 255.0) ** 2.2; l = lin @ np.array([0.3, 0.59, 0.11]); l = l / max(l.mean(), 1e-6) * 0.5; chans.append(np.clip(l, 0, 1))
    img = np.stack(chans, -1)
    mips = []; cur = img
    while True:
        mips.append(np.clip(np.round(cur * 255), 0, 255).astype(np.uint8).tobytes())
        if cur.shape[0] <= 1: break
        w = cur.shape[0] // 2; cur = cur.reshape(w, 2, w, 2, 4).mean((1, 3))
    hdr = struct.pack('<4s7I', b'DDS ', 124, 0x1 | 0x2 | 0x4 | 0x8 | 0x1000 | 0x20000, N, N, N * 4, 0, len(mips)) + b'\0' * 44
    hdr += struct.pack('<2I4s5I', 32, 0x4, b'DX10', 0, 0, 0, 0, 0) + struct.pack('<5I', 0x1000 | 0x400000 | 0x8, 0, 0, 0, 0)
    hdr += struct.pack('<5I', 28, 3, 0, 1, 0)
    open(path, 'wb').write(hdr + b''.join(mips)); print(path, len(mips))
lum_pack([T + '0580ea39-Rock026_2K-JPG/Rock026_2K-JPG_Color.jpg',                          # r light grey granite cliff (Huangshan / Taishan / Sanqing / Wudang)
          T + '1791360725146_rock_0007_2k_x2Lzhd/rock_0007_color_2k.jpg',                  # g red sandstone (Danxia / Wuyi)
          T + '1791360725146_rock_0006_2k_3vWpt8/rock_0006_color_2k.jpg',                  # b mossy rock (Stone Forest / Zhangjiajie / the wet south)
          T + '1791361130981_rock_face_4k_blend/textures/rock_face_diff_4k.jpg'],          # a sheer rock face (Huashan / Jianmen / Emei / Wu Gorge)
         OUT + 'season_rock2.dds')
lum_pack([T + '1791360725146_cliff_side_4k_blend/textures/cliff_side_diff_4k.jpg',         # r layered cliff bands (Taihang / Songshan)
          T + '1791361130982_excavated_soil_wall_4k_blend/textures/excavated_soil_wall_diff_4k.jpg',   # g loess walls (Loess plateau)
          T + '1791361130981_rock_0008_2k_WN3hen/rock_0008_color_2k.jpg',                  # b snow-dusted rock (Qilian / high crests)
          T + '1791361130982_gray_rocks_4k_blend/textures/gray_rocks_diff_4k.jpg'],        # a grey limestone (Guilin / Wutai)
         OUT + 'season_rock3.dds')
# ---- named-site tags (512x512 RGBA8): r = site id / 255, g = blend weight toward the site's centre ----
tags = np.load('/home/claude/mtn/tags17.npy')                     # 2049 x 2049, 0 = none, 1..18 = FEAT order
small = tags[::4, ::4][:512, :512]
tag_img = np.zeros((512, 512, 4), np.uint8)
ids = small.astype(np.uint8)
d_in = np.zeros((512, 512))
for k in range(1, 19):
    m = ids == k
    if not m.any(): continue
    d_in[m] = nd.distance_transform_edt(m)[m]
wgt = np.clip(d_in / 4.0, 0, 1)
tag_img[..., 0] = ids; tag_img[..., 1] = np.round(wgt * 255).astype(np.uint8); tag_img[..., 3] = 255
hdr = struct.pack('<4s7I', b'DDS ', 124, 0x1 | 0x2 | 0x4 | 0x8 | 0x1000, 512, 512, 512 * 4, 0, 1) + b'\0' * 44
hdr += struct.pack('<2I4s5I', 32, 0x4, b'DX10', 0, 0, 0, 0, 0) + struct.pack('<5I', 0x1000, 0, 0, 0, 0) + struct.pack('<5I', 28, 3, 0, 1, 0)
open(OUT + 'season_tags.dds', 'wb').write(hdr + tag_img.tobytes()); print('tags', [int((ids == k).sum()) for k in range(1, 19)])
# ---- the freeze mask: add the Liaodong west coast (sea cells cols 184..197, rows 8..31, terrain 14) to the r channel ----
fz = bytearray(open('/home/claude/season/pkgLT/game/ShaderFixes/season_freeze.dds', 'rb').read())
img = np.frombuffer(bytes(fz[148:]), np.uint8).reshape(2048, 2048, 4).copy()
terr = np.fromfile('/home/claude/rules/hex_terr.bin', np.uint8).reshape(220, 220)
yy, xx = np.mgrid[0:2048, 0:2048]
mask = np.zeros((2048, 2048), bool)
for r in range(8, 32):
    for c in range(184, 198):
        if terr[r, c] != 14: continue
        X = 375.5 + 7.0 * c; Y = 138.5 + 8.0829 * (r - 0.5 * (c % 2))
        mask |= ((xx - X) ** 2 + (yy - Y) ** 2) <= 5.2 ** 2
img[..., 0] = np.where(mask, np.maximum(img[..., 0], 120), img[..., 0])      # 120/255 < 0.5: the shaders read it as "coast ice: needs a hard frost"
fz[148:] = img.tobytes(); open(OUT + 'season_freeze.dds', 'wb').write(bytes(fz)); print('freeze: liaodong pixels', int(mask.sum()))
