<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetIndexWeights(array('index1' => 10, 'index2' => 20));
$cl->Query('query');

?>