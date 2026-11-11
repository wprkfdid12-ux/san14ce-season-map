import numpy as np, sys
sys.path.insert(0,'/home/claude/climdata/land')
from scipy import ndimage as nd
from PIL import Image
from common import terr, W, N
SC=6966.2/65536
h=np.load('/home/claude/tv/newh11_613.npy')[:2048,:2048]*SC
cls=np.load('/home/claude/tv/tex1_185.npy')[:2048,:2048,2]
# hillshade (light from north-west)
gy,gx=np.gradient(nd.gaussian_filter(h,0.7))
gx/=100.0; gy/=100.0
nx,ny,nz=-gx*1.0,-gy*1.0,np.ones_like(h)
L=np.array([-0.6,-0.6,0.55]); L/=np.linalg.norm(L)
shade=np.clip((nx*L[0]+ny*L[1]+nz*L[2])/np.sqrt(nx*nx+ny*ny+1),0,1)
# pixel -> hex cell
yy,xx=np.mgrid[0:2048,0:2048].astype(np.float32)+0.5
col=np.clip(np.round((xx-375.5)/7.0),0,W-1).astype(int)
row=np.clip(np.round((yy-138.5)/8.0829+0.5*(col%2)),0,W-1).astype(int)
cell=row*W+col
t=terr[cell]
base=np.zeros((2048,2048,3),np.float32)
P={1:(0.42,0.55,0.25),2:(0.55,0.52,0.38),3:(0.18,0.36,0.14),4:(0.45,0.56,0.27),5:(0.40,0.50,0.25),6:(0.38,0.46,0.24),7:(0.45,0.45,0.30),
   8:(0.40,0.47,0.26),9:(0.42,0.44,0.30),10:(0.48,0.46,0.38),11:(0.50,0.48,0.42),12:(0.20,0.28,0.32),13:(0.16,0.24,0.30),14:(0.10,0.16,0.20),
   15:(0.55,0.50,0.40),16:(0.60,0.55,0.45),17:(0.55,0.50,0.40),18:(0.50,0.48,0.40),19:(0.62,0.56,0.50),20:(0.50,0.42,0.35),21:(0.55,0.45,0.30),
   22:(0.66,0.55,0.52),23:(0.08,0.08,0.10),24:(0.09,0.13,0.17)}
for k,v in P.items(): base[t==k]=v
water=(cls==9)
base[water]=(0.10,0.15,0.19)
img=base*(0.35+0.85*shade[...,None])
img[water]=base[water]*0.9
img=np.clip(img,0,1)
np.save('base.npy',(img*255).astype(np.uint8))
Image.fromarray((img[120:1930,355:1930]*255).astype(np.uint8)).resize((1024,1176)).save('base_small.png')
