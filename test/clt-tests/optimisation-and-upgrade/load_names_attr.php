#!/usr/bin/php
<?php
print_r($argv);

if (count($argv) < 6) {
    die("Usage: " . __FILE__ . " <batch size> <concurrency> <docs> <multiplier> <num_chunks>\n");
}

$num_chunks = (int)$argv[5];
$docs_per_chunk = (int)$argv[3] / $num_chunks;
$c = 1;

for ($chunk = 1; $chunk <= $num_chunks; $chunk++) {
    $end_id = $c + $docs_per_chunk - 1;

    echo "Inserting chunk $chunk (IDs $c to $end_id)...\n";

    while ($c <= $end_id) {
        $batch[] = "($c,'".$names[rand(0, $namesCount - 1)].' '.$surnames[rand(0, $surnamesCount - 1)]."', 'a')";
        $c++;
        if (floor($c/1000) == $c/1000) echo "\r".(($c - 1)/$docs_per_chunk*100)."%       ";
        if (count($batch) == $argv[1]) {
            $batches[] = $query_start.implode(',', $batch);
            $batch = [];
        }
    }

    echo "Flushing RAMCHUNK...\n";
    if (!mysqli_query($all_links[0], "FLUSH RAMCHUNK;")) {
        echo "Error during FLUSH RAMCHUNK: " . mysqli_error($all_links[0]) . "\n";
    }
}
/*
Requires:

wget https://raw.githubusercontent.com/zeraye/names-surnames-list/master/male-names-list.txt > names.txt
wget https://raw.githubusercontent.com/zeraye/names-surnames-list/master/female-names-list.txt >> names.txt
wget https://raw.githubusercontent.com/zeraye/names-surnames-list/master/surnames-list.txt > surnames.txt
*/

// This function waits for an idle mysql connection for the $query, runs it and exits
function process($query) {
    global $all_links;
    global $requests;
    foreach ($all_links as $k=>$link) {
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
            foreach ($links as $j=>$link) {
                $res = @mysqli_reap_async_query($links[$j]);
                foreach ($all_links as $i=>$link_orig) if ($all_links[$i] === $links[$j]) break;
                if ($link->error) {
                    echo "ERROR: {$link->error}\n";
                    if (!mysqli_ping($link)) {
                        echo "ERROR: mysql connection is down, removing it from the pool\n";
                        unset($all_links[$i]); // remove the original link from the pool
                        unset($requests[$i]); // and from the $requests too
                    }
                    return false;
                }
                if ($res === false and !$link->error) continue;
                if (is_object($res)) {
                    mysqli_free_result($res);
                }
                $requests[$i] = microtime(true);
        mysqli_query($link, $query, MYSQLI_ASYNC); // making next query
                return true;
            }
        };
    } while (true);
    return true;
}

$all_links = [];
$requests = [];
$c = 0;
for ($i=0;$i<$argv[2];$i++) {
  $m = @mysqli_connect('127.0.0.1', '', '', '', 9306);
      if (mysqli_connect_error()) die("Cannot connect to Manticore\n");
      $all_links[] = $m;
  }

// init
echo "Dropping existing table if it exists...\n";
mysqli_query($all_links[0], "drop table if exists name");

echo "Creating table 'name'...\n";
mysqli_query($all_links[0], "create table name(username text, s string) min_infix_len='2' expand_keywords='1'");
if (mysqli_errno($all_links[0])) {
    die("Error creating table: " . mysqli_error($all_links[0]) . "\n");
}

$result = mysqli_query($all_links[0], "SHOW TABLES LIKE 'name';");
if (mysqli_num_rows($result) == 0) {
    die("Error: Table 'name' does not exist after creation.\n");
}
echo "Table 'name' created successfully.\n";

$batch = [];
$query_start = "insert into name(id, username, s) values ";
$names = file('./test/clt-tests/optimisation-and-upgrade/names.txt', FILE_IGNORE_NEW_LINES|FILE_SKIP_EMPTY_LINES);
$namesCount = count($names);
$surnames = file('./test/clt-tests/optimisation-and-upgrade/surnames.txt', FILE_IGNORE_NEW_LINES|FILE_SKIP_EMPTY_LINES);
$surnamesCount = count($surnames);

echo "preparing...\n";
$error = false;
$cache_file_name = '/tmp/'.md5($query_start).'_'.$argv[1].'_'.$argv[3];
$c = 0;
if (!file_exists($cache_file_name)) {
    $batches = [];
    while ($c < $argv[3]) {
      $batch[] = "($c,'".$names[rand(0, $namesCount - 1)].' '.$surnames[rand(0, $surnamesCount - 1)]."', 'a')";
      $c++;
      if (floor($c/1000) == $c/1000) echo "\r".($c/$argv[3]*100)."%       ";
        if (count($batch) == $argv[1]) {
          $batches[] = $query_start.implode(',', $batch);
          $batch = [];
        }
    }
    if ($batch) $batches[] = $query_start.implode(',', $batch);
    file_put_contents($cache_file_name, serialize($batches));
} else {
    echo "found in cache\n";
    $batches = unserialize(file_get_contents($cache_file_name));
}

$batchesMulti = [];
for ($n=0;$n<$argv[4];$n++) $batchesMulti = array_merge($batchesMulti, $batches);
$batches = $batchesMulti;

echo "querying...\n";

$t = microtime(true);

foreach ($batches as $batch) {
  if (!process($batch)) die("ERROR\n");
}

// wait until all the workers finish
do {
  $links = $errors = $reject = array();
  foreach ($all_links as $link)  $links[] = $errors[] = $reject[] = $link;
  $count = @mysqli_poll($links, $errors, $reject, 0, 100);
} while (count($all_links) != count($links) + count($errors) + count($reject));

echo "finished inserting\n";
echo "Total time: ".(microtime(true) - $t)."\n";
echo round($argv[3] * $argv[4] / (microtime(true) - $t))." docs per sec\n";
