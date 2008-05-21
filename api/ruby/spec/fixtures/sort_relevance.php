<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetSortMode(SPH_SORT_RELEVANCE);
$cl->Query('query');

?>