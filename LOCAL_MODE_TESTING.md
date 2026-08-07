# Testing configless local mode

This guide covers the manual checks that are useful while developing `searchd -l` and `searchd -e`. Automated lifecycle and PTY coverage lives in `test/clt-tests/test-configuration/test-zero-config-local-mode.rec`.

## Build and create an isolated instance

```bash
cd /path/to/manticoresearch
# Ensure the MCL API headers match the revision pinned by this branch.
git submodule update --init --recursive mcl
cmake --build build --target searchd -j6

BIN="$PWD/build/src/searchd"
TESTDIR=$(mktemp -d)
cd "$TESTDIR"
```

Run the submodule update after switching branches or pulling a commit that changes the `mcl` gitlink. If you intentionally use a different MCL checkout, it must provide the API expected by the daemon sources.

Local-mode paths are relative to the current directory, so an isolated directory avoids affecting another daemon.

If the current directory has no `manticore_data`, `searchd -e` follows the same default configuration lookup as ordinary `searchd`. It reads that config and connects to its first usable binary (`SPHINX`, which also accepts HTTP) or HTTP listener. Absolute Unix listeners are supported. Relative Unix paths and TCP port ranges are skipped because the client cannot infer the daemon's runtime path or selected port safely. If the config declares no listeners, the normal binary default at `127.0.0.1:9312` applies:

```bash
cd "$TESTDIR"
"$BIN" -e 'SELECT 1 AS configured_value'
test ! -e manticore_data
```

Use `-c` or `--config` to select a specific config. An explicit config is authoritative even when the current directory contains `manticore_data`:

```bash
"$BIN" -c /path/to/manticore.conf -e 'SELECT 1'
"$BIN" --config /path/to/manticore.conf -e
```

Without an explicit config, a detected local instance always takes precedence over the configured daemon. Any existing `manticore_data` marker pins execution to the local path; malformed markers, stale/empty local directories, and inaccessible paths fail rather than routing a query elsewhere. Configured interactive sessions keep command history in memory for that session but do not create a local history directory.

## Start and query

```bash
"$BIN" -l
"$BIN" -e 'SELECT 1 AS value'
"$BIN" --execute 'SELECT 2 AS value'
```

Expected local files:

```text
manticore_data/searchd.sock
manticore_data/searchd.pid
manticore_data/searchd.log
```

The local instance should listen on its Unix socket and should not expose a TCP listener.

## Interactive editing and history

Run the interactive client from a real terminal:

```bash
"$BIN" -e
```

Check:

1. Execute two different `SELECT` statements.
2. Press **Up** and **Down** to navigate them.
3. Type a draft, press **Up**, then **Down**; the draft should return.
4. Exit and start a second interactive client; **Up** should recall persisted commands.
5. Press **Ctrl-D** at an empty prompt; the client should exit cleanly.

History is stored as plain text, one command per line:

```text
manticore_data/.history
```

It retains the newest 1,000 commands. `.history` and the persistent `.history.lock` use mode `0600`; `manticore_data` uses mode `0700`. Concurrent clients lock, reload, merge, and atomically replace history so they do not overwrite each other.

Malformed, binary, oversized, nonregular, or unreadable history is reported and preserved. A temporary lock or save failure is reported; unsaved commands remain bounded in memory and are retried after the path recovers.

## Interrupt recovery

At the interactive prompt:

1. Type an incomplete command without Enter.
2. Press **Ctrl-C**.
3. Run a valid `SELECT`; it should execute normally.
4. Repeat with **Ctrl-\\**.
5. Press **Up**; completed commands should remain available, while interrupted drafts should not appear.

## Output modes

Interactive terminal output uses framed tables. Redirected and piped output remains tab-separated. End an individual statement with `\G` to render that result vertically, including when output is redirected:

```bash
"$BIN" -e 'SELECT 1 AS one, 22 AS two'
"$BIN" -e 'SELECT 1 AS one, 22 AS two' > result.tsv
"$BIN" -e 'SELECT 1 AS one, 22 AS two\G'
printf 'SELECT 3 AS value;\n' | "$BIN" -e
```

Multiple statements share one connection, so session settings persist:

```bash
printf 'SET SESSION wait_timeout=123;\nSELECT @@wait_timeout;\nexit\n' | "$BIN" -e
```

## Stop and clean up

```bash
"$BIN" -l --stopwait
cd /
rm -rf "$TESTDIR"
```

After `--stopwait`, the local socket and PID file should be gone. A separately configured daemon must remain running.
