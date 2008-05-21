<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetSortMode(SPH_SORT_ATTR_DESC, 'sortby');
$cl->Query('query');

?>