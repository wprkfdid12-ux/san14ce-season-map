import cv2, numpy as np, sys
cls=np.load('/home/claude/tv/tex1_185.npy')[:2048,:2048,2]
mw=(cls==9).astype(np.float32)
def clean(w):
    w=(w>0).astype(np.uint8)
    w=cv2.morphologyEx(w,cv2.MORPH_OPEN,np.ones((9,9),np.uint8))
    n,lab,st,_=cv2.connectedComponentsWithStats(w)
    keep=np.zeros_like(w)
    for i in range(1,n):
        if st[i,4]>4000: keep[lab==i]=1
    return keep.astype(np.float32)
def run(nm,A0,uimask):
    sw=clean(np.load(f'water_{nm}.npy'))
    for (x0,y0,x1,y1) in uimask: sw[y0:y1,x0:x1]=0.5
    S=0.25
    sws=cv2.resize(sw,None,fx=S,fy=S,interpolation=cv2.INTER_AREA)
    sws=cv2.GaussianBlur(sws,(0,0),3)
    H0=np.array([[A0[0],0,A0[2]],[0,A0[1],A0[3]],[0,0,1]],np.float64)
    Sm=np.diag([S,S,1.0])
    Hs=(Sm@H0).astype(np.float32)
    mws=cv2.GaussianBlur(mw,(0,0),3*4*0.4)
    # ECC finds warp mapping template coords -> input coords: we warp the map (input) into screenshot frame (template)
    crit=(cv2.TERM_CRITERIA_EPS|cv2.TERM_CRITERIA_COUNT,300,1e-6)
    warp=np.linalg.inv(Hs).astype(np.float32)   # screenshot(small) -> map
    for mode in (cv2.MOTION_AFFINE,cv2.MOTION_HOMOGRAPHY):
        w=warp if mode==cv2.MOTION_HOMOGRAPHY else warp[:2].copy()
        cc,w=cv2.findTransformECC(sws.astype(np.float32),mws.astype(np.float32),w,mode,crit,None,5)
        warp=w if mode==cv2.MOTION_HOMOGRAPHY else np.vstack([w,[0,0,1]]).astype(np.float32)
        print(nm,'mode',mode,'cc',round(cc,4))
    Hfull=np.linalg.inv(Sm)@np.linalg.inv(warp.astype(np.float64))   # map -> screenshot full
    np.save(f'H_{nm}.npy',Hfull)
    # check overlay
    im=cv2.imread(f'shot_{nm}.jpg')
    mwarp=cv2.warpPerspective(mw,Hfull,(im.shape[1],im.shape[0]))
    edge=cv2.Canny((mwarp>0.5).astype(np.uint8)*255,50,150)
    im[cv2.dilate(edge,np.ones((5,5),np.uint8))>0]=(0,0,255)
    cv2.imwrite(f'check_{nm}.jpg',cv2.resize(im,(1600,900)),[cv2.IMWRITE_JPEG_QUALITY,85])
UI_N=[(0,0,1300,300),(2880,0,3840,200),(0,1750,1450,2160)]
UI_S=[(0,0,1300,300),(2880,0,3840,200),(0,550,500,1780),(0,1750,1450,2160)]
run('n',(1.62,1.70,194,-191),UI_N)
