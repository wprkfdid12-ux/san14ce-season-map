# v26k19: rock relief for the mountain flanks - two normal maps (from real displacement scans) packed in one RGBA8 texture with mips:
#   r,g = sheer rock face with vertical erosion grooves (projected on the flank, from the side)
#   b,a = broken rock (Rock026, projected from above on gentler mountain ground)
import numpy as np, struct
from PIL import Image
from scipy import ndimage as nd
Image.MAX_IMAGE_PIXELS = None
T = '/home/claude/tex_in/'
N = 1024
def disp(path):
    im = Image.open(path); a = np.asarray(im).astype(np.float64)
    if a.ndim == 3: a = a[..., 0]
    a = a / a.max()
    a = np.asarray(Image.fromarray((a * 65535).astype(np.uint16)).resize((N, N), Image.LANCZOS)).astype(np.float64) / 65535 if False else np.asarray(Image.fromarray(a.astype(np.float32), mode='F').resize((N, N), Image.LANCZOS)).astype(np.float64)
    return a
def seam(a):
    sh = np.roll(np.roll(a, N // 2, 0), N // 2, 1); y, x = np.mgrid[0:N, 0:N] / N
    m = np.minimum(np.minimum(x, 1 - x), np.minimum(y, 1 - y)); w = np.clip(m / 0.2, 0, 1)
    return a * w + sh * (1 - w)
def normal_xy(h, k):
    h = seam(h); h = h - nd.gaussian_filter(h, 60, mode='wrap')            # remove the broad tilt of the scan
    gx = (np.roll(h, -1, 1) - np.roll(h, 1, 1)) * 0.5 * k; gy = (np.roll(h, -1, 0) - np.roll(h, 1, 0)) * 0.5 * k
    n = np.dstack([-gx, gy, np.ones_like(h)]); n /= np.linalg.norm(n, axis=2, keepdims=True)   # OpenGL: +y up the image
    return n[..., 0], n[..., 1]
a = disp(T + '1791361130981_rock_face_4k_blend/textures/rock_face_disp_4k.png')
b = disp(T + '0580ea39-Rock026_2K-JPG/Rock026_2K-JPG_Displacement.jpg')
sx, sy = normal_xy(a, 110.0); tx, ty = normal_xy(b, 170.0)
print('side tilt mean %.3f top %.3f' % (np.hypot(sx, sy).mean(), np.hypot(tx, ty).mean()))
img = np.stack([sx, sy, tx, ty], -1) * 0.5 + 0.5
mips = []; cur = img
while True:
    mips.append(np.clip(np.round(cur * 255), 0, 255).astype(np.uint8).tobytes())
    if cur.shape[0] <= 1: break
    w = cur.shape[0] // 2; cur = cur.reshape(w, 2, w, 2, 4).mean((1, 3))
hdr = struct.pack('<4s7I', b'DDS ', 124, 0x1 | 0x2 | 0x4 | 0x8 | 0x1000 | 0x20000, N, N, N * 4, 0, len(mips)) + b'\0' * 44
hdr += struct.pack('<2I4s5I', 32, 0x4, b'DX10', 0, 0, 0, 0, 0) + struct.pack('<5I', 0x1000 | 0x400000 | 0x8, 0, 0, 0, 0)
hdr += struct.pack('<5I', 28, 3, 0, 1, 0)
open('/home/claude/season/v26/season_relief.dds', 'wb').write(hdr + b''.join(mips)); print('relief', len(mips))
Image.fromarray((img[..., :3] * 255).astype(np.uint8)).resize((512, 512)).save('/tmp/claude-0/relief_side.png')
