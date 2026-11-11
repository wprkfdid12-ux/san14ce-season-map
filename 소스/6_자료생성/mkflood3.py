# v26k14: the whole-river flood data appended to SeasonRules_v26.dat (block 'FLD3'): river stretches (river, downstream stretch,
# name cell, bankfull flow), and per cell: the stretch whose water floods it, the stage that reaches it (1 flood, 2 big, 3 great),
# the stretch its rain drains to, and the stretch's own river water (for the ice).
import numpy as np, struct, json, sys
sys.path.insert(0,'/home/claude/climdata/land')
from common import terr, W, N
z=np.load('/tmp/claude-0/fl14/stretch.npz'); st=json.load(open('/tmp/claude-0/fl14/stretches.json'))
bank=json.load(open(sys.argv[1])) if len(sys.argv)>1 else None      # bankfull flows from the calibration run
P='/home/claude/rules/SeasonRules_v26.dat'
d=bytearray(open(P,'rb').read())
need2=24+N*5+36*(N//8)+36*N+N*64+N*4+81*2; off=(need2+15)&~15
d=d[:off]+b'\0'*(off-len(d)) if len(d)<off else d[:off]
NS=len(st); out=bytearray(b'FLD3')+struct.pack('<I',NS)
for i,(r,cells) in enumerate(st,1):
    mid=cells[len(cells)//2]; B=float(bank[i-1]) if bank else 0.0
    out+=struct.pack('<BBHHHff',r,int(z['dn'][i]),mid,len(cells),0,B,0.0)
fstr=np.where(z['need']<9,z['fown'],0).astype(np.uint8); need=np.where(z['need']<9,z['need'],0).astype(np.uint8)
catch=z['catch'].astype(np.uint8)
stem=np.zeros(N,np.uint8); ss=z['stem_sid']; wown=z['wown']
stem[(wown>0)&np.isin(terr,[13,14,24])]=wown[(wown>0)&np.isin(terr,[13,14,24])].astype(np.uint8)
out+=fstr.tobytes()+need.tobytes()+catch.tobytes()+stem.tobytes()
d+=out; d+=b'\0'*((16-len(d)%16)%16)
open(P,'wb').write(d)
print('FLD3 block at',off,'stretches',NS,'size',len(out),'file',len(d),'bankfull', 'set' if bank else 'not yet')
