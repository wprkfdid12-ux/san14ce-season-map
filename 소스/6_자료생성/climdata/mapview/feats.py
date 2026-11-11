import numpy as np, sys
sys.path.insert(0,'/home/claude/climdata/land')
from common import terr, W, N
cols=np.arange(N)%W; rows=np.arange(N)//W
X=375.5+7.0*cols; Y=138.5+8.0829*(rows-0.5*(cols%2))
PASS=[8,9,10,11]; IMP=[19,22]; PLAIN=[1,2,4,5,6,7]; FOREST=[3]
F=[ # key, label, box (x0,x1,y0,y1) in map px, kind, cell types
 ('taihang','태항산',(1140,1235,250,610),'low',PASS+IMP),
 ('taishan','태산',(1555,1660,545,680),'low',PASS+IMP),
 ('lushan','여산',(1250,1500,1420,1510),'low',PASS+IMP),
 ('tianmu','천목산',(1530,1760,1290,1500),'low',PASS+IMP),
 ('chuandong','사천 동부 산줄기',(700,865,1270,1465),'high',PASS+IMP),
 ('nanling','남령',(985,1375,1580,1810),'high',PASS+IMP),
 ('yungui','운귀고원 평지',(355,1060,1600,1930),'plat',PLAIN),
 ('loess','황토고원',(625,955,315,675),'loess',PASS+IMP),
 ('lanzhou','난주·천수 숲',(355,805,270,750),'forest',FOREST),
 ('qinling','진령',(580,985,720,960),'good',PASS+IMP),
 ('shudao','촉도',(505,805,970,1150),'good',PASS+IMP),
 ('dabie','대별산',(1075,1405,1060,1195),'good',PASS+IMP),
 ('yanshan','연산',(1255,1705,120,285),'good',PASS+IMP),
 ('wuyi','무이산',(1375,1645,1465,1690),'good',PASS+IMP),
 ('sanxia','삼협',(850,1045,1240,1360),'good',PASS+IMP),
 ('funiu','복우산',(985,1195,750,900),'good',PASS+IMP),
]
ELL={'taihang':(1192,430,195,48,88),'taishan':(1605,640,62,62,0),'lushan':(1330,1462,95,45,0),'tianmu':(1650,1400,115,75,-30),
 'qinling':(790,825,230,75,-3),'shudao':(650,1055,120,85,65),'dabie':(1250,1130,150,55,-5),'yanshan':(1480,195,235,60,-5),
 'wuyi':(1505,1580,150,58,-38),'sanxia':(955,1300,95,42,10),'funiu':(1090,825,110,50,-10)}
def cells_k(key,box,types):
    if key in ELL:
        cx,cy,a,b,ang=ELL[key]; t=np.radians(ang); dx=X-cx; dy=Y-cy
        u=(dx*np.cos(t)+dy*np.sin(t))/a; v=(-dx*np.sin(t)+dy*np.cos(t))/b
        return np.nonzero((u*u+v*v<=1)&np.isin(terr,types))[0]
    return cells(box,types)
def cells(box,types):
    x0,x1,y0,y1=box
    return np.nonzero((X>=x0)&(X<=x1)&(Y>=y0)&(Y<=y1)&np.isin(terr,types))[0]
if __name__=='__main__':
    for k,l,b,kind,ty in F:
        c=cells(b,ty); t=terr[c]
        print('%-14s %-6s cells %4d  passable %4d  impassable %4d  plain %4d forest %4d'%(l,kind,len(c),np.isin(t,PASS).sum(),np.isin(t,IMP).sum(),np.isin(t,PLAIN).sum(),np.isin(t,FOREST).sum()))
