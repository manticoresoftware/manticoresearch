<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetLimits(10, 20);
$cl->Query('query');

?>