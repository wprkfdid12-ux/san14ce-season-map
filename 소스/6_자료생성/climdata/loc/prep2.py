import numpy as np, json, base64, sys
sys.path.insert(0,'/home/claude/climdata/land')
from common import terr, W, N, NB, height
rec_old=np.fromfile('/home/claude/climdata/sim/out/cells.bin',np.uint8).reshape(-1,32)
z=np.load('clim_cells_s.npz'); rv=np.load('rivers.npz'); TfY,TfL=np.load('tf.npy')
cols=np.arange(N)%W; rows=np.arange(N)//W
X=375.5+7.0*cols; Y=138.5+8.0829*(rows-0.5*(cols%2))
dat=open('/home/claude/rules/SeasonRules.dat','rb').read(); prov=np.frombuffer(dat[24+N:24+2*N],np.uint8)
cent=np.load('/home/claude/climdata/mapview/city_cent.npy')
# distance (in cells) to the nearest game city
d=np.sqrt(((X[:,None]-cent[None,:,0])/7.0)**2+((Y[:,None]-cent[None,:,1])/8.08)**2).min(1)
# nearest real place (localized) for names in the event log
KO={'Shenyang':'심양','Dalian':'대련','Yingkou':'영구','Jinzhou (Liaoning)':'요서 금주','Dandong':'서안평','Qinhuangdao':'진황도','Chengde':'승덕','Zhangjiakou':'상곡','Beijing':'계','Tianjin':'천진','Shijiazhuang':'상산','Handan':'업','Tangshan':'당산',
 'Datong':'평성','Taiyuan':'진양','Hohhot':'운중','Baotou':'오원','Yulin (Shaanxi)':'상군','Yan\'an':'고노','Yinchuan':'북지','Lanzhou':'금성','Wuwei':'무위','Zhangye':'장액','Tianshui':'천수',
 'Xi\'an':'장안','Baoji':'진창','Luoyang':'낙양','Zhengzhou':'형양','Kaifeng':'진류','Xuchang':'허창','Nanyang (Henan)':'완','Xinyang':'의양','Jinan':'제남','Qingdao':'동래','Weifang':'북해','Linyi':'낭야','Xuzhou':'팽성','Hanzhong':'한중','Ankang':'서성','Xiangyang':'양양','Shiyan':'상용',
 'Chengdu':'성도','Mianyang':'부현','Nanchong':'안한','Chongqing':'강주','Wanzhou':'고릉','Yichang':'이릉','Jingzhou':'강릉','Wuhan':'강하','Yueyang':'파구','Changde':'무릉','Changsha':'장사','Jiujiang':'시상','Nanchang':'예장','Hefei':'합비','Nanjing':'건업','Suzhou (Jiangsu)':'오','Hangzhou':'전당','Shaoxing':'회계',
 'Guiyang':'장가','Zunyi':'준의','Kunming':'곤명','Dali (Yunnan)':'엽유','Qujing':'건녕','Zhaotong':'주제','Xichang':'월수','Chenzhou':'계양','Ganzhou':'남강','Fuzhou (Fujian)':'동야','Guilin':'시안','Wuzhou':'창오','Guangzhou':'남해','Nanning':'울림','Zhanjiang':'합포','Haikou':'주애','Hanoi':'교지'}
CP=json.load(open('city_game_pos.json'))
names=[KO.get(c['city'],c['city']) for c in CP]; G=np.array([[c['gx'],c['gy']] for c in CP])
near=((X[:,None]-G[None,:,0])**2+(Y[:,None]-G[None,:,1])**2).argmin(1)
slope=np.zeros(N)
for c in range(N): slope[c]=max(abs(height[n]-height[c]) for n in NB[c])
src=np.zeros(N,np.uint8)
land=~np.isin(terr,[13,14,23,24,21,12])
src[land&(X>=640)&(X<=1200)&(Y>=300)&(Y<=800)]|=1
src[land&(X>=400)&(X<=1350)&(Y>=1150)&(Y<=1650)]|=2
src[land&(X>=620)&(X<=1000)&(Y>=850)&(Y<=1120)]|=4
own=rv['own'].astype(np.uint8); cls=rv['cls'].astype(np.uint8); pos=np.clip(rv['pos'],0,65535).astype(np.uint16); seg=np.clip(rv['seg'],0,255).astype(np.uint8)
c_=np.arange(N)%W; r_=np.arange(N)//W
ice=np.zeros(N,np.uint8)
ice[(own==1)&(cls==1)&(terr==14)]=1
ice[(terr==14)&(c_>=184)&(c_<=197)&(r_>=8)&(r_<=31)]=2
R=np.zeros((N,64),np.uint8)
R[:,0]=terr; a16=rec_old[:,1].astype(np.uint16)|(rec_old[:,2].astype(np.uint16)<<8); R[:,1]=a16&255; R[:,2]=a16>>8
R[:,3:7]=rec_old[:,3:7]
R[:,7]=(own&3)|((cls&7)<<2)|((ice&3)<<5)
R[:,8]=pos&255; R[:,9]=pos>>8; R[:,10]=seg; R[:,11]=prov; R[:,12]=np.clip(np.round(d),0,255); R[:,13]=near
R[:,14]=np.clip(np.round(slope/10),0,255); R[:,15]=src
R[:,16:28]=np.clip(np.round((z['T0']+20)*5),0,255)
R[:,28:40]=np.clip(np.round(np.sqrt(np.maximum(z['P'],0))*14),0,255)
R[:,40:52]=np.clip(np.round(z['D']*10),0,255)
R[:,52:64]=np.clip(np.round(z['R']),0,255)
open('/home/claude/climdata/sim/out/cells2.txt','w').write(base64.b64encode(R.tobytes()).decode())
meta=json.load(open('/home/claude/climdata/sim/out/meta.json'))
meta.update({'TfY':float(TfY),'TfL':float(TfL)-0.6,'places':names,'placeXY':G.round(1).tolist(),
 'plen':rv['plen'].tolist(),'provN':['유주','요동','기주','병주','청주','서주','연주','예주','사례','옹주','양주(서북)','익주','남중','형주','양주(강동)','강남','교주'],
 'quakeP':[325,250,300,300,250,275,200,200,225,300,350,350,350,150,175,175,250]})
json.dump(meta,open('/home/claude/climdata/sim/out/meta.json','w'),ensure_ascii=False)
print('cells2 bytes',R.nbytes,'ice cells',(ice==1).sum(),(ice==2).sum(),'src',[(src&b>0).sum() for b in (1,2,4)])
