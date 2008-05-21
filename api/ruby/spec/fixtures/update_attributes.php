<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->UpdateAttributes('index', array('group'), array(123 => array(456)));

?>