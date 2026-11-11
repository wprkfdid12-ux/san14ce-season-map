import numpy as np, sys
sys.path.insert(0,'/home/claude/rules')
SF='/home/claude/season/pkg24/game/ShaderFixes/'
W=220; N=W*W
H=np.load('/home/claude/rules/hex.npy'); terr=H[:,16].astype(np.int16)
def dds(path,shape,dt):
    b=open(SF+path,'rb').read()[148:]
    return np.frombuffer(b,dt).reshape(shape)
haz=dds('season_hazard.dds',(512,512,4),np.uint8).astype(np.float32)/255
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
hz=bil(haz,u,v)
rid=np.where(hz[:,1]<0.15,0,np.where(hz[:,1]<0.5,1,np.where(hz[:,1]<0.8,3,2)))
order=hz[:,0]
height=bil(hgt,u,v)
def nbrs(c):
    x,y=c%W,c//W; out=[]
    if x%2==0: cand=[(x,y-1),(x,y+1),(x-1,y),(x+1,y),(x-1,y+1),(x+1,y+1)]
    else:      cand=[(x,y-1),(x,y+1),(x-1,y-1),(x+1,y-1),(x-1,y),(x+1,y)]
    for a,b in cand:
        if 0<=a<W and 0<=b<W: out.append(b*W+a)
    return out
NB=[nbrs(c) for c in range(N)]
