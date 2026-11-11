import re,hashlib,struct,zlib,numpy as np,json
from zpad import zexact
P='/home/claude/pkg42c/SAN14CE_SeasonMap_v26/'
ps=open(P+'tools/Terrain26.ps1',encoding='utf-8-sig').read()
pay=bytearray(b''.join(open('payload/season26.part%d'%i,'rb').read() for i in (1,2,3)))   # mhk already rebuilt (k42c)
sizes=[len(open('payload/season26.part%d'%i,'rb').read()) for i in (1,2,3)]
U='/mnt/user-data/uploads/Romance_of_the_Three_Kingdoms_14/0002/'
K=6966.2/65535
D={'948':(np.fromfile('n948.f64')-np.fromfile('g948.f64'))*K, '613':(np.fromfile('n613.f64')-np.fromfile('g613.f64'))*K}
def tex_offs(raw):
    to=struct.unpack_from('<I',raw,0xc)[0]; n=struct.unpack_from('<I',raw,0x10)[0]
    return [struct.unpack_from('<I',raw,to+4*j)[0]+to for j in range(n)]
done={}; res={}
CAP=16000
for m in re.finditer(r"Name='(tex_\w+)';File='([^']+)';FileLen=\[long\](\d+);Offset=\[long\](\d+);Len=\[int\](\d+);Orig='(\w+)';New='(\w+)';Prev='([^']*)';Segs='([^']+)'",ps):
    nm,fn,fl,off,ln,orig,new,prev,segs=m.groups(); off=int(off); ln=int(ln)
    f=U+(fn if fn!='FieldEditor4.rdb.bin2' else 'FieldEditor4.rdb.bin')
    d=open(f,'rb'); d.seek(off); cur=d.read(ln); assert hashlib.sha256(cur).hexdigest()==new, nm
    S=segs.split(';'); pos=0; prange=None
    for s in S:
        k,a,n=s.split(','); a=int(a); n=int(n)
        if k=='P': prange=(pos,a,n)
        pos+=n
    sp,pa,pn=prange
    if pa in done:
        newsec=bytes(cur[:sp])+bytes(pay[pa:pa+pn])+bytes(cur[sp+pn:]); assert len(newsec)==ln
        res[nm]=hashlib.sha256(newsec).hexdigest(); print(nm,'same payload slice as',done[pa]); continue
    hdr=int.from_bytes(cur[8:12],'little')-int.from_bytes(cur[16:20],'little')
    p=hdr; cs=[]; starts=[]
    while True:
        L,=struct.unpack_from('<I',cur,p)
        if L==0: break
        starts.append(p); cs.append(cur[p+4:p+4+L]); p+=4+L
    end=p
    raw=bytearray(b''.join(zlib.decompress(c) for c in cs))
    o0=tex_offs(raw)[0]+28
    h=np.frombuffer(bytes(raw[o0:o0+2049*2049*4]),'<f4').astype(np.float64)
    var='948' if np.abs(h-np.fromfile('g948.f64')*K).mean() < np.abs(h-np.fromfile('g613.f64')*K).mean() else '613'
    dl=D[var]; ch=np.nonzero(np.abs(dl)>1e-6)[0]
    hn=h.copy(); hn[ch]=np.round((h[ch]+dl[ch])*64)/64   # 1/64 of a world unit: invisible, and the changed floats compress as well as the old ones
    raw[o0:o0+2049*2049*4]=hn.astype('<f4').tobytes()
    bpos=o0+4*ch
    touched=sorted(set((bpos//16384).tolist())|set(((bpos+3)//16384).tolist()))
    CH=len(cs); csz=[len(zlib.decompress(c)) for c in cs]
    cum=np.cumsum([0]+csz)
    def chunkdata(k): return bytes(raw[cum[k]:cum[k+1]])
    # map byte pos -> chunk index (chunks are 16384 except the last)
    touched=sorted(set(int(np.searchsorted(cum,b,side='right')-1) for b in np.concatenate([bpos,bpos+3])))
    inP=lambda k: starts[k]>=sp and starts[k]+4+len(cs[k])<=sp+pn
    assert all(inP(k) for k in touched), 'touched chunk outside payload range'
    newc=list(cs)
    for k in touched: newc[k]=zlib.compress(chunkdata(k),9)
    slack=sum(map(len,cs))-sum(map(len,newc))
    order=sorted([k for k in range(CH) if k not in set(touched) and inP(k)], key=lambda k:-len(cs[k])); freed=0
    while slack<40:
        k=order.pop(0); c9=zlib.compress(chunkdata(k),9)
        if len(c9)<len(newc[k]): slack+=len(newc[k])-len(c9); newc[k]=c9; freed+=1
    for k in list(touched)+[k for k in range(CH) if newc[k] is not cs[k] and k not in set(touched)]:
        if slack==0: break
        room=CAP-len(newc[k])
        if room<40: continue
        add=min(slack,room)
        if 0<slack-add<40: add=max(0,slack-40)
        if add<40: continue
        s=None
        for a_ in range(add,add-6,-1):
            s=zexact(chunkdata(k),len(newc[k])+a_)
            if s: break
        if s: slack-=len(s)-len(newc[k]); newc[k]=s
    assert slack==0,(nm,slack); assert max(map(len,newc))<=CAP
    body=b''.join(struct.pack('<I',len(c))+c for c in newc)
    newsec=bytes(cur[:hdr])+body+bytes(cur[end:])
    assert len(newsec)==ln
    # verify
    p=hdr; out=[]
    while True:
        L,=struct.unpack_from('<I',newsec,p); p+=4
        if L==0: break
        out.append(zlib.decompress(newsec[p:p+L])); p+=L
    assert b''.join(out)==bytes(raw)
    # differences must be inside the payload range
    diff=[i for i in (0,hdr-1,end,ln-1) if newsec[i]!=cur[i]]
    assert newsec[:sp]==cur[:sp] and newsec[sp+pn:]==cur[sp+pn:], 'change outside P range'
    pay[pa:pa+pn]=newsec[sp:sp+pn]
    res[nm]=hashlib.sha256(newsec).hexdigest(); done[pa]=nm
    print(nm,'freed',freed,'variant',var,'changed px',len(ch),'chunks',len(touched),'max',max(map(len,newc)),'ok')
p=0
for i,s in enumerate(sizes,1):
    open('payload/season26.part%d'%i,'wb').write(pay[p:p+s]); p+=s
json.dump(res,open('newsha_tex.json','w')); print(res)
