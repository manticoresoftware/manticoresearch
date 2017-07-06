<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetFilterFloatRange('attr', 10.5, 20.3, true);
$cl->Query('query');

?>