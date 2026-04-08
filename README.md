# flare-c

C SDK for building **flares**: Wasm functions that run on the
[Flaron](https://flaron.dev) CDN edge. A flare receives an HTTP request (or
WebSocket event) at the nearest edge, runs your C code in a sandboxed
WebAssembly runtime, and returns a response with single-digit-millisecond
latency.

This SDK is the C-language counterpart to `flare-rs`, `flare-go`,
`flare-as`, `flare-zig`, and `flare-cpp`. It produces small, freestanding
`.wasm` modules that link only against Flaron's `flaron/v1` host module:
no libc, no WASI, no runtime dependencies.

## What you get

| Module           | What it does                                            |
|------------------|---------------------------------------------------------|
| `flare/request`  | Read the inbound request (method, URL, headers, body)  |
| `flare/response` | Build the outbound response (status, headers, body)    |
| `flare/spark`    | Per-site KV with TTL, persisted to disk on the edge    |
| `flare/plasma`   | Cross-edge CRDT KV: counters, presence, leaderboards  |
| `flare/beam`     | Outbound HTTP fetch from the edge                      |
| `flare/ws`       | WebSocket: send, close, read events                    |
| `flare/edgeops`  | Crypto (hash, HMAC, JWT, AES, RNG), encoding, IDs, time |
| `flare/log`      | Structured logs to the edge node's slog stream         |

## Prerequisites

You need a clang/LLVM toolchain that supports the `wasm32` target plus
`wasm-ld`. Apple's bundled `/usr/bin/clang` does **not** ship the wasm
backend on every release, so the easiest path on macOS is Homebrew:

```sh
brew install llvm@20 lld@20
```

The Makefile auto-detects `/opt/homebrew/opt/llvm@20/bin/clang` and
`/opt/homebrew/opt/lld@20/bin/wasm-ld`. On Linux, install your distro's
LLVM (>= 17) and lld; the Makefile falls back to whatever is on `PATH`.

For testing on the host machine, any C11-capable compiler works
(`clang`, `gcc`).

## Quick start: HTTP echo

```c
#include <flare.h>

static flare_action_t handle_request(void) {
    flare_resp_set_status(200);
    flare_resp_set_header("content-type", 12, "text/plain", 10);
    flare_resp_set_body((const uint8_t *)"hello from flare-c", 18);
    return FLARE_ACTION_RESPOND;
}

FLARE_EXPORT_HANDLE_REQUEST(handle_request)
```

Compile it (assuming you've checked out flare-c at `flare-c/`):

```sh
clang --target=wasm32 -nostdlib -ffreestanding -O2 \
      -Iflare-c/include -c my-flare.c -o my-flare.o
wasm-ld --no-entry --import-undefined --allow-undefined \
        --export-dynamic my-flare.o flare-c/build/libflare.a \
        -o my-flare.wasm
```

Or use the Makefile: drop your `main.c` into `examples/<name>/` and
`make examples` will build it.

Deploy `my-flare.wasm` via the Flaron dashboard or `flaronctl`.

## Building this repository

```sh
make build      # produces build/libflare.a (wasm32 static archive)
make test       # runs the native test suite (with mock host)
make examples   # builds every example flare to .wasm
make clean      # removes build/
```

The native test build uses `clang` and a mock host that implements every
`flaron/v1` extern with in-memory state, so you can drive the SDK in
process and assert against the recorded host calls. See `tests/mock_host.h`
for the fixture API.

## Memory model

Each flare invocation gets a fresh **256 KiB bump arena**. Every host
function call that returns data writes into the arena via the guest's
`alloc` export, and the SDK resets the arena at the top of every
invocation. You never call `free`; when the handler returns, the arena
is reclaimed.

The `FLARE_EXPORT_HANDLE_REQUEST` macro takes care of the reset for you.
Only call `flare_reset_arena()` directly if you wire up your own export
functions.

## Examples

| Path                       | What it shows                                |
|----------------------------|----------------------------------------------|
| `examples/hello`           | Smallest possible HTTP echo                  |
| `examples/spark-counter`   | Per-edge persistent counter                  |
| `examples/plasma-counter`  | Global cross-edge counter                    |
| `examples/secret-jwt`      | Issue HS256 JWT signed with a domain secret  |
| `examples/websocket-echo`  | WebSocket open/message/close lifecycle       |
| `examples/beam-fetch`      | Outbound HTTP from the edge                  |
| `examples/edge-ops`        | Hash, UUID v7, and edge timestamp            |

Each example has its own `README.md` describing required capabilities and
deployment notes.

## Conventions

- Every public symbol is prefixed `flare_`, so there is no global namespace pollution.
- Errors are returned as `flare_status_t` enum values; out-parameters
  receive results via pointers.
- The SDK never allocates with `malloc`. All temporary buffers come from
  the per-invocation bump arena.
- Headers are in `include/flare/`; `<flare.h>` is the umbrella header.
- All public APIs are `const`-correct; you never need to cast away
  constness to use the SDK.

## Status codes

`flare_status_t` distinguishes Spark/Plasma/WebSocket errors from generic
ones. Use `flare_status_str(status)` for a human-readable message.

| Range          | Meaning                              |
|----------------|--------------------------------------|
| `0`            | `FLARE_OK`                           |
| `1` to `6`     | Generic errors (not found, host…)    |
| `101` to `109` | Spark errors                         |
| `201` to `206` | Plasma errors                        |
| `301` to `302` | WebSocket errors                     |

## Documentation

- [flaron.dev](https://flaron.dev): main docs site
- [Building Flares guide](https://flaron.dev/docs/building-flares)
- The Rust SDK at [github.com/flaron-cdn/flare-rs](https://github.com/flaron-cdn/flare-rs)
  is the canonical reference for the host ABI.

## License

MIT. See `LICENSE`.
