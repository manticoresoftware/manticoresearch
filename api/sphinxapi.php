<?php

//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

/////////////////////////////////////////////////////////////////////////////
// PHP version of Sphinx searchd client (PHP API)
/////////////////////////////////////////////////////////////////////////////

/// known searchd commands
define ( "SEARCHD_COMMAND_SEARCH",	0 );
define ( "SEARCHD_COMMAND_EXCERPT",	1 );
define ( "SEARCHD_COMMAND_UPDATE",	2 );

/// current client-side command implementation versions
define ( "VER_COMMAND_SEARCH",		0x110 );
define ( "VER_COMMAND_EXCERPT",		0x100 );
define ( "VER_COMMAND_UPDATE",		0x100 );

/// known searchd status codes
define ( "SEARCHD_OK",				0 );
define ( "SEARCHD_ERROR",			1 );
define ( "SEARCHD_RETRY",			2 );
define ( "SEARCHD_WARNING",			3 );

/// known match modes
define ( "SPH_MATCH_ALL",			0 );
define ( "SPH_MATCH_ANY",			1 );
define ( "SPH_MATCH_PHRASE",		2 );
define ( "SPH_MATCH_BOOLEAN",		3 );
define ( "SPH_MATCH_EXTENDED",		4 );
define ( "SPH_MATCH_FULLSCAN",		5 );
define ( "SPH_MATCH_EXTENDED2",		6 );	// extended engine V2 (TEMPORARY, WILL BE REMOVED)

/// known ranking modes (ext2 only)
define ( "SPH_RANK_PROXIMITY_BM25",	0 );	///< default mode, phrase proximity major factor and BM25 minor one
define ( "SPH_RANK_BM25",			1 );	///< statistical mode, BM25 ranking only (faster but worse quality)
define ( "SPH_RANK_NONE",			2 );	///< no ranking, all matches get a weight of 1

/// known sort modes
define ( "SPH_SORT_RELEVANCE",		0 );
define ( "SPH_SORT_ATTR_DESC",		1 );
define ( "SPH_SORT_ATTR_ASC",		2 );
define ( "SPH_SORT_TIME_SEGMENTS", 	3 );
define ( "SPH_SORT_EXTENDED", 		4 );

/// known filter types
define ( "SPH_FILTER_VALUES",		0 );
define ( "SPH_FILTER_RANGE",		1 );
define ( "SPH_FILTER_FLOATRANGE",	2 );

/// known attribute types
define ( "SPH_ATTR_INTEGER",		1 );
define ( "SPH_ATTR_TIMESTAMP",		2 );
define ( "SPH_ATTR_ORDINAL",		3 );
define ( "SPH_ATTR_BOOL",			4 );
define ( "SPH_ATTR_FLOAT",			5 );
define ( "SPH_ATTR_MULTI",			0x40000000 );

/// known grouping functions
define ( "SPH_GROUPBY_DAY",			0 );
define ( "SPH_GROUPBY_WEEK",		1 );
define ( "SPH_GROUPBY_MONTH",		2 );
define ( "SPH_GROUPBY_YEAR",		3 );
define ( "SPH_GROUPBY_ATTR",		4 );
define ( "SPH_GROUPBY_ATTRPAIR",	5 );

/// sphinx searchd client class
class SphinxClient
{
	var $_host;			///< searchd host (default is "localhost")
	var $_port;			///< searchd port (default is 3312)
	var $_offset;		///< how many records to seek from result-set start (default is 0)
	var $_limit;		///< how many records to return from result-set starting at offset (default is 20)
	var $_mode;			///< query matching mode (default is SPH_MATCH_ALL)
	var $_weights;		///< per-field weights (default is 1 for all fields)
	var $_sort;			///< match sorting mode (default is SPH_SORT_RELEVANCE)
	var $_sortby;		///< attribute to sort by (defualt is "")
	var $_min_id;		///< min ID to match (default is 0, which means no limit)
	var $_max_id;		///< max ID to match (default is 0, which means no limit)
	var $_filters;		///< search filters
	var $_groupby;		///< group-by attribute name
	var $_groupfunc;	///< group-by function (to pre-process group-by attribute value with)
	var $_groupsort;	///< group-by sorting clause (to sort groups in result set with)
	var $_groupdistinct;///< group-by count-distinct attribute
	var $_maxmatches;	///< max matches to retrieve
	var $_cutoff;		///< cutoff to stop searching at (default is 0)
	var $_retrycount;	///< distributed retries count
	var $_retrydelay;	///< distributed retries delay
	var $_anchor;		///< geographical anchor point
	var $_indexweights;	///< per-index weights
	var $_ranker;		///< ranking mode (default is SPH_RANK_PROXIMITY_BM25)

