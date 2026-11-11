import numpy as np
def bc1(data,w,h):
    b=np.frombuffer(data,dtype=np.uint8)[:w*h//2].reshape(-1,8)
    c0=b[:,0].astype(np.uint32)|(b[:,1].astype(np.uint32)<<8); c1=b[:,2].astype(np.uint32)|(b[:,3].astype(np.uint32)<<8)
    def rgb(c): return np.stack([((c>>11)&31)*255//31,((c>>5)&63)*255//63,(c&31)*255//31],1).astype(np.float32)
    a=rgb(c0); bb=rgb(c1)
    gt=(c0>c1)[:,None]
    p2=np.where(gt,(2*a+bb)/3,(a+bb)/2); p3=np.where(gt,(a+2*bb)/3,0)
    pal=np.stack([a,bb,p2,p3],1)  # n,4,3
    idx=(b[:,4:8].astype(np.uint32)); bits=idx[:,0]|(idx[:,1]<<8)|(idx[:,2]<<16)|(idx[:,3]<<24)
    sel=np.stack([(bits>>(2*i))&3 for i in range(16)],1)  # n,16
    px=pal[np.arange(len(b))[:,None],sel]  # n,16,3
    bw,bh=w//4,h//4
    return px.reshape(bh,bw,4,4,3).transpose(0,2,1,3,4).reshape(h,w,3).astype(np.uint8)
