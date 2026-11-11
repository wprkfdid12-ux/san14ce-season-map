
const fs=require('fs');
const W=220,N=W*W,HM=2049,HS=6966.2/65535/100;
const cx=c=>375.94+7.02*(c%W);
const cy=c=>{const col=c%W,row=(c/W)|0;return 133.353+8.106*row+(col%2===0?4.053:0);};
function rd(f){const b=fs.readFileSync(f);return new Float64Array(b.buffer,b.byteOffset,b.length/8);}
const base=rd(process.argv[2]), heightsO=rd('ho.f64');
let heightsS=base, heights=null;
const edits=new Map(JSON.parse(fs.readFileSync(process.argv[4]||'edits.json')));
const gl={}; function draw(){} function setStatus(){} function loadBin(){throw 0}
const SLOPE = { 1: .12, 2: .12, 3: .12, 4: .10, 5: .15, 6: .15, 7: .10, 8: .30, 11: .30, 9: .50, 10: .80, 12: .10, 13: .10, 14: .10, 24: .15 };
let heightsW = Float64Array.from(base), loadingO = false; const shaped = new Map();          // 지금 산세에 반영된 칸 -> 지형
function near2(px, py) {                                   // 가장 가까운 칸, 경계 너머 칸, 경계까지 거리
  const col0 = Math.round((px - 375.94) / 7.02); let c1 = -1, d1 = 1e9, c2 = -1, d2 = 1e9;
  for (let col = col0 - 1; col <= col0 + 1; col++) { if (col < 0 || col >= W) continue; const off = col % 2 === 0 ? 4.053 : 0, row0 = Math.round((py - 133.353 - off) / 8.106);
    for (let row = row0 - 1; row <= row0 + 1; row++) { if (row < 0 || row >= W) continue; const c = row * W + col, d = (cx(c) - px) ** 2 + (cy(c) - py) ** 2;
      if (d < d1) { c2 = c1; d2 = d1; c1 = c; d1 = d; } else if (d < d2) { c2 = c; d2 = d; } } }
  return [c1, c2, (d2 - d1) / (2 * 8.106)];
}
function nbrs(c) { const col = c % W, row = (c / W) | 0, out = []; const add = (x, y) => { if (x >= 0 && x < W && y >= 0 && y < W) out.push(y * W + x); };
  add(col, row - 1); add(col, row + 1); if (col % 2 === 0) { add(col - 1, row); add(col + 1, row); add(col - 1, row + 1); add(col + 1, row + 1); } else { add(col - 1, row - 1); add(col + 1, row - 1); add(col - 1, row); add(col + 1, row); } return out; }