	var $_error;		///< last error message
	var $_warning;		///< last warning message

	var $_reqs;			///< requests array for multi-query

	/////////////////////////////////////////////////////////////////////////////
	// common stuff
	/////////////////////////////////////////////////////////////////////////////

	/// create a new client object and fill defaults
	function SphinxClient ()
	{
		// per-client-object settings
		$this->_host		= "localhost";
		$this->_port		= 3312;

		// per-query settings
		$this->_offset		= 0;
		$this->_limit		= 20;
		$this->_mode		= SPH_MATCH_ALL;
		$this->_weights		= array ();
		$this->_sort		= SPH_SORT_RELEVANCE;
		$this->_sortby		= "";
		$this->_min_id		= 0;
		$this->_max_id		= 0;
		$this->_filters		= array ();
		$this->_groupby		= "";
		$this->_groupfunc	= SPH_GROUPBY_DAY;
		$this->_groupsort	= "@group desc";
		$this->_groupdistinct= "";
		$this->_maxmatches	= 1000;
		$this->_cutoff		= 0;
		$this->_retrycount	= 0;
		$this->_retrydelay	= 0;
		$this->_anchor		= array ();
		$this->_indexweights= array ();
		$this->_ranker		= SPH_RANK_PROXIMITY_BM25;

		// per-reply fields (for single-query case)
		$this->_error		= "";
		$this->_warning		= "";

		// requests storage (for multi-query case)
		$this->_reqs		= array ();
	}

	/// get last error message (string)
	function GetLastError ()
	{
		return $this->_error;
	}

	/// get last warning message (string)
	function GetLastWarning ()
	{
		return $this->_warning;
	}

	/// set searchd server
	function SetServer ( $host, $port )
	{
		assert ( is_string($host) );
		assert ( is_int($port) );
		$this->_host = $host;
		$this->_port = $port;
	}

	/////////////////////////////////////////////////////////////////////////////

	/// connect to searchd server
	function _Connect ()
	{
		if (!( $fp = @fsockopen ( $this->_host, $this->_port ) ) )
		{
			$this->_error = "connection to {$this->_host}:{$this->_port} failed";
			return false;
		}

		// check version
		list(,$v) = unpack ( "N*", fread ( $fp, 4 ) );
		$v = (int)$v;
		if ( $v<1 )
		{
			fclose ( $fp );
			$this->_error = "expected searchd protocol version 1+, got version '$v'";
			return false;
		}

		// all ok, send my version
		fwrite ( $fp, pack ( "N", 1 ) );
		return $fp;
	}

	/// get and check response packet from searchd server
	function _GetResponse ( $fp, $client_ver )
	{
		$response = "";
		$len = 0;

		$header = fread ( $fp, 8 );
		if ( strlen($header)==8 )
		{
			list ( $status, $ver, $len ) = array_values ( unpack ( "n2a/Nb", $header ) );
			$left = $len;
			while ( $left>0 && !feof($fp) )
			{
				$chunk = fread ( $fp, $left );
				if ( $chunk )
				{
					$response .= $chunk;
					$left -= strlen($chunk);
				}
			}
		}
		fclose ( $fp );

		// check response
		$read = strlen ( $response );
		if ( !$response || $read!=$len )
		{
			$this->_error = $len
				? "failed to read searchd response (status=$status, ver=$ver, len=$len, read=$read)"
				: "received zero-sized searchd response";
			return false;
		}

		// check status
		if ( $status==SEARCHD_WARNING )
		{
			list(,$wlen) = unpack ( "N*", substr ( $response, 0, 4 ) );
			$this->_warning = substr ( $response, 4, $wlen );
			return substr ( $response, 4+$wlen );
		}
		if ( $status==SEARCHD_ERROR )
		{
			$this->_error = "searchd error: " . substr ( $response, 4 );
			return false;
		}
		if ( $status==SEARCHD_RETRY )
		{
			$this->_error = "temporary searchd error: " . substr ( $response, 4 );
			return false;
		}
		if ( $status!=SEARCHD_OK )
		{
			$this->_error = "unknown status code '$status'";
			return false;
		}

		// check version
		if ( $ver<$client_ver )
		{
			$this->_warning = sprintf ( "searchd command v.%d.%d older than client's v.%d.%d, some options might not work",
				$ver>>8, $ver&0xff, $client_ver>>8, $client_ver&0xff );
		}

		return $response;
	}

