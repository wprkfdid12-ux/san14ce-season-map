# localized climate: real city normals placed at their game-map positions (TPS warp), then spread to every cell
import numpy as np, json, glob
from scipy import ndimage as nd
W=220; N=W*W
cols=np.arange(N)%W; rows=np.arange(N)//W
X=375.5+7.0*cols; Y=138.5+8.0829*(rows-0.5*(cols%2))
SC=6966.2/65536; hgt=np.load('/home/claude/tv/newh11_613.npy')*SC
cls=np.load('/home/claude/tv/tex1_185.npy')[...,2]; land=cls!=9
idx=nd.distance_transform_edt(~land,return_distances=False,return_indices=True)
hf=hgt[idx[0],idx[1]]
base=nd.gaussian_filter(nd.grey_opening(hf,size=(41,41)),12)
pos={c['city']:(c['gx'],c['gy']) for c in json.load(open('city_game_pos.json'))}
C=[]
for fn in sorted(glob.glob('/home/claude/climdata/raw/cities_*.json')):
    for c in json.load(open(fn)):
        if not c.get('tmean') or c['city'] not in pos: continue
        if c['city']=='Hanoi' and c.get('alt'):      # Hanoi: the wiki table's max/min rows are broken; the means are fine
            pass
        C.append(c)
def ground(x,y):
    i,j=int(np.clip(y,0,2048)),int(np.clip(x,0,2048)); r=150
    win=base[max(0,i-r):i+r,max(0,j-r):j+r]; return float(min(np.percentile(win,10),1600.0))
T0=[];P=[];D=[];R=[];S=[];G=[]
for c in C:
    gx,gy=pos[c['city']]; hc=ground(gx,gy)
    T0.append(np.array(c['tmean'],float)-1.0+0.0045*max(hc-750.0,0.0))   # 3rd century ~1 C colder; to the game's sea-level equivalent
    P.append(np.array(c['precip_mm'],float))
    D.append(np.array([v if v is not None else np.nan for v in (c.get('precip_days') or [np.nan]*12)],float))
    R.append(np.array([v if v is not None else np.nan for v in (c.get('rh') or [np.nan]*12)],float))
    G.append((gx,gy))
T0=np.array(T0);P=np.array(P);D=np.array(D);R=np.array(R);G=np.array(G)
def idw(V,k=7,pw=2.0):
    out=np.zeros((N,V.shape[1]))
    for m in range(V.shape[1]):
        ok=~np.isnan(V[:,m]); g=G[ok]; v=V[ok,m]
        d2=(X[:,None]-g[None,:,0])**2+(Y[:,None]-g[None,:,1])**2+400.0
        ii=np.argsort(d2,1)[:,:k]; dd=np.take_along_axis(d2,ii,1); w=1/dd**(pw/2); w/=w.sum(1,keepdims=True)
        out[:,m]=(w*v[ii]).sum(1)
    return out
cT=idw(T0); cP=idw(P); cD=idw(D); cR=idw(R)
np.savez('clim_cells.npz',T0=cT,P=cP,D=cD,R=cR)
print('cities',len(C),'T0 jan range',cT[:,0].min().round(1),cT[:,0].max().round(1),'P jul range',cP[:,6].min().round(0),cP[:,6].max().round(0))
