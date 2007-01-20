<?php

//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
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

/// current client-side command implementation versions
define ( "VER_COMMAND_SEARCH",		0x106 );
define ( "VER_COMMAND_EXCERPT",		0x100 );

/// known searchd status codes
define ( "SEARCHD_OK",				0 );
define ( "SEARCHD_ERROR",			1 );
define ( "SEARCHD_RETRY",			2 );

/// known match modes
define ( "SPH_MATCH_ALL",			0 );
define ( "SPH_MATCH_ANY",			1 );
define ( "SPH_MATCH_PHRASE",		2 );
define ( "SPH_MATCH_BOOLEAN",		3 );
define ( "SPH_MATCH_EXTENDED",		4 );

/// known sort modes
define ( "SPH_SORT_RELEVANCE",		0 );
define ( "SPH_SORT_ATTR_DESC",		1 );
define ( "SPH_SORT_ATTR_ASC",		2 );
define ( "SPH_SORT_TIME_SEGMENTS", 	3 );
define ( "SPH_SORT_EXTENDED", 		4 );

/// known attribute types
define ( "SPH_ATTR_INTEGER",		1 );
define ( "SPH_ATTR_TIMESTAMP",		2 );

/// known grouping functions
define ( "SPH_GROUPBY_DAY",			0 );
define ( "SPH_GROUPBY_WEEK",		1 );
define ( "SPH_GROUPBY_MONTH",		2 );
define ( "SPH_GROUPBY_YEAR",		3 );
define ( "SPH_GROUPBY_ATTR",		4 );

/// sphinx searchd client class
class SphinxClient
{
	var $_host;		///< searchd host (default is "localhost")
	var $_port;		///< searchd port (default is 3312)
	var $_offset;	///< how many records to seek from result-set start (default is 0)
	var $_limit;	///< how many records to return from result-set starting at offset (default is 20)
	var $_mode;		///< query matching mode (default is SPH_MATCH_ALL)
	var $_weights;	///< per-field weights (default is 1 for all fields)
	var $_sort;		///< match sorting mode (default is SPH_SORT_RELEVANCE)
	var $_sortby;	///< attribute to sort by (defualt is "")
	var $_min_id;	///< min ID to match (default is 0)
	var $_max_id;	///< max ID to match (default is UINT_MAX)
	var $_filters;	///< search filters
	var $_groupby;	///< group-by attribute name
	var $_groupfunc;///< function to pre-process group-by attribute value with
	var $_maxmatches;	///< max matches to retrieve
	var $_sortbygroup;	///< whether to sort grouped results by group, or by current sorting func

	var $_error;	///< last error message
	var $_warning;	///< last warning message

	/////////////////////////////////////////////////////////////////////////////
	// common stuff
	/////////////////////////////////////////////////////////////////////////////

	/// create a new client object and fill defaults
	function SphinxClient ()
	{
		$this->_host	= "localhost";
		$this->_port	= 3312;
		$this->_offset	= 0;
		$this->_limit	= 20;
		$this->_mode	= SPH_MATCH_ALL;
		$this->_weights	= array ();
		$this->_sort	= SPH_SORT_RELEVANCE;
		$this->_sortby	= "";
		$this->_min_id	= 0;
		$this->_max_id	= 0xFFFFFFFF;
		$this->_filters	= array ();
		$this->_groupby		= "";
		$this->_groupfunc	= SPH_GROUPBY_DAY;
		$this->_maxmatches	= 1000;
		$this->_sortbygroup	= true;

		$this->_error	= "";
		$this->_warning	= "";
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
		$header = fread ( $fp, 8 );
		list ( $status, $ver, $len ) = array_values ( unpack ( "n2a/Nb", $header ) );
		$response = "";
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

	/// set match offset, count, and max number to retrieve
	function SetLimits ( $offset, $limit, $max=0 )
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
	}

	/// set match mode
	function SetMatchMode ( $mode )
	{
		assert ( $mode==SPH_MATCH_ALL
			|| $mode==SPH_MATCH_ANY
			|| $mode==SPH_MATCH_PHRASE
			|| $mode==SPH_MATCH_BOOLEAN
			|| $mode==SPH_MATCH_EXTENDED );
		$this->_mode = $mode;
	}

	/// set sort mode
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
				assert ( is_int($value) );

			$this->_filters[] = array ( "attr"=>$attribute, "exclude"=>$exclude, "values"=>$values );
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

