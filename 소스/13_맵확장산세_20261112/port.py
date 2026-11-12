# faithful Python port of SeasonME.Merge (zlib level 6 stands in for .NET Optimal)
import struct, zlib, numpy as np, math
CAP=16000
def U32(b,o): return struct.unpack_from('<I',b,o)[0]
class Cont:
    def __init__(s,b):
        s.B=b; s.Hdr=U32(b,8)-U32(b,16); p=s.Hdr; s.Pos=[]; s.Len=[]
        while True:
            L=U32(b,p)
            if L==0: break
            s.Pos.append(p+4); s.Len.append(L); p+=4+L
        s.End=p; s.Raw={}; s.CH=len(s.piece(0))
    def piece(s,k):
        if k not in s.Raw: s.Raw[k]=bytearray(zlib.decompressobj(-15).decompress(s.B[s.Pos[k]+2:s.Pos[k]+s.Len[k]-4]))
        return s.Raw[k]
    def rawlen(s): return U32(s.B,24)
    def readraw(s,o,n):
        out=bytearray()
        while len(out)<n:
            q=o+len(out); k=q//s.CH; w=q%s.CH; pc=s.piece(k); c=min(n-len(out),len(pc)-w); out+=pc[w:w+c]
        return bytes(out)
def rawdeflate(d):
    c=zlib.compressobj(6,zlib.DEFLATED,-15); return c.compress(bytes(d))+c.flush()
def zl(d):
    return b'\x78\x9c'+rawdeflate(d)+struct.pack('>I',zlib.adler32(bytes(d))&0xffffffff)
def exact(d,size):
    for m in range(0,81):
        df=rawdeflate(d[m:]); fixed=2+(5+m if m>0 else 0)+len(df)+4; rest=size-fixed
        if rest<0:
            if m==0: return None
            continue
        if rest%5: continue
        o=b'\x78\x9c'+b'\x00\x00\x00\xff\xff'*(rest//5)
        if m>0: o+=bytes([0,m&255,m>>8,(~m)&255,((~m)>>8)&255])+bytes(d[:m])
        o+=df+struct.pack('>I',zlib.adler32(bytes(d))&0xffffffff); assert len(o)==size and zlib.decompress(o)==bytes(d); return o
    return None
def grid(c,kind):
    if kind==0: return 32, int(round(math.sqrt((c.rawlen()-32)/4)))
    h=c.readraw(0,64); to=U32(h,0xc); t=c.readraw(to,8); o0=U32(t,0)+to; o1=U32(t,4)+to; return o0+28, int(round(math.sqrt((o1-o0-28)/4)))
def rg(c,s,w): return np.frombuffer(c.readraw(s,w*w*4),'<u4').copy()
def r64(a): f=a.view('<f4'); return (np.round(f.astype(np.float64)*64)/64).astype('<f4').view('<u4')
def merge(mec,oc,nc,kind):
    me,co,cn=Cont(mec),Cont(oc),Cont(nc)
    sm,wm=grid(me,kind); so,wo=grid(co,kind); sn,wn=grid(cn,kind)
    go=rg(co,so,wo); gn=rg(cn,sn,wn)
    if kind==1: ch=gn!=go; gn[ch]=r64(gn[ch])
    gm=rg(me,sm,wm); dr=dc=-1
    probes=[]
    for r in range(wo//7, wo-1, 97):
        for c in range(wo//9, wo-64, 89):
            if len(probes)>=8: break
            b0=r*wo+c
            if (go[b0:b0+64]==gn[b0:b0+64]).all() and (go[b0:b0+64]!=go[b0]).any(): probes.append((r,c))
    gb=gm.tobytes()
    for r0,c0 in probes:
        b=r0*wo+c0; pat=go[b:b+64].tobytes(); k=gb.find(pat)
        while k>=0 and k%4: k=gb.find(pat,k+1)
        if k>=0: i=k//4; dr=i//wm-r0; dc=i%wm-c0; break
    GO=go.reshape(wo,wo); GN=gn.reshape(wo,wo); GM=gm.reshape(wm,wm); sub=GM[dr:dr+wo,dc:dc+wo]
    chg=GO!=GN; already=(chg&(sub==GN)).sum(); app=chg&(sub==GO)&(sub!=GN); theirs=chg&(sub!=GO)&(sub!=GN)
    ys,xs=np.nonzero(app); J=(ys+dr)*wm+xs+dc; bp=sm+4*J; touched=set()
    for b_,v in zip(bp.tolist(), GN[ys,xs].tolist()):
        nb=struct.pack('<I',v)
        for q in range(4):
            kq=(b_+q)//me.CH; wq=(b_+q)%me.CH; me.piece(kq)[wq]=nb[q]; touched.add(kq)
    n=len(me.Len); pieces=[None]*n
    for k in touched: pieces[k]=zl(me.Raw[k])
    slack=sum(me.Len)-sum(len(pieces[k]) if pieces[k] else me.Len[k] for k in range(n))
    print('offset',dr,dc,'applied',app.sum(),'already',already,'theirs',theirs.sum(),'touched',len(touched),'slack before pad',slack)
    if slack<0:
        for k in sorted([k for k in range(n) if k not in touched], key=lambda k:-me.Len[k]):
            if slack>=0: break
            z=zl(me.piece(k))
            if len(z)<me.Len[k]: slack+=me.Len[k]-len(z); pieces[k]=z; touched.add(k)
        if slack<0: print('DOES NOT FIT', slack); return None
    for k in sorted(touched):
        if slack==0: break
        cur=len(pieces[k]); room=CAP-cur
        if room<=0: continue
        add=min(slack,room); ex=None
        for a in range(add, max(0,add-12)-1, -1):
            if a==0: break
            ex=exact(me.Raw[k],cur+a)
            if ex: break
        if not ex: continue
        slack-=len(ex)-cur; pieces[k]=ex
    assert slack==0, slack
    out=bytearray(mec[:me.Hdr])
    for k in range(n):
        if pieces[k] is None: out+=mec[me.Pos[k]-4:me.Pos[k]+me.Len[k]]
        else: assert len(pieces[k])<=CAP; out+=struct.pack('<I',len(pieces[k]))+pieces[k]
    assert len(out)==me.End; out+=mec[me.End:]; assert len(out)==len(mec)
    return bytes(out)
