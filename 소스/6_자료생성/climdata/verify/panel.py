import numpy as np, cv2, sys
sys.path.insert(0,'/home/claude/climdata/land')
from common import terr, W, N
cols=np.arange(N)%W; rows=np.arange(N)//W
X=375.5+7.0*cols; Y=138.5+8.0829*(rows-0.5*(cols%2))
R=4.67; ang=np.arange(6)*np.pi/3
def draw(nm,codes,color,crop,out,fill=False):
    H=np.load(f'H_{nm}.npy'); im=cv2.imread(f'shot_{nm}.jpg')
    ov=im.copy()
    for c in np.nonzero(np.isin(terr,codes))[0]:
        pts=np.stack([X[c]+R*np.cos(ang),Y[c]+R*np.sin(ang),np.ones(6)])
        P=H@pts; P=(P[:2]/P[2]).T.astype(np.int32)
        if fill: cv2.fillPoly(ov,[P],color)
        else: cv2.polylines(im,[P],True,color,2)
    if fill: im=cv2.addWeighted(im,0.55,ov,0.45,0)
    x0,y0,x1,y1=crop; z=im[y0:y1,x0:x1]
    cv2.imwrite(out,z,[cv2.IMWRITE_JPEG_QUALITY,88])
if __name__=='__main__':
    crop=(1500,700,2900,1500)
    draw('n',[3],(255,0,255),crop,'p_code3.jpg')
    draw('n',[5,6],(0,255,255),crop,'p_code56.jpg')
    draw('n',[4],(255,255,0),crop,'p_code4.jpg')
    draw('n',[3],(255,0,255),crop,'p_raw.jpg') if False else cv2.imwrite('p_raw.jpg',cv2.imread('shot_n.jpg')[700:1500,1500:2900])
