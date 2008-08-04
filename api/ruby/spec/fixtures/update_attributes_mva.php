<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->UpdateAttributes('index', array('group', 'category'), array(123 => array(array(456, 789), array(1, 2, 3))), true);

?>
