import struct,sys
N="add and break breakc call callc case continue continuec cut default deriv_rtx deriv_rty discard div dp2 dp3 dp4 else emit emitthencut endif endloop endswitch eq exp frc ftoi ftou ge iadd if ieq ige ilt imad imax imin imul ine ineg ishl ishr itof label ld ld_ms log loop lt mad min max customdata mov movc mul ne nop not or resinfo ret retc round_ne round_ni round_pi round_z rsq sample sample_c sample_c_lz sample_l sample_d sample_b sqrt switch sincos udiv ult uge umul umad umax umin ushr utof xor".split()
OPN={i:n for i,n in enumerate(N)}
OPN.update({0x6b:'lod',0x6c:'gather4',0x80:'rcp',0x81:'f32tof16',0x82:'f16tof32',0x85:'countbits',0x89:'ubfe',0x8a:'ibfe',0x8b:'bfi',0x8c:'bfrev',0x79:'deriv_rtx_coarse',0x7a:'deriv_rtx_fine',0x7b:'deriv_rty_coarse',0x7c:'deriv_rty_fine',0x7d:'gather4_c',0x78:'bufinfo',0xa7:'ld_structured',0xa5:'ld_raw'})
DCL=set(range(88,107))|set(range(0x8f,0xa3))|{0x6a}
OT={0:'r',1:'v',2:'o',3:'x',4:'l',6:'s',7:'t',8:'cb',9:'icb',0xc:'null',0x17:'vPrim',0x20:'vThreadID',0x23:'vCoverage',0x24:'vThreadIDInGroup',0x25:'vThreadIDInGroupFlattened',0x1:'v'}
def f2s(u):
    f=struct.unpack('<f',struct.pack('<I',u))[0]
    if u in (0,) : return '0'
    if abs(f)<1e-30 or abs(f)>1e30 or u&0x7f800000==0: return 'i%d'%struct.unpack('<i',struct.pack('<I',u))[0]
    return repr(f)
def operand(t,p):
    tok=t[p]; p+=1
    nc=[0,1,4,0][tok&3]; sel=(tok>>2)&3; comp=(tok>>4)&0xff
    typ=(tok>>12)&0xff; idim=(tok>>20)&3; reps=[(tok>>22)&7,(tok>>25)&7,(tok>>28)&7]
    mod=''
    if tok>>31:
        ext=t[p]; p+=1
        m=(ext>>6)&0xff; mod={1:'-',2:'|',3:'-|'}.get(m,'')
    if typ==4:
        vals=t[p:p+nc]; p+=nc
        return 'l(%s)'%','.join(f2s(v) for v in vals),p
    if typ==5:
        p+=nc*2; return 'd(...)',p
    idx=[]
    for k in range(idim):
        r=reps[k]
        if r==0: idx.append(str(t[p])); p+=1
        elif r==1: idx.append(str(t[p])); p+=2
        elif r==2:
            s,p=operand(t,p); idx.append(s)
        elif r==3:
            base=t[p]; p+=1; s,p=operand(t,p); idx.append('%s+%d'%(s,base))
    name=OT.get(typ,'T%d'%typ)
    if idim==0: s=name
    elif idim==1: s=name+(idx[0] if reps[0]==0 else '['+idx[0]+']')
    else: s=name+idx[0]+''.join('['+i+']' for i in idx[1:])
    if nc==4:
        if sel==0: s+='.'+''.join('xyzw'[i] for i in range(4) if comp&(1<<i))
        elif sel==1: s+='.'+''.join('xyzw'[(comp>>(2*i))&3] for i in range(4))
        else: s+='.'+'xyzw'[comp&3]
    if mod=='-': s='-'+s
    elif mod=='|': s='|'+s+'|'
    elif mod=='-|': s='-|'+s+'|'
    return s,p
def dis(path):
    d=open(path,'rb').read(); n=struct.unpack_from('<I',d,28)[0]
    for i in range(n):
        o=struct.unpack_from('<I',d,32+4*i)[0]
        if d[o:o+4] in (b'SHEX',b'SHDR'):
            sz=struct.unpack_from('<I',d,o+4)[0]
            t=struct.unpack_from('<%dI'%(sz//4),d,o+8)
    p=2; out=[]; ind=0
    while p<len(t):
        tok=t[p]; op=tok&0x7ff; ln=(tok>>24)&0x7f
        if op==53: ln=t[p+1]
        if ln==0: break
        if op in DCL or op==53:
            out.append('  '*ind+'dcl#%d %s'%(op,' '.join(hex(x) for x in t[p+1:p+min(ln,6)]))); p+=ln; continue
        name=OPN.get(op,'op%d'%op)
        sat=(tok>>13)&1; tst=(tok>>18)&1
        q=p+1; ext=tok>>31
        while ext:
            e=t[q]; q+=1; ext=e>>31
        ops=[]
        while q<p+ln:
            s,q=operand(t,q); ops.append(s)
        if name in ('if','discard','breakc','retc','continuec'): name+=('_nz' if tst else '_z')
        if name in ('endif','endloop','else'): ind=max(ind-1,0)
        out.append('  '*ind+name+('_sat' if sat else '')+' '+', '.join(ops))
        if name.startswith('if') or name in ('loop','else'): ind+=1
        p+=ln
    return out
if __name__=='__main__':
    for l in dis(sys.argv[1]): print(l)
