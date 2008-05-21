<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->BuildExcerpts(array('10', '20'), 'index', 'word1 word2', array('exact_phrase' => true,
                                                                    'single_passage' => true,
                                                                    'use_boundaries' => true,
                                                                    'weight_order' => true));

?>