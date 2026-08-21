# `manticore` command-line tool

`manticore` is the primary command-line tool for connecting to and managing Manticore Search. It provides:

- an interactive SQL client;
- one-shot SQL execution;
- start, stop, and status commands for a local or configured instance.

`searchd` remains the server binary and provides advanced daemon options. Use [`searchd`](Starting_the_server/Manually.md) directly when you need low-level options such as `--console`, `--listen`, or replication bootstrap flags.

> `manticore` client and lifecycle commands are currently supported on Unix-like systems. On Windows, only `manticore --help` and `manticore --version` are supported.

## Syntax

```bash
manticore [CLIENT_OPTIONS]
manticore start [local|global]
manticore stop [local|global]
manticore status [local|global]
manticore --help
manticore --version
```

## Connecting to Manticore Search

Run `manticore` without a lifecycle command to open an interactive SQL session:

```bash
manticore
```

```text
manticore> SELECT 1;
```

Use `-e` or `--execute` to execute SQL once and exit:

```bash
manticore -e 'SHOW TABLES'
```

You can also pipe SQL to the client:

```bash
printf 'SHOW TABLES;\n' | manticore
```

When output is redirected, query results use a tab-separated format suitable for scripts.

### Client options

- `-e SQL`, `--execute SQL` executes SQL once and exits.
- `-c FILE`, `--config FILE` ignores `./manticore_data` and uses exactly the specified configuration file.
- `--local` requires the current-directory local instance.
- `--global` ignores `./manticore_data` and uses normal configuration discovery.
- `-h`, `--help` displays command help.
- `-v`, `--version` displays the matching Manticore Search version.

`--local`, `--global`, and `--config` are mutually exclusive.

### Automatic target selection

Without an explicit target, `manticore` selects the instance from the current directory:

1. If `./manticore_data` exists, `manticore` uses the local instance and connects through `./manticore_data/searchd.sock`.
2. If `./manticore_data` does not exist, `manticore` uses normal configuration discovery and connects to the first usable HTTP or binary listener. MySQL-protocol listeners are skipped.

An existing `manticore_data` path is authoritative. If it is invalid, inaccessible, stale, or its local daemon is not running, `manticore` reports an error instead of silently connecting to a configured instance. Use `--global` when you intentionally want the configured instance from a directory that contains `manticore_data`:

```bash
manticore --global -e 'SHOW TABLES'
```

Use `--config` when you need a specific configuration file:

```bash
manticore --config /etc/manticoresearch/manticore.conf -e 'SHOW TABLES'
```

## Starting an instance

```bash
manticore start [local|global]
```

The command waits until Manticore Search is ready to execute SQL before returning success.

Without an explicit target, it starts the local instance when `./manticore_data` exists and the configured instance otherwise. `manticore start local` is the exception that can create a new `./manticore_data` directory:

```bash
mkdir my-search-project
cd my-search-project
manticore start local
```

For a local instance, `manticore` invokes the matching `searchd --local`. For a configured instance, it uses `manticore.service` when systemd is available and the service is installed; otherwise it invokes the matching `searchd` directly. If systemd is selected and the service operation fails, `manticore` reports the failure and does not start a second daemon directly.

## Stopping an instance

```bash
manticore stop [local|global]
```

`manticore stop` always waits for shutdown to finish. There is no separate `manticore stopwait` command.

Without an explicit target, it stops the local instance when `./manticore_data` exists and the configured instance otherwise:

```bash
manticore stop          # automatic target selection
manticore stop local    # require the current-directory instance
manticore stop global   # ignore ./manticore_data
```

Local shutdown targets only the PID and Unix socket under `./manticore_data`. Invalid or stale local process metadata is reported as an error rather than used to signal an unrelated process.

## Checking status

```bash
manticore status [local|global]
```

For a local instance, status checks the local data directory, PID ownership, process, Unix socket, and SQL readiness. Example output:

```text
Target: local (/work/catalog/manticore_data)
State: ready
PID: 18472
Socket: manticore_data/searchd.sock (present)
```

For a configured instance, `manticore status global` runs the matching `searchd --status` and preserves its output and exit status.

## Local instance files

A [local configless instance](Starting_the_server/Manually.md#Local-configless-mode) stores its state under `./manticore_data`:

- `searchd.sock` — local HTTP Unix socket;
- `searchd.pid` — daemon PID file;
- `searchd.log` — daemon log;
- `.history` — interactive client history;
- table and binary-log files created by the local instance.

Because this directory selects local mode automatically, run `manticore --global` to address the configured instance without renaming or removing it.

## Exit codes

| Code | Meaning |
|---:|---|
| `0` | The requested operation completed successfully |
| `1` | Runtime, connection, SQL, readiness, or lifecycle failure |
| `2` | Invalid command-line usage |
| `126` | A required executable exists but cannot be executed |
| `127` | A required executable or backend command was not found |
| `130` | Waiting was interrupted with Ctrl-C |

For one-shot or piped SQL, any SQL error makes the final exit status nonzero.

<!-- proofread -->
