#!/usr/bin/php
<?php

// Default values
$defaults = [
    'batch-size' => 1000,
    'concurrency' => 4,
    'docs' => 1000000,
    'start-id' => 1,
    'port' => 9306,
    'drop-table' => true
];

// Parse command-line arguments
$options = [
    'batch-size::',
    'concurrency::',
    'docs::',
    'min-infix-len::',
    'start-id::',
    'rt-mem-limit::',
    'port::',
    'drop-table',
    'no-drop-table',
    'help'
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
    echo "  --rt-mem-limit=<value>     Set the real-time memory limit (e.g., 256M, default: none)\n";
    echo "  --port=<number>            MySQL port to connect to (default: {$defaults['port']})\n";
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

// Set rt-mem-limit argument if specified, with quotes around the value
$rtMemLimit = isset($args['rt-mem-limit']) ? "rt_mem_limit='" . $args['rt-mem-limit'] . "'" : "";

// Determine drop-table behavior
$dropTable = !isset($args['no-drop-table']);
$batchSize = intval($args['batch-size']);
$concurrency = intval($args['concurrency']);
$docs = intval($args['docs']);
$startId = intval($args['start-id']);
$port = intval($args['port']);

// Connect to database
function connectDb($port) {
    $host = '127.0.0.1';
    $m = @mysqli_connect($host, '', '', '', $port);
    if (mysqli_connect_error()) {
        die("Cannot connect to Manticore on port $port: " . mysqli_connect_error() . "\n");
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
    $all_links[] = connectDb($port);
}

// Initialize table if drop-table is true
$pdo = $all_links[0];
if ($dropTable) {
    mysqli_query($pdo, "DROP TABLE IF EXISTS name");
    echo "Table 'name' dropped and recreated.\n";
    $createTableQuery = "CREATE TABLE name (
        id BIGINT,
        username TEXT,
        gender STRING,
        age INTEGER,
        salary FLOAT,
        discount FLOAT,
        is_active BOOL,
        last_login TIMESTAMP,
        product_codes MULTI,
        large_values MULTI64,
        additional_info JSON,
        location_vector FLOAT_VECTOR KNN_TYPE='hnsw' KNN_DIMS='4' HNSW_SIMILARITY='l2'
    ) $minInfixLen expand_keywords='1'";
    if ($rtMemLimit) {
        $createTableQuery .= " $rtMemLimit";
    }
    mysqli_query($pdo, $createTableQuery) or die("Create table failed: " . mysqli_error($pdo));
}

$batch = [];
$query_start = "INSERT INTO name(id, username, gender, age, salary, discount, is_active, last_login, product_codes, large_values, additional_info, location_vector) VALUES ";
$names = file('./test/clt-tests/scripts/names.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$namesCount = count($names);
$surnames = file('./test/clt-tests/scripts/surnames.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$surnamesCount = count($surnames);

// Sample data for generation
$genders = ['male', 'female'];
$cities = ['New York', 'London', 'Tokyo', 'Moscow', 'Sydney'];
$hobbies = ['reading', 'gaming', 'sports', 'traveling', 'cooking'];

// Ensure repeatable random data
srand(42); // Required for other tasks
mt_srand(42); // Fixed seed for mt_rand
echo "preparing...\n";
$c = $startId; // Start at the specified start ID

$batches = [];
while ($c < $startId + $docs) {
    $username = $names[($c - 1) % $namesCount] . ' ' . $surnames[($c - 1) % $surnamesCount];
    $gender = $genders[($c - 1) % count($genders)];
    $age = mt_rand(18, 65);
    $salary = mt_rand(30000, 120000) + mt_rand(0, 99) / 100.0;
    $discount = mt_rand(0, 50) + mt_rand(0, 99) / 100.0;
    $is_active = mt_rand(0, 1);
    $last_login = 1700000000 - mt_rand(0, 30 * 24 * 60 * 60); // Fixed base timestamp
    $product_codes = '(' . mt_rand(100, 999) . ',' . mt_rand(100, 999) . ')';
    $large_values = '(' . mt_rand(1000000, 9999999) . ',' . mt_rand(1000000, 9999999) . ')';
    $additional_info = json_encode([
        'city' => $cities[($c - 1) % count($cities)],
        'hobby' => $hobbies[($c - 1) % count($hobbies)]
    ]);
    $location_vector = '(' . implode(',', array(
        mt_rand(-90, 90) + mt_rand(0, 99) / 100.0,
        mt_rand(-180, 180) + mt_rand(0, 99) / 100.0,
        mt_rand(0, 1000) / 1000.0,
        mt_rand(0, 1000) / 1000.0
    )) . ')';

    $batch[] = "($c,'$username','$gender',$age,$salary,$discount,$is_active,$last_login,$product_codes,$large_values,'$additional_info',$location_vector)";
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