function reshape() {
  if (!heightsS || !heightsW) return;
  const want = new Map(); for (const [c, v] of edits) if (v >= 100 || SLOPE[v]) want.set(c, v);
  if (!heightsO && [...want.values()].some(v => v >= 100)) { if (!loadingO) { loadingO = true; setStatus('순정 산세를 불러오는 중…'); loadBin('h_orig.png').then(a => { heightsO = a; loadingO = false; setStatus('순정 산세 준비됨'); reshape(); }).catch(() => { loadingO = false; setStatus('순정 산세를 불러오지 못했습니다', true); }); } return; }
  const touched = []; for (const [c, v] of want) if (shaped.get(c) !== v) touched.push(c); for (const c of shaped.keys()) if (!want.has(c)) touched.push(c);
  if (!touched.length) return;
  const done = new Set();
  for (const c0 of touched) {
    if (done.has(c0)) continue;
    // 붙어 있는 고친 칸 무리 전체를 한 번에 (경계가 어긋나지 않게)
    // (고친 칸끼리 4칸 안쪽이면 다듬는 범위가 겹치므로 같은 무리로 함께 계산)
    const pool = [...new Set([...want.keys(), ...shaped.keys()])];
    const grp = new Set([c0]), q = [c0];
    while (q.length) { const a = q.pop(); for (const b of pool) if (!grp.has(b) && (cx(a) - cx(b)) ** 2 + (cy(a) - cy(b)) ** 2 < 30 * 30) { grp.add(b); q.push(b); } }
    grp.forEach(c => done.add(c));
    let x0 = 1e9, y0 = 1e9, x1 = -1e9, y1 = -1e9; for (const c of grp) { x0 = Math.min(x0, cx(c)); x1 = Math.max(x1, cx(c)); y0 = Math.min(y0, cy(c)); y1 = Math.max(y1, cy(c)); }
    x0 = Math.max(0, Math.floor(x0 - 9)); y0 = Math.max(0, Math.floor(y0 - 9)); x1 = Math.min(HM - 1, Math.ceil(x1 + 9)); y1 = Math.min(HM - 1, Math.ceil(y1 + 9));
    const w = x1 - x0 + 1, h = y1 - y0 + 1, n = w * h;
    const H = new Float32Array(n), M = new Float32Array(n), G = new Float32Array(n), MO = new Float32Array(n);
    for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) {
      const k = j * w + i, px = x0 + i, py = y0 + j; H[k] = heightsS[py * HM + px];
      const [c1, c2, e] = near2(px, py);
      const set = (cc, wt) => { const v = want.get(cc); if (v >= 100) MO[k] = wt; else { M[k] = wt; G[k] = SLOPE[v] / HS; } };
      if (want.has(c1)) set(c1, 1); else if (c2 >= 0 && want.has(c2) && e < 2.2) set(c2, 1 - e / 2.2);
    }
    // 순정 산세 칸: 순정 높이를 넣되, 둘레와의 높이 차이는 매끈하게 이어 붙임 (막 펴기)
    if (heightsO && MO.some(v => v > 0)) {
      const Dd = new Float32Array(n); let sum = 0, cnt = 0;
      for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) { const k = j * w + i; Dd[k] = H[k] - heightsO[(y0 + j) * HM + x0 + i]; if (MO[k] === 0) { sum += Dd[k]; cnt++; } }
      const mean = cnt ? sum / cnt : 0; for (let k = 0; k < n; k++) if (MO[k] > 0) Dd[k] = mean;
      for (let it = 0; it < 400; it++) for (let j = 1; j < h - 1; j++) for (let i = 1; i < w - 1; i++) { const k = j * w + i; if (MO[k] === 0) continue;
        Dd[k] += 1.85 * ((Dd[k - 1] + Dd[k + 1] + Dd[k - w] + Dd[k + w]) * 0.25 - Dd[k]); }
      for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) { const k = j * w + i; if (MO[k] > 0) H[k] = H[k] * (1 - MO[k]) + (heightsO[(y0 + j) * HM + x0 + i] + Dd[k]) * MO[k]; }
    }
    if (M.some(v => v > 0)) {
      const D = new Float32Array(n), pairs = [[1, 0, 1], [0, 1, 1], [1, 1, Math.SQRT2], [-1, 1, Math.SQRT2]];
      for (let it = 0; it < 260; it++) {
        D.fill(0); let moved = 0;
        for (let j = 1; j < h - 1; j++) for (let i = 1; i < w - 1; i++) { const a = j * w + i;
          for (const [dx, dy, len] of pairs) { const b = a + dy * w + dx, ma = M[a], mb = M[b]; if (ma === 0 && mb === 0) continue;
            const g = (ma > 0 && mb > 0 ? Math.min(G[a], G[b]) : (ma > 0 ? G[a] : G[b])) * len, d = H[a] - H[b], ex = Math.abs(d) - g; if (ex <= 0) continue;
            const m = ex * 0.22, s = d > 0 ? 1 : -1; D[a] -= s * m * ma; D[b] += s * m * mb; moved += m; } }
        for (let k = 0; k < n; k++) H[k] += D[k];
        if (moved < 1) break;
      }
    }
    for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) heightsW[(y0 + j) * HM + x0 + i] = H[j * w + i];
    if (heights === heightsW) { const f = new Float32Array(n); for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) f[j * w + i] = heightsW[(y0 + j) * HM + x0 + i] * HS;
      gl.bindTexture(gl.TEXTURE_2D, hTex); gl.texSubImage2D(gl.TEXTURE_2D, 0, x0, y0, w, h, gl.RED, gl.FLOAT, f); }
  }
  shaped.clear(); for (const [c, v] of want) shaped.set(c, v);
  draw();
}

reshape();
fs.writeFileSync(process.argv[3], Buffer.from(heightsW.buffer));
let n=0,mx=0; for(let i=0;i<base.length;i++){const d=Math.abs(heightsW[i]-base[i]); if(d>0.5)n++; if(d>mx)mx=d;}
console.log('changed px',n,'max',mx.toFixed(1),'shaped',shaped.size);
