<?php
// Simple prepared statement test for Manticore via MySQL protocol.

$host = getenv('MANTICORE_HOST') ?: '127.0.0.1';
$port = intval(getenv('MANTICORE_PORT') ?: '9315');
$user = getenv('MANTICORE_USER') ?: '';
$pass = getenv('MANTICORE_PASS') ?: '';
$db = getenv('MANTICORE_DB') ?: '';

$mysqli = new mysqli($host, $user, $pass, $db, $port);
if ($mysqli->connect_errno) {
    fwrite(STDERR, "Connect failed: {$mysqli->connect_error}\n");
    exit(1);
}

$mysqli->set_charset("utf8");

function fail($msg) {
    fwrite(STDERR, $msg . "\n");
    exit(1);
}

function ok($msg) {
    fwrite(STDOUT, $msg . "\n");
}

function fetch_blob($stmt, $id) {
    $stmt->bind_param("i", $id);
    if (!$stmt->execute()) {
        fail("Execute blob select failed: {$stmt->error}");
    }
    $res = $stmt->get_result();
    $row = $res->fetch_assoc();
    return $row ? $row['blob'] : null;
}

function create_table($mysqli, $name, $schema) {
    $mysqli->query("DROP TABLE IF EXISTS {$name}");
    if (!$mysqli->query("CREATE TABLE {$name} ({$schema})")) {
        fail("Create table {$name} failed: {$mysqli->error}");
    }
}

$mysqli->query("DROP TABLE IF EXISTS ps_test");
$create = "CREATE TABLE ps_test (id bigint, title string, price float)";
if (!$mysqli->query($create)) {
    fwrite(STDERR, "Create failed: {$mysqli->error}\n");
    exit(1);
}

// Insert using prepared statement.
$insert = $mysqli->prepare("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)");
if (!$insert) {
    fwrite(STDERR, "Prepare insert failed: {$mysqli->error}\n");
    exit(1);
}

$id = 1;
$title = "hello world";
$price = 9.5;
$insert->bind_param("isd", $id, $title, $price);
if (!$insert->execute()) {
    fwrite(STDERR, "Execute insert failed: {$insert->error}\n");
    exit(1);
}
$insert->close();

// Select using prepared statement with NULL and MATCH.
$select = $mysqli->prepare("SELECT id, price FROM ps_test WHERE id = ? OR title = ?");
if (!$select) {
    fwrite(STDERR, "Prepare select failed: {$mysqli->error}\n");
    exit(1);
}

$exact = "hello world";
$select->bind_param("is", $id, $exact);
if (!$select->execute()) {
    fwrite(STDERR, "Execute select failed: {$select->error}\n");
    exit(1);
}

$result = $select->get_result();
while ($row = $result->fetch_assoc()) {
    echo json_encode($row, JSON_UNESCAPED_SLASHES) . "\n";
}
$select->close();

// Injection attempts: ensure prepared statements treat values as data.
$injInsert = $mysqli->prepare("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)");
if (!$injInsert) {
    fail("Prepare injection insert failed: {$mysqli->error}");
}
$injId = 2;
$injTitle = "hello'; DROP TABLE ps_test; --";
$injPrice = 1.23;
$injInsert->bind_param("isd", $injId, $injTitle, $injPrice);
if (!$injInsert->execute()) {
    fail("Execute injection insert failed: {$injInsert->error}");
}
$injInsert->close();

$injSelect = $mysqli->prepare("SELECT id FROM ps_test WHERE title = ?");
if (!$injSelect) {
    fail("Prepare injection select failed: {$mysqli->error}");
}
$injSelect->bind_param("s", $injTitle);
if (!$injSelect->execute()) {
    fail("Execute injection select failed: {$injSelect->error}");
}
$injRes = $injSelect->get_result();
if ($injRes->num_rows !== 1) {
    fail("Injection select expected 1 row, got {$injRes->num_rows}");
}
$injRow = $injRes->fetch_assoc();
if ((int)$injRow['id'] !== $injId) {
    fail("Injection select returned unexpected id: {$injRow['id']}");
}
$injCheck = $mysqli->prepare("SELECT title FROM ps_test WHERE id = ?");
if (!$injCheck) {
    fail("Prepare injection value check failed: {$mysqli->error}");
}
$injCheck->bind_param("i", $injId);
if (!$injCheck->execute()) {
    fail("Execute injection value check failed: {$injCheck->error}");
}
$injCheckRes = $injCheck->get_result();
$injCheckRow = $injCheckRes->fetch_assoc();
if ($injCheckRow['title'] !== $injTitle) {
    fail("Injection value mismatch: expected '{$injTitle}', got '{$injCheckRow['title']}'");
}
$injCheck->close();
$injSelect->close();

