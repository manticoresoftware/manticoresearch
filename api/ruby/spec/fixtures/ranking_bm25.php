<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetRankingMode(SPH_RANK_BM25);
$cl->Query('query');

?>