	/////////////////////////////////////////////////////////////////////////////
	// searching
	/////////////////////////////////////////////////////////////////////////////

	/// set offset and count into result set,
	/// and max-matches and cutoff to use while searching
	function SetLimits ( $offset, $limit, $max=0, $cutoff=0 )
	{
		assert ( is_int($offset) );
		assert ( is_int($limit) );
		assert ( $offset>=0 );
		assert ( $limit>0 );
		assert ( $max>=0 );
		$this->_offset = $offset;
		$this->_limit = $limit;
		if ( $max>0 )
			$this->_maxmatches = $max;
		if ( $cutoff>0 )
			$this->_cutoff = $cutoff;
	}

	/// set match mode
	function SetMatchMode ( $mode )
	{
		assert ( $mode==SPH_MATCH_ALL
			|| $mode==SPH_MATCH_ANY
			|| $mode==SPH_MATCH_PHRASE
			|| $mode==SPH_MATCH_BOOLEAN
			|| $mode==SPH_MATCH_EXTENDED
			|| $mode==SPH_MATCH_EXTENDED2 );
		$this->_mode = $mode;
	}

	/// set ranking mode
	function SetRankingMode ( $ranker )
	{
		assert ( $ranker==SPH_RANK_PROXIMITY_BM25
			|| $ranker==SPH_RANK_BM25
			|| $ranker==SPH_RANK_NONE );
		$this->_ranker = $ranker;
	}

	/// set matches sorting mode
	function SetSortMode ( $mode, $sortby="" )
	{
		assert (
			$mode==SPH_SORT_RELEVANCE ||
			$mode==SPH_SORT_ATTR_DESC ||
			$mode==SPH_SORT_ATTR_ASC ||
			$mode==SPH_SORT_TIME_SEGMENTS ||
			$mode==SPH_SORT_EXTENDED );
		assert ( is_string($sortby) );
		assert ( $mode==SPH_SORT_RELEVANCE || strlen($sortby)>0 );

		$this->_sort = $mode;
		$this->_sortby = $sortby;
	}

	/// set per-field weights
	function SetWeights ( $weights )
	{
		assert ( is_array($weights) );
		foreach ( $weights as $weight )
			assert ( is_int($weight) );

		$this->_weights = $weights;
	}

	/// set per-index weights
	function SetIndexWeights ( $weights )
	{
		assert ( is_array($weights) );
		foreach ( $weights as $index=>$weight )
		{
			assert ( is_string($index) );
			assert ( is_int($weight) );
		}
		$this->_indexweights = $weights;
	}

	/// set IDs range to match
	/// only match those records where document ID
	/// is beetwen $min and $max (including $min and $max)
	function SetIDRange ( $min, $max )
	{
		assert ( is_int($min) );
		assert ( is_int($max) );
		assert ( $min<=$max );
		$this->_min_id = $min;
		$this->_max_id = $max;
	}

	/// set values filter
	/// only match those records where $attribute column values
	/// are in specified set
	function SetFilter ( $attribute, $values, $exclude=false )
	{
		assert ( is_string($attribute) );
		assert ( is_array($values) );
		assert ( count($values) );

		if ( is_array($values) && count($values) )
		{
			foreach ( $values as $value )
				assert ( is_numeric($value) );

			$this->_filters[] = array ( "type"=>SPH_FILTER_VALUES, "attr"=>$attribute, "exclude"=>$exclude, "values"=>$values );
		}
	}