// Confirm table still exists and original row is present.
$check = $mysqli->prepare("SELECT COUNT(*) AS cnt FROM ps_test WHERE title = ?");
if (!$check) {
    fail("Prepare injection check failed: {$mysqli->error}");
}
$checkTitle = "hello world";
$check->bind_param("s", $checkTitle);
if (!$check->execute()) {
    fail("Execute injection check failed: {$check->error}");
}
$checkRes = $check->get_result();
$checkRow = $checkRes->fetch_assoc();
if ((int)$checkRow['cnt'] !== 1) {
    fail("Injection check failed: expected 1 row, got {$checkRow['cnt']}");
}
$check->close();

// Edge cases: empty string, zero, negative, big integer.
$edgeInsert = $mysqli->prepare("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)");
if (!$edgeInsert) {
    fail("Prepare edge insert failed: {$mysqli->error}");
}
$edgeId = 0;
$edgeTitle = "";
$edgePrice = 0.0;
$edgeInsert->bind_param("isd", $edgeId, $edgeTitle, $edgePrice);
if (!$edgeInsert->execute()) {
    fail("Execute edge insert (empty/zero) failed: {$edgeInsert->error}");
}
$edgeId = -1;
$edgeTitle = "negative";
$edgePrice = -1.5;
try {
    if (!$edgeInsert->execute()) {
        fail("Execute edge insert (negative) failed: {$edgeInsert->error}");
    }
    ok("negative id insert unexpectedly succeeded");
} catch (Throwable $e) {
    ok("negative id insert rejected as expected: " . $e->getMessage());
}
$edgeId = 9223372036854775807;
$edgeTitle = "bigint";
$edgePrice = 1.0;
if (!$edgeInsert->execute()) {
    fail("Execute edge insert (bigint) failed: {$edgeInsert->error}");
}
$edgeInsert->close();
ok("edge inserts ok");

// Test long data (bind_param + send_long_data) via insert + select.
create_table($mysqli, "ps_blob", "id bigint, blob string");

$selectBlob = $mysqli->prepare("SELECT blob FROM ps_blob WHERE id = ?");
if (!$selectBlob) {
    fwrite(STDERR, "Prepare long data select failed: {$mysqli->error}\n");
    exit(1);
}

$insertBlob = $mysqli->prepare("INSERT INTO ps_blob (id, blob) VALUES (?, ?)");
if (!$insertBlob) {
    fwrite(STDERR, "Prepare long data insert failed: {$mysqli->error}\n");
    exit(1);
}

// Long data only.
$blobId = 1;
$blob = "";
$insertBlob->bind_param("ib", $blobId, $blob);
$insertBlob->send_long_data(1, str_repeat("x", 1024));
$insertBlob->send_long_data(1, str_repeat("y", 512));
if (!$insertBlob->execute()) {
    fwrite(STDERR, "Execute long data insert failed: {$insertBlob->error}\n");
    exit(1);
}
$blobVal = fetch_blob($selectBlob, $blobId);
echo "blob_col length: " . strlen($blobVal) . "\n";

// Long data should ignore inline value.
$blobId = 2;
$blob = "tail";
$insertBlob->send_long_data(1, "head");
if (!$insertBlob->execute()) {
    fwrite(STDERR, "Execute long data+inline insert failed: {$insertBlob->error}\n");
    exit(1);
}
$blobVal = fetch_blob($selectBlob, $blobId);
if ($blobVal !== "head") {
    fail("Long data+inline mismatch: expected 'head', got '" . ($blobVal === null ? "NULL" : $blobVal) . "'");
}

