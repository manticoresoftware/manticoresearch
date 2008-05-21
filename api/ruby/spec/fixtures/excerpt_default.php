<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->BuildExcerpts(array('10', '20'), 'index', 'word1 word2');

?>