	/// set range filter
	/// only match those records where $attribute column value
	/// is beetwen $min and $max (including $min and $max)
	function SetFilterRange ( $attribute, $min, $max, $exclude=false )
	{
		assert ( is_string($attribute) );
		assert ( is_int($min) );
		assert ( is_int($max) );
		assert ( $min<=$max );

		$this->_filters[] = array ( "type"=>SPH_FILTER_RANGE, "attr"=>$attribute, "exclude"=>$exclude, "min"=>$min, "max"=>$max );
	}

	/// set float range filter
	/// only match those records where $attribute column value
	/// is beetwen $min and $max (including $min and $max)
	function SetFilterFloatRange ( $attribute, $min, $max, $exclude=false )
	{
		assert ( is_string($attribute) );
		assert ( is_float($min) );
		assert ( is_float($max) );
		assert ( $min<=$max );

		$this->_filters[] = array ( "type"=>SPH_FILTER_FLOATRANGE, "attr"=>$attribute, "exclude"=>$exclude, "min"=>$min, "max"=>$max );
	}

	/// setup geographical anchor point
	/// required to use @geodist in filters and sorting
	/// distance will be computed to this point
	///
	/// $attrlat is the name of latitude attribute
	/// $attrlong is the name of longitude attribute
	/// $lat is anchor point latitude, in radians
	/// $long is anchor point longitude, in radians
	function SetGeoAnchor ( $attrlat, $attrlong, $lat, $long )
	{
		assert ( is_string($attrlat) );
		assert ( is_string($attrlong) );
		assert ( is_float($lat) );
		assert ( is_float($long) );

		$this->_anchor = array ( "attrlat"=>$attrlat, "attrlong"=>$attrlong, "lat"=>$lat, "long"=>$long );
	}

	/// set grouping attribute and function
	///
	/// in grouping mode, all matches are assigned to different groups
	/// based on grouping function value.
	///
	/// each group keeps track of the total match count, and the best match
	/// (in this group) according to current sorting function.
	///
	/// the final result set contains one best match per group, with
	/// grouping function value and matches count attached.
	///
	/// groups in result set could be sorted by any sorting clause,
	/// including both document attributes and the following special
	/// internal Sphinx attributes:
	///
	/// - @id - match document ID;
	/// - @weight, @rank, @relevance -  match weight;
	/// - @group - groupby function value;
	/// - @count - amount of matches in group.
	///
	/// the default mode is to sort by groupby value in descending order,
	/// ie. by "@group desc".
	///
	/// "total_found" would contain total amount of matching groups over
	/// the whole index.
	///
	/// WARNING: grouping is done in fixed memory and thus its results
	/// are only approximate; so there might be more groups reported
	/// in total_found than actually present. @count might also
	/// be underestimated. 
	///
	/// for example, if sorting by relevance and grouping by "published"
	/// attribute with SPH_GROUPBY_DAY function, then the result set will
	/// contain one most relevant match per each day when there were any
	/// matches published, with day number and per-day match count attached,
	/// and sorted by day number in descending order (ie. recent days first).
	function SetGroupBy ( $attribute, $func, $groupsort="@group desc" )
	{
		assert ( is_string($attribute) );
		assert ( is_string($groupsort) );
		assert ( $func==SPH_GROUPBY_DAY
			|| $func==SPH_GROUPBY_WEEK
			|| $func==SPH_GROUPBY_MONTH
			|| $func==SPH_GROUPBY_YEAR
			|| $func==SPH_GROUPBY_ATTR
			|| $func==SPH_GROUPBY_ATTRPAIR );

		$this->_groupby = $attribute;
		$this->_groupfunc = $func;
		$this->_groupsort = $groupsort;
	}

	/// set count-distinct attribute for group-by queries
	function SetGroupDistinct ( $attribute )
	{
		assert ( is_string($attribute) );
		$this->_groupdistinct = $attribute;
	}

	/// set distributed retries count and delay
	function SetRetries ( $count, $delay=0 )
	{
		assert ( is_int($count) && $count>=0 );
		assert ( is_int($delay) && $delay>=0 );
		$this->_retrycount = $count;
		$this->_retrydelay = $delay;
	}

	//////////////////////////////////////////////////////////////////////////////

	/// clear all filters (for multi-queries)
	function ResetFilters ()
	{
		$this->_filters = array();
		$this->_anchor = array();
	}

