<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetFilter('attr2', array(40, 50));
$cl->SetFilter('attr1', array(10, 20, 30));
$cl->Query('query');

?>