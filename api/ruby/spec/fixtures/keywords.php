<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->BuildKeywords('test', 'index', true);

?>