	/// clear groupby settings
	function ResetGroupBy ()
	{
		$this->_groupby		= "";
		$this->_groupfunc	= SPH_GROUPBY_DAY;
		$this->_groupsort	= "@group desc";
		$this->_groupdistinct= "";
	}

	//////////////////////////////////////////////////////////////////////////////

	/// connect to searchd server and run given search query
	///
	/// $query is query string
	///
	/// $index is index name (or names) to query. default value is "*" which means
	/// to query all indexes. accepted characters for index names are letters, numbers,
	/// dash, and underscore; everything else is considered a separator. therefore,
	/// all the following calls are valid and will search two indexes:
	///
	///		$cl->Query ( "test query", "main delta" );
	///		$cl->Query ( "test query", "main;delta" );
	///		$cl->Query ( "test query", "main, delta" );
	///
	/// index order matters. if identical IDs are found in two or more indexes,
	/// weight and attribute values from the very last matching index will be used
	/// for sorting and returning to client. therefore, in the example above,
	/// matches from "delta" index will always "win" over matches from "main".
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
	function Query ( $query, $index="*" )
	{
		assert ( empty($this->_reqs) );

		$this->AddQuery ( $query, $index );
		$results = $this->RunQueries ();

		if ( !is_array($results) )
			return false; // probably network error; error message should be already filled

		$this->_error = $results[0]["error"];
		$this->_warning = $results[0]["warning"];
		if ( $results[0]["status"]==SEARCHD_ERROR )
			return false;
		else
			return $results[0];
	}

	/// helper to pack floats in network byte order
	function _PackFloat ( $f )
	{
		$t1 = pack ( "f", $f ); // machine order
		list(,$t2) = unpack ( "L*", $t1 ); // int in machine order
		return pack ( "N", $t2 );
	}

	/// add query to batch
	///
	/// batch queries enable searchd to perform internal optimizations,
	/// if possible; and reduce network connection overheads in all cases.
	///
	/// for instance, running exactly the same query with different
	/// groupby settings will enable searched to perform expensive
	/// full-text search and ranking operation only once, but compute
	/// multiple groupby results from its output.
	///
	/// parameters are exactly the same as in Query() call
	/// returns index to results array returned by RunQueries() call
	function AddQuery ( $query, $index="*" )
	{
		// build request
		$req = pack ( "NNNNN", $this->_offset, $this->_limit, $this->_mode, $this->_ranker, $this->_sort ); // mode and limits
		$req .= pack ( "N", strlen($this->_sortby) ) . $this->_sortby;
		$req .= pack ( "N", strlen($query) ) . $query; // query itself
		$req .= pack ( "N", count($this->_weights) ); // weights
		foreach ( $this->_weights as $weight )
			$req .= pack ( "N", (int)$weight );
		$req .= pack ( "N", strlen($index) ) . $index; // indexes
		$req .= pack ( "NNN", 0, (int)$this->_min_id, (int)$this->_max_id ); // id32 range

		// filters
		$req .= pack ( "N", count($this->_filters) );
		foreach ( $this->_filters as $filter )
		{
			$req .= pack ( "N", strlen($filter["attr"]) ) . $filter["attr"];
			$req .= pack ( "N", $filter["type"] );
			switch ( $filter["type"] )
			{
				case SPH_FILTER_VALUES:
					$req .= pack ( "N", count($filter["values"]) );
					foreach ( $filter["values"] as $value )
						$req .= pack ( "N", floatval($value) ); // this uberhack is to workaround 32bit signed int limit on x32 platforms
					break;

				case SPH_FILTER_RANGE:
					$req .= pack ( "NN", $filter["min"], $filter["max"] );
					break;

				case SPH_FILTER_FLOATRANGE:
					$req .= $this->_PackFloat ( $filter["min"] ) . $this->_PackFloat ( $filter["max"] );
					break;

				default:
					assert ( 0 && "internal error: unhandled filter type" );
			}
			$req .= pack ( "N", $filter["exclude"] );
		}

		// group-by clause, max-matches count, group-sort clause, cutoff count
		$req .= pack ( "NN", $this->_groupfunc, strlen($this->_groupby) ) . $this->_groupby;
		$req .= pack ( "N", $this->_maxmatches );
		$req .= pack ( "N", strlen($this->_groupsort) ) . $this->_groupsort;
		$req .= pack ( "NNN", $this->_cutoff, $this->_retrycount, $this->_retrydelay );
		$req .= pack ( "N", strlen($this->_groupdistinct) ) . $this->_groupdistinct;

		// anchor point
		if ( empty($this->_anchor) )
		{
			$req .= pack ( "N", 0 );
		} else
		{
			$a =& $this->_anchor;
			$req .= pack ( "N", 1 );
			$req .= pack ( "N", strlen($a["attrlat"]) ) . $a["attrlat"];
			$req .= pack ( "N", strlen($a["attrlong"]) ) . $a["attrlong"];
			$req .= $this->_PackFloat ( $a["lat"] ) . $this->_PackFloat ( $a["long"] );
		}

		// per-index weights
		$req .= pack ( "N", count($this->_indexweights) );
		foreach ( $this->_indexweights as $idx=>$weight )
			$req .= pack ( "N", strlen($idx) ) . $idx . pack ( "N", $weight );

		// store request to requests array
		$this->_reqs[] = $req;
		return count($this->_reqs)-1;
	}

