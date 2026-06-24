# Configuration

The loader reads `config.toml` from the `RobloxModLoader` folder. It is created with sensible
defaults on first run; edit it to change behavior. Core settings live under `[core.*]` tables, and
each installed mod is described by a `[[mods]]` entry.

## Core settings

### `[core.logging]`

Controls the loader's log output.

| Key | Type | Description |
| --- | --- | --- |
| `level` | string | Minimum level to emit (for example `trace`, `debug`, `info`, `warn`, `error`). |
| `enable_console` | bool | Write logs to the console. |
| `enable_file_logging` | bool | Write logs to files in `log_directory`. |
| `enable_async_logging` | bool | Log on a background thread to reduce overhead. |
| `log_directory` | string | Directory for log files. |
| `log_retention_hours` | int | How long to keep old log files. |
| `max_log_files` | int | Maximum number of log files to retain. |

### `[core.performance]`

| Key | Type | Description |
| --- | --- | --- |
| `enable_profiling` | bool | Collect performance metrics. |
| `thread_pool_size` | int | Worker threads available to the loader. |
| `hook_timeout_ms` | int | Timeout, in milliseconds, applied to hook operations. |

### `[core.security]`

| Key | Type | Description |
| --- | --- | --- |
| `verify_mod_signatures` | bool | Require mods to be signed before loading. |
| `sandbox_mods` | bool | Apply additional isolation to loaded mods. |
| `max_memory_per_mod` | int | Per-mod memory ceiling. |

### `[core.developer]`

| Key | Type | Description |
| --- | --- | --- |
| `debug_mode` | bool | Enable extra diagnostics intended for development. |
| `enable_hot_reload` | bool | Reload mods when their files change. |
| `verbose_logging` | bool | Emit additional detail in the logs. |

## Mods

Each installed mod has an entry describing its metadata and how the loader should treat it.

```toml
[[mods]]
name = "My Mod"
version = "1.0.0"
author = "You"
description = "What it does"

  [mods.runtime]
  enabled = true
  auto_load = true
  priority = 0
```

| Key | Type | Description |
| --- | --- | --- |
| `name`, `version`, `author`, `description` | string | Mod metadata. |
| `runtime.enabled` | bool | Whether the mod is allowed to load at all. |
| `runtime.auto_load` | bool | Load the mod automatically at startup. |
| `runtime.priority` | int | Relative load order; higher priority loads earlier. |

## Example

```toml
[core.logging]
level = "info"
enable_console = true
enable_file_logging = true

[core.developer]
debug_mode = false
enable_hot_reload = true

[[mods]]
name = "My Mod"
version = "1.0.0"
author = "You"

  [mods.runtime]
  enabled = true
  auto_load = true
  priority = 0
```
