<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetRankingMode(SPH_RANK_PROXIMITY);
$cl->Query('query');

?>
