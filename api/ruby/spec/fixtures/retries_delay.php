<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetRetries(10, 20);
$cl->Query('query');

?>