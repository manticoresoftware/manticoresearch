#!/usr/bin/php
<?php
if (count($argv) < 6) die("Usage: ".__FILE__." <batch size> <concurrency> <docs> <multiplier> <rt_mem_limit kb>\n");

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
mysqli_query($all_links[0], "drop table if exists name");
$query = "create table name(username text) min_infix_len='2' expand_keywords='1'";
if (!empty($argv[5])) {
	$query .= " rt_mem_limit='" . (int)$argv[5] . "k'";
}
mysqli_query($all_links[0], $query);

$batch = [];
$query_start = "insert into name(id, username) values ";
$names = file('./test/clt-tests/comparison-overhead-json-sql/names.txt', FILE_IGNORE_NEW_LINES|FILE_SKIP_EMPTY_LINES);
$namesCount = count($names);
$surnames = file('./test/clt-tests/comparison-overhead-json-sql/surnames.txt', FILE_IGNORE_NEW_LINES|FILE_SKIP_EMPTY_LINES);
$surnamesCount = count($surnames);

echo "preparing...\n";
$error = false;
$cache_file_name = '/tmp/'.md5($query_start).'_'.$argv[1].'_'.$argv[3];
$c = 0;
if (!file_exists($cache_file_name)) {
    $batches = [];
    while ($c < $argv[3]) {
      $batch[] = "($c,'".$names[rand(0, $namesCount - 1)].' '.$surnames[rand(0, $surnamesCount - 1)]."')";
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
