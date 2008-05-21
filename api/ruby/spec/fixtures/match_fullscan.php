<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetMatchMode(SPH_MATCH_FULLSCAN);
$cl->Query('query');

?>