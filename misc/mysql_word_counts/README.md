# mysql_word_counts

Small Rust utility to count words from `select * from t limit -1` on a Manticore
MySQL protocol port and print `word<TAB>count`.

## Build

```
cargo build
```

## Run

Default DSN is `mysql://127.0.0.1:9306`:

```
cargo run -- --dsn mysql://127.0.0.1:9306
```

You can also use `MANTICORE_DSN` instead of `--dsn`.

Progress output (to stderr) every N rows:

```
cargo run -- --progress-every 10000
```

Indextool dump mode (reads stdin, uses keyword and hits columns):

```
indextool --dumpdict t | cargo run -- --from-indextool
```

To run the compiled binary directly:

```
cargo build --release
./target/release/mysql_word_counts --dsn mysql://127.0.0.1:9306
```
