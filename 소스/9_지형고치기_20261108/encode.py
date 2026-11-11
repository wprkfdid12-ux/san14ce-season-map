import numpy as np, zlib, struct, hashlib
from zpad import zexact
P='/home/claude/pkg41/SAN14CE_SeasonMap_v26/'
pay=bytearray(b''.join(open(P+'payload/season26.part%d'%i,'rb').read() for i in (1,2,3)))
sizes=[len(open(P+'payload/season26.part%d'%i,'rb').read()) for i in (1,2,3)]
def chunks(body):
    p=0; out=[]
    while p<len(body):
        L=struct.unpack('<I',body[p:p+4])[0]; out.append(bytes(body[p+4:p+4+L])); p+=4+L
    assert p==len(body); return out
def rebuild(off, ln, base_f, new_f, name):
    body=pay[off:off+ln]; cs=chunks(body)
    raw=bytearray(b''.join(zlib.decompress(c) for c in cs))
    g=np.fromfile(base_f); n=np.fromfile(new_f); NV=(len(raw)-32)//4
    v=np.frombuffer(bytes(raw[32:]),'<u4').copy()
    ch=np.nonzero(np.abs(n[:NV]-g[:NV])>0.5)[0]
    newv=(np.clip(np.round(n[ch]*256),0,0xffffff).astype(np.uint32)<<8)
    v[ch]=newv; raw[32:]=v.tobytes()
    touched=sorted(set(((32+4*ch)//16384).tolist())|set(((32+4*ch+3)//16384).tolist()))
    newc=list(cs)
    for k in touched: newc[k]=zlib.compress(bytes(raw[k*16384:(k+1)*16384]),9)
    CAP=16000   # the game reads each chunk into a fixed buffer: no chunk may be bigger than the season data already uses
    slack=sum(len(c) for c in cs)-sum(len(c) for c in newc)
    order=sorted([k for k in range(len(cs)) if k not in touched], key=lambda k:-len(cs[k]))
    extra=[]
    while slack<0:
        k=order.pop(0); c9=zlib.compress(bytes(raw[k*16384:(k+1)*16384]),9); slack+=len(newc[k])-len(c9); newc[k]=c9; extra.append(k)
    # spread the left-over bytes over the touched chunks (each padded inside its own zlib stream, never above CAP)
    for k in touched:
        if slack==0: break
        data=bytes(raw[k*16384:(k+1)*16384]); room=CAP-len(newc[k])
        if room<40: continue
        add=min(slack,room)
        if 0<slack-add<40: add=max(0,slack-40)
        if add<40: continue
        s=None
        for a in range(add, add-6, -1):
            s=zexact(data,len(newc[k])+a)
            if s: break
        if not s: continue
        slack-=len(s)-len(newc[k]); newc[k]=s
    assert slack==0, ('left-over', slack)
    assert max(len(c) for c in newc)<=CAP
    nb=b''.join(struct.pack('<I',len(c))+c for c in newc); assert len(nb)==ln,(len(nb),ln)
    # verify
    raw2=b''.join(zlib.decompress(c) for c in chunks(nb)); assert raw2==bytes(raw)
    print(name,'changed px',len(ch),'chunks',len(touched),'freed extra',len(extra),'max chunk',max(len(c) for c in newc),'body ok')
    pay[off:off+ln]=nb
rebuild(7395715,11927573,'g948.f64','n948.f64','mhk_948')
rebuild(19323288,12127465,'g613.f64','n613.f64','mhk_613')
import os; os.makedirs('payload',exist_ok=True)
p=0
for i,s in enumerate(sizes,1):
    open('payload/season26.part%d'%i,'wb').write(pay[p:p+s]); print('part',i,hashlib.sha256(pay[p:p+s]).hexdigest()); p+=s
assert p==len(pay)
