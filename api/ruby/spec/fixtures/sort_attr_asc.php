<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetSortMode(SPH_SORT_ATTR_ASC, 'sortby');
$cl->Query('query');

?>