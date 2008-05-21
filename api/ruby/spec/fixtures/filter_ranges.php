<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetFilterRange('attr2', 30, 40);
$cl->SetFilterRange('attr1', 10, 20);
$cl->Query('query');

?>