# per-cell data for the map check + climate simulator page (same math as rules/mkrules.py, current release v25.9i)
import numpy as np, struct, cv2, json, sys
from scipy import ndimage as nd
SF='/home/claude/season/pkg259i/game/ShaderFixes/'
W=220; N=W*W
H=np.load('/home/claude/rules/hex.npy'); terr=H[:,16].astype(np.uint8)
def dds(path,shape,dt):
    b=open(SF+path,'rb').read(); n=int(np.prod(shape))*np.dtype(dt).itemsize
    return np.frombuffer(b[-n:],dt).reshape(shape)
clim=dds('season_climate.dds',(13,128,128,4),np.float16).astype(np.float32)
zone=dds('season_zone.dds',(512,512,4),np.uint8).astype(np.float32)/255
frz=dds('season_freeze.dds',(2048,2048,4),np.uint8).astype(np.float32)/255
SC=6966.2/65536; hgt=np.load('/home/claude/tv/newh11_613.npy')*SC
cols=np.arange(N)%W; rows=np.arange(N)//W
X=375.5+7.0*cols; Y=138.5+8.0829*(rows-0.5*(cols%2))
u=X/2048.0; v=Y/2048.0
def bil(img,u,v):
    h,w=img.shape[:2]; px=np.clip(u*w-0.5,0,w-1.001); py=np.clip(v*h-0.5,0,h-1.001)
    x0=px.astype(int); y0=py.astype(int); fx=(px-x0); fy=(py-y0)
    if img.ndim==3: fx=fx[:,None]; fy=fy[:,None]
    a=img[y0,x0]; b=img[y0,x0+1]; c=img[y0+1,x0]; d=img[y0+1,x0+1]
    return (a*(1-fx)+b*fx)*(1-fy)+(c*(1-fx)+d*fx)*fy
z=bil(zone,u,v); northZ=z[:,0]; nwZ=z[:,1]
fm=bil(nd.maximum_filter(frz[...,0],size=9),u,v); imp=bil(frz[...,2],u,v)
alt=np.maximum(bil(hgt,u,v)-750.0,0)
T0=np.stack([bil(clim[m],u,v)[:,0] for m in range(12)],1)      # sea-level temperature by real month
D=np.stack([bil(clim[m],u,v)[:,2] for m in range(12)],1)       # dryness by real month
# flood zones of the current rules (SeasonRules.dat)
dat=open('/home/claude/rules/SeasonRules.dat','rb').read()
p=24; vterr=np.frombuffer(dat[p:p+N],np.uint8); p+=N; prov=np.frombuffer(dat[p:p+N],np.uint8); p+=N
floodz=np.frombuffer(dat[p:p+N],np.uint8); p+=N; slidez=np.frombuffer(dat[p:p+N],np.uint8); p+=N; firez=np.frombuffer(dat[p:p+N],np.uint8); p+=N
frzbits=[np.unpackbits(np.frombuffer(dat[p+t*(N//8):p+(t+1)*(N//8)],np.uint8),bitorder='little')[:N] for t in range(36)]; p+=36*(N//8)
snw=np.stack([np.frombuffer(dat[p+t*N:p+(t+1)*N],np.uint8) for t in range(36)]); p+=36*N
assert (vterr==terr).all()
rec=np.zeros((N,32),np.uint8)
rec[:,0]=terr
a16=np.clip(np.round(alt),0,65535).astype(np.uint16); rec[:,1]=a16&255; rec[:,2]=a16>>8
rec[:,3]=np.clip(np.round(northZ*255),0,255); rec[:,4]=np.clip(np.round(nwZ*255),0,255)
rec[:,5]=np.clip(np.round(imp*255),0,255); rec[:,6]=np.clip(np.round(fm*255),0,255)
rec[:,7]=floodz
rec[:,8:20]=np.clip(np.round(T0*4)+128,0,255).astype(np.uint8)       # 0.25 C steps, offset 128
rec[:,20:32]=np.clip(np.round(D*255),0,255).astype(np.uint8)
rec.tofile('out/cells.bin')
# reference: what the current rules table says for snow / ice per turn (to check the page's math against)
np.save('ref_snow.npy',snw); np.save('ref_frz.npy',np.stack(frzbits))
print('cells.bin',rec.nbytes,'firez types',np.unique(terr[firez>0]))
