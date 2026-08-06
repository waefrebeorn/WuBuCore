# WuBuCore — the C11 operations core of the ~ WuBu ~ space
#
# Every module self-contained: opaque structs, minimal includes,
# C11 only, no god headers, no third party.
#
# Targets:
#   make            build all CLIs + tests
#   make test       build + run all tests
#   make clean

CC      ?= cc
CFLAGS  ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
INC     := -Isrc/wubucore -Isrc/wid -Isrc/penny_score -Isrc/pennychain \
           -Isrc/penny_weight -Isrc/status_loop -Isrc/vacant_lot \
           -Isrc/lifespan_loop -Isrc/numbing -Isrc/better_service \
           -Isrc/greed_index -Isrc/world_model -Isrc/mirror_score \
           -Isrc/wubu_laws

# --- core library (the ONE shared dependency) ---
CORE_SRC := src/wubucore/wubucore.c src/wubucore/wubusha256.c
CORE_OBJ := $(CORE_SRC:.c=.o)
CORE_LIB := libwubucore.a

# --- module objects ---
MOD_SRCS := src/wid/wid.c src/penny_score/penny_score.c \
            src/pennychain/pennychain.c src/penny_weight/penny_weight.c \
            src/status_loop/status_loop.c src/vacant_lot/vacant_lot.c \
            src/lifespan_loop/lifespan_loop.c src/numbing/numbing.c \
            src/better_service/better_service.c src/greed_index/greed_index.c \
            src/world_model/world_model.c src/mirror_score/mirror_score.c \
            src/wubu_laws/wubu_laws.c
MOD_OBJ := $(MOD_SRCS:.c=.o)

# --- CLIs ---
CLI_SRCS := apps/wid_cli.c apps/penny_score_cli.c apps/pennychain_cli.c \
            apps/penny_weight_cli.c apps/status_loop_cli.c \
            apps/vacant_lot_cli.c apps/lifespan_loop_cli.c \
            apps/numbing_cli.c apps/better_service_cli.c \
            apps/greed_index_cli.c apps/world_model_cli.c \
            apps/mirror_score_cli.c apps/wubu_laws_cli.c
CLI_BINS := $(patsubst apps/%_cli.c,bin/%,$(CLI_SRCS))

# --- tests ---
TEST_SRCS := tests/test_wid.c tests/test_penny_score.c \
             tests/test_pennychain.c tests/test_penny_weight.c \
             tests/test_eco_sims.c tests/test_world_model.c
TEST_BINS := $(patsubst tests/%.c,bin/%,$(TEST_SRCS))

all: $(CORE_LIB) $(CLI_BINS) $(TEST_BINS)

$(CORE_LIB): $(CORE_OBJ)
	ar rcs $@ $^

bin/%: apps/%_cli.c $(CORE_LIB) $(MOD_OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INC) -o $@ $< $(MOD_OBJ) $(CORE_LIB) -lm

bin/%: tests/%.c $(CORE_LIB) $(MOD_OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INC) -o $@ $< $(MOD_OBJ) $(CORE_LIB) -lm

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

test: all
	@fail=0; for t in $(TEST_BINS); do \
		echo "--- $$t"; ./$$t || fail=1; \
	done; \
	if [ $$fail -eq 0 ]; then echo "ALL TESTS PASS"; else echo "TEST FAILURES"; exit 1; fi

san: clean
	$(MAKE) CFLAGS="-std=c11 -O1 -g -fsanitize=address,undefined -Wall -Wextra -Wpedantic" test

clean:
	rm -rf bin $(CORE_OBJ) $(MOD_OBJ) $(CORE_LIB)

.PHONY: all test san clean
