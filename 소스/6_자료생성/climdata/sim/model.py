import numpy as np
rec=np.fromfile('/home/claude/climdata/sim/out/cells.bin',np.uint8).reshape(-1,32)
terr=rec[:,0].astype(int); alt=(rec[:,1].astype(int)|(rec[:,2].astype(int)<<8)).astype(float)
northZ=rec[:,3]/255.0; nwZ=rec[:,4]/255.0; imp=rec[:,5]/255.0; fm=rec[:,6]/255.0
T0=(rec[:,8:20].astype(float)-128)/4.0; D=rec[:,20:32]/255.0
def sm(e0,e1,x): t=np.clip((x-e0)/(e1-e0),0,1); return t*t*(3-2*t)
LAND=~np.isin(terr,[19,20,21,22,23,14,24,0]); HILL=np.isin(terr,[4,5,6,7,8,9,10])
def state(t,dT=0.0):
    tpos=t//3+2+((t%3)-1)/3.0; tf=int(np.floor(tpos)); f=tpos-tf
    i0=tf%12; i1=(tf+1)%12
    c0=T0[:,i0]*(1-f)+T0[:,i1]*f
    g=lambda i:T0[:,(i+1)%12]-T0[:,(i-1)%12]
    gr=g(i0)*(1-f)+g(i1)*f
    T=c0-0.0045*alt-1.2*sm(0.25,0.75,imp)+dT
    rising=sm(-1.5,1.5,gr)
    Ts=T-4.0*northZ
    nf=max(0.0,1.0-abs(tpos-10.0)/0.67)
    if nf>0:
        Tdec=T0[:,11]-0.0045*alt-1.2*sm(0.25,0.75,imp)-4.0*northZ+dT
        Ts=Ts*(1-nf)+np.minimum(Ts,0.5*(Ts+Tdec))*nf
    snowT=sm(2.0,-9.0,Ts)
    Ti=T-2.0*northZ-2.0*rising*(1-sm(4.0,8.0,T))
    fr=np.minimum(fm*3.0,1.0)*sm(1.5,-5.0,Ti)
    frozen=(terr==14)&(fr>=0.30)
    need=np.where(rising>0.5,0.80,0.62)
    lying=(snowT>=need)&(nwZ<0.5)&LAND
    snow=np.where(lying,np.where(HILL|(alt>900),2,1),0)
    return T,snow,frozen
if __name__=='__main__':
    rs=np.load('ref_snow.npy'); rf=np.load('ref_frz.npy')
    bad_s=bad_f=0
    for t in range(36):
        T,s,fz=state(t); bad_s+=(s!=rs[t]).sum(); bad_f+=(fz!=rf[t].astype(bool)).sum()
    print('mismatch vs current rules table: snow',bad_s,'of',36*len(s),' ice',bad_f)