		$this->_filters[] = array ( "attr"=>$attribute, "exclude"=>$exclude, "min"=>$min, "max"=>$max );
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
	/// result set could be sorted either by 1) grouping function value
	/// in descending order (this is the default mode, when $sortbygroup
	/// is set to true); or by 2) current sorting function (when $sortbygroup
	/// is false).
	///
	/// WARNING, when sorting by current function there might be less
	/// matching groups reported than actually present. @count might also be
	/// underestimated. 
	///
	/// for example, if sorting by relevance and grouping by "published"
	/// attribute with SPH_GROUPBY_DAY function, then the result set will
	/// contain one most relevant match per each day when there were any
	/// matches published, with day number and per-day match count attached,
	/// and sorted by day number in descending order (ie. recent days first).
	function SetGroupBy ( $attribute, $func, $sortbygroup=true )
	{
		assert ( is_string($attribute) );
		assert ( $func==SPH_GROUPBY_DAY
			|| $func==SPH_GROUPBY_WEEK
			|| $func==SPH_GROUPBY_MONTH
			|| $func==SPH_GROUPBY_YEAR
			|| $func==SPH_GROUPBY_ATTR );

		$this->_groupby = $attribute;
		$this->_groupfunc = $func;
		$this->_sortbygroup = $sortbygroup;
	}

	/// connect to searchd server and run given search query
	///
	/// $query is query string
	/// $index is index name to query, default is "*" which means to query all indexes
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
		if (!( $fp = $this->_Connect() ))
			return false;

		/////////////////
		// build request
		/////////////////

		$req = pack ( "NNNN", $this->_offset, $this->_limit, $this->_mode, $this->_sort ); // mode and limits
		$req .= pack ( "N", strlen($this->_sortby) ) . $this->_sortby;
		$req .= pack ( "N", strlen($query) ) . $query; // query itself
		$req .= pack ( "N", count($this->_weights) ); // weights
		foreach ( $this->_weights as $weight )
			$req .= pack ( "N", (int)$weight );
		$req .= pack ( "N", strlen($index) ) . $index; // indexes
		$req .= // id range
			pack ( "N", (int)$this->_min_id ) .
			pack ( "N", (int)$this->_max_id );

		// filters
		$req .= pack ( "N", count($this->_filters) );
		foreach ( $this->_filters as $filter )
		{
			$req .= pack ( "N", strlen($filter["attr"]) ) . $filter["attr"];
			if ( isset($filter["values"]) )
			{
				$req .= pack ( "N", count($filter["values"]) );
				foreach ( $filter["values"] as $value )
					$req .= pack ( "N", $value );
			} else
			{
				$req .= pack ( "NNN", 0, $filter["min"], $filter["max"] );
			}
			$req .= pack ( "N", $filter["exclude"] );
		}

		// group-by, max matches, sort-by-group flag
		$req .= pack ( "NN", $this->_groupfunc, strlen($this->_groupby) ) . $this->_groupby;
		$req .= pack ( "N", $this->_maxmatches );
		$req .= pack ( "N", $this->_sortbygroup );

		////////////////////////////
		// send query, get response
		////////////////////////////

		$len = strlen($req);
		$req = pack ( "nnN", SEARCHD_COMMAND_SEARCH, VER_COMMAND_SEARCH, $len ) . $req; // add header
		fwrite ( $fp, $req, $len+8 );
		if (!( $response = $this->_GetResponse ( $fp, VER_COMMAND_SEARCH ) ))
			return false;

		//////////////////
		// parse response
		//////////////////

		$result = array();
		$max = strlen($response); // protection from broken response

		// read schema
		$p = 0;
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

		// read matches
		while ( $count-->0 && $p<$max )
		{
			list ( $doc, $weight ) = array_values ( unpack ( "N*N*",
				substr ( $response, $p, 8 ) ) );
			$p += 8;

			$result["matches"][$doc]["weight"] = $weight;
			foreach ( $attrs as $attr=>$type )
			{
				list(,$val) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
				$result["matches"][$doc]["attrs"][$attr] = $val;
			}
		}
		list ( $result["total"], $result["total_found"], $result["time"], $words ) =
			array_values ( unpack ( "N*N*N*N*", substr ( $response, $p, 16 ) ) );
		$result["time"] = sprintf ( "%.3f", $result["time"]/1000 );
		$p += 16;

		while ( $words-->0 )
		{
			list(,$len) = unpack ( "N*", substr ( $response, $p, 4 ) ); $p += 4;
			$word = substr ( $response, $p, $len ); $p += $len;
			list ( $docs, $hits ) = array_values ( unpack ( "N*N*", substr ( $response, $p, 8 ) ) ); $p += 8;

			$result["words"][$word] = array ( "docs"=>$docs, "hits"=>$hits );
		}

		return $result;
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

		/////////////////
		// build request
		/////////////////

		// v.1.0 req
		$req = pack ( "NN", 0, 1 ); // mode=0, flags=1 (remove spaces)
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
			$res[] = substr ( $response, $pos, $len );
			$pos += $len;
		}

		return $res;
	}
}

//
// $Id$
//

?>