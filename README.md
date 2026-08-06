# WuBuCore

**The C11 operations core of the ~ WuBu ~ space.**

Every instrument the space runs on — citizenship numbers, the Penny Score,
the PennyChain ledger, the penny-weight reserve audit, and the full
economics/instrument suite — as one self-contained C11 codebase. No third
party. No external dependencies beyond libc. We are the first ones; we
properly make our own code.

## Why C11

The tools were prototyped in Python and verified against real outputs. This
repo is the porting: the same math, the same verified numbers, in C11 with
the discipline the space demands.

## The discipline (soul.md §10)

- **Opaque structs** — `typedef struct wid wid;` in the header, definition in
  the `.c`. Callers never touch layout.
- **Minimal includes / IWYU** — C11 only: `-std=c11 -Wall -Wextra -Wpedantic`.
- **No god headers** — every module is self-contained in its own directory.
- **Reuse, never duplicate** — one shared core (`src/wubucore`), everything
  else depends on it and nothing else.

## Modules

| Module | Dir | What it is | RND/ECO ref |
|---|---|---|---|
| WID | `src/wid` | WuBu IDentification Number, mod-11 self-verifying | citizenship |
| Penny Score | `src/penny_score` | 0-1000, v2 weights (20/30/20/20/10) | RND-1 |
| PennyChain | `src/pennychain` | keyless SHA-256 Merkle forest ledger | RND-3/4 |
| Penny Weight | `src/penny_weight` | class-counted reserve audit by mass | RND-6 |
| Status Loop | `src/status_loop` | scarcity/abundance/feedback worlds | ECO-3 |
| Vacant Lot | `src/vacant_lot` | unearned capture + LVT | ECO-4 |
| Lifespan Loop | `src/lifespan_loop` | generations alive per region | ECO-5 |
| Numbing | `src/numbing` | chronic stress downregulates reward | ECO-6 |
| Better Service | `src/better_service` | Newell: enforce vs serve | ECO-7 |
| Greed Index | `src/greed_index` | Capture+Nexus+Malice, published | ECO-8 |
| World Model | `src/world_model` | mean/median/skill-weighted/hybrid | RND-17 |
| Mirror Score | `src/mirror_score` | entropy audit, 0=tool 100=mirror | GOV-3/RND-18 |
| WuBu Laws | `src/wubu_laws` | the five laws of tools, compliance | RND-19 |
| Plain Lang | `src/plain_lang` | the Language Doctrine checker: Flesch-Kincaid score + banned words + long-sentence flags | RND-20 |

`src/wubucore` holds the one shared dependency: `wubu_clamp`, `wubu_sigmoid`,
`wubu_tanh`, `wubu_fnv1a`, `wubu_rand64`, and a clean-room SHA-256.

## Build

```sh
make          # build all CLIs + tests (0 warnings)
make test     # run the full suite
make san      # ASan+UBSan gate (0 leaks, 0 UB)
make clean
```

## CLIs

```sh
bin/wid generate WB 26 0001        # -> WID-WB-26-0001-8
bin/wid validate WID-WB-26-0001-8  # -> VALID
bin/penny_score --literacy 1.0 --punctual 12 --late 1 --missed 0 \
    --vigilance 3 --creation 5 --years 2    # -> 764/1000 Crew
bin/pennychain mint rows.csv       # Merkle-mint the ledger
bin/pennychain verify rows.csv P-0001
bin/penny_weight audit --copper 3110 --zinc 25000
bin/status_loop feedback --years 30
bin/vacant_lot capture --bought 35000 --sold 60000 --years 15
bin/vacant_lot generations --price-then 40000 --income-then 16000 \
    --price-now 400000 --income-now 56000
bin/lifespan_loop 84 25
bin/lifespan_loop timeline
bin/numbing nexus --gens 5
bin/better_service serve --years 25
bin/greed_index --capture 80 --nexus 70 --malice 60
bin/world_model --human "0.55 0.60 0.70 0.45 0.80" \
    --skills "3 5 2 4 6" --silicon "0.52 0.58 0.65 0.50 0.75"
bin/mirror_score --agree 8 --disagree 0 --novel 0 --confirm 9 --comfort 7
bin/wubu_laws --no-harm 1 --filing-over-comfort 1 --can-say-no 1 \
    --explicable 1 --no-companion 1
bin/plain_lang README.md        # Language Doctrine check (RND-20)
```

## Verification

Every test asserts numbers verified against the original Python tools
(the space's independent oracle): WID check digits, the 764 Crew score,
the 11000-penny/$110 reserve, the 0.09 nexus sensitivity, the 95.5% serve
adoption, the 0.703 recalibrated world model, and the SHA-256 known vector
`sha256("abc") = ba7816bf…`. The sanitizer gate runs the same suite.

## License

WaefreBeorn Umbrella License v3.0 — see [LICENSE](LICENSE).
