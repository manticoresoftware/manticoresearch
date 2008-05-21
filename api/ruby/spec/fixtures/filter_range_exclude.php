<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetFilterRange('attr', 10, 20, true);
$cl->Query('query');

?>