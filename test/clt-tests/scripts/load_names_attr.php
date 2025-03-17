#!/usr/bin/php
<?php

// Default values
$defaults = [
    'batch-size' => 1000,
    'concurrency' => 4,
    'docs' => 1000000,
    'start-id' => 1,
    'drop-table' => true  // Drop and create table by default
];

// Parse command-line arguments
$options = [
    'batch-size::',     // Optional argument, controlled by --batch-size=
    'concurrency::',    // Optional argument, controlled by --concurrency=
    'docs::',           // Optional argument, controlled by --docs=
    'min-infix-len::',  // Optional argument, controlled by --min-infix-len=
    'start-id::',       // Optional argument, controlled by --start-id=
    'rt-mem-limit::',   // Optional argument, controlled by --rt-mem-limit=
    'drop-table',       // Flag argument, controlled by --drop-table (default true)
    'no-drop-table',    // Flag to disable dropping and creating the table
    'help'              // Help argument
];
$args = getopt("", $options);

// Display help if --help is passed
if (isset($args['help'])) {
    echo "Usage: " . __FILE__ . " [options]\n";
    echo "Options:\n";
    echo "  --batch-size=<number>      Number of records per batch (default: {$defaults['batch-size']})\n";
    echo "  --concurrency=<number>     Number of concurrent connections (default: {$defaults['concurrency']})\n";
    echo "  --docs=<number>            Total number of documents to insert (default: {$defaults['docs']})\n";
    echo "  --min-infix-len=<number>   Optional minimum infix length for table (default: none, minimum: 2)\n";
    echo "  --start-id=<number>        Starting ID for document insertion (default: {$defaults['start-id']})\n";
    echo "  --rt-mem-limit=<value>     Set the real-time memory limit (optional, default: none)\n";
    echo "  --drop-table               Drop and create the table before inserting data (default: true)\n";
    echo "  --no-drop-table            Prevent the table from being dropped and created\n";
    echo "  --help                     Show this help message\n";
    exit(0);
}

// Apply defaults for any arguments not specified
$args = array_merge($defaults, $args);

// Validate min-infix-len argument
if (isset($args['min-infix-len'])) {
    $minInfixLenValue = intval($args['min-infix-len']);
    if ($minInfixLenValue < 2) {
        die("Error: --min-infix-len cannot be less than 2.\n");
    }
    $minInfixLen = "min_infix_len='" . $minInfixLenValue . "'";
} else {
    $minInfixLen = "";
}

// Set rt-mem-limit argument if specified
$rtMemLimit = isset($args['rt-mem-limit']) ? "rt_mem_limit=" . $args['rt-mem-limit'] : "";

// Determine drop-table behavior
$dropTable = !isset($args['no-drop-table']); // Drop and create table unless --no-drop-table is specified
$batchSize = intval($args['batch-size']);
$concurrency = intval($args['concurrency']);
$docs = intval($args['docs']);
$startId = intval($args['start-id']);

// Connect to database
function connectDb() {
    $host = '127.0.0.1';
    $port = '9306';

    $m = @mysqli_connect($host, '', '', '', $port);
    if (mysqli_connect_error()) {
        die("Cannot connect to Manticore\n");
    }
    return $m;
}

// Execute a query
function process($link, $query) {
    $result = mysqli_query($link, $query);
    if (!$result) {
        echo "Request Execution Error: " . mysqli_error($link) . "\n";
        return false;
    }
    return true;
}

// Create connections
$all_links = [];
for ($i = 0; $i < $concurrency; $i++) {
    $all_links[] = connectDb();
}

// Initialize table if drop-table is true
$pdo = $all_links[0];
if ($dropTable) {
    mysqli_query($pdo, "DROP TABLE IF EXISTS name");
    echo "Table 'name' dropped and recreated.\n";
    $createTableQuery = "CREATE TABLE name(id INTEGER, username TEXT, s STRING) $minInfixLen expand_keywords='1'";
    if ($rtMemLimit) {
        $createTableQuery .= " $rtMemLimit";
    }
    mysqli_query($pdo, $createTableQuery);
}

$batch = [];
$query_start = "INSERT INTO name(id, username, s) VALUES ";
$names = file('./test/clt-tests/scripts/names.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$namesCount = count($names);
$surnames = file('./test/clt-tests/scripts/surnames.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$surnamesCount = count($surnames);

// Ensure repeatable random data
srand(42);
mt_srand(42);

echo "preparing...\n";
$c = $startId; // Start at the specified start ID

$batches = [];
while ($c < $startId + $docs) {
    $batch[] = "($c,'" . $names[mt_rand(0, $namesCount - 1)] . ' ' . $surnames[mt_rand(0, $surnamesCount - 1)] . "', 'a')";
    $c++;
    if (($c - $startId) % 1000 == 0) echo "\r" . (($c - $startId) / $docs * 100) . "%       ";
    if (count($batch) == $batchSize) {
        $batches[] = $query_start . implode(',', $batch);
        $batch = [];
    }
}
if ($batch) $batches[] = $query_start . implode(',', $batch);

echo "querying...\n";

$t = microtime(true);

foreach ($batches as $batch) {
    $link = $all_links[array_rand($all_links)];
    if (!process($link, $batch)) die("ERROR\n");
}

echo "finished inserting\n";
echo "Total time: " . (microtime(true) - $t) . "\n";
echo round($docs / (microtime(true) - $t)) . " docs per sec\n";

?>
