<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetFilterRange('attr1', -10, 20);
$cl->SetFilterRange('attr2', -1099511627770, 1099511627780);
$cl->Query('query');

?>
