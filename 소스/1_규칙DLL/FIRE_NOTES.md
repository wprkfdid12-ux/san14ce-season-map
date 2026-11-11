# 화계 / 불 칸 — game code locations (found 2026-10-07 from saves 10/11 + EXE)

- Message log in save confirmed: "제갈량부대, 화계 성공 후 대기" (log entry type 0x12, cell 0x3d23 = 15651).
- RVA 0x267630  int FireRate(officer*, hex*, int showMsg)  -> 0..100 (100 if officer has skill 0x19).
  - prologue: 40 56 41 54 41 57 | 48 81 ec b0 02 00 00  (steal 6 bytes, same trampoline style as install_speed)
  - callers: 0x2e333a (real execution: success if rand(1..100) <= rate), 0x7cfe4 (AI node GetFireStrategySuccess), 0x7c5c94 (UI %, clamps 0..100)
  - cell of target: (hex - *(u8**)(db+0xe270)) >> 5   (game fn 0x22d2c0)
- RVA 0x2e3230  Execute 화계(army*, hex*) ; on success calls 0x2e2cf0(hex, duration, 1)
- RVA 0x2e2cf0  StartFire(hex*, duration, flag): skips if hex+0x16 != 0, terrain must allow fire
- RVA 0x2ed050  SetHexFire(hex*, value): hex+0x16 = clamp(value,0,720); >0 adds fire visual, 0 removes it (used by 끄기 0x415ca8)
- Burning state = hex cell u16 at +0x16 (remaining fire). Base duration const 36 (0x141a48cb8).
- Fire spread copies the timer to the neighbour (0x2d975a).

Plan:
- hook FireRate: rain tier at target cell -> light x0.8, rain x0.5, heavy -> 0 (only when rate > 0).
- daily: every hex with +0x16 > 0 under rain -> SetHexFire(hex, v*0.75 / v*0.5 / 0).
