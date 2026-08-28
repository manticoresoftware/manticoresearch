#!/usr/bin/env manticore-executor
<?php
srand(1024);

$docs_per_chunk = $argv[1];
$chunks = $argv[2];

echo "DROP TABLE IF EXISTS t; create table t(id int, j json);\n";
echo "insert into t(id, j) values (1, '{\"a\": \"a\", \"b\": \"a\", \"c\": \"a\", \"d\": \"a\"}');\n";
echo "insert into t(id, j) values (2, '{\"a\": \"b\", \"b\": \"b\", \"c\": \"b\", \"d\": \"b\"}')";

for ($id = 3; $id < $docs_per_chunk * $chunks; $id++) {
  if ($id % $docs_per_chunk === 0) {
    echo ";\n";
    echo "FLUSH RAMCHUNK t;\n";
    echo "insert into t (id, j) values \n";
    
  } elseif  ($id % 100 === 0) {
    echo ";\n";
    echo "insert into t (id, j) values \n";
    
  } else {
    echo "\n,";
  }

  $rand1 = rand(1, 200);
  $rand2 = rand(1, 10000);
  $rand3 = rand(1, 1000);
  $rand4 = rand();

  echo "(" . $id . ", '{\"a\": $rand1, \"b\": $rand2, \"c\": $rand3, \"d\": $rand4}' )";
}

echo ";\n";
echo "FLUSH RAMCHUNK t;\n";