	/// run queries batch
	///
	/// returns an array of result sets on success
	/// returns false on network IO failure
	///
	/// each result set in returned array is a hash which containts
	/// the same keys as the hash returned by Query(), plus:
	///		"error"
	///			search error for this query
	///		"words"
	///			hash which maps query terms (stemmed!) to ( "docs", "hits" ) hash
	function RunQueries ()
	{
		if ( empty($this->_reqs) )
		{
			$this->_error = "no queries defined, issue AddQuery() first";
			return false;
		}

		if (!( $fp = $this->_Connect() ))
			return false;

		////////////////////////////
		// send query, get response
		////////////////////////////

		$nreqs = count($this->_reqs);
		$req = join ( "", $this->_reqs );
		$len = 4+strlen($req);
		$req = pack ( "nnNN", SEARCHD_COMMAND_SEARCH, VER_COMMAND_SEARCH, $len, $nreqs ) . $req; // add header

		fwrite ( $fp, $req, $len+8 );
		if (!( $response = $this->_GetResponse ( $fp, VER_COMMAND_SEARCH ) ))
			return false;

		$this->_reqs = array ();

		//////////////////
		// parse response
		//////////////////

		$p = 0; // current position
		$max = strlen($response); // max position for checks, to protect against broken responses

		$results = array ();
		for ( $ires=0; $ires<$nreqs && $p<$max; $ires++ )
		{
			$results[] = array();
			$result =& $results[$ires];

			$result["error"] = "";
			$result["warning"] = "";

			// extract status
			list(,$status) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
			$result["status"] = $status;
			if ( $status!=SEARCHD_OK )
			{
				list(,$len) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
				$message = substr ( $response, $p, $len ); $p += $len;

				if ( $status==SEARCHD_WARNING )
				{
					$result["warning"] = $message;
				} else
				{
					$result["error"] = $message;
					continue;
				}
			}

			// read schema
			$fields = array ();
			$attrs = array ();

			list(,$nfields) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
			while ( $nfields-->0 && $p<$max )
			{
				list(,$len) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
				$fields[] = substr ( $response, $p, $len ); $p += $len;
			}
			$result["fields"] = $fields;

			list(,$nattrs) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
			while ( $nattrs-->0 && $p<$max  )
			{
				list(,$len) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
				$attr = substr ( $response, $p, $len ); $p += $len;
				list(,$type) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
				$attrs[$attr] = $type;
			}
			$result["attrs"] = $attrs;

			// read match count
			list(,$count) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
			list(,$id64) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;

			// read matches
			while ( $count-->0 && $p<$max )
			{
				if ( $id64 )
				{
					list ( $dochi, $doclo, $weight ) = array_values ( unpack ( "N*N*N*",
						substr ( $response, $p, 12 ) ) );
					$p += 12;
					$doc = (((int)$dochi)<<32) + ((int)$doclo);
				} else
				{
					list ( $doc, $weight ) = array_values ( unpack ( "N*N*",
						substr ( $response, $p, 8 ) ) );
					$p += 8;
					$doc = sprintf ( "%u", $doc ); // workaround for php signed/unsigned braindamage
				}

				$weight = sprintf ( "%u", $weight );
				$result["matches"][$doc]["weight"] = $weight;

				$attrvals = array ();
				foreach ( $attrs as $attr=>$type )
				{
					// handle floats
					if ( $type==SPH_ATTR_FLOAT )
					{
						list(,$uval) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
						list(,$fval) = unpack ( "f*", pack ( "L", $uval ) ); 
						$attrvals[$attr] = $fval;
						continue;
					}

					// handle everything else as unsigned ints
					list(,$val) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
					if ( $type & SPH_ATTR_MULTI )
					{
						$attrvals[$attr] = array ();
						$nvalues = $val;
						while ( $nvalues-->0 && $p<$max )
						{
							list(,$val) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
							$attrvals[$attr][] = sprintf ( "%u", $val );
						}
					} else
					{
						$attrvals[$attr] = sprintf ( "%u", $val );
					}
				}
				$result["matches"][$doc]["attrs"] = $attrvals;
			}

			list ( $total, $total_found, $msecs, $words ) =
				array_values ( unpack ( "N*N*N*N*", substr ( $response, $p, 16 ) ) );
			$result["total"] = sprintf ( "%u", $total );
			$result["total_found"] = sprintf ( "%u", $total_found );
			$result["time"] = sprintf ( "%.3f", $msecs/1000 );
			$p += 16;

			while ( $words-->0 && $p<$max )
			{
				list(,$len) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
				$word = substr ( $response, $p, $len ); $p += $len;
				list ( $docs, $hits ) = array_values ( unpack ( "N*N*", substr ( $response, $p, 8 ) ) ); $p += 8;
				$result["words"][$word] = array (
					"docs"=>sprintf ( "%u", $docs ),
					"hits"=>sprintf ( "%u", $hits ) );
			}
		}

		return $results;
	}

