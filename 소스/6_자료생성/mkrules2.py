# SeasonRules.dat v2 for the season map v26: the v1 tables (unchanged - the "normal year" baseline the player approved)
# followed by the per-cell climate records the weather engine uses (same 64-byte layout as the preview page's cells2).
import numpy as np, struct, base64, json
from scipy import ndimage as nd
W=220; N=W*W
old=open('/home/claude/season/pkg259i/game/SAN14Mods/SeasonRules/SeasonRules.dat','rb').read()
assert old[:8]==b'S14SEAS1' and len(old)==24+N*5+36*(N//8)+36*N
v1=old[24:]
R=np.frombuffer(base64.b64decode(open('/home/claude/climdata/sim/out/cells2.txt').read()),np.uint8).reshape(N,64).copy()
terr=R[:,0]; assert (terr==np.frombuffer(v1[:N],np.uint8)).all()
alt=(R[:,1].astype(int)|(R[:,2].astype(int)<<8)).astype(float); imp=R[:,5]/255.0
cols=np.arange(N)%W; rows=np.arange(N)//W
# ---- rain shadow: the lee of high impassable ranges, for the two seasonal winds ----
#  NW wind (winter, blows toward SE): upwind = NW.   SE wind (summer, blows toward NW): upwind = SE.
A=alt.reshape(W,W); I=imp.reshape(W,W)
def shadow(dx,dy):
    out=np.zeros((W,W))
    for d in range(1,13):
        sx=np.clip(np.arange(W)[None,:]-dx*d,0,W-1); sy=np.clip(np.arange(W)[:,None]-dy*d,0,W-1)
        up=A[sy,sx]; ui=I[sy,sx]
        s=np.clip((up-A-500.0)/1500.0,0,1)*np.clip(ui*1.5,0,1)*np.exp(-(d-1)/7.0)
        out=np.maximum(out,s)
    return out
shNW=shadow(-1,-1); shSE=shadow(+1,+1)
# ---- weather nodes 9x9: mountain roughness (slower wind), mean altitude ----
GX=9
rough=np.zeros(GX*GX); nalt=np.zeros(GX*GX)
for j in range(GX):
    for i in range(GX):
        x0=int(i/(GX-1)*(W-1)); y0=int(j/(GX-1)*(W-1))
        xs=slice(max(0,x0-14),min(W,x0+15)); ys=slice(max(0,y0-14),min(W,y0+15))
        rough[j*GX+i]=I[ys,xs].mean(); nalt[j*GX+i]=A[ys,xs].mean()
# ---- fuel for fires (forest 3, mountain below the treeline 2, grass 1), people weight ----
LAND=~np.isin(terr,[0,12,13,14,24,19,20,21,22,23])
X=375.5+7.0*cols; Y=138.5+8.0829*(rows-0.5*(cols%2))
lat=45.3-25.2*(Y/2048.0); tl=2730+(34-lat)*np.where(lat<34,46.7,70)
def sm(e0,e1,x): t=np.clip((x-e0)/(e1-e0),0,1); return t*t*(3-2*t)
fuel=np.where(np.isin(terr,[5,6]),3.0,np.where((terr>=8)&(terr<=11),2.0,np.where(terr==1,1.0,0.0)))
fuel*=1-sm(tl-300,tl+200,alt+750)
# distance to the nearest city / site over the hex grid (hops), from the cell records (byte 12 = nearest game city in cells)
cd=R[:,12].astype(float)
people=(0.35+0.65*np.minimum(1.5,np.exp(-cd/12.0)))*np.where(lat<28,2.0,1.0)
# ---- pack v2 ----
out=bytearray()
out+=struct.pack('<8sIIII',b'S14SEAS2',2,W,W,36)
out+=v1
out+=R.tobytes()                                           # N*64 climate records
out+=np.clip(np.round(shNW.ravel()*255),0,255).astype(np.uint8).tobytes()
out+=np.clip(np.round(shSE.ravel()*255),0,255).astype(np.uint8).tobytes()
out+=np.clip(np.round(fuel*60),0,255).astype(np.uint8).tobytes()      # fuel*60 (0..180)
out+=np.clip(np.round(people*100),0,255).astype(np.uint8).tobytes()   # people*100
out+=np.clip(np.round(rough*255),0,255).astype(np.uint8).tobytes()    # 81 node roughness
out+=np.clip(np.round(nalt/20),0,255).astype(np.uint8).tobytes()      # 81 node altitude / 20
out+=b'\0'*(16-len(out)%16 if len(out)%16 else 0)
open('/home/claude/rules/SeasonRules_v26.dat','wb').write(out)
print('v2 dat bytes',len(out),'shadowNW>0.3 cells',int((shNW>0.3).sum()),'shadowSE>0.3',int((shSE>0.3).sum()),'fuel>0',int((fuel>0).sum()))
# a picture of the shadow maps and fuel for the check
import PIL.Image as Im
pic=np.zeros((W,W*3,3),np.uint8)
pic[:,:W,0]=(shNW*255).astype(np.uint8); pic[:,W:2*W,1]=(shSE*255).astype(np.uint8); pic[:,2*W:,2]=(fuel.reshape(W,W)*80).astype(np.uint8)
Im.fromarray(pic).resize((W*6,W*2),Im.NEAREST).save('/home/claude/rules/v26_tables.png')
