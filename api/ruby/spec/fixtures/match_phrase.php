<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetMatchMode(SPH_MATCH_PHRASE);
$cl->Query('query');

?>