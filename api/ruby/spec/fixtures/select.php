<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetSelect('attr1, attr2');
$cl->Query('query');

?>
