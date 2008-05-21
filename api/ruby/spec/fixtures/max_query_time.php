<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetMaxQueryTime(1000);
$cl->Query('query');

?>