# clog

A small C logging library for embedded and desktop projects. Copy `clog.c` and `clog.h` into your tree, tune macros in `clog.h`, and call the APIs below.

## Features

- Log levels: `LL_DBG`, `LL_WAR`, `LL_ERR`, `LL_RUN`
- Compile-time stripping via `BUILD_LOG_LEVEL`
- Runtime filtering via `clog_init()` / `clog_get_level()`
- File/line prefix on each line
- Text, hex dump, and mixed (printable + hex) helpers
- Optional extra instances (`target_log`, or your own `clog_inf_t`) and custom output callback (`_clog_custom`)
- Platform hooks: PC `printf`, OpenLuat, Neoway trace, SEGGER RTT
- **Not thread-safe by default** — see [Multi-threading](#multi-threading)

## Quick start (usage flow)

```
1. Include clog.h
2. clog_init(&default_log, min_level)   // allocate buffer; must succeed before macros
3. clog_dbg / clog_war / clog_err / clog_run (and hex/mix variants)
4. clog_deinit(&default_log)            // free buffer (and close log file if loc-save enabled)
```

Until `clog_init()` succeeds, `default_log.clog_buf` is `NULL` and log macros do nothing.

## Demo

Minimal program (same idea as the old `clog_demo.c`):

```c
#include "clog.h"

int main(void)
{
    if (clog_init(&default_log, LL_DBG) != 0) {
        return 1;
    }

    clog_dbg("debug value: %d", 42);
    clog_war("warning");
    clog_err("error code: %d", -1);
    clog_run("always-on style message when RUN is enabled");

    uint8_t buf[] = {0x01, 0x02, 0x41, 0x42};
    clog_hex_dbg(buf, sizeof(buf));
    clog_mix_dbg(buf, sizeof(buf));

    clog_deinit(&default_log);
    return 0;
}
```

Build (example, PC):

```bash
gcc -std=c99 -Wall -o demo your_app.c clog.c
./demo
```

Example output (ANSI colors on terminals that support them):

```
[your_app.c:8] [DBG] debug value: 42
[your_app.c:9] [WAR] warning
[your_app.c:10] [ERR] error code: -1
...
```

## Log levels

| Macro / enum | Value | Typical use |
|--------------|-------|-------------|
| `LL_DBG`     | 0     | Verbose debug |
| `LL_WAR`     | 1     | Warnings (yellow background tag) |
| `LL_ERR`     | 2     | Errors (red background tag) |
| `LL_RUN`     | 3     | Important lifecycle messages |
| `LL_NONE`    | 4     | Disable logging (`clog_get_level` sentinel) |

Runtime rule: a message is printed when `message_level >= log->clog_level` (e.g. `clog_init(&default_log, LL_WAR)` drops `LL_DBG` only).

## Macros (default instance)

All of these use `default_log` and add `[file:line]` via `CLOG_FORMAT`.

| Text | Hex dump | Mixed dump |
|------|----------|------------|
| `clog_dbg(fmt, ...)` | `clog_hex_dbg(data, len)` | `clog_mix_dbg(data, len)` |
| `clog_war(fmt, ...)` | `clog_hex_war(data, len)` | `clog_mix_war(data, len)` |
| `clog_err(fmt, ...)` | `clog_hex_err(data, len)` | `clog_mix_err(data, len)` |
| `clog_run(fmt, ...)` | `clog_hex_run(data, len)` | `clog_mix_run(data, len)` |

Hex/mixed dumps are capped by `CLOG_HEX_MAX_BYTES` (derived from `CLOG_BUF_SIZE`, default 256 bytes total buffer).

## Multi-instance (brief)

| Object | Role |
|--------|------|
| `default_log` | Used by all `clog_*` macros |
| `target_log` | Second global instance; macros do **not** use it automatically |
| Your `clog_inf_t` | Per-module context, e.g. `static clog_inf_t module_log` |

Each `clog_inf_t` has its own `clog_buf` (allocated in `clog_init`) and `clog_level`. Macros always write `default_log`; other instances need `CLOG_FORMAT(&ctx)` then `_clog(&ctx, ...)`.

Instance rules (same as threading on one buffer):

- **Safe:** one instance, one caller at a time, or one mutex per instance.
- **Unsafe:** two threads logging through the same instance without serialization.

See [Multi-threading](#multi-threading) below — instance separation alone does not make clog thread-safe if the backend or `default_log` is still shared.

## Multi-threading

### Current behavior (现状)

clog is a **single-buffer, lock-free** logger:

1. `CLOG_FORMAT(log)` clears `log->clog_buf` and writes prefix (`[datetime]` if enabled, then `[file:line]`).
2. `_clog` / `_clog_hex` / `_clog_mix` / `_clog_custom` **append** level tag and body into the **same** buffer.
3. `clog_process()` sends the full line to `clog_printf` or your custom function (e.g. `usb_log`).

There is **no mutex**, **no per-thread buffer**, and **no re-entrancy guard** inside clog.

| Item | Thread-safe? |
|------|----------------|
| `clog_inf_t` internal state (`clog_buf`, levels) | No — one writer at a time per instance |
| `clog_*` macros (`default_log`) | No — all threads share one buffer |
| `clog_printf` / UART / USB backend | Depends on platform/library; assume **no** |
| `clog_get_datetime()` (`localtime` on PC) | Often **no** on newlib/picolibc unless you use `localtime_r` |

A complete log line is intended to be **atomic from the caller’s view**, but clog only achieves that if **callers serialize** access to each `clog_inf_t` and to the output device.

### Risks (风险)

| # | Scenario | What can go wrong |
|---|----------|-------------------|
| 1 | Task A and Task B call `clog_err()` at the same time | Both use `default_log.clog_buf`: mixed prefix/body, truncated line, or partial `printf` |
| 2 | Task A in `usb_log()` callback, Task B calls `clog_*` | Callback sees overwritten `clog_buf` if B logs before A finishes copying |
| 3 | Nested log inside custom sink on same instance | `CLOG_FORMAT` does `memset` on buffer still in use by outer log |
| 4 | `ENABLE_DATETIME TRUE` + multi-thread | `localtime()` race; wrong or corrupted timestamp |
| 5 | Hex/mix log while another task logs | Same as (1); long hex loop increases race window |
| 6 | Only “separate instances”, shared `printf`/USB | Instances differ but output interleaves on one physical port |

Symptoms in the field: garbled lines, two files merged in one line, missing `[file:line]`, rare hard faults if callback assumes stable buffer lifetime.

### Recommendations (建议)

**1. Global log lock (most common, recommended for RTOS)**

Serialize every path that touches clog: all `clog_*`, `usb_log_*`, and any `CLOG_FORMAT` + `_clog*`.

```c
static osMutexId_t g_clog_mtx;

void clog_lock(void)   { (void)osMutexAcquire(g_clog_mtx, osWaitForever); }
void clog_unlock(void) { (void)osMutexRelease(g_clog_mtx); }

/* Wrap project macros, e.g. in log.h */
#define usb_log_err(fmt, ...) \
    do { \
        clog_lock(); \
        usb_log_err_impl(fmt, ##__VA_ARGS__); \
        clog_unlock(); \
    } while (0)
```

Keep critical section **short**: format in clog, copy to TX queue in callback, release lock before blocking IO if possible.

**2. Per-instance mutex**

Use when `default_log` and `target_log` (or module contexts) must log concurrently to different sinks **and** backends are independent:

- Lock A around all use of `&instance_a`
- Lock B around all use of `&instance_b`
- Still lock shared `printf`/USB if both sink to the same port

**3. Per-task `clog_inf_t` (no shared instance)**

Each RTOS task owns `static clog_inf_t my_log`, `clog_init(&my_log, ...)`, and only that task calls `CLOG_FORMAT(&my_log)` + `_clog`. This avoids buffer races between tasks **only if** no other task uses the same instance. You still need a lock on the **output driver** if all tasks share one UART/USB.

**4. Custom callback (`_clog_custom` / `usb_log`)**

- Treat `line` as valid **only until the callback returns**; copy to your own buffer for async transmit.
- Do **not** call clog again on the **same** instance inside the callback (re-entrancy).
- If you must log from callback, use another `clog_inf_t` or defer to a low-priority log task with the global lock.

**5. `ENABLE_DATETIME` on multi-threaded builds**

- Hold the same global log lock across `clog_get_datetime` + line output, **or**
- Implement `clog_get_datetime()` with RTC hardware or `localtime_r`, **or**
- Disable `ENABLE_DATETIME` on SMP/RTOS unless you add locking.

**6. Bare-metal / single-thread**

No extra work: one main loop or one ISR policy (avoid logging from ISR unless you use a separate buffer/lockless queue).

### Quick decision table

| Your setup | Suggested approach |
|------------|-------------------|
| Single thread / super loop | Use `clog_*` as-is |
| RTOS, many tasks, one USB log | Global mutex around all log macros |
| RTOS, module A / B, different UARTs | Per-instance mutex + per-instance `clog_inf_t` |
| High rate hex dump + text | Global mutex; consider raising `CLOG_BUF_SIZE` or lowering log rate |

### `CLOG_FORMAT` and Keil / ARM Compiler

`CLOG_FORMAT(log)` expands to a full statement ending with `;`. Use either form inside `do { ... } while (0)`:

```c
do { CLOG_FORMAT((&default_log)) _clog_custom(...); } while (0)   /* OK */
do { CLOG_FORMAT((&default_log)); _clog(...); } while (0)        /* OK */
```

## Second log instance (`target_log`)

Macros are wired to `default_log` only. For `target_log` or any extra instance:

```c
clog_init(&target_log, LL_ERR);
CLOG_FORMAT(&target_log);
_clog(&target_log, LL_ERR, "module B: %s", "failed");
clog_deinit(&target_log);
```

## Custom output callback

```c
static void my_sink(const char* line)
{
    /* send line to UART, ring buffer, etc. */
    (void)line;
}

clog_init(&default_log, LL_DBG);
CLOG_FORMAT(&default_log);
_clog_custom(&default_log, (void*)my_sink, LL_DBG, "custom: %d", 1);
```

Project wrappers (e.g. `usb_log_err`) should follow the same pattern: `CLOG_FORMAT` then `_clog_custom`, with a trailing `;` after `CLOG_FORMAT` optional because the macro already includes one.

## Timestamp prefix (`ENABLE_DATETIME`)

Set `#define ENABLE_DATETIME TRUE` in `clog.h`. Each log line then starts with:

```text
[YY/MM/DD HH:MM:SS] [file:line] [LEVEL] ...
```

Each `clog_*` macro expands to `CLOG_FORMAT` then `_clog` (or hex/mix). One line in `log->clog_buf` is built as:

1. `CLOG_FORMAT(log)` → `clog_format_prefix()`: `[YY/MM/DD HH:MM:SS] ` (if `ENABLE_DATETIME`) then `[file:line] `
2. `_clog` / `_clog_hex` / `_clog_mix`: level tag + body (append only; does not clear the buffer)
3. `clog_process()`: prints the full buffer

`__FILE__` / `__LINE__` appear only inside the `CLOG_FORMAT` macro, not in `_clog` parameters.

The 6-byte layout for `clog_get_datetime()`:

| Index | Field  | Printed position |
|-------|--------|------------------|
| `[5]` | year   | `YY` (00–99)     |
| `[4]` | month  | `MM`             |
| `[3]` | day    | `DD`             |
| `[2]` | hour   | `HH`             |
| `[1]` | minute | `MM`             |
| `[0]` | second | `SS`             |

- **`SDK_PLATFORM == PLATFORM_COMMON`**: `clog_get_datetime()` is implemented in `clog.c` via `localtime()`.
- **Other platforms** (OpenLuat, Neoway, RTT, etc.): you must provide `clog_get_datetime()` (RTC / SNTP / AT command), same as before.

Example with datetime enabled:

```c
#define ENABLE_DATETIME TRUE   /* in clog.h before including or edit clog.h */

clog_init(&default_log, LL_DBG);
clog_dbg("boot");   /* e.g. [26/05/20 14:30:01] [main.c:10] [DBG] boot */
```

## Build-time configuration (`clog.h`)

| Macro | Default | Meaning |
|-------|---------|---------|
| `CLOG_BUF_SIZE` | 256 | Line buffer size |
| `BUILD_LOG_LEVEL` | `LL_DBG` | Strip macros below this level at compile time |
| `ENABLE_DATETIME` | `FALSE` | Prefix `[YY/MM/DD HH:MM:SS]` before file:line (see below) |
| `SDK_PLATFORM` | `PLATFORM_COMMON` | Select `clog_printf` backend |
| `SUPPORT_LOC_SAVE` | `FALSE` | File rotation on device (experimental; keep off unless you fix loc-save integration) |

## API reference

```c
int  clog_init(clog_inf_t* log, int level);   // 0 ok, -1 alloc failed
void clog_deinit(clog_inf_t* log);
int  clog_get_level(clog_inf_t* log);

void clog_format_prefix(clog_inf_t* log, const char* file, int line);
void _clog(clog_inf_t* log, int level, const char* fmt, ...);
void _clog_hex(clog_inf_t* log, int level, const uint8_t* data, uint16_t len);
void _clog_mix(clog_inf_t* log, int level, const uint8_t* data, uint16_t len);

extern clog_inf_t default_log;
extern clog_inf_t target_log;
```

## Integration checklist

1. Add `clog.c` / `clog.h` to your project (or copy into `Core/Inc` / `Core/Src` as `log.h` / `log.c`).
2. Set `SDK_PLATFORM` (and includes) for your board/SDK if not PC.
3. Adjust `BUILD_LOG_LEVEL` and `CLOG_BUF_SIZE` for release vs debug builds.
4. Call `clog_init(&default_log, ...)` before any `clog_*` macro; check return value `0`.
5. If multiple threads: add a lock around log calls or use one instance per thread (see above).
6. Custom sinks: copy `log->clog_buf` in the callback if output is asynchronous.
7. Call `clog_deinit(&default_log)` on shutdown for each initialized instance.

## Pre-commit / release notes (API changes)

- `clog_init(clog_inf_t* log, int level)` — was `clog_init(int level)` with global buffer.
- Removed `LL_INF`; level enum values shifted (`LL_WAR`=1, `LL_ERR`=2, …).
- `CLOG_BUF_SIZE` default 256 (was 512 in older tree).
- `clog_demo.c` removed; see [Demo](#demo) in this file.
- `CLOG_FORMAT(log)` must be used before `_clog` / `_clog_custom` when not using `clog_*` macros.

## License

See [LICENSE](LICENSE).
