<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetFilterRange('attr1', 10, 20, true);
$cl->SetFilter('attr3', array(30, 40, 50));
$cl->SetFilterRange('attr1', 60, 70);
$cl->SetFilter('attr2', array(80, 90, 100), true);
$cl->SetFilterFloatRange('attr1', 60.8, 70.5);
$cl->Query('query');

?>