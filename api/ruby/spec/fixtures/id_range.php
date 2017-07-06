<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetIDRange(10, 20);
$cl->Query('query');

?>