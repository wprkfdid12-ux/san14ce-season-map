import numpy as np, struct
F = np.float32
def table(seed=7):
    rng = np.random.default_rng(seed)
    B = np.full((200, 4), 0.45, F); A = np.full((200, 4), 0.3, F); Q = np.zeros((200, 4), F)
    K1 = rng.uniform(0.7, 3.1, (200, 4)).astype(F); K2 = rng.uniform(0.7, 3.1, (200, 4)).astype(F); K3 = rng.uniform(0.1, 0.9, (200, 4)).astype(F)
    P = rng.uniform(0, 6.28, (200, 4)).astype(F)
    def put(s, b, a, q=None):
        B[s] = b; A[s] = a
        if q is not None: Q[s] = q
    put(3, (9.0, 9.0, 0.5, 1.0), (8.99, 8.99, 0.5, 0.0), (255, 255, 0, 0))          # LandControl: material ids / 255, blend
    put(110, (900, 0, 0, 0), (700, 0, 0, 0))                                         # height
    put(109, (10, 0, 0.5, -5), (15, 2, 0.4, 10))                                     # climate: temp, trend, dryness, coldest
    put(113, (0.5,) * 4, (0.5,) * 4); put(108, (0.4,) * 4, (0.45,) * 4)
    put(107, (0.5, 0.5, 0.5, 0.5), (0.5, 0.4, 0.5, 0.5))
    put(103, (10, 0.5, 0.5, 0.5), (9.5, 0.5, 0.5, 0.5), (255, 0, 0, 0))
    put(119, (0.5, -0.2, 0.5, 0.5), (0.5, 0.3, 0.5, 0.5))
    put(118, (0.5,) * 4, (0.5,) * 4)
    return dict(B=B, A=A, K1=K1, K2=K2, K3=K3, P=P, Q=Q)
def write(t, path):
    with open(path, 'wb') as f:
        for k in ('B', 'A', 'K1', 'K2', 'K3', 'P', 'Q'): f.write(t[k].astype('<f4').tobytes())
def texfn(t, slot, x, y, z):
    s = slot % 200; o = np.zeros(4, F)
    for i in range(4):
        a = F(F(F(t['K1'][s, i] * F(x)) + F(t['K2'][s, i] * F(y))) + F(t['K3'][s, i] * F(z))) + t['P'][s, i]
        v = F(t['B'][s, i] + t['A'][s, i] * F(np.sin(np.float32(a))))
        if t['Q'][s, i] > 0: v = F(np.floor(v) / t['Q'][s, i])
        o[i] = v
    return o
