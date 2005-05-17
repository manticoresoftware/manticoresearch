<?php

//
// $Id$
//

//
// Copyright (c) 2001-2005, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

/////////////////////////////////////////////////////////////////////////////
// Sphinx PHP API
/////////////////////////////////////////////////////////////////////////////

/// this functions connects to sphinx searchd server,
/// executes given query, and returns search results as a hash
///
/// $server is searchd server IP address or hostname
/// $port is searchd server port
/// $query is query string
/// $start is offset into the result set to start retrieveing from
/// $rpp is result count to retrieve (Rows Per Page)
/// $weights is an array of weights for each index field
/// $any is search mode, false to match all words and true to match any word
/// $groups is an array of groups to limit matching to
///
/// returns false on failure
/// returns hash which has the following keys on success:
///		"matches"
///			hash which maps found document_id to ( "weight", "group" ) hash
///		"total"
///			total matches count
///		"time"
///			search time
///		"words"
///			hash which maps query terms (stemmed!) to ( "docs", "hits" ) hash

function sphinxQuery ( $server, $port, $query, $start=0, $rpp=20,
	$weights=array(), $any=false, $groups=array() )
{
	$start = (int)$start;
	$rpp = (int)$rpp;

	if (!( $fp = @fsockopen ( $server, $port ) ) )
		return false;

	// check version
	$s = trim ( fgets ( $fp, 1024 ) );
	if ( $s!="VER 1" )
	{
		fclose ( $fp );
		return false;
	}

	// build request
	$req = pack ( "VVV", $start, $rpp, $any ? 1 : 0 ); // mode/limits part
	$req .= pack ( "V", count($groups) ); // groups
	foreach ( $groups as $group )
		$req .= pack ( "V", $group );
	$req .= pack ( "V", strlen($query) ) . $query; // query string
	$req .= pack ( "V", count($weights) ); // weights
	foreach ( $weights as $weight )
		$req .= pack ( "V", (int)$weight );

	// do query
	fputs ( $fp, $req );

	$result = array();
	while ( !feof ( $fp ) )
	{
		$s = trim ( fgets ( $fp, 1024 ) );
		if ( substr ( $s, 0, 6 )=="MATCH " )
		{
			list ( $dummy, $group, $doc, $weight ) = explode ( " ", $s );
			$result["matches"][$doc] = array ( "weight" => $weight, "group" => $group );

		} elseif ( substr ( $s, 0, 6 )=="TOTAL " )
		{
			$result["total"] = substr($s, 6);

		} elseif ( substr ( $s, 0, 5 )=="TIME " )
		{
			$result["time"] = substr ( $s, 5 );

		} elseif ( substr ( $s, 0, 5 ) == "WORD " )
		{
			list ( $dummy, $word, $docs, $hits ) = explode ( " ", $s );
			$result["words"][$word]["docs"] = $docs;
			$result["words"][$word]["hits"] = $hits;
		}

		// for now, simply ignore unknown response
	}

	fclose ( $fp );
	return $result;
}

//
// $Id$
//

?>