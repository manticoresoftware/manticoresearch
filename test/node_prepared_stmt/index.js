const mysql = require("mysql2/promise");

function envOr(key, def) {
  return process.env[key] || def;
}

function envOrInt(key, def) {
  const val = process.env[key];
  if (!val) return def;
  const parsed = parseInt(val, 10);
  return Number.isNaN(parsed) ? def : parsed;
}

async function mustExec(conn, sql, params = []) {
  await conn.execute(sql, params);
}

async function main() {
  const host = envOr("MANTICORE_HOST", "127.0.0.1");
  const port = envOrInt("MANTICORE_PORT", 9306);
  const user = envOr("MANTICORE_USER", "");
  const pass = envOr("MANTICORE_PASS", "");
  const db = envOr("MANTICORE_DB", "");

  const connOpts = {
    host,
    port,
    user,
    password: pass,
  };
  if (db) {
    connOpts.database = db;
  } else {
    connOpts.flags = ["-CONNECT_WITH_DB"];
  }
  const conn = await mysql.createConnection(connOpts);

  await mustExec(conn, "DROP TABLE IF EXISTS ps_test");
  await mustExec(conn, "CREATE TABLE ps_test (id bigint, title string, price float)");

  await conn.execute("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)", [
    1n,
    "hello world",
    9.5,
  ]);

  const [rows] = await conn.execute(
    "SELECT id, price FROM ps_test WHERE title = ?",
    ["hello world"]
  );
  if (rows.length > 0) {
    const row = rows[0];
    console.log(`{"id":${row.id},"price":"${Number(row.price).toFixed(6)}"}`);
  }

  await conn.execute("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)", [
    0n,
    "",
    0.0,
  ]);
  try {
    await conn.execute("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)", [
      -1n,
      "negative",
      -1.5,
    ]);
    console.log("negative id insert succeeded (mysql2 sends DOUBLE for numbers)");
  } catch (err) {
    console.log(`negative id insert rejected as expected: ${err.message}`);
  }
  await conn.execute("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)", [
    9223372036854775807n,
    "bigint",
    1.0,
  ]);
  console.log("edge inserts ok");

  await mustExec(conn, "DROP TABLE IF EXISTS ps_types");
  await mustExec(
    conn,
    "CREATE TABLE ps_types (id bigint, title string, i int, b bool, ts timestamp, f float, s string, j json, m multi, m64 multi64, vec float_vector)"
  );

  await conn.execute(
    "INSERT INTO ps_types (id, title, i, b, ts, f, s, j, m, m64, vec) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
    [
      1n,
      "types",
      42n,
      1n,
      1700000000n,
      1.25,
      "hello",
      '{"a":1,"b":"x"}',
      "(1,2,5)",
      "(10000000000,10000000001)",
      "(0.1,0.2,0.3)",
    ]
  );

  await conn.execute(
    "UPDATE ps_types SET m = TO_MULTI(?), m64 = TO_MULTI(?), vec = TO_VECTOR(?) WHERE title = ?",
    ["(7,8,9)", "(20000000000,20000000001)", "(0.4,0.5,0.6)", "types"]
  );
  try {
    await conn.execute(
      "UPDATE ps_types SET m = TO_MULTI(?), m64 = TO_MULTI(?), vec = TO_VECTOR(?) WHERE title = ?",
      ["1,2", "[3,4]", "(1, bad)", "types"]
    );
    console.log("bad vector/mva update unexpectedly succeeded");
  } catch (err) {
    console.log(`bad vector/mva update rejected as expected: ${err.message}`);
  }

  const [typesRows] = await conn.execute(
    "SELECT id, title, i, b, ts, f, s, j, m, m64, vec FROM ps_types WHERE title = ?",
    ["types"]
  );
  if (typesRows.length > 0) {
    const r = typesRows[0];
    const fVal = Number(r.f).toFixed(6);
    console.log(
      `types row: {"id":${r.id},"title":"${r.title}","i":${r.i},"b":${r.b},"ts":${r.ts},"f":"${fVal}","s":"${r.s}","j":${r.j},"m":"${r.m}","m64":"${r.m64}","vec":"${r.vec}"}`
    );
  }
  await mustExec(conn, "DROP TABLE IF EXISTS ps_blob");
  await mustExec(conn, "CREATE TABLE ps_blob (id bigint, tag string, blob string)");
  const blob = "x".repeat(1024) + "y".repeat(512);
  await conn.execute("INSERT INTO ps_blob (id, tag, blob) VALUES (?, ?, ?)", [
    1,
    "blob-1",
    blob,
  ]);
  const [blobRows] = await conn.execute(
    "SELECT blob FROM ps_blob WHERE tag = ?",
    ["blob-1"]
  );
  if (blobRows.length > 0) {
    console.log(`blob_col length: ${blobRows[0].blob.length}`);
  }

  try {
    await conn.beginTransaction();
    await conn.execute("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)", [
      1001n,
      "tx",
      3.14,
    ]);
    await conn.rollback();

    const [checkRows] = await conn.execute(
      "SELECT id FROM ps_test WHERE title = ?",
      ["tx"]
    );
    if (checkRows.length > 0) {
      console.log("rollback failed or not effective");
    } else {
      console.log("rollback ok");
    }
  } catch (err) {
    console.log(`rollback failed or not effective: ${err.message}`);
  }

  const iterations = envOrInt("MANTICORE_ITERATIONS", 1000);
  await mustExec(conn, "DROP TABLE IF EXISTS ps_load");
  await mustExec(conn, "CREATE TABLE ps_load (id bigint, val int)");
  const start = Date.now();
  for (let i = 1; i <= iterations; i += 1) {
    await conn.execute("INSERT INTO ps_load (id, val) VALUES (?, ?)", [
      BigInt(i),
      i % 1000,
    ]);
  }
  const elapsed = (Date.now() - start) / 1000;
  console.log(`load inserts ok: ${iterations} rows in ${elapsed.toFixed(3)}s`);

  const workers = envOrInt("MANTICORE_WORKERS", 4);
  const perWorker = envOrInt("MANTICORE_WORKER_OPS", 200);
  const poolOpts = {
    host,
    port,
    user,
    password: pass,
    connectionLimit: Math.max(4, workers),
  };
  if (db) {
    poolOpts.database = db;
  } else {
    poolOpts.flags = ["-CONNECT_WITH_DB"];
  }
  const pool = mysql.createPool(poolOpts);

  await mustExec(pool, "DROP TABLE IF EXISTS ps_concurrency");
  await mustExec(pool, "CREATE TABLE ps_concurrency (id bigint, v int)");

  const tasks = Array.from({ length: workers }, async (_, w) => {
    for (let i = 0; i < perWorker; i += 1) {
      const id = w * perWorker + i + 1;
      await pool.execute("INSERT INTO ps_concurrency (id, v) VALUES (?, ?)", [
        BigInt(id),
        i,
      ]);
    }
  });
  await Promise.all(tasks);
  console.log(`concurrency ok: ${workers} workers x ${perWorker}`);

  await pool.end();
  await conn.end();
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
