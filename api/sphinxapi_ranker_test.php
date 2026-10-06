<?php

require __DIR__ . '/sphinxapi.php';

function check_request ( $client, $flags, $ranker )
{
	$client->AddQuery ( 'query' );
	$values = unpack ( 'Nflags/Noffset/Nlimit/Nmode/Nranker', $client->_reqs[0] );
	if ( $values['flags']!=$flags || $values['ranker']!=$ranker )
		throw new RuntimeException ( "unexpected request: flags={$values['flags']} ranker={$values['ranker']}" );
	$client->_reqs = array();
}

$client = new SphinxClient();
$defaults = (1<<3) | (1<<6);
check_request ( $client, $defaults, SPH_RANK_PROXIMITY_BM25 );
$client->SetRankingMode ( SPH_RANK_PROXIMITY_BM25 );
check_request ( $client, $defaults | (1<<15), SPH_RANK_PROXIMITY_BM25 );
$client->SetRankingMode ( SPH_RANK_BM25 );
check_request ( $client, $defaults | (1<<15), SPH_RANK_BM25 );
$client->SetQueryFlag ( 'boolean_simplify', true );
$client->SetQueryFlag ( 'boolean_simplify', false );
check_request ( $client, (1<<6) | (1<<15), SPH_RANK_BM25 );
$client->ResetQueryFlag();
check_request ( $client, (1<<6) | (1<<15), SPH_RANK_BM25 );