// NULL should override long data.
$blobId = 3;
$blob = null;
$insertBlob->send_long_data(1, "should_be_ignored");
try {
    if ($insertBlob->execute()) {
        $blobVal = fetch_blob($selectBlob, $blobId);
        if (!is_null($blobVal)) {
            fail("Long data+NULL mismatch: expected NULL, got '" . $blobVal . "'");
        }
    } else {
        ok("long data+NULL insert rejected as expected: {$insertBlob->error}");
    }
} catch (Throwable $e) {
    ok("long data+NULL insert rejected as expected: " . $e->getMessage());
}

$insertBlob->close();
$selectBlob->close();

// Test prepared statements with all supported types.
create_table($mysqli, "ps_types", "id bigint, title text, i int, b bool, ts timestamp, f float, s string, j json, m multi, m64 multi64, vec float_vector");
$insertTypes = $mysqli->prepare("INSERT INTO ps_types (id, title, i, b, ts, f, s, j, m, m64, vec) VALUES (?, ?, ?, ?, ?, ?, ?, ?, (?VEC?), (?VEC?), (?VEC?))");
if (!$insertTypes) {
    fail("Prepare types insert failed: {$mysqli->error}");
}
$typeId = 1;
$typeTitle = "types";
$typeI = 42;
$typeB = 1;
$typeTs = 1700000000;
$typeF = 1.25;
$typeS = "hello";
$typeJ = "{\"a\":1,\"b\":\"x\"}";
$typeM = "1,2,5";
$typeM64 = "10000000000,10000000001";
$typeVec = "0.1,0.2,0.3";
$insertTypes->bind_param("isiiidsssss", $typeId, $typeTitle, $typeI, $typeB, $typeTs, $typeF, $typeS, $typeJ, $typeM, $typeM64, $typeVec);
if (!$insertTypes->execute()) {
    fail("Execute types insert failed: {$insertTypes->error}");
}
$insertTypes->close();

$updateTypes = $mysqli->prepare("UPDATE ps_types SET m = (?VEC?), m64 = (?VEC?), vec = (?VEC?) WHERE id = ?");
if (!$updateTypes) {
    fail("Prepare types update failed: {$mysqli->error}");
}
$upM = "7,8,9";
$upM64 = "20000000000,20000000001";
$upVec = "0.4,0.5,0.6";
$updateTypes->bind_param("sssi", $upM, $upM64, $upVec, $typeId);
if (!$updateTypes->execute()) {
    fail("Execute types update failed: {$updateTypes->error}");
}
$updateTypes->close();

$badTypes = $mysqli->prepare("UPDATE ps_types SET m = (?VEC?), m64 = (?VEC?), vec = (?VEC?) WHERE id = ?");
if (!$badTypes) {
    fail("Prepare types bad update failed: {$mysqli->error}");
}
$badM = "1,2";
$badM64 = "[3,4]";
$badVec = "(1, bad)";
$badTypes->bind_param("sssi", $badM, $badM64, $badVec, $typeId);
try {
    if ($badTypes->execute()) {
        ok("bad vector/mva update unexpectedly succeeded");
    } else {
        ok("bad vector/mva update rejected as expected: {$badTypes->error}");
    }
} catch (Throwable $e) {
    ok("bad vector/mva update rejected as expected: " . $e->getMessage());
}
$badTypes->close();

$nullTypes = $mysqli->prepare("UPDATE ps_types SET m = (?VEC?), m64 = (?VEC?), vec = (?VEC?) WHERE id = ?");
if (!$nullTypes) {
    fail("Prepare types NULL update failed: {$mysqli->error}");
}
$nullM = null;
$nullM64 = null;
$nullVec = null;
$nullTypes->bind_param("sssi", $nullM, $nullM64, $nullVec, $typeId);
try {
    if ($nullTypes->execute()) {
        ok("NULL vector/mva update unexpectedly succeeded");
    } else {
        ok("NULL vector/mva update rejected as expected: {$nullTypes->error}");
    }
} catch (Throwable $e) {
    ok("NULL vector/mva update rejected as expected: " . $e->getMessage());
}
$nullTypes->close();

$exprTypes = $mysqli->prepare("UPDATE ps_types SET m = m + ? WHERE id = ?");
if (!$exprTypes) {
    fail("Prepare types expr update failed: {$mysqli->error}");
}
$exprVal = "(1,2)";
$exprTypes->bind_param("si", $exprVal, $typeId);
try {
    if ($exprTypes->execute()) {
        ok("expression update unexpectedly succeeded");
    } else {
        ok("expression update rejected as expected: {$exprTypes->error}");
    }
} catch (Throwable $e) {
    ok("expression update rejected as expected: " . $e->getMessage());
}
$exprTypes->close();

