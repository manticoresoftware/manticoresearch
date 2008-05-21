<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetSortMode(SPH_SORT_TIME_SEGMENTS, 'sortby');
$cl->Query('query');

?>