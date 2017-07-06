<?php

require ("sphinxapi.php");

$cl = new SphinxClient();
$cl->BuildExcerpts(array('10', '20'), 'index', 'word1 word2', array('before_match' => 'before',
                                                                    'after_match' => 'after',
                                                                    'chunk_separator' => 'separator',
                                                                    'limit' => 10));

?>