	/////////////////////////////////////////////////////////////////////////////
	// excerpts generation
	/////////////////////////////////////////////////////////////////////////////

	/// connect to searchd server and generate exceprts from given documents
	///
	/// $docs is an array of strings which represent the documents' contents
	/// $index is a string specifiying the index which settings will be used
	///		for stemming, lexing and case folding
	/// $words is a string which contains the words to highlight
	/// $opts is a hash which contains additional optional highlighting parameters:
	///		"before_match"
	///			a string to insert before a set of matching words, default is "<b>"
	///		"after_match"
	///			a string to insert after a set of matching words, default is "<b>"
	///		"chunk_separator"
	///			a string to insert between excerpts chunks, default is " ... "
	///		"limit"
	///			max excerpt size in symbols (codepoints), default is 256
	///		"around"
	///			how much words to highlight around each match, default is 5
	///		"exact_phrase"
	///			whether to highlight exact phrase matches only, default is false
	///		"single_passage"
	///			whether to extract single best passage only, default is false
	///
	/// returns false on failure
	/// returns an array of string excerpts on success
	function BuildExcerpts ( $docs, $index, $words, $opts=array() )
	{
		assert ( is_array($docs) );
		assert ( is_string($index) );
		assert ( is_string($words) );
		assert ( is_array($opts) );

		if (!( $fp = $this->_Connect() ))
			return false;

		/////////////////
		// fixup options
		/////////////////

		if ( !isset($opts["before_match"]) )		$opts["before_match"] = "<b>";
		if ( !isset($opts["after_match"]) )			$opts["after_match"] = "</b>";
		if ( !isset($opts["chunk_separator"]) )		$opts["chunk_separator"] = " ... ";
		if ( !isset($opts["limit"]) )				$opts["limit"] = 256;
		if ( !isset($opts["around"]) )				$opts["around"] = 5;
		if ( !isset($opts["exact_phrase"]) )		$opts["exact_phrase"] = false;
		if ( !isset($opts["single_passage"]) )		$opts["single_passage"] = false;
		if ( !isset($opts["use_boundaries"]) )		$opts["use_boundaries"] = false;
		if ( !isset($opts["weight_order"]) )		$opts["weight_order"] = false;

		/////////////////
		// build request
		/////////////////

		// v.1.0 req
		$flags = 1; // remove spaces
		if ( $opts["exact_phrase"] )	$flags |= 2;
		if ( $opts["single_passage"] )	$flags |= 4;
		if ( $opts["use_boundaries"] )	$flags |= 8;
		if ( $opts["weight_order"] )	$flags |= 16;
		$req = pack ( "NN", 0, $flags ); // mode=0, flags=$flags
		$req .= pack ( "N", strlen($index) ) . $index; // req index
		$req .= pack ( "N", strlen($words) ) . $words; // req words

		// options
		$req .= pack ( "N", strlen($opts["before_match"]) ) . $opts["before_match"];
		$req .= pack ( "N", strlen($opts["after_match"]) ) . $opts["after_match"];
		$req .= pack ( "N", strlen($opts["chunk_separator"]) ) . $opts["chunk_separator"];
		$req .= pack ( "N", (int)$opts["limit"] );
		$req .= pack ( "N", (int)$opts["around"] );

		// documents
		$req .= pack ( "N", count($docs) );
		foreach ( $docs as $doc )
		{
			assert ( is_string($doc) );
			$req .= pack ( "N", strlen($doc) ) . $doc;
		}

		////////////////////////////
		// send query, get response
		////////////////////////////

		$len = strlen($req);
		$req = pack ( "nnN", SEARCHD_COMMAND_EXCERPT, VER_COMMAND_EXCERPT, $len ) . $req; // add header
		$wrote = fwrite ( $fp, $req, $len+8 );
		if (!( $response = $this->_GetResponse ( $fp, VER_COMMAND_EXCERPT ) ))
			return false;

		//////////////////
		// parse response
		//////////////////

		$pos = 0;
		$res = array ();
		$rlen = strlen($response);
		for ( $i=0; $i<count($docs); $i++ )
		{
			list(,$len) = unpack ( "N*", substr ( $response, $pos, 4 ) );
			$pos += 4;

			if ( $pos+$len > $rlen )
			{
				$this->_error = "incomplete reply";
				return false;
			}
			$res[] = $len ? substr ( $response, $pos, $len ) : "";
			$pos += $len;
		}

		return $res;
	}

