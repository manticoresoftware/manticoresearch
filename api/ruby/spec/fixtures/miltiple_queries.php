<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetRetries(10, 20);
$cl->AddQuery('test1');
$cl->SetGroupBy('attr', SPH_GROUPBY_DAY);
$cl->AddQuery('test2');
$cl->RunQueries();

?>