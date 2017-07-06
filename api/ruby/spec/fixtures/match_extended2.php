<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetMatchMode(SPH_MATCH_EXTENDED2);
$cl->Query('query');

?>