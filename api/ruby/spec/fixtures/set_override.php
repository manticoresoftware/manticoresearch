<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetOverride('attr1', SPH_ATTR_INTEGER, array(10 => 20));
$cl->SetOverride('attr2', SPH_ATTR_FLOAT, array(11 => 30.3));
$cl->SetOverride('attr3', SPH_ATTR_BIGINT, array(12 => 1099511627780));
$cl->Query('query');

?>
