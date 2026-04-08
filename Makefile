# Flaron C SDK build system
#
# Targets:
#   make build     -- compile the SDK to a wasm32 static archive
#   make test      -- compile and run the native test suite (with mock host)
#   make examples  -- build all example flares to wasm
#   make clean     -- remove all build artifacts
#
# Tooling: clang + wasm-ld from LLVM 20+. The Makefile auto-detects Homebrew
# locations on macOS; override CLANG, AR, WASMLD on the command line if you
# install LLVM somewhere else.

# ---------- toolchain ----------
#
# Apple's /usr/bin/clang has the WebAssembly backend disabled in some
# distributions, so we prefer Homebrew's llvm@20 (or any LLVM with wasm32
# enabled) for the wasm build. Native test builds tolerate either.

WASM_CLANG ?= $(shell \
  if [ -x /opt/homebrew/opt/llvm@20/bin/clang ]; then echo /opt/homebrew/opt/llvm@20/bin/clang; \
  elif [ -x /usr/local/opt/llvm@20/bin/clang ]; then echo /usr/local/opt/llvm@20/bin/clang; \
  elif command -v clang-20 >/dev/null 2>&1; then command -v clang-20; \
  else echo clang; fi)

CLANG    ?= $(shell command -v clang || echo /opt/homebrew/opt/llvm@20/bin/clang)
LLVM_AR  := $(shell \
  if [ -x /opt/homebrew/opt/llvm@20/bin/llvm-ar ]; then echo /opt/homebrew/opt/llvm@20/bin/llvm-ar; \
  else command -v llvm-ar || echo llvm-ar; fi)
WASMLD   := $(shell \
  if [ -x /opt/homebrew/opt/lld@20/bin/wasm-ld ]; then echo /opt/homebrew/opt/lld@20/bin/wasm-ld; \
  else command -v wasm-ld || echo wasm-ld; fi)

# ---------- common ----------

INCLUDE   := include
SRC_DIR   := src
TEST_DIR  := tests
BUILD_DIR := build
EX_DIR    := examples

SRCS := $(wildcard $(SRC_DIR)/*.c)
HDRS := $(wildcard $(INCLUDE)/flare/*.h) $(INCLUDE)/flare.h

WARN := -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes \
        -Wmissing-prototypes -Wundef -Wcast-align -Wpointer-arith \
        -Werror

# ---------- wasm build ----------

WASM_TARGET   := --target=wasm32
WASM_CFLAGS   := $(WASM_TARGET) -nostdlib -ffreestanding -O2 \
                 -fvisibility=hidden -fno-builtin -std=c11 \
                 -I$(INCLUDE) $(WARN)
WASM_LDFLAGS  := --no-entry --import-undefined --allow-undefined \
                 --export-dynamic --strip-all
WASM_OBJ_DIR  := $(BUILD_DIR)/wasm
WASM_OBJS     := $(SRCS:$(SRC_DIR)/%.c=$(WASM_OBJ_DIR)/%.o)
WASM_LIB      := $(BUILD_DIR)/libflare.a

$(WASM_OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HDRS)
	@mkdir -p $(WASM_OBJ_DIR)
	$(WASM_CLANG) $(WASM_CFLAGS) -c $< -o $@

$(WASM_LIB): $(WASM_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(LLVM_AR) rcs $@ $^

build: $(WASM_LIB)

# ---------- native test build ----------

NATIVE_CFLAGS := -O0 -g -std=c11 -I$(INCLUDE) -I$(TEST_DIR) $(WARN) \
                 -fno-strict-aliasing
NATIVE_OBJ_DIR := $(BUILD_DIR)/native
NATIVE_SRC_OBJS  := $(SRCS:$(SRC_DIR)/%.c=$(NATIVE_OBJ_DIR)/%.o)
NATIVE_TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
NATIVE_TEST_OBJS := $(NATIVE_TEST_SRCS:$(TEST_DIR)/%.c=$(NATIVE_OBJ_DIR)/%.o)
NATIVE_BIN     := $(BUILD_DIR)/test_runner

$(NATIVE_OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HDRS)
	@mkdir -p $(NATIVE_OBJ_DIR)
	$(CLANG) $(NATIVE_CFLAGS) -c $< -o $@

$(NATIVE_OBJ_DIR)/%.o: $(TEST_DIR)/%.c $(HDRS) $(TEST_DIR)/test_runner.h $(TEST_DIR)/mock_host.h
	@mkdir -p $(NATIVE_OBJ_DIR)
	$(CLANG) $(NATIVE_CFLAGS) -c $< -o $@

$(NATIVE_BIN): $(NATIVE_SRC_OBJS) $(NATIVE_TEST_OBJS)
	$(CLANG) -o $@ $^

test: $(NATIVE_BIN)
	@$(NATIVE_BIN)

# ---------- examples ----------

EX_NAMES := hello spark-counter plasma-counter secret-jwt websocket-echo beam-fetch edge-ops
EX_WASMS := $(addprefix $(BUILD_DIR)/examples/,$(addsuffix .wasm,$(EX_NAMES)))

$(BUILD_DIR)/examples/%.wasm: $(EX_DIR)/%/main.c $(WASM_LIB)
	@mkdir -p $(BUILD_DIR)/examples
	$(WASM_CLANG) $(WASM_CFLAGS) -o $(BUILD_DIR)/examples/$*.o -c $<
	$(WASMLD) $(WASM_LDFLAGS) $(BUILD_DIR)/examples/$*.o $(WASM_LIB) -o $@

examples: $(EX_WASMS)

# ---------- meta ----------

clean:
	rm -rf $(BUILD_DIR)

.PHONY: build test examples clean
