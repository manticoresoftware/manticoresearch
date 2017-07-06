<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetIDRange(8589934591, 17179869183);
$cl->Query('query');

?>