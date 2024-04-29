#!/usr/bin/env manticore-executor
<?php
srand(1024);

$id=0;
echo "create table t(id int, f text, a int, b float, j json, m multi, s string, e BOOL, d TIMESTAMP, v MULTI64, fv float_vector);\n";
echo "insert into t (id) values (0);\n";
for ($i = 0; $i < $argv[1]; $i++) {
  if ($i % 10000 === 0) {
    if ($i !== 0) {
      echo ";\n";
    }
    echo 'insert into t (id, f, a, b, j, m, s, e, d, v, fv) values ';
  } else {
    echo ',';
  }

  $rand1 = rand() / 10000;
  $rand2 = rand();
  $rand3 = rand();
  $rand4 = rand();
  $rand5 = rand();
  $rand6 = rand();
  $rand7 = rand();
  $rand8 = rand();
  $rand85 = rand(0,1);
  $rand9 = rand(1, 1713433221);
  $rand10 = rand();
  $rand11 = rand();
  $s = str_repeat('b', rand(0,100));

echo "(" . ++$id . ", '$s', $rand1, $rand2, '{\"a\": [$rand3,$rand4], \"b\": $rand5}', ($rand6,$rand7,$rand8), '$s', $rand85, $rand9, ($rand10, $rand10), ($rand11))";
}
