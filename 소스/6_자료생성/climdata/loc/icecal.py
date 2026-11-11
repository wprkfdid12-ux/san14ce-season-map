import numpy as np, sys
sys.path.insert(0,'/home/claude/climdata/sim')
import model
W=220
z=np.load('clim_cells_s.npz'); rv=np.load('rivers.npz')
terr=model.terr; c=np.arange(48400)%W; r=np.arange(48400)//W
Y=(rv['own']==1)&(rv['cls']==1)&(terr==14)
L=(terr==14)&(c>=184)&(c<=197)&(r>=8)&(r<=31)
def sm(e0,e1,x): t=np.clip((x-e0)/(e1-e0),0,1); return t*t*(3-2*t)
def Ti_all(T0src,dT=0.0):
    out=[]; frz_old=[]
    model.T0[:]=T0src
    for t in range(36):
        tpos=t//3+2+((t%3)-1)/3.0; tf=int(np.floor(tpos)); f=tpos-tf; i0=tf%12; i1=(tf+1)%12
        T=model.T0[:,i0]*(1-f)+model.T0[:,i1]*f-0.0045*model.alt-1.2*sm(0.25,0.75,model.imp)+dT
        g=lambda i:model.T0[:,(i+1)%12]-model.T0[:,(i-1)%12]
        rising=sm(-1.5,1.5,g(i0)*(1-f)+g(i1)*f)
        Ti=T-2*model.northZ-2*rising*(1-sm(4,8,T)); out.append(Ti)
        fr=np.minimum(model.fm*3,1)*sm(1.5,-5,Ti); frz_old.append((terr==14)&(fr>=0.3))
    return np.array(out),np.array(frz_old)
if __name__=='__main__':
    oldT=model.T0.copy()
    Ti,fo=Ti_all(z['T0'])
    k5=np.sort(Ti[:,Y],0)[5]; TfY=float(k5.max()+0.2)
    nY=(Ti[:,Y]<TfY).sum(0)
    k7=np.sort(Ti[:,L],0)[6]; TfL=float(k7.max()+0.2)
    nL=(Ti[:,L]<TfL).sum(0)
    print('Yellow cells',Y.sum(),'TfY',round(TfY,2),'frozen turns per cell min/median/max',nY.min(),np.median(nY),nY.max())
    print('Liaodong cells',L.sum(),'TfL',round(TfL,2),'turns',nL.min(),np.median(nL),nL.max())
    for dT in (-1.5,0,1.5):
        Ti2,_=Ti_all(z['T0'],dT)
        print('dT',dT,'Yellow frozen turns (min/med/max)',(Ti2[:,Y]<TfY).sum(0).min(),np.median((Ti2[:,Y]<TfY).sum(0)),(Ti2[:,Y]<TfY).sum(0).max(),
              ' all-Yellow-frozen turns',int(((Ti2[:,Y]<TfY).all(1)).sum()),' Liaodong med',np.median((Ti2[:,L]<TfL).sum(0)))
    # existing Bohai-coast freeze cells (old rule) under new climate vs old table
    ref=np.load('/home/claude/climdata/sim/ref_frz.npy').astype(bool)
    other=(terr==14)&~Y&~L
    print('other freezable cell-turns old table',int((ref[:,other]).sum()),' new climate same rule',int(fo[:,other].sum()))
    np.save('tf.npy',np.array([TfY,TfL]))
    model.T0[:]=oldT
