import numpy as np, sys, json
sys.path.insert(0,'/home/claude/climdata/land')
from common import terr, W, N
from scipy import ndimage as nd
from scipy.interpolate import RBFInterpolator
cols=np.arange(N)%W; rows=np.arange(N)//W
X=375.5+7.0*cols; Y=138.5+8.0829*(rows-0.5*(cols%2))
# city clusters (terrain 16) in map pixels
m=(terr==16).reshape(W,W)
lab,n=nd.label(m,structure=np.ones((3,3)))
cent=[]
for i in range(1,n+1):
    idx=np.nonzero(lab.ravel()==i)[0]; cent.append((X[idx].mean(),Y[idx].mean(),len(idx)))
cent=np.array(cent); print('city clusters',len(cent))
# anchors: (name, lon, lat, approx map units from the old climate table)
A=[('양평/심양',123.43,41.80,14.2,1.3),('계',116.40,39.91,11.5,2.3),('진양',112.55,37.87,8.3,3.3),('업',114.6,36.3,10.2,4.0),
 ('장안',108.94,34.26,6.6,5.2),('낙양',112.45,34.62,8.8,5.6),('허창',113.85,34.04,9.8,6.0),('제남',117.02,36.67,12.2,4.4),('하비/서주',117.19,34.27,11.6,6.4),
 ('한중',107.02,33.07,5.6,6.8),('수춘/합비',117.23,31.82,11.4,8.2),('건업',118.78,32.06,12.3,8.8),('회계',120.58,30.05,13.6,9.6),('강하',114.30,30.60,9.4,9.2),
 ('강릉',112.24,30.34,8.0,9.0),('장사',112.94,28.23,8.8,10.9),('성도',104.06,30.66,4.4,9.0),('강주',106.55,29.56,5.8,10.0),('장가/귀양',106.63,26.65,6.2,12.0),
 ('곤명',102.71,25.05,3.8,12.9),('건안/복주',119.30,26.07,13.8,12.4),('남해/번우',113.26,23.13,9.6,14.2),('울림/남녕',108.33,22.82,6.8,14.4),('교지',105.85,21.0,4.6,15.4),
 ('무위',102.64,37.93,2.4,2.6),('금성/난주',103.83,36.06,3.4,4.6)]
pts=[];LL=[]
for nm,lo,la,mx,my in A:
    p=np.array([mx/16*2048,my/16*2048]); d=np.hypot(cent[:,0]-p[0],cent[:,1]-p[1]); k=d.argmin()
    print('%-10s guess (%5.0f,%5.0f) -> city (%5.0f,%5.0f) dist %4.0f px'%(nm,p[0],p[1],cent[k,0],cent[k,1],d[k]))
    pts.append(cent[k,:2]); LL.append((lo,la))
pts=np.array(pts); LL=np.array(LL)
np.save('anchors.npy',np.hstack([LL,pts]))
json.dump([a[0] for a in A],open('anchor_names.json','w'),ensure_ascii=False)
np.save('city_cent.npy',cent)
