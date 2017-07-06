<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetFilter('attr', array(10, 20, 30), true);
$cl->Query('query');

?>