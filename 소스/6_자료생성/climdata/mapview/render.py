import numpy as np, sys
from PIL import Image, ImageDraw, ImageFont
from feats import *
base=Image.fromarray(np.load('base.npy')).convert('RGBA')
ov=Image.new('RGBA',base.size,(0,0,0,0)); d=ImageDraw.Draw(ov)
R=4.67
hexo=[(R*np.cos(a),R*np.sin(a)) for a in np.arange(6)*np.pi/3]
COL={'low':(255,40,40),'high':(255,150,0),'plat':(255,235,0),'loess':(190,80,255),'forest':(150,90,30),'good':(60,230,90)}
MT=[9,10,11,19,22]
SEL={'low':MT,'high':IMP,'plat':PLAIN,'loess':IMP,'forest':FOREST,'good':MT}
for k,l,b,kind,ty in F:
    c=cells_k(k,b,SEL[kind]); col=COL[kind]; a=110 if kind!='good' else 70
    for i in c:
        pts=[(X[i]+px,Y[i]+py) for px,py in hexo]
        d.polygon(pts,fill=col+(a,),outline=col+(230,))
img=Image.alpha_composite(base,ov).convert('RGB')
dd=ImageDraw.Draw(img)
f=ImageFont.truetype('/usr/share/fonts/opentype/noto/NotoSansCJK-Black.ttc',26,index=1)
NUM={}
for n,(k,l,b,kind,ty) in enumerate(F):
    x0,x1,y0,y1=b; cx,cy=((x0+x1)/2,(y0+y1)/2) if k not in ELL else ELL[k][:2]
    NUM[k]=n+1
    txt='%d %s'%(n+1,l)
    dd.text((cx,cy),txt,fill=(255,255,255),font=f,anchor='mm',stroke_width=4,stroke_fill=(0,0,0))
img.save('marked_full.png')
# legend
def legend(im,scale=1.0):
    L=[('low','진입 가능 칸인데 실제는 험준한 명산 → 실제보다 낮춰야'),('high','진입 불가 칸인데 실제는 낮은 산 → 실제보다 부풀려야'),
       ('plat','고원인데 평지 칸 → 높은 땅 느낌 안 남'),('loess','황토고원 진입 불가 칸 → 탁자 모양으로 타협'),('forest','실제는 반건조 초원인데 숲 칸 → 산불 잦을 수 있음'),('good','잘 맞는 곳 → 실제 지리 거의 그대로')]
    fs=int(24*scale); fl=ImageFont.truetype('/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc',fs,index=1)
    h=int((len(L)*(fs+12)+20)); w=int(fs*28)
    box=Image.new('RGBA',(w,h),(20,20,24,255)); bd=ImageDraw.Draw(box)
    for i,(k,t) in enumerate(L):
        y=10+i*(fs+12); bd.rectangle([10,y+3,10+fs,y+fs+3],fill=COL[k]+(255,)); bd.text((20+fs,y),t,fill=(255,255,255),font=fl)
    out=Image.new('RGB',(im.size[0],im.size[1]+h),(20,20,24)); out.paste(im,(0,0)); out.paste(box.convert('RGB'),(10,im.size[1])); return out
full=img.crop((355,120,1930,1930))
legend(full.resize((1100,int(1100*1810/1575))),0.95).save('map_overview.png',quality=92)
Z={'A_북동(태항산·태산·연산)':(1080,150,1720,720),'B_서북(황토고원·난주 숲·진령·복우산)':(355,250,1220,980),
   'C_사천(사천 동부·삼협·촉도)':(430,950,1080,1480),'D_남쪽(남령·운귀고원·무이산)':(355,1450,1720,1930),'E_동남(여산·천목산·대별산)':(1040,1030,1800,1560)}
for name,(x0,y0,x1,y1) in Z.items():
    z=img.crop((x0,y0,x1,y1)); s=1100/z.size[0]; z=z.resize((1100,int(z.size[1]*s)))
    legend(z,0.8).save('zoom_%s.png'%name.split('_')[0],quality=92)
print('ok')
