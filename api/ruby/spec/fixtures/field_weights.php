<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetFieldWeights(array('field1' => 10, 'field2' => 20));
$cl->Query('query');

?>