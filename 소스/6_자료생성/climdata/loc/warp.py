import numpy as np, json, glob
from scipy.interpolate import RBFInterpolator
A=[ # game map px (x,y), real lon, lat, label
 ((851,704),108.94,34.26,'장안'),((1068,732),112.45,34.62,'낙양'),((711,898),107.02,33.07,'한중'),((1124,417),112.55,37.87,'진양'),
 ((452,1298),104.06,30.66,'성도'),((704,1435),106.55,29.56,'강주'),((1782,231),123.17,41.27,'양평'),((417,372),102.64,37.93,'무위'),
 ((985,655),110.25,34.60,'동관(황하 꺾임)'),((975,300),111.20,39.60,'하투 동북 모서리'),((1795,1200),120.20,31.20,'태호'),
 ((1817,1447),120.58,30.00,'회계'),((1341,1811),113.26,23.13,'남해'),((1656,1654),118.32,27.03,'건안'),((1250,530),114.60,36.30,'업'),
 ((886,1015),110.23,32.23,'상용'),((529,671),105.72,34.58,'천수'),((1848,470),122.60,37.40,'산동반도 끝'),((1724,358),121.25,38.80,'요동반도 끝')]
G=np.array([a[0] for a in A],float); L=np.array([[a[1],a[2]] for a in A],float)
f=RBFInterpolator(L,G,kernel='thin_plate_spline',smoothing=2.0)
def to_game(lon,lat): return f(np.array([[lon,lat]]))[0]
if __name__=='__main__':
    res=f(L)-G; print('anchor residual px',np.round(np.hypot(res[:,0],res[:,1]),1))
    cities=[]
    for fn in sorted(glob.glob('/home/claude/climdata/raw/cities_*.json')):
        for c in json.load(open(fn)):
            if c.get('tmean') and c.get('lat'): cities.append(c)
    P=f(np.array([[c['lon'],c['lat']] for c in cities]))
    json.dump([{'city':c['city'],'gx':float(p[0]),'gy':float(p[1])} for c,p in zip(cities,P)],open('city_game_pos.json','w'),ensure_ascii=False)
    print(len(cities))
