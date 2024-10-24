#!/usr/bin/php
<?php
// Minimum number of arguments
if (count($argv) < 5) die("Usage: ".__FILE__." <batch size> <concurrency> <docs> <multiplier> [rt_mem_limit] [initial_id] [min_infix_len] [async]\n");

// Arguments for configuration
$rt_mem_limit = $argv[5] ?? null;
$initial_id = $argv[6] ?? 0;
$min_infix_len = isset($argv[7]) ? "min_infix_len='" . intval($argv[7]) . "'" : "";
$async = isset($argv[8]) && $argv[8] == 'async';

// Set fixed seed for random data generation
srand(1);

// Function for deterministic data generation
function get_name($names, $surnames, $namesCount, $surnamesCount, $index) {
    return $names[$index % $namesCount] . ' ' . $surnames[$index % $surnamesCount];
}

// Function to connect to the database
function connectDb() {
    $host = '127.0.0.1';
    $port = '9306';
    $m = @mysqli_connect($host, '', '', '', $port);
    if (mysqli_connect_error()) {
        die("Cannot connect to Manticore\n");
    }
    return $m;
}

// Function to process asynchronous queries
function process_async($query) {
    global $all_links;
    global $requests;
    foreach ($all_links as $k => $link) {
        if (@$requests[$k]) continue;
        mysqli_query($link, $query, MYSQLI_ASYNC);
        @$requests[$k] = microtime(true);
        return true;
    }
    do {
        $links = $errors = $reject = array();
        foreach ($all_links as $link) {
            $links[] = $errors[] = $reject[] = $link;
        }
        $count = @mysqli_poll($links, $errors, $reject, 0, 1000);
        if ($count > 0) {
            foreach ($links as $j => $link) {
                $res = @mysqli_reap_async_query($link);
                foreach ($all_links as $i => $link_orig) if ($all_links[$i] === $link) break;
                if ($link->error) {
                    echo "ERROR: {$link->error}\n";
                    unset($all_links[$i]);
                    unset($requests[$i]);
                    return false;
                }
                if (is_object($res)) {
                    mysqli_free_result($res);
                }
                $requests[$i] = microtime(true);
                mysqli_query($link, $query, MYSQLI_ASYNC);
                return true;
            }
        }
    } while (true);
    return true;
}

// Function to process synchronous queries
function process_sync($link, $query) {
    $result = mysqli_query($link, $query);
    if (!$result) {
        echo "Request Execution Error: " . mysqli_error($link) . "\n";
        return false;
    }
    return true;
}

$all_links = [];
$requests = [];
$c = $initial_id;
for ($i = 0; $i < $argv[2]; $i++) {
    $all_links[] = connectDb();
}

// Table creation
if ($initial_id == 1) {
    $create_table_query = "CREATE TABLE IF NOT EXISTS name(username text, s string) $min_infix_len expand_keywords='1'";
    if ($rt_mem_limit) {
        $create_table_query .= " rt_mem_limit='{$rt_mem_limit}'";
    }
    mysqli_query($all_links[0], $create_table_query) or die(mysqli_error($all_links[0]));
}

$batch = [];
$query_start = "INSERT INTO name(id, username, s) VALUES ";
$names = file('./test/clt-tests/scripts/names.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$namesCount = count($names);
$surnames = file('./test/clt-tests/scripts/surnames.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$surnamesCount = count($surnames);

echo "preparing...\n";
$cache_file_name = '/tmp/' . md5($query_start) . '_' . $argv[1] . '_' . $argv[3] . '_from_' . $initial_id;

if (!file_exists($cache_file_name)) {
    $batches = [];
    while ($c < $initial_id + $argv[3]) {
        $batch[] = "($c,'" . get_name($names, $surnames, $namesCount, $surnamesCount, $c) . "', 'a')";
        $c++;
        if ($c % 1000 == 0) echo "\r" . (($c - $initial_id) / $argv[3] * 100) . "%       ";
        if (count($batch) == $argv[1]) {
            $batches[] = $query_start . implode(',', $batch);
            $batch = [];
        }
    }
    if ($batch) $batches[] = $query_start . implode(',', $batch);
    file_put_contents($cache_file_name, serialize($batches));
} else {
    echo "found in cache\n";
    $batches = unserialize(file_get_contents($cache_file_name));
}

$batchesMulti = [];
for ($n = 0; $n < $argv[4]; $n++) $batchesMulti = array_merge($batchesMulti, $batches);
$batches = $batchesMulti;

echo "querying...\n";
$t = microtime(true);

foreach ($batches as $batch) {
    if ($async) {
        if (!process_async($batch)) die("ERROR\n");
    } else {
        $link = $all_links[array_rand($all_links)];
        if (!process_sync($link, $batch)) die("ERROR\n");
    }
}

// Finish async queries (if async mode is enabled)
if ($async) {
    do {
        $links = $errors = $reject = array();
        foreach ($all_links as $link) $links[] = $errors[] = $reject[] = $link;
        $count = @mysqli_poll($links, $errors, $reject, 0, 100);
    } while (count($all_links) != count($links) + count($errors) + count($reject));
}

echo "finished inserting\n";
echo "Total time: " . (microtime(true) - $t) . "\n";
echo round($argv[3] * $argv[4] / (microtime(true) - $t)) . " docs per sec\n";

?>