	/////////////////////////////////////////////////////////////////////////////
	// attribute updates
	/////////////////////////////////////////////////////////////////////////////

	/// update specified attributes on specified documents
	///
	/// $index is a name of the index to be updated
	/// $attrs is an array of attribute name strings
	/// $values is a hash where key is document id, and value is an array of
	///		new attribute values
	///
	/// returns number of actually updated documents (0 or more) on success
	/// returns -1 on failure
	///
	/// usage example:
	///		$cl->UpdateAttributes ( "test1", array("group_id"), array(1=>array(456)) );
	function UpdateAttributes ( $index, $attrs, $values )
	{
		// verify everything
		assert ( is_string($index) );

		assert ( is_array($attrs) );
		foreach ( $attrs as $attr )
			assert ( is_string($attr) );

		assert ( is_array($values) );
		foreach ( $values as $id=>$entry )
		{
			assert ( is_int($id) );
			assert ( is_array($entry) );
			assert ( count($entry)==count($attrs) );
			foreach ( $entry as $v )
				assert ( is_int($v) );
		}

		// build request
		$req = pack ( "N", strlen($index) ) . $index;

		$req .= pack ( "N", count($attrs) );
		foreach ( $attrs as $attr )
			$req .= pack ( "N", strlen($attr) ) . $attr;

		$req .= pack ( "N", count($values) );
		foreach ( $values as $id=>$entry )
		{
			$req .= pack ( "N", $id );
			foreach ( $entry as $v )
				$req .= pack ( "N", $v );
		}

		// connect, send query, get response
		if (!( $fp = $this->_Connect() ))
			return -1;

		$len = strlen($req);
		$req = pack ( "nnN", SEARCHD_COMMAND_UPDATE, VER_COMMAND_UPDATE, $len ) . $req; // add header
		fwrite ( $fp, $req, $len+8 );

		if (!( $response = $this->_GetResponse ( $fp, VER_COMMAND_UPDATE ) ))
			return -1;

		// parse response
		list(,$updated) = unpack ( "N*", substr ( $response, 0, 4 ) );
		return $updated;
	}
}

//
// $Id$
//

?>