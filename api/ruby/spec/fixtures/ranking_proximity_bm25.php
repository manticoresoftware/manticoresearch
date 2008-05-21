<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetRankingMode(SPH_RANK_PROXIMITY_BM25);
$cl->Query('query');

?>