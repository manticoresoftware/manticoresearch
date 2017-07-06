<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetRetries(10);
$cl->Query('query');

?>