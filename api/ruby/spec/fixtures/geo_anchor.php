<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->SetGeoAnchor('attrlat', 'attrlong', 20.3, 40.7);
$cl->Query('query');

?>