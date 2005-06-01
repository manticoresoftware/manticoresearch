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

/// known match modes
define ( "SPH_MATCH_ALL",			0 );
define ( "SPH_MATCH_ANY",			1 );
define ( "SPH_MATCH_PHRASE",		2 );

/// known sort modes
define ( "SPH_SORT_RELEVANCE",		0 );
define ( "SPH_SORT_DATE_DESC",		1 );
define ( "SPH_SORT_DATE_ASC",		2 );
define ( "SPH_SORT_TIME_SEGMENTS", 	3 );

/// sphinx searchd client class
class SphinxClient
{
	var $_host;		///< searchd host (default is "localhost")
	var $_port;		///< searchd port (default is 3312)
	var $_offset;	///< how much records to seek from result-set start (default is 0)
	var $_limit;	///< how much records to return from result-set starting at offset (default is 20)
	var $_mode;		///< query matching mode (default is SPH_MATCH_ALL)
	var $_weights;	///< per-field weights (default is 1 for all fields)
	var $_groups;	///< groups to limit searching to (default is not to limit)
	var $_sort;		///< match sorting mode (default is SPH_SORT_RELEVANCE)

	var $_error;	///< last error message

	/// create a new client object and fill defaults
	function SphinxClient ()
	{
		$this->_host	= "localhost";
		$this->_port	= 3312;
		$this->_offset	= 0;
		$this->_limit	= 20;
		$this->_mode	= SPH_MATCH_ALL;
		$this->_weights	= array ();
		$this->_groups	= array ();
		$this->_sort	= SPH_SORT_RELEVANCE;

		$this->_error	= "";
	}

	/// get last error message (string)
	function GetLastError ()
	{
		return $this->_error;
	}

	/// set searchd server
	function SetServer ( $host, $port )
	{
		assert ( is_string($host) );
		assert ( is_int($port) );
		$this->_host = $host;
		$this->_port = $port;
	}

	/// set match offset/limits
	function SetLimits ( $offset, $limit )
	{
		assert ( is_int($offset) );
		assert ( is_int($limit) );
		assert ( $offset>=0 );
		assert ( $limit>0 );
		$this->_offset = $offset;
		$this->_limit = $limit;
	}

	/// set match mode
	function SetMatchMode ( $mode )
	{
		assert ( $mode==SPH_MATCH_ALL || $mode==SPH_MATCH_ANY || $mode==SPH_MATCH_PHRASE );
		$this->_mode = $mode;
	}

	/// set match mode
	function SetSortMode ( $sort )
	{
		assert ( $sort==SPH_SORT_RELEVANCE || $sort==SPH_SORT_DATE_DESC || $sort==SPH_SORT_DATE_ASC );
		$this->_sort = $sort;
	}

	/// set per-field weights
	function SetWeights ( $weights )
	{
		assert ( is_array($weights) );
		foreach ( $weights as $weight )
			assert ( is_int($weight) );

		$this->_weights = $weights;
	}

	/// set groups
	function SetGroups ( $groups )
	{
		assert ( is_array($groups) );
		foreach ( $groups as $group )
			assert ( is_int($group) );

		$this->_groups = $groups;
	}

	/// connect to server and run given query
	///
	/// $query is query string
	///
	/// returns false on failure
	/// returns hash which has the following keys on success:
	///		"matches"
	///			hash which maps found document_id to ( "weight", "group" ) hash
	///		"total"
	///			total amount of matches retrieved (upto SPH_MAX_MATCHES, see sphinx.h)
	///		"total_found"
	///			total amount of matching documents in index
	///		"time"
	///			search time
	///		"words"
	///			hash which maps query terms (stemmed!) to ( "docs", "hits" ) hash
	function Query ( $query )
	{
		if (!( $fp = @fsockopen ( $this->_host, $this->_port ) ) )
		{
			$this->_error = "connection to {$this->_host}:{$this->_port} failed";
			return false;
		}

		// check version
		$s = trim ( fgets ( $fp, 1024 ) );
		if ( substr ( $s, 0, 4 )!="VER " )
		{
			fclose ( $fp );
			$this->_error = "expected searchd protocol version, got '$s'";
			return false;
		}
		$ver = (int)substr ( $s, 4 ); 
		if ( $ver!=1 )
		{
			fclose ( $fp );
			$this->_error = "expected protocol version 1, got $ver";
			return false;
		}

		/////////////////
		// build request
		/////////////////

		// mode/limits part
		$req = pack ( "VVVV", $this->_offset, $this->_limit, $this->_mode, $this->_sort );

		// groups
		$req .= pack ( "V", count($this->_groups) );
		foreach ( $this->_groups as $group )
			$req .= pack ( "V", $group );

		// query string
		$req .= pack ( "V", strlen($query) ) . $query; 

		// weights
		$req .= pack ( "V", count($this->_weights) );
		foreach ( $this->_weights as $weight )
			$req .= pack ( "V", (int)$weight );

		////////////
		// do query
		////////////

		fputs ( $fp, $req );

		$result = array();
		while ( !feof ( $fp ) )
		{
			$s = trim ( fgets ( $fp, 1024 ) );
			if ( substr ( $s, 0, 6 )=="MATCH " )
			{
				list ( $dummy, $group, $doc, $weight, $stamp ) = explode ( " ", $s );
				$result["matches"][$doc] = array ( "weight"=>$weight, "group"=>$group,
					"stamp"=>$stamp );

			} elseif ( substr ( $s, 0, 6 )=="TOTAL " )
			{
				list ( $dummy, $returned, $found ) = explode ( " ", $s );
				$result["total"] = $returned;
				$result["total_found"] = $found;

			} elseif ( substr ( $s, 0, 5 )=="TIME " )
			{
				$result["time"] = substr ( $s, 5 );

			} elseif ( substr ( $s, 0, 5 ) == "WORD " )
			{
				list ( $dummy, $word, $docs, $hits ) = explode ( " ", $s );
				$result["words"][$word] = array ( "docs"=>$docs, "hits"=>$hits );
			}
			// for now, simply ignore unknown response
		}

		fclose ( $fp );
		return $result;
	}
}

//
// $Id$
//

?>