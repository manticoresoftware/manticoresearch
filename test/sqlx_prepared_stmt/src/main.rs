use sqlx::mysql::{MySqlConnectOptions, MySqlPoolOptions};
use sqlx::Row;
use std::env;
use std::time::Instant;

fn env_or(key: &str, default: &str) -> String {
    env::var(key).unwrap_or_else(|_| default.to_string())
}

fn env_or_usize(key: &str, default: usize) -> usize {
    env::var(key)
        .ok()
        .and_then(|v| v.parse::<usize>().ok())
        .unwrap_or(default)
}

fn env_or_bool(key: &str, default: bool) -> bool {
    env::var(key)
        .ok()
        .map(|v| matches!(v.as_str(), "1" | "true" | "TRUE" | "yes" | "YES"))
        .unwrap_or(default)
}

fn build_options() -> MySqlConnectOptions {
    let host = env_or("MANTICORE_HOST", "127.0.0.1");
    let port = env_or("MANTICORE_PORT", "9306");
    let user = env_or("MANTICORE_USER", "");
    let pass = env_or("MANTICORE_PASS", "");
    let db = env_or("MANTICORE_DB", "");

    let mut opts = MySqlConnectOptions::new()
        .host(&host)
        .port(port.parse::<u16>().unwrap_or(9306))
        .pipes_as_concat(false)
        .no_engine_subsitution(false)
        .timezone(None)
        .set_names(false);

    if !user.is_empty() {
        opts = opts.username(&user);
    }
    if !pass.is_empty() {
        opts = opts.password(&pass);
    }
    if !db.is_empty() {
        opts = opts.database(&db);
    }
    opts
}

fn ok(msg: &str) {
    println!("{}", msg);
}

