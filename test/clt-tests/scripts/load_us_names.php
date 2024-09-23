#!/usr/bin/php
<?php
if (count($argv) < 5) die("Usage: ".__FILE__." <batch size> <concurrency> <docs> <multiplier>\n");

function connectDb() {
    $host = '127.0.0.1';
    $port = '9306';

    $m = @mysqli_connect($host, '', '', '', $port);
    if (mysqli_connect_error()) {
        die("Cannot connect to Manticore\n");
    }
    return $m;
}

function process($link, $query) {
    $result = mysqli_query($link, $query);
    if (!$result) {
        echo "Request Execution Error: " . mysqli_error($link) . "\n";
        return false;
    }
    return true;
}

$all_links = [];
for ($i = 0; $i < $argv[2]; $i++) {
    $all_links[] = connectDb();
}

// init
$pdo = $all_links[0];
mysqli_query($pdo, "DROP TABLE IF EXISTS name");
mysqli_query($pdo, "CREATE TABLE name(id INTEGER, username TEXT) min_infix_len='2' expand_keywords='1'");

$batch = [];
$query_start = "INSERT INTO name(id, username) VALUES ";
$names = file('./test/clt-tests/comparison-overhead-json-sql/names.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$namesCount = count($names);
$surnames = file('./test/clt-tests/comparison-overhead-json-sql/surnames.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$surnamesCount = count($surnames);

echo "preparing...\n";
srand(1);
$cache_file_name = '/tmp/' . md5($query_start) . '_' . $argv[1] . '_' . $argv[3];
$c = 0;

if (!file_exists($cache_file_name)) {
    $batches = [];
    while ($c < $argv[3]) {
        $batch[] = "($c,'" . $names[rand(0, $namesCount - 1)] . ' ' . $surnames[rand(0, $surnamesCount - 1)] . "')";
        $c++;
        if ($c % 1000 == 0) echo "\r" . ($c / $argv[3] * 100) . "%       ";
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
    $link = $all_links[array_rand($all_links)];
    if (!process($link, $batch)) die("ERROR\n");
}

echo "finished inserting\n";
echo "Total time: " . (microtime(true) - $t) . "\n";
echo round($argv[3] * $argv[4] / (microtime(true) - $t)) . " docs per sec\n";

?>
