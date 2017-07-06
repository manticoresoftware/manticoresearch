<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetSortMode(SPH_SORT_EXTENDED, 'sortby');
$cl->Query('query');

?>