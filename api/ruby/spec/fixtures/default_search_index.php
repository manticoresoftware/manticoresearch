<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->Query('query', 'index');

?>