fn fail(msg: &str) -> Result<(), Box<dyn std::error::Error>> {
    Err(msg.into())
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let pool = MySqlPoolOptions::new()
        .max_connections(32)
        .connect_with(build_options())
        .await?;

    let mysql_mode = env_or_bool("MYSQL_COMPAT", false);

    sqlx::query("DROP TABLE IF EXISTS ps_test").execute(&pool).await?;
    if mysql_mode {
        sqlx::query("CREATE TABLE ps_test (id bigint, title varchar(255), price float)")
            .execute(&pool)
            .await?;
    } else {
        sqlx::query("CREATE TABLE ps_test (id bigint, title string, price float)")
            .execute(&pool)
            .await?;
    }

    // Insert prepared.
    sqlx::query("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)")
        .bind(1_i64)
        .bind("hello world")
        .bind(9.5_f64)
        .execute(&pool)
        .await?;

    // Select prepared.
    let row = sqlx::query("SELECT id, price FROM ps_test WHERE id = ? OR title = ?")
        .bind(1_i64)
        .bind("hello world")
        .fetch_one(&pool)
        .await?;
    let id: i64 = row.try_get("id")?;
    let price: f32 = row.try_get("price")?;
    println!("{{\"id\":{},\"price\":\"{:.6}\"}}", id, price);

    // Edge cases.
    sqlx::query("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)")
        .bind(0_i64)
        .bind("")
        .bind(0.0_f64)
        .execute(&pool)
        .await?;

    match sqlx::query("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)")
        .bind(if mysql_mode { 2_i64 } else { -1_i64 })
        .bind("negative")
        .bind(-1.5_f64)
        .execute(&pool)
        .await
    {
        Ok(_) if mysql_mode => ok("mysql mode: negative id test skipped"),
        Ok(_) => ok("negative id insert unexpectedly succeeded"),
        Err(e) => ok(&format!("negative id insert rejected as expected: {}", e)),
    }

    sqlx::query("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)")
        .bind(i64::MAX)
        .bind("bigint")
        .bind(1.0_f64)
        .execute(&pool)
        .await?;
    ok("edge inserts ok");

    // All types.
    sqlx::query("DROP TABLE IF EXISTS ps_types").execute(&pool).await?;
    if mysql_mode {
        sqlx::query("CREATE TABLE ps_types (id bigint, title text, i int, b bool, ts timestamp, f float, s varchar(255), j json)")
            .execute(&pool)
            .await?;
    } else {
        sqlx::query("CREATE TABLE ps_types (id bigint, title text, i int, b bool, ts timestamp, f float, s string, j json, m multi, m64 multi64, vec float_vector)")
            .execute(&pool)
            .await?;
    }

    if mysql_mode {
        sqlx::query("INSERT INTO ps_types (id, title, i, b, ts, f, s, j) VALUES (?, ?, ?, ?, ?, ?, ?, ?)")
            .bind(1_i64)
            .bind("types")
            .bind(42_i32)
            .bind(1_i32)
            .bind("2023-11-14 00:00:00")
            .bind(1.25_f64)
            .bind("hello")
            .bind("{\"a\":1,\"b\":\"x\"}")
            .execute(&pool)
            .await?;
    } else {
        sqlx::query("INSERT INTO ps_types (id, title, i, b, ts, f, s, j, m, m64, vec) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)")
            .bind(1_i64)
            .bind("types")
            .bind(42_i32)
            .bind(1_i32)
            .bind(1_700_000_000_i64)
            .bind(1.25_f64)
            .bind("hello")
            .bind("{\"a\":1,\"b\":\"x\"}")
            .bind("(1,2,5)")
            .bind("(10000000000,10000000001)")
            .bind("(0.1,0.2,0.3)")
            .execute(&pool)
            .await?;
    }

    if !mysql_mode {
        sqlx::query("UPDATE ps_types SET m = TO_MULTI(?), m64 = TO_MULTI(?), vec = TO_VECTOR(?) WHERE id = ?")
            .bind("(7,8,9)")
            .bind("(20000000000,20000000001)")
            .bind("(0.4,0.5,0.6)")
            .bind(1_i64)
            .execute(&pool)
            .await?;

        match sqlx::query("UPDATE ps_types SET m = TO_MULTI(?), m64 = TO_MULTI(?), vec = TO_VECTOR(?) WHERE id = ?")
            .bind("1,2")
            .bind("[3,4]")
            .bind("(1, bad)")
            .bind(1_i64)
            .execute(&pool)
            .await
        {
            Ok(_) => ok("bad vector/mva update unexpectedly succeeded"),
            Err(e) => ok(&format!("bad vector/mva update rejected as expected: {}", e)),
        }
    }

    if mysql_mode {
        let row = sqlx::query("SELECT id, title, i, b, CAST(ts AS CHAR) AS ts, f, s, CAST(j AS CHAR) AS j FROM ps_types WHERE id = ?")
            .bind(1_i64)
            .fetch_one(&pool)
            .await?;
        let id: i64 = row.try_get("id")?;
        let title: String = row.try_get("title")?;
        let i_val: i32 = row.try_get("i")?;
        let b_val: i32 = row.try_get("b")?;
        let ts: String = row.try_get("ts")?;
        let f_val: f32 = row.try_get("f")?;
        let s_val: String = row.try_get("s")?;
        let j_val: String = row.try_get("j")?;
        println!(
            "types row: {{\"id\":{},\"title\":\"{}\",\"i\":{},\"b\":{},\"ts\":{},\"f\":\"{:.6}\",\"s\":\"{}\",\"j\":{}}}",
            id, title, i_val, b_val, ts, f_val, s_val, j_val
        );
    } else {
        let row = sqlx::query("SELECT id, title, i, b, ts, f, s, j, m, m64, vec FROM ps_types WHERE id = ?")
            .bind(1_i64)
            .fetch_one(&pool)
            .await?;
        let id: i64 = row.try_get("id")?;
        let title: String = row.try_get("title")?;
        let i_val: i32 = row.try_get("i")?;
        let b_val: i32 = row.try_get("b")?;
        let ts: i64 = row.try_get("ts")?;
        let f_val: f32 = row.try_get("f")?;
        let s_val: String = row.try_get("s")?;
        let j_val: String = row.try_get("j")?;
        let m_val: String = row.try_get("m")?;
        let m64_val: String = row.try_get("m64")?;
        let vec_val: String = row.try_get("vec")?;
        println!(
            "types row: {{\"id\":{},\"title\":\"{}\",\"i\":{},\"b\":{},\"ts\":{},\"f\":\"{:.6}\",\"s\":\"{}\",\"j\":{},\"m\":\"{}\",\"m64\":\"{}\",\"vec\":\"{}\"}}",
            id, title, i_val, b_val, ts, f_val, s_val, j_val, m_val, m64_val, vec_val
        );
    }

    // Long data insert + select.
    sqlx::query("DROP TABLE IF EXISTS ps_blob").execute(&pool).await?;
    if mysql_mode {
        sqlx::query("CREATE TABLE ps_blob (id bigint, blob_col longtext)")
            .execute(&pool)
            .await?;
    } else {
        sqlx::query("CREATE TABLE ps_blob (id bigint, blob string)")
            .execute(&pool)
            .await?;
    }

    let blob = "x".repeat(1024) + &"y".repeat(512);
    sqlx::query(if mysql_mode { "INSERT INTO ps_blob (id, blob_col) VALUES (?, ?)" } else { "INSERT INTO ps_blob (id, blob) VALUES (?, ?)" })
        .bind(1_i64)
        .bind(&blob)
        .execute(&pool)
        .await?;

    let row = sqlx::query(if mysql_mode { "SELECT blob_col FROM ps_blob WHERE id = ?" } else { "SELECT blob FROM ps_blob WHERE id = ?" })
        .bind(1_i64)
        .fetch_one(&pool)
        .await?;
    let got_blob: String = row.try_get(if mysql_mode { "blob_col" } else { "blob" })?;
    println!("blob_col length: {}", got_blob.len());

    // Transaction mode.
    let mut tx = pool.begin().await?;
    sqlx::query("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)")
        .bind(1001_i64)
        .bind("tx")
        .bind(3.14_f64)
        .execute(&mut *tx)
        .await?;
    match tx.rollback().await {
        Ok(()) => {
            let row = sqlx::query("SELECT id FROM ps_test WHERE id = ?")
                .bind(1001_i64)
                .fetch_optional(&pool)
                .await?;
            if row.is_some() {
                ok("rollback failed or not effective");
            } else {
                ok("rollback ok");
            }
        }
        Err(e) => ok(&format!("rollback not supported; continuing: {}", e)),
    }

    // High load.
    let iterations = env_or_usize("MANTICORE_ITERATIONS", 1000);
    sqlx::query("DROP TABLE IF EXISTS ps_load").execute(&pool).await?;
    sqlx::query("CREATE TABLE ps_load (id bigint, val int)")
        .execute(&pool)
        .await?;

    let t0 = Instant::now();
    for i in 1..=iterations {
        sqlx::query("INSERT INTO ps_load (id, val) VALUES (?, ?)")
            .bind(i as i64)
            .bind((i % 1000) as i32)
            .execute(&pool)
            .await?;
    }
    ok(&format!(
        "load inserts ok: {} rows in {:.3}s",
        iterations,
        t0.elapsed().as_secs_f64()
    ));

    // Concurrency.
    let workers = env_or_usize("MANTICORE_WORKERS", 4);
    let per_worker = env_or_usize("MANTICORE_WORKER_ITERS", 200);
    let mut tasks = Vec::new();
    for w in 0..workers {
        let pool = pool.clone();
        tasks.push(tokio::spawn(async move {
            let base = 1_000_000_i64 + (w as i64 * per_worker as i64);
            for i in 0..per_worker {
                let id = base + i as i64;
                let val = (id % 1000) as i32;
                sqlx::query("INSERT INTO ps_load (id, val) VALUES (?, ?)")
                    .bind(id)
                    .bind(val)
                    .execute(&pool)
                    .await?;
            }
            Ok::<(), sqlx::Error>(())
        }));
    }
    for task in tasks {
        if let Err(e) = task.await? {
            return fail(&format!("worker failed: {}", e));
        }
    }
    ok(&format!("concurrency ok: {} workers x {}", workers, per_worker));

    Ok(())
}
