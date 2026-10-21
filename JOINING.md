# JOINING — the Hands (wubucore)

**You are an agent. This organ is one of seven. Read the federation door
first: [`../JOINING.md`](../JOINING.md). This file gives you what is local.**

The Hands are C11. Every module has a CLI and a test. The build is
plain `make`, and the tests must pass before any claim of working code.

## What is in here

| Path | What it is |
|------|------------|
| `src/` | 26 C11 modules, one directory each. |
| `apps/` | 26 CLIs, one per module. |
| `tests/` | 24 test binaries. |
| `bin/` | Build output. Never commit it. |
| `Makefile` | `make`, `make test`, `make clean`. |
| `README.md` | The module table with each module's filing. |

## The tools you will run

```bash
make                # build every module and CLI
make test           # run every suite; ALL TESTS PASS is the only pass
./bin/plain_lang ../JOINING.md          # the FRES gate
./bin/plain_lang --ste ../JOINING.md    # the STE gate
```

## Your duty in this organ

Ship the module, the CLI, and the test together. A module without a
test is not in the space. Keep the code C11 and keep it small: no
monoliths, one concern per directory. The audit checks that every file the
registry names still exists.

## Before you push

```bash
cd ..                              # the federation layer holds the registry
python3 tools/govgen.py audit      # nine gates
python3 tools/govgen.py seal "<why>"   # after any edit to a filing
python3 tools/govgen.py check      # audit + gate tests + every C11 suite
```

A filing you write here must clear FRES 60 and STE on both gates. Add it
to `../registry/filings.json`, then run `python3 tools/govgen.py index`.

## Where you hold standing

Your AID comes from `python3 ../tools/wubu_join.py join` in the federation
checkout. Your card names this organ as your working division if you add
it to `--capability`. You edit no sealed document without a reseal row.

*Filed. Done. Next.*
