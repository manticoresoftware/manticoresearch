// Usage (requires Cargo and the mysql crate):
//   - Create a Cargo project with this file as src/main.rs
//   - Add `mysql = "24"` to Cargo.toml dependencies
//   - Run: cargo run -- --dsn mysql://127.0.0.1:9306
//   - Optional: --progress-every 10000 (0 disables)
//   - Optional: --from-indextool (read indextool --dumpdict output from stdin)
//
// Environment variables:
//   MANTICORE_DSN (default: mysql://127.0.0.1:9306)

use std::collections::HashMap;
use std::env;
use std::error::Error;
use std::io::{self, BufRead, IsTerminal, Write};

use mysql::prelude::Queryable;
use mysql::{Opts, Pool};

fn main() -> Result<(), Box<dyn Error>> {
    let mut dsn: Option<String> = None;
    let mut progress_every: u64 = 10000;
    let mut from_indextool = false;

    let mut args = env::args().skip(1).peekable();
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--dsn" => {
                if let Some(val) = args.next() {
                    dsn = Some(val);
                }
            }
            "--progress-every" => {
                if let Some(val) = args.next() {
                    if let Ok(parsed) = val.parse::<u64>() {
                        progress_every = parsed;
                    }
                }
            }
            "--from-indextool" => {
                from_indextool = true;
            }
            _ => {}
        }
    }

    let mut counts: HashMap<String, u64> = HashMap::new();
    let mut processed_rows: u64 = 0;

    if from_indextool {
        if io::stdin().is_terminal() {
            eprintln!("--from-indextool expects stdin (e.g. indextool --dumpdict t | ./mysql_word_counts --from-indextool)");
            return Ok(());
        }
        let stdin = io::stdin();
        let input = stdin.lock();
        let mut in_data = false;

        for line in input.lines() {
            let line = line?;
            if !in_data {
                if line.trim() == "keyword,docs,hits,offset,docs_eff,hits_eff,chunk_id"
                    || line.trim() == "keyword,docs,hits,offset"
                {
                    in_data = true;
                }
                continue;
            }

            if line.trim().is_empty() {
                continue;
            }

            let mut parts = line.split(',');
            let keyword = match parts.next() {
                Some(v) if !v.is_empty() => v,
                _ => continue,
            };
            let _docs = parts.next();
            let hits = match parts.next() {
                Some(v) => v,
                None => continue,
            };
            let mut hits_val: u64 = match hits.parse() {
                Ok(v) => v,
                Err(_) => continue,
            };
            let _offset = parts.next();
            if let Some(docs_eff) = parts.next() {
                let _hits_eff = match parts.next() {
                    Some(v) => v,
                    None => continue,
                };
                if let Ok(parsed) = _hits_eff.parse::<u64>() {
                    hits_val = parsed;
                }
                let _chunk_id = parts.next();
                let _ = docs_eff;
                let _ = _chunk_id;
            }

            let key = keyword.to_ascii_lowercase();
            *counts.entry(key).or_insert(0) += hits_val;

            processed_rows += 1;
            if progress_every > 0 && processed_rows % progress_every == 0 {
                eprint!("\rprocessed rows: {}", processed_rows);
                io::stderr().flush().ok();
            }
        }
    } else {
        let dsn = dsn
            .or_else(|| env::var("MANTICORE_DSN").ok())
            .unwrap_or_else(|| "mysql://127.0.0.1:9306".to_string());

        let opts = Opts::from_url(&dsn)?;
        let pool = Pool::new(opts)?;
        let mut conn = pool.get_conn()?;

        let rows: Vec<mysql::Row> = conn.query("select * from t limit -1")?;
        for row in rows {
            let f: Option<String> = row.get("f");
            if let Some(text) = f {
                let mut buf = String::new();
                for ch in text.chars() {
                    if ch.is_alphanumeric() {
                        buf.push(ch.to_ascii_lowercase());
                    } else if !buf.is_empty() {
                        *counts.entry(buf.clone()).or_insert(0) += 1;
                        buf.clear();
                    }
                }
                if !buf.is_empty() {
                    *counts.entry(buf).or_insert(0) += 1;
                }
            }

            processed_rows += 1;
            if progress_every > 0 && processed_rows % progress_every == 0 {
                eprint!("\rprocessed rows: {}", processed_rows);
                io::stderr().flush().ok();
            }
        }
    }

    if progress_every > 0 {
        eprint!("\r\x1b[2K");
        io::stderr().flush().ok();
    }

    let mut sorted: Vec<(String, u64)> = counts.into_iter().collect();
    sorted.sort_by(|a, b| b.1.cmp(&a.1).then_with(|| a.0.cmp(&b.0)));

    let stdout = io::stdout();
    let mut out = stdout.lock();
    for (word, count) in sorted {
        if let Err(err) = writeln!(out, "{}\t{}", word, count) {
            if err.kind() == io::ErrorKind::BrokenPipe {
                break;
            }
            return Err(err.into());
        }
    }

    Ok(())
}
