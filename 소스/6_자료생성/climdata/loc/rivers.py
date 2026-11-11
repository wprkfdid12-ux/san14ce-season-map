import numpy as np, sys, json
from collections import deque
sys.path.insert(0,'/home/claude/climdata/land')
from common import terr, W, N, NB, height, order
rid_n=np.load('/home/claude/climdata/land/rid_nearest.npy')
WATER=np.isin(terr,[14,24])
def cell(c,r): return r*W+c
def path(a,b):
    prev=np.full(N,-1); seen=np.zeros(N,bool); q=deque([a]); seen[a]=True
    while q:
        x=q.popleft()
        if x==b: break
        for n in NB[x]:
            if WATER[n] and not seen[n]: seen[n]=True; prev[n]=x; q.append(n)
    p=[b]
    while p[-1]!=a: p.append(prev[p[-1]]); assert p[-1]>=0
    return p[::-1]
R={1:(cell(82,60),cell(158,57)),2:(cell(74,146),cell(205,117)),3:(cell(68,112),cell(121,137))}
paths={r:path(*R[r]) for r in R}
for r in paths: print('river',r,'path len',len(paths[r]))
# band: water cells within 5 steps (through water) of a path, given to the nearest river
own=np.zeros(N,np.int8); pos=np.full(N,-1,np.int32); dist=np.full(N,99,np.int16)
q=deque()
for r,p in paths.items():
    for i,c in enumerate(p): own[c]=r; pos[c]=i; dist[c]=0; q.append(c)
while q:
    x=q.popleft()
    if dist[x]>=5: continue
    for n in NB[x]:
        if WATER[n] and dist[n]>dist[x]+1: dist[n]=dist[x]+1; own[n]=own[x]; pos[n]=pos[x]; q.append(n)
band=WATER&(own>0)
# land side: banks (adjacent to band), walls (cliff 20 adjacent to band), low ground behind
BAD=np.isin(terr,[0,9,10,11,19,21,22,23,24,12,13,14,15,16,17,18])
cls=np.zeros(N,np.uint8)   # 1 water band, 2 open bank, 3 wall, 4 low ground (normal flood), 5 low ground (big flood only), 6 behind a wall
cls[band]=1
for c in np.nonzero(band)[0]:
    for n in NB[c]:
        if terr[n]==20 and cls[n]==0: cls[n]=3; own[n]=own[c]; pos[n]=pos[c]
        elif not BAD[n] and not WATER[n] and cls[n]==0 and height[n]-height[c]<=550: cls[n]=2; own[n]=own[c]; pos[n]=pos[c]
# low ground from the flood order map (order<=0.20 normal, <=0.45 big), reached over land from open banks without crossing walls -> 4/5, else behind wall -> 6
d2=np.full(N,99,np.int16); q=deque()
for c in np.nonzero(cls==2)[0]: d2[c]=0; q.append(c)
while q:
    x=q.popleft()
    if d2[x]>=8: continue
    for n in NB[x]:
        if cls[n] in (1,3) or BAD[n] or WATER[n]: continue
        if d2[n]>d2[x]+1 and order[n]<=0.45 and rid_n[n]>0:
            d2[n]=d2[x]+1; own[n]=own[x]; pos[n]=pos[x]; q.append(n)
for c in np.nonzero((d2>0)&(d2<99))[0]:
    cls[c]=4 if order[c]<=0.20 else 5
# behind walls: low-ground cells not reached from open banks but next to a wall chain
d3=np.full(N,99,np.int16); q=deque()
for c in np.nonzero(cls==3)[0]: d3[c]=0; q.append(c)
while q:
    x=q.popleft()
    if d3[x]>=4: continue
    for n in NB[x]:
        if cls[n]!=0 or BAD[n] or WATER[n]: continue
        if d3[n]>d3[x]+1 and order[n]<=0.45: d3[n]=d3[x]+1; own[n]=own[x]; pos[n]=pos[x]; q.append(n)
cls[(d3>0)&(d3<99)&(cls==0)]=6
# wall segments: connected wall chains
seg=np.zeros(N,np.int16); ns=0
for c in np.nonzero(cls==3)[0]:
    if seg[c]: continue
    ns+=1; q=deque([c]); seg[c]=ns
    while q:
        x=q.popleft()
        for n in NB[x]:
            if cls[n]==3 and not seg[n]: seg[n]=ns; q.append(n)
# behind-wall cells take the segment of the nearest wall
q=deque(np.nonzero(cls==3)[0].tolist())
while q:
    x=q.popleft()
    for n in NB[x]:
        if cls[n]==6 and not seg[n]: seg[n]=seg[x]; q.append(n)
for r in (1,2,3):
    m=own==r
    print('river',r,{k:int(((cls==k)&m).sum()) for k in range(1,7)},'wall segments',len(set(seg[m&(cls==3)].tolist())))
np.savez('rivers.npz',cls=cls,own=own,pos=pos,seg=seg,plen=np.array([len(paths[1]),len(paths[2]),len(paths[3])]),
         p1=np.array(paths[1]),p2=np.array(paths[2]),p3=np.array(paths[3]))