$selectTypes = $mysqli->prepare("SELECT id, title, i, b, ts, f, s, j, m, m64, vec FROM ps_types WHERE id = ?");
if (!$selectTypes) {
    fail("Prepare types select failed: {$mysqli->error}");
}
$selectTypes->bind_param("i", $typeId);
if (!$selectTypes->execute()) {
    fail("Execute types select failed: {$selectTypes->error}");
}
$typesRes = $selectTypes->get_result();
$typesRow = $typesRes->fetch_assoc();
echo "types row: " . json_encode($typesRow, JSON_UNESCAPED_SLASHES) . "\n";
$selectTypes->close();

// Mode check: autocommit off and rollback (may be unsupported).
$mysqli->autocommit(false);
$txStmt = $mysqli->prepare("INSERT INTO ps_test (id, title, price) VALUES (?, ?, ?)");
if (!$txStmt) {
    fail("Prepare tx insert failed: {$mysqli->error}");
}
$txId = 1001;
$txTitle = "tx";
$txPrice = 3.14;
$txStmt->bind_param("isd", $txId, $txTitle, $txPrice);
if (!$txStmt->execute()) {
    fail("Execute tx insert failed: {$txStmt->error}");
}
if (!$mysqli->rollback()) {
    ok("rollback not supported; continuing");
} else {
    $chk = $mysqli->prepare("SELECT id FROM ps_test WHERE id = ?");
    $chk->bind_param("i", $txId);
    $chk->execute();
    $res = $chk->get_result();
    if ($res && $res->fetch_assoc()) {
        ok("rollback failed or not effective");
    } else {
        ok("rollback ok");
    }
    $chk->close();
}
$txStmt->close();
$mysqli->autocommit(true);

// High load test.
$iterations = intval(getenv("MANTICORE_ITERATIONS") ?: "1000");
create_table($mysqli, "ps_load", "id bigint, val int");
$loadInsert = $mysqli->prepare("INSERT INTO ps_load (id, val) VALUES (?, ?)");
if (!$loadInsert) {
    fail("Prepare load insert failed: {$mysqli->error}");
}
$t0 = microtime(true);
for ($i = 1; $i <= $iterations; $i++) {
    $v = $i % 1000;
    $loadInsert->bind_param("ii", $i, $v);
    if (!$loadInsert->execute()) {
        fail("Execute load insert failed at {$i}: {$loadInsert->error}");
    }
}
$loadInsert->close();
$t1 = microtime(true);
ok("load inserts ok: {$iterations} rows in " . sprintf("%.3f", $t1 - $t0) . "s");

// Concurrency test (requires pcntl).
$workers = intval(getenv("MANTICORE_WORKERS") ?: "4");
$perWorker = intval(getenv("MANTICORE_WORKER_ITERS") ?: "200");
if (function_exists("pcntl_fork")) {
    $pids = [];
    for ($w = 0; $w < $workers; $w++) {
        $pid = pcntl_fork();
        if ($pid === -1) {
            fail("pcntl_fork failed");
        }
        if ($pid === 0) {
            $m = new mysqli($host, $user, $pass, $db, $port);
            if ($m->connect_errno) {
                fail("Worker connect failed: {$m->connect_error}");
            }
            $stmt = $m->prepare("INSERT INTO ps_load (id, val) VALUES (?, ?)");
            if (!$stmt) {
                fail("Worker prepare failed: {$m->error}");
            }
            $base = 1000000 + ($w * $perWorker);
            for ($i = 0; $i < $perWorker; $i++) {
                $id = $base + $i;
                $val = $id % 1000;
                $stmt->bind_param("ii", $id, $val);
                if (!$stmt->execute()) {
                    fail("Worker execute failed: {$stmt->error}");
                }
            }
            $stmt->close();
            $m->close();
            exit(0);
        }
        $pids[] = $pid;
    }
    foreach ($pids as $pid) {
        pcntl_waitpid($pid, $status);
    }
    ok("concurrency ok: {$workers} workers x {$perWorker}");
} else {
    ok("pcntl not available; skipped concurrency test");
}

$mysqli->close();
