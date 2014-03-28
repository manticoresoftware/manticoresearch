<?php

$db = new mysqli('0', '', '', 'testdb', 9306);
$result = $db->query('SELECT * FROM rt');
$row = $result->fetch_row();
printf("%s | %s | %s | %s\n", $row[0], $row[1], $row[2], $row[3]);

?>