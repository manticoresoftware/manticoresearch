package main

import (
	"database/sql"
	"fmt"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"

	_ "github.com/go-sql-driver/mysql"
)

func envOr(key, def string) string {
	val := os.Getenv(key)
	if val == "" {
		return def
	}
	return val
}

func envOrInt(key string, def int) int {
	val := os.Getenv(key)
	if val == "" {
		return def
	}
	if n, err := strconv.Atoi(val); err == nil {
		return n
	}
	return def
}

func dsnFromEnv() string {
	host := envOr("MANTICORE_HOST", "127.0.0.1")
	port := envOr("MANTICORE_PORT", "9306")
	user := envOr("MANTICORE_USER", "")
	pass := envOr("MANTICORE_PASS", "")
	db := envOr("MANTICORE_DB", "")

	addr := fmt.Sprintf("tcp(%s:%s)", host, port)
	var auth string
	if user != "" {
		auth = user
		if pass != "" {
			auth += ":" + pass
		}
		auth += "@"
	}
	return fmt.Sprintf("%s%s/%s?multiStatements=true", auth, addr, db)
}

func mustExec(db *sql.DB, query string, args ...any) {
	if _, err := db.Exec(query, args...); err != nil {
		panic(err)
	}
}

func main() {
	db, err := sql.Open("mysql", dsnFromEnv())
	if err != nil {
		panic(err)
	}
	defer db.Close()

	if err := db.Ping(); err != nil {
		panic(err)
	}

	mustExec(db, "DROP TABLE IF EXISTS ps_test")
	mustExec(db, "CREATE TABLE ps_test (id bigint, title string, price float)")

	insertStmt, err := db.Prepare("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)")
	if err != nil {
		panic(err)
	}
	defer insertStmt.Close()

	if _, err := insertStmt.Exec(int64(1), "hello world", 9.5); err != nil {
		panic(err)
	}

	selectStmt, err := db.Prepare("SELECT id, price FROM ps_test WHERE id = ? OR title = ?")
	if err != nil {
		panic(err)
	}
	defer selectStmt.Close()

	row := selectStmt.QueryRow(int64(1), "hello world")
	var id int64
	var price float64
	if err := row.Scan(&id, &price); err != nil {
		panic(err)
	}
	fmt.Printf("{\"id\":%d,\"price\":\"%.6f\"}\n", id, price)

	if _, err := insertStmt.Exec(int64(2), "hello'; DROP TABLE ps_test; --", 1.23); err != nil {
		panic(err)
	}
	injRow := selectStmt.QueryRow(int64(0), "hello'; DROP TABLE ps_test; --")
	var injID int64
	var injPrice float64
	if err := injRow.Scan(&injID, &injPrice); err != nil {
		panic(err)
	}
	if injID != 2 {
		panic(fmt.Sprintf("injection select returned unexpected id: %d", injID))
	}
	checkStmt, err := db.Prepare("SELECT title FROM ps_test WHERE id = ?")
	if err != nil {
		panic(err)
	}
	defer checkStmt.Close()
	var injTitle string
	if err := checkStmt.QueryRow(int64(2)).Scan(&injTitle); err != nil {
		panic(err)
	}
	if injTitle != "hello'; DROP TABLE ps_test; --" {
		panic(fmt.Sprintf("injection value mismatch: expected '%s', got '%s'", "hello'; DROP TABLE ps_test; --", injTitle))
	}

	if _, err := insertStmt.Exec(int64(0), "", 0.0); err != nil {
		panic(err)
	}

	if _, err := insertStmt.Exec(int64(-1), "negative", -1.5); err == nil {
		fmt.Println("negative id insert unexpectedly succeeded")
	} else {
		fmt.Printf("negative id insert rejected as expected: %v\n", err)
	}

	if _, err := insertStmt.Exec(int64(9223372036854775807), "bigint", 1.0); err != nil {
		panic(err)
	}
	fmt.Println("edge inserts ok")

	mustExec(db, "DROP TABLE IF EXISTS ps_types")
	mustExec(db, "CREATE TABLE ps_types (id bigint, title text, i int, b bool, ts timestamp, f float, s string, j json, m multi, m64 multi64, vec float_vector)")

	typeInsert, err := db.Prepare("INSERT INTO ps_types (id, title, i, b, ts, f, s, j, m, m64, vec) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)")
	if err != nil {
		panic(err)
	}
	if _, err := typeInsert.Exec(int64(1), "types", int32(42), int32(1), int64(1700000000), 1.25, "hello", "{\"a\":1,\"b\":\"x\"}", "(1,2,5)", "(10000000000,10000000001)", "(0.1,0.2,0.3)"); err != nil {
		panic(err)
	}
	_ = typeInsert.Close()

	typeUpdate, err := db.Prepare("UPDATE ps_types SET m = TO_MULTI(?), m64 = TO_MULTI(?), vec = TO_VECTOR(?) WHERE id = ?")
	if err != nil {
		panic(err)
	}
	if _, err := typeUpdate.Exec("(7,8,9)", "(20000000000,20000000001)", "(0.4,0.5,0.6)", int64(1)); err != nil {
		panic(err)
	}
	if _, err := typeUpdate.Exec("1,2", "[3,4]", "(1, bad)", int64(1)); err == nil {
		fmt.Println("bad vector/mva update unexpectedly succeeded")
	} else {
		fmt.Printf("bad vector/mva update rejected as expected: %v\n", err)
	}
	if _, err := typeUpdate.Exec(nil, nil, nil, int64(1)); err == nil {
		fmt.Println("NULL vector/mva update unexpectedly succeeded")
	} else {
		fmt.Printf("NULL vector/mva update rejected as expected: %v\n", err)
	}
	_ = typeUpdate.Close()

	exprUpdate, err := db.Prepare("UPDATE ps_types SET m = m + ? WHERE id = ?")
	if err != nil {
		panic(err)
	}
	if _, err := exprUpdate.Exec("(1,2)", int64(1)); err == nil {
		fmt.Println("expression update unexpectedly succeeded")
	} else {
		fmt.Printf("expression update rejected as expected: %v\n", err)
	}
	_ = exprUpdate.Close()

	typeSelect, err := db.Prepare("SELECT id, title, i, b, ts, f, s, j, m, m64, vec FROM ps_types WHERE id = ?")
	if err != nil {
		panic(err)
	}
	var tID int64
	var tTitle, tS, tJ, tM, tM64, tVec string
	var tI, tB int32
	var tTS int64
	var tF float32
	if err := typeSelect.QueryRow(int64(1)).Scan(&tID, &tTitle, &tI, &tB, &tTS, &tF, &tS, &tJ, &tM, &tM64, &tVec); err != nil {
		panic(err)
	}
	fmt.Printf("types row: {\"id\":%d,\"title\":\"%s\",\"i\":%d,\"b\":%d,\"ts\":%d,\"f\":\"%.6f\",\"s\":\"%s\",\"j\":%s,\"m\":\"%s\",\"m64\":\"%s\",\"vec\":\"%s\"}\n", tID, tTitle, tI, tB, tTS, tF, tS, tJ, tM, tM64, tVec)
	_ = typeSelect.Close()

	mustExec(db, "DROP TABLE IF EXISTS ps_blob")
	mustExec(db, "CREATE TABLE ps_blob (id bigint, blob string)")

	blob := strings.Repeat("x", 1024) + strings.Repeat("y", 512)
	blobStmt, err := db.Prepare("INSERT INTO ps_blob (id, blob) VALUES (?, ?)")
	if err != nil {
		panic(err)
	}
	defer blobStmt.Close()

	if _, err := blobStmt.Exec(int64(1), blob); err != nil {
		panic(err)
	}

	blobSelect, err := db.Prepare("SELECT blob FROM ps_blob WHERE id = ?")
	if err != nil {
		panic(err)
	}
	defer blobSelect.Close()

	var gotBlob string
	if err := blobSelect.QueryRow(int64(1)).Scan(&gotBlob); err != nil {
		panic(err)
	}
	fmt.Printf("blob_col length: %d\n", len(gotBlob))

	tx, err := db.Begin()
	if err != nil {
		panic(err)
	}
	txStmt, err := tx.Prepare("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)")
	if err != nil {
		panic(err)
	}
	if _, err := txStmt.Exec(int64(1001), "tx", 3.14); err != nil {
		panic(err)
	}
	_ = txStmt.Close()
	if err := tx.Rollback(); err != nil {
		fmt.Printf("rollback not supported; continuing: %v\n", err)
	} else {
		checkStmt, err := db.Prepare("SELECT id FROM ps_test WHERE id = ?")
		if err != nil {
			panic(err)
		}
		defer checkStmt.Close()
		var found int64
		err = checkStmt.QueryRow(int64(1001)).Scan(&found)
		if err == nil {
			fmt.Println("rollback failed or not effective")
		} else {
			fmt.Println("rollback ok")
		}
	}

	iterations := envOrInt("MANTICORE_ITERATIONS", 1000)
	mustExec(db, "DROP TABLE IF EXISTS ps_load")
	mustExec(db, "CREATE TABLE ps_load (id bigint, val int)")

	loadStmt, err := db.Prepare("INSERT INTO ps_load (id, val) VALUES (?, ?)")
	if err != nil {
		panic(err)
	}
	defer loadStmt.Close()

	start := time.Now()
	for i := 1; i <= iterations; i++ {
		if _, err := loadStmt.Exec(int64(i), int32(i%1000)); err != nil {
			panic(err)
		}
	}
	fmt.Printf("load inserts ok: %d rows in %.3fs\n", iterations, time.Since(start).Seconds())

	workers := envOrInt("MANTICORE_WORKERS", 4)
	perWorker := envOrInt("MANTICORE_WORKER_ITERS", 200)
	var wg sync.WaitGroup
	wg.Add(workers)
	var once sync.Once
	var workerErr error
	for w := 0; w < workers; w++ {
		w := w
		go func() {
			defer wg.Done()
			stmt, err := db.Prepare("INSERT INTO ps_load (id, val) VALUES (?, ?)")
			if err != nil {
				once.Do(func() { workerErr = err })
				return
			}
			defer stmt.Close()
			base := int64(1_000_000 + w*perWorker)
			for i := 0; i < perWorker; i++ {
				id := base + int64(i)
				val := int32(id % 1000)
				if _, err := stmt.Exec(id, val); err != nil {
					once.Do(func() { workerErr = err })
					return
				}
			}
		}()
	}
	wg.Wait()
	if workerErr != nil {
		panic(workerErr)
	}
	fmt.Printf("concurrency ok: %d workers x %d\n", workers, perWorker)
}
