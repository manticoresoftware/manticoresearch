#!/usr/bin/env manticore-executor
<?php
srand(1024);

$id=0;
echo "create table t(f text, a int, b float, j json, m multi, s string);\n";
for ($i = 0; $i < 1000000; $i++) {
  if ($i % 10000 === 0) {
    if ($i !== 0) {
      echo ";\n";
    }
    echo 'insert into t (id, f, a, b, j, m, s) values ';
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
  $s = str_repeat('a', $rand4 % 10);
  $random_text = '';
  for ($j = 0; $j < 3; $j++) {
    $n = $j + 2;
    $var = "rand$n";
    $random_text .= chr($$var % 26 + 65 + $$var % 2 * 32);
  }
  echo "(" . ++$id . ", '$s', $rand1, $rand2, '{\"a\": [$rand3,$rand4], \"b\": $rand5}', ($rand6,$rand7,$rand8), '$random_text')";
}
