<?php

//
// $Id$
//

ini_set ( "display_errors", 1 );
ini_set ( "error_reporting", E_ALL | E_STRICT );

require_once ( $g_locals['api'] );

function MyMicrotime ()
{
	$q = @gettimeofday();
	return (float)($q["usec"] / 1000000) + $q["sec"];
}

function sphFormatTime ( $time )
{
	if ( $time < 60 )
		return sprintf ( '%.0fs', $time );

	$time = (int)$time;
	if ( $time < 3600 )
		$u = array ( 'm', 's' );
	else
	{
		$time = $time / 60;
		$u = array ( 'h', 'm' );
	}
	return sprintf ( '%d%s:%d%s', $time / 60, $u[0], $time % 60, $u[1] );
}


function ConnectDB ()
{
	global $g_locals;

	if ( !function_exists ( "mysql_connect" ) )
	{
		print ( "ERROR: missing required mysql_connect(); add php_mysql.so (.dll on Windows) to your php.ini!\n" );
		exit ( 1 );
	}

	$conn = @mysql_connect (
		$g_locals["db-host"] . ":" . $g_locals["db-port"],
		$g_locals["db-user"],
		$g_locals["db-password"],
		true );

	if ( $conn === false ||
		!@mysql_query ( "CREATE DATABASE IF NOT EXISTS " . $g_locals['db-name'], $conn ) ||
		!@mysql_select_db ( $g_locals['db-name'], $conn ) )
			return false;

	return $conn;
}


function CreateDB ( $db_drop, $db_create, $db_insert, $custom_insert )
{
	$conn = ConnectDB ();

	foreach ( $db_drop as $q )
		if ( !@mysql_query ( $q, $conn ) )
			return false;

	foreach ( $db_create as $q )
		if ( !@mysql_query ( $q, $conn ) )
			return false;

	foreach ( $db_insert as $q )
		if ( !@mysql_query ( $q, $conn ) )
			return false;

	foreach ( $custom_insert as $code )
	{
		$function = create_function( '', $code );
		$function();
	}

	return $conn;
}


function RunIndexer ( &$error, $params )
{
	global $g_locals;

	$path = $g_locals['indexer'];
	if ( !is_executable($path) )
	{
		$error = "$path: indexer not found";
		return 1;
	}

	$retval = 0;
	exec ( "$path --quiet --config config.conf $params", $error, $retval );

	$error = join ( "\n", $error );
	return ( $retval==0 && !empty($error) ) ? 2 : $retval;
}


function CheckSearchdLog ( $error_file, &$retval )
{
	$rawlog = file ( $error_file );
	$error = "";

	foreach ( $rawlog as $line )
	{
		foreach ( array ( "WARNING", "ERROR", "FATAL" ) as $tag )
		{
			$t = stristr ( $line, $tag );
			if ( $t )
			{
				$error .= $t;
				if ( $tag!="WARNING" )
					$retval = 1;
			}
		}
	}

	return $error;
}


function StartSearchd ( $config_file, $error_file, $pidfile, &$error )
{
	global $g_locals, $windows, $action_retries, $action_wait_timeout;

	$path = $g_locals['searchd'];
	if ( !is_executable($path) )
	{
		$error = "$path: searchd not found";
		return 1;
	}

	if ( !@touch($error_file) )
	{
		$error = "$error_file: unable to create error file";
		return 1;
	}

	$retval = 0;
	if ( $windows )
	{
		// using start /min to fire it "in background"
		// using cmd /c for redirection to work
		if ( file_exists ( $pidfile ) )
			unlink ( $pidfile );
		$process = popen ("start /min cmd /c \"$path --config $config_file --pidfile > $error_file\"", "r" );
		pclose ( $process );
	}
	else
		system ( "$path --config $config_file > $error_file", $retval );

	// wait until pid appears
	for ( $i=0; $i<$action_retries && !file_exists($pidfile); $i++ )
		usleep ( $action_wait_timeout );

	if ( !file_exists($pidfile) )
	{
		$error = "PID file ($pidfile) was not created";
		return 1;
	}

	// check for early crash
	$error = CheckSearchdLog ( $error_file, &$retval );

	// on windows, searchd starts *fully* async
	// so lets also wait until pidfile gets real data
	// (meaning that index precaching is actually done)
	if ( $retval!=1 && $windows )
	{
		$STARTUP_TRIES = 1000;
		$STARTUP_TICK = 50000; // msec

		// FIXME! add a better check that searchd is still alive than just file_exists
		for ( $i=0; $i<$STARTUP_TRIES && file_exists($pidfile); $i++ )
		{
			$pid = file($pidfile);
			if ( count($pid) )
				break;
			usleep ( $STARTUP_TICK );
		}
	}

//	// we've got a pid file; but lets check the log file for startup errors
//	for ( $i=0; $i<$action_retries && !file_exists($error_file); $i++ )
//		usleep ( $action_wait_timeout );

	if ( $retval==0 && !empty($error) )
		$retval = 2; // no errors, but there were warnings

	return $retval;
}


function StopSearchd ( $config, $pidfile )
{
	global $g_locals, $action_retries, $action_wait_timeout;

	if ( file_exists($pidfile) && count(file($pidfile)) )
	{
		$path = $g_locals['searchd'];
		exec ( "$path --config $config --stop" );

		$i = 0;
		while ( file_exists ( $pidfile ) && $i < $action_retries )
		{
			usleep ( $action_wait_timeout );
			$i++;
		}
	}
}


function IsModelGenMode ()
{
	global $g_model;
	return $g_model;
}


function ChildrenArray ( $node, $name="" )
{
	$res = array ();
	if ( !empty($node) && $node->hasChildNodes() )
		for ( $i=0; $i<$node->childNodes->length; $i++ )
	{
		$child = $node->childNodes->item ( $i );
		if ( $name=="" || strtolower($child->nodeName)==$name )
			$res[] = $child;
	}
	return $res;
}


function GetFirstChild ( $node, $name )
{
	$children = ChildrenArray ( $node, $name );
	return empty($children) ? NULL : $children[0];
}


function GetFirstChildValue ( $node, $name, $default="" )
{
	$child = GetFirstChild ( $node, $name );
	return is_null($child) ? $default : $child->nodeValue;
}


class SphinxConfig
{
	private $_name;
	private $_db_create;
	private $_db_drop;
	private $_db_insert;
	private $_custom_insert;
	private $_counters;
	private $_dynamic_entries;
	private $_queries;
	private $_sphqueries;
	private $_query_settings;
	private $_query_attributes;
	private $_indexer_runs;
	private $_custom_test;
	private	$_sd_address;
	private	$_sd_port;
	private $_sd_sphinxql_port;
	private	$_sd_pid_file;
	private $_num_agents;
	private $_subtest;
	private $_subtestcount;
	private $_results;
	private $_results_model;
	private $_prereqs;
	private $_config;				///< config DOM node
	private $_use_sphinxql;			///< true, if sphinxql queries exist
	private $_indexdata;			///< data for use "insert into" instead of run indexer
	private $_connection;			///< mysql connection (since we cound use mysql ans sqphinxql together)
	private $_testdir;				///< the path to the directory with current test (namely for accessing data without knowing the test name)


	function SetConnection($connection)
	{
		$this->_connection = $connection;
	}
	function SphinxConfig ()
	{
		global $sd_address, $sd_port, $sd_sphinxql_port, $sd_pid_file;

		$this->_counters 		= array ();
		$this->_dynamic_entries = array ();
		$this->_queries 		= array ();
		$this->_sphqueries		= array ();
		$this->_results			= array ();
		$this->_results_model	= array ();
		$this->_query_attributes = array ();
		$this->_indexer_runs	= array ();
		$this->_db_create		= array ();
		$this->_db_drop			= array ();
		$this->_db_insert		= array ();
		$this->_custom_insert	= array ();
		$this->_num_agents		= 1;
		$this->_subtest 		= 0;
		$this->_subtestcount	= 0;
		$this->_sd_address		= $sd_address;
		$this->_sd_port			= $sd_port;
		$this->_sd_sphinxql_port	= $sd_sphinxql_port;
		$this->_sd_pid_file		= $sd_pid_file;
		$this->_custom_test		= "";
		$this->_compat098		= false;
		$this->_skip_indexer	= false;
		$this->_use_sphinxql	= false;
		$this->_indexdata		= array ();
		$this->_connection		= false;
		$this->_testdir			= "";
	}

	function EnableCompat098 ()		{ $this->_compat098 = true; }	
	function SubtestNo ()			{ return $this->_subtest; }
	function SubtestCount ()		{ return $this->_subtestcount; }
	function Name ()				{ return $this->_name; }
	function DB_Drop ()				{ return $this->_db_drop; }
	function DB_Create ()			{ return $this->_db_create; }
	function DB_Insert ()			{ return $this->_db_insert; }
	function DB_CustomInsert ()		{ return $this->_custom_insert; }
	function NumAgents ()			{ return $this->_num_agents; }
	function Requires ( $name )		{ return isset($this->_prereqs[$name]); }
	function IsQueryTest ()			{ return strlen ( $this->_custom_test ) == 0;	}
	function IsSphinxqlTest ()		{ return $this->_use_sphinxql; }
	function IsNeedDB()				{ return ! ( empty ( $this->_db_drop )
										&& empty ( $this->_db_create )
										&& empty ( $this->_db_insert ) ); }
	function IsRt()
	{
		global $g_locals;
		if ( !array_key_exists ('rt_mode', $g_locals) )
			return false;
		return $g_locals['rt_mode'];
	}
	function NeedIndexerEx ()
	{
		return count ( $this->_indexer_runs ) > 0;
	}
	function Results ()				{ return $this->_results; }
	function GetQuery ( $i )		{ return $this->_queries[$i]; }
	function IsSkipIndexer ()		{ return $this->_skip_indexer; }

	function SetTestDir ( $dir )	{ $this->_testdir = $dir; }

	function GetLocal ( $key )
	{
		global $g_locals;
		if ( !array_key_exists ( $key, $g_locals ) )
		{
			printf ( "FATAL: unbound local variable '%s' (go add it at ~/.sphinx).\n", $key );
			exit ( 1 );
		}
		return $g_locals[$key];
	}

	function CreateNextConfig ()
	{
		return $this->GenNextCfg ( 0 );
	}


	function SubtestFinished ()
	{
		$this->_subtest++;
	}


	function SubtestFailed ()
	{
		$this->_subtest++;

		$failed = array ();
		array_push ( $failed, "failed" );

		if ( IsModelGenMode () )
			array_push ( $this->_results_model, $failed );
	}

	
	function ModelSubtestFailed ()
	{
		$failed = array ();
		array_push ( $failed, "failed" );

		return $this->_results_model [$this->SubtestNo ()] == $failed;
	}


	function SetAgent ( $agent )
	{
		if ( !is_array ( $agent ) )
			return;

		$this->_sd_address = $agent ["address"];
		$this->_sd_port = $agent ["port"];
		$this->_sd_sphinxql_port = $agent ["sqlport"];
	}

	
	function SetPIDFile ( $pidfile )
	{
		$this->_sd_pid_file = $pidfile;
	}


	function GenNextCfg ( $i )
	{
		if ( count ( $this->_dynamic_entries ) == 0 )
			return FALSE;

		$num_variants = count ( ChildrenArray ( $this->_dynamic_entries[$i], "variant" ) );
	
		if ( $this->_counters [$i] == $num_variants - 1 )
		{
			if ( $i == count ( $this->_dynamic_entries ) - 1 )
				return FALSE;
			else
			{
				$this->_counters [$i] = 0;
				return $this->GenNextCfg ( $i + 1 );
			}
		}
		else
			$this->_counters [$i]++;

		return TRUE;
	}
	
	
	function WriteCustomTestResults ( $fp )
	{
		$res_fmt = $this->FormatResultSet ( 0, $this->_results );
		fwrite ( $fp, $res_fmt );
	}

	function GatherEntities ( $node, &$array )
	{
		foreach ( ChildrenArray($node) as $child )
			if ( $child->nodeType == XML_ELEMENT_NODE )
				array_push ( $array, $child->nodeValue );
	}


	function GatherNodes ( $node )
	{
		if ( $node->nodeType != XML_TEXT_NODE && $node->nodeType != XML_DOCUMENT_NODE
			&& strtolower ( $node->nodeName ) == "dynamic" )
		{
			$node->id = count ( $this->_dynamic_entries );
			array_push ( $this->_dynamic_entries, $node );
			array_push ( $this->_counters, 0 );
		}

		for ( $i = 0; !is_null ( $node->childNodes ) && $i < $node->childNodes->length; $i++ )
			$this->GatherNodes ( $node->childNodes->item ( $i ) );
	}


	function ParseRange ( $range )
	{
		if ( !$range )
			return false;

		$values = explode ( ' ', $range );
		if ( count($values) != 2 )
		{
			printf ( "ERROR: malformed range attribute: '%s'\n", $range );
			return false;
		}

		return array ( 'min' => $values[0], 'max' => $values[1] );
	}

	function ParseIndexWeights ( $weights )
	{
		if ( !$weights )
			return false;

		$result = array();
		preg_match_all ( '/([^\s]+):(\d+)/', $weights, $matches, PREG_SET_ORDER );
		foreach ( $matches as $match )
			$result [ $match[1] ] = (int)$match[2];

		return $result;
	}

	function Load ( $config_file )
	{
		// load the file
		$doc = new DOMDocument ( "1.0", "utf-8" );
		if ( !$doc->load ( $config_file ) )
			return false;

		// check for proper root node
		if ( !$doc->hasChildNodes() )
			return false;

		$xml = $doc->childNodes->item(0);
		if ( strtolower($xml->nodeName)!="test" )
			return false;

		$custom = GetFirstChild ( $xml, "custom_test" );
		if ( $custom )
		{
			$this->_custom_test = $custom->nodeValue;
			if ( $doc->encoding != 'utf-8' )
				$this->_custom_test = iconv ( 'utf-8', $doc->encoding, $this->_custom_test );
		}

		// extract indexer run params
		$indexer_run = GetFirstChild ( $xml, "indexer" );
		if ( $indexer_run )
		{
			foreach ( ChildrenArray ( $indexer_run, "run" ) as $run )
				$this->_indexer_runs [] = $run->nodeValue;
		}

		// extract queries
		$qs = GetFirstChild ( $xml, "queries" );
		if ( $qs )
		{
			// new and cool
			foreach ( ChildrenArray ( $qs, "query" ) as $q )
			{
				$res = array ( "query" => array ( $q->nodeValue ) );

				// parse query mode
				$mode = 0;
				$mode_s = $q->getAttribute("mode");
				switch ( $mode_s )
				{
					case "":			$mode_s = "(default)"; break;
					case "all":			$mode = SPH_MATCH_ALL; break;
					case "any":			$mode = SPH_MATCH_ANY; break;
					case "phrase":		$mode = SPH_MATCH_PHRASE; break;
					case "extended":	$mode = SPH_MATCH_EXTENDED; break;
					case "extended2":	$mode = SPH_MATCH_EXTENDED2; break;
					default:
						printf ( "$config_file: unknown matching mode '%s'\n", $mode_s );
						return false;
				}
				$res["mode"] = $mode;
				$res["mode_s"] = $mode_s;

				// parse ranker
				$ranker = 0;
				$ranker_s = $q->getAttribute("ranker");
				switch ( $ranker_s )
				{
					case "":				$ranker_s = "(default)"; break;
					case "proximity_bm25":	$ranker = SPH_RANK_PROXIMITY_BM25; break;
					case "bm25":			$ranker = SPH_RANK_BM25; break;
					case "none":			$ranker = SPH_RANK_NONE; break;
					case "wordcount":		$ranker = SPH_RANK_WORDCOUNT; break;
					default:
						printf ( "$config_file: unknown ranker '%s'\n", $ranker_s );
						return false;
				}
				$res["ranker"] = $ranker;
				$res["ranker_s"] = $ranker_s;

				// parse filter
				$res["filter"] = $q->getAttribute("filter");
				$res["filter_value"] = $q->getAttribute("filter_value" );
				$res["filter_range"] = $this->ParseRange ( $q->getAttribute("filter_range" ) );

				// parse sort mode and get clause
				$sortmode = 0;
				$sortmode_s = $q->getAttribute("sortmode");
				switch ( $sortmode_s )
				{
					case "":			$sortmode_s = "(default)"; break;
					case "extended":	$sortmode = SPH_SORT_EXTENDED; break;
					case "expr":		$sortmode = SPH_SORT_EXPR; break;
					default:
						printf ( "$config_file: unknown sorting mode '%s'\n", $sortmode_s );
						return false;
				}
				$res["sortmode"] = $sortmode;
				$res["sortmode_s" ] = $sortmode_s;
				$res["sortby"] = $q->getAttribute("sortby");

				// groupby
				$groupfunc = 0;
				$groupfunc_s = $q->getAttribute("groupfunc");
				switch ( $groupfunc_s )
				{
					case "":			$groupfunc = SPH_GROUPBY_ATTR; $groupfunc_s = "attr"; break;
					case "day":			$groupfunc = SPH_GROUPBY_DAY; break;
					case "week":		$groupfunc = SPH_GROUPBY_WEEK; break;
					case "month":		$groupfunc = SPH_GROUPBY_MONTH; break;
					case "year":		$groupfunc = SPH_GROUPBY_YEAR; break;
					case "attr":		$groupfunc = SPH_GROUPBY_ATTR; break;
					case "attrpair":	$groupfunc = SPH_GROUPBY_ATTRPAIR; break;
					default:
						printf ( "$config_file: unknown groupby func '%s'\n", $groupfunc_s );
						return false;
				}

				$res["groupfunc"] = $groupfunc;
				$res["groupfunc_s"] = $groupfunc_s;
				$res["groupattr"] = $q->getAttribute("groupattr");
				$groupsort = $q->getAttribute("groupsort");
				if ( $groupsort == "" )
					$groupsort = "@group desc";

				$res["groupsort"] = $groupsort;
				$res["groupdistinct"] = $q->getAttribute("groupdistinct");

				$res["resarray"] = $q->getAttribute("resarray");
				$res["index"] = $q->getAttribute("index");
				$res["select"] = $q->getAttribute("select");
				$res["id_range"] = $this->ParseRange ( $q->getAttribute("id_range") );
				$res["index_weights"] = $this->ParseIndexWeights ( $q->getAttribute("index_weights") );
				$res["roundoff"] = $q->getAttribute("roundoff");
				$res["expect_error"] = $q->getAttribute("expect_error");
				$res["tag"] = $q->getAttribute("tag");
				$res["cutoff"] = $q->getAttribute("cutoff");

				// add query
				if ( $q->getAttribute("source") )
				{
					$source = $q->getAttribute("source");
					if ( substr ( $source, 0, 6 ) == "local:" )
						$source = $this->GetLocal ( substr ( $source, 6 ) );
					if ( !is_readable($source) )
					{
						printf ( "FATAL: query source file '%s' not found.\n", $source );
						exit ( 1 );
					}
					$queries = file ( $source, FILE_IGNORE_NEW_LINES );
					$limit = $this->GetLocal('qlimit');
					$res["query"] = $limit ? array_slice( $queries, 0, $limit ) : $queries;
				}
				$this->_queries[] = $res;
			}
		}
		else
		{
			// legacy
			$qs = array ();
			$this->GatherEntities ( GetFirstChild ( $xml, "query" ), $qs );
			foreach ( $qs as $q )
			{
				$this->_queries[] = array ( "query"		=> array ( $q ),
											"mode"		=> 0,
											"mode_s"	=> "(default)",
											"ranker"	=> 0,
											"ranker_s"	=> "(default)" );
			}
		}

		// extract queries
		$qs = GetFirstChild ( $xml, "sphqueries" );
		if ( $qs )
		{
			$this->_use_sphinxql = true;
			// new and cool
			foreach ( ChildrenArray ( $qs, "sphinxql" ) as $q )
				$this->_sphqueries[] = array ( "query" => $q->nodeValue );
		}


		// extract my settings
		$this->_config = GetFirstChild ( $xml, "config" );
		$this->GatherNodes ( $this->_config );
		$this->GatherEntities ( GetFirstChild ( $xml, "query_attributes" ), $this->_query_attributes );

		foreach ( ChildrenArray ( $xml, "db_create" ) as $node )
			$this->_db_create []=$node->nodeValue;

		foreach ( ChildrenArray ( $xml, "db_drop" ) as $node )
			$this->_db_drop []=$node->nodeValue;

		foreach ( ChildrenArray ( $xml, "db_insert" ) as $node )
			$this->_db_insert []=$node->nodeValue;

		foreach ( ChildrenArray ( $xml, "custom_insert" ) as $node )
			$this->_custom_insert []=$node->nodeValue;

		$this->_name			= GetFirstChildValue ( $xml, "name" );
		$this->_query_settings	= GetFirstChildValue ( $xml, "query_settings" );
		$this->_num_agents		= GetFirstChildValue ( $xml, "num_agents", 1 );
		$this->_skip_indexer	= GetFirstChildValue ( $xml, "skip_indexer", false );

		$this->_prereqs = array();
		$prereqs = GetFirstChild ( $xml, "requires", false );
		if ( $prereqs )
			foreach ( ChildrenArray ( $prereqs ) as $node )
				$this->_prereqs [ $node->nodeName ] = 1;

		// precalc subtests count
		$this->_subtestcount = 1;
		foreach ( $this->_dynamic_entries as $entry )
		{
			$variants = count ( ChildrenArray ( $entry, "variant" ) );
			$this->_subtestcount *= max ( $variants, 1 );
		}

		return true;
	}


	function RunIndexerEx ( &$error )
	{
		foreach ( $this->_indexer_runs as $param )
		{
			$retval = RunIndexer ( $error, $param );
			if ( $retval != 0 )
				return $retval;
		}

		return 0;
	}

	function RunQuerySphinxQL ( &$error )
	{
		global $sd_address, $sd_sphinxql_port, $action_retries, $action_wait_timeout;
		$query_results = array ();
		$total = $done = 0;
		if ($sd_address == "localhost")
			$connect_string = "127.0.0.1:$sd_sphinxql_port";
		else
			$connect_string = "$sd_address:$sd_sphinxql_port";
		$connection = @mysql_connect ( $connect_string );
		if ( $connection === false )
			return false;
		for ( $n=0; $n<count($this->_sphqueries); $n++ )
		{
			$query_result = array ();
			$query = $this->_sphqueries[$n]["query"];
			$query_result["sphinxql"]=$query;
			$result = @mysql_query ($query,$connection);
			if ($result===true)
				$query_result["total_affected"] = mysql_affected_rows($connection);
			else if ($result===false)
			{
				$query_result["error"] = mysql_error( $connection );
				$query_result["errno"] = mysql_errno( $connection );
			} else
			{
				$query_result["total_rows"] = mysql_num_rows($result);
				while ($row = mysql_fetch_array($result, MYSQL_ASSOC))
				{
					foreach ($row as $key => $value)
						$foo[$key] = $value;
					$query_result["rows"][] = $foo;
				}
				mysql_free_result($result);
			}
			$query_results[] = $query_result;
		}
		$this->_results = $query_results;

		if ( IsModelGenMode () )
			array_push ( $this->_results_model, $query_results );

		mysql_close ( $connection );
		return true;
	}


	function RunQuery ( $index, &$error, $benchmark = null )
	{
		global $sd_address, $sd_port, $action_retries, $action_wait_timeout;
		
		$query_results = array ();		
		$total = $done = 0;
		if ( $benchmark )
		{
			foreach ( $this->_queries as $qinfo )
				$total += count($qinfo['query']);
			$prefix = $benchmark;
			$compact = true;
			$tm = 0;
			$start = MyMicrotime();
		}
		else
			$compact = false;

		$cl = new SphinxClient;
		$pconn = $benchmark && method_exists ( $cl, 'Open' );
		if ( $pconn )
		{
			$cl = new SphinxClient ();
			$cl->SetServer ( $sd_address, $sd_port );
			$cl->Open ();
		}

		$retries = 1;
		if ( !$benchmark )
			$retries = $action_retries;

		for ( $n=0; $n<count($this->_queries); $n++ )
		{
			$qinfo = $this->_queries[$n];
			foreach ( $qinfo['query'] as $query )
			{
				if ( $benchmark && MyMicrotime() > $tm )
				{
					$tm = MyMicrotime();
					$est = $done ? ( ( $tm - $start ) / $done ) * ( $total - $done ) : 0 ;
					$qps = $done / ( $tm - $start );
					printf ( "\r$prefix %d/%d (est. %s, qps %.1f)", $done, $total, sphFormatTime($est), $qps );
					$tm += 1;
				}
				$bOk = FALSE;
				for ( $i=0; $i<$retries && !$bOk; $i++ )
				{
					if ( !$pconn )
					{
						$cl = new SphinxClient ();
						$cl->SetServer ( $sd_address, $sd_port );
					} else
					{
						$cl->ResetFilters ();
						$cl->ResetGroupBy ();
					}

					$results = 0;
					if ( empty($this->_query_settings) )
					{
						$my_index = $index;
						if ( @$qinfo["mode"] )		$cl->SetMatchMode ( $qinfo["mode"] );
						if ( @$qinfo["ranker"] )	$cl->SetRankingMode ( $qinfo["ranker"] );
						if ( @$qinfo["sortmode"] )	$cl->SetSortMode ( $qinfo["sortmode"], $qinfo["sortby"] );
						if ( @$qinfo["groupattr"] )	$cl->SetGroupBy ( $qinfo["groupattr"], $qinfo["groupfunc"], $qinfo["groupsort"] );
						if ( @$qinfo["groupdistinct"] )	$cl->SetGroupDistinct ( $qinfo["groupdistinct"] );
						if ( @$qinfo["resarray"] )	$cl->SetArrayResult ( true );
						if ( @$qinfo["select"] )	$cl->SetSelect ( $qinfo["select"] );
						if ( @$qinfo["id_range"] )	$cl->SetIDRange ( $qinfo["id_range"]["min"], $qinfo["id_range"]["max"] );
						if ( @$qinfo["index"] )		$my_index = $qinfo["index"];
						if ( @$qinfo["index_weights"] ) $cl->SetIndexWeights ( $qinfo["index_weights"] );
						if ( @$qinfo["cutoff"] )	$cl->SetLimits ( 0, 20, 0, $qinfo["cutoff"] );
						if ( @$qinfo["filter"] )
						{
							$name = $qinfo["filter"];
							if ( @$qinfo["filter_value"] )
								$cl->SetFilter ( $name, array ( $qinfo["filter_value"] ) );
							elseif ( @$qinfo["filter_range"] )
							{
								$range = $qinfo["filter_range"];
								$cl->SetFilterRange ( $name, $range['min'], $range['max'] );
							}
						}
						
						$results = $cl->Query ( $query, $my_index, "run".(1+$this->SubtestNo()) );
						if ( is_array($results) )
						{
							$results["resarray"] = (int)@$qinfo["resarray"];
							$results["roundoff"] = (int)@$qinfo["roundoff"];
						}
					}
					else
					{
						$run_func = create_function( '$client, $query, $index, &$results', $this->_query_settings );
						$run_func ( $cl, $query, $index, $results ); 
					}
					
					if ( $results )
					{
						// let also work with "array of arrays" result
						if ( array_key_exists ( "error",$results ) )
						{
							$bOk = TRUE;
							if ( $compact )
								$results = array ( $n, $results['total'], $results['total_found'], $results['time'] );
							else
								$results ["query"] = $query;

							array_push ( $query_results, $results );
						} else
						foreach ( $results as $result )
						{
							$bOk = TRUE;
							if ( $compact )
								$result = array ( $n, $result['total'], $result['total_found'], $result['time'] );
							else
								$result ["query"] = $query;

							array_push ( $query_results, $result );
						}
					}
					else if ( @$qinfo["expect_error"] && !$cl->IsConnectError() )
					{
						$bOk = true;
						array_push ( $query_results, array (
							"query" => $query,
							"error" => $cl->GetLastError(),
							"warning" => "",
							"total" => 0,
							"total_found" => 0,
							"time" => 0 ) );
					}
					else
					{
						if ( method_exists ( $cl, 'IsConnectError' ) && $cl->IsConnectError() )
							usleep ( $action_wait_timeout );
						else if ( $benchmark && $done )
						{
							array_push ( $query_results, array ( $n, -1, -1, 0 ) );
							$bOk = true;
						}
						else
							break;
					}
				}
				$done++;

				if ( !$bOk )
				{
					$error = $cl->GetLastError ();
					return FALSE;
				}
			}
		}

		$this->_results = $query_results;

		if ( IsModelGenMode () && count($this->_queries) !=0 )
			array_push ( $this->_results_model, $query_results );

		if ( $benchmark )
			printf ( " - done in %s\n", sphFormatTime ( MyMicrotime() - $start ) );

		if ( $pconn )
			$cl->Close ();

		return TRUE;
	}

	
	function RunCustomTest ( & $error )
	{
		global $sd_address, $sd_port, $action_retries, $action_wait_timeout, $g_locals;

		$bOk = false;
		$results = false;

		for ( $i = 0; $i < $action_retries && !$bOk; $i++ )
		{
			$cl = new SphinxClient ();
			$cl->SetServer ( $sd_address, $sd_port );

			$results = false;
			$run_func = create_function( '$client, &$results', $this->_custom_test );

			if ( !@mysql_connect ( $g_locals['db-host'].":".$g_locals['db-port'],
								$g_locals['db-user'], $g_locals['db-password'] ) ||
								!@mysql_select_db ( $g_locals['db-name'] ) )
					return FALSE;

			$run_func ( $cl, $results );

			@mysql_close();

			if ( $results )
				$bOk = TRUE;
			else
				usleep ( $action_wait_timeout );
		}

		if ( !$bOk )
		{
			$error = $cl->GetLastError ();
			return FALSE;
		}

		$my_results = array ();
		$my_results [] = $results;

		$this->_results = $my_results;

		if ( IsModelGenMode () )
			array_push ( $this->_results_model, $my_results );

		return TRUE;
	}


	function FixKeys ( $v )
	{
		if ( is_array($v) )
		{
			$result = array();
			foreach ( $v as $key=>$value )
			{
				if ( $key==PHP_INT_MAX || $key==-PHP_INT_MAX-1 )
					$key = (int)$key;
				$result[$key] = $this->FixKeys ( $value );
			}
			return $result;
		}
		else
			return $v;
	}


	function LoadModel ( $filename )
	{
		if ( ! IsModelGenMode () )
		{
			if ( ! file_exists ( $filename ) )
				return FALSE;

			$contents = file_get_contents ( $filename );
			if ( ! $contents )
				return FALSE;

			$this->_results_model = $this->FixKeys ( unserialize ( $contents ) );
		}

		return TRUE;
	}


	function CompareToModel ()
	{
		return $this->CompareResults ( $this->FixKeys ( $this->_results ), $this->_results_model [$this->SubtestNo ()] );
	}


	function CompareResultSetFixup ( &$set, $roundoff )
	{
		if ( !is_array($set) )
			return;

		if ( $roundoff && !@$set["resarray"] ) // FIXME! support resarray too
			foreach ( $set["attrs"] as $name=>$type )
				if ( $type==SPH_ATTR_FLOAT )
		{
			foreach ( $set["matches"] as $id=>$match )
				$set["matches"][$id]["attrs"][$name] = sprintf ( "%.{$roundoff}f",
					$set["matches"][$id]["attrs"][$name] );
		}

		foreach ( preg_split ( "/\\W+/", "time warning status fields resarray roundoff" ) as $key )
			unset ( $set[$key] );
	}

	function CompareResultSets ( $set1, $set2 )
	{
		$roundoff = 0;
		if ( isset($set1["roundoff"]) ) $roundoff = $set1["roundoff"];
		if ( isset($set2["roundoff"]) ) $roundoff = $set2["roundoff"];

		$this->CompareResultSetFixup ( $set1, $roundoff );
		$this->CompareResultSetFixup ( $set2, $roundoff );

		return $set1==$set2;
	}

	function CompareResults ( $query1, $query2 )
	{
		if ( count($query1)!=count($query2) )
			return false;

		for ( $i=0; $i<count($query1); $i++ )
			if ( !$this->CompareResultSets ( $query1[$i], $query2[$i] ) )
				return false;

		return true;
	}


	function CheckVariants ( $output_path )
	{
		if ( !$this->Requires("variant_match") )
			return true;
		
		$total = count ( $this->_results_model );
		if ( $total==1 )
		{
			printf ( "Variant match is required, but there are no variants.\n" );
			return false;
		}
		else if ( !$this->IsQueryTest() )
		{
			printf ( "Variant match is not supported for non-query tests.\n" );
			return false;
		}

		$failed = false;
		$output = '';
		for ( $i=1; $i<$total; $i++ )
		{
			$nqueries = count ( $this->_results_model[0] );
			for ( $k=0; $k<$nqueries; $k++ )
			if ( !$this->CompareResultSets ( $this->_results_model[0][$k], $this->_results_model[$i][$k] ) )
			{
				$first = $this->FormatResultSet ( $k+1, $this->_results_model[0][$k] );
				$current = $this->FormatResultSet ( $k+1, $this->_results_model[$i][$k] );

				file_put_contents ( "first", $first );
				file_put_contents ( "current", $current );
				system ( "diff --unified=3 first current > diff.txt" );
						
				$diff = file_get_contents ( "diff.txt" );
				unlink ( "current" );
				unlink ( "first" );
				unlink ( "diff.txt" );

				$output .= $diff . "\n";
				$failed = true;
			}
		}

		if ( $failed )
			file_put_contents ( $output_path, $output );
		return !$failed;
	}


	function WriteReportHeader ( $fp )
	{
		fprintf ( $fp, "==== Run %d ====\n", $this->SubtestNo () + 1 );
		fwrite ( $fp, "Settings:\n" );
		$this->WriteDiff ( $fp );
		fwrite ( $fp, "\n" );

		if ( !empty ( $this->_query_settings ) )
			fprintf ( $fp, "Query settings:\n%s\n", $this->_query_settings );
	}


	function FormatResultSet ( $nquery, $result, $opts=array() )
	{
		if ( !$this->IsQueryTest () || !is_array($result) )
			return var_export ( $result, true )."\n";

		if ( array_key_exists ("sphinxql", $result) )
		{
			$str = "sphinxql> $result[sphinxql];\n";
			if ( array_key_exists ("total_affected", $result) )
				$str .= "Query OK, $result[total_affected] rows affected\n";
			else if ( array_key_exists ("error", $result) )
				$str .= "ERROR $result[errno]: $result[error]\n";
			else if (array_key_exists ("rows", $result) )
			{
				foreach ( $result["rows"][0] as $key=>$s )
					$str .= "\t$key";
				$str .= "\n";
				foreach ($result["rows"] as $row)
				{
					foreach ($row as $value)
						$str .= "\t$value";
					$str .="\n";
				}
				$str .="$result[total_rows] rows in set\n";
		}
			return $str."\n";
	}

		// format header
		$qinfo = @$this->_queries[$nquery-1];
		if ( @array_key_exists ( "index", $qinfo ) && $qinfo ["index"] != '*' )
			$str = "--- Query $nquery (mode=$qinfo[mode_s],ranker=$qinfo[ranker_s],index=$qinfo[index]) ---\n";
		else
			$str = "--- Query $nquery (mode=$qinfo[mode_s],ranker=$qinfo[ranker_s]) ---\n";

		if ( @$qinfo["groupattr"] )
			$str .= "GroupBy: attr: '".$qinfo["groupattr"]."' func: '".$qinfo["groupfunc_s"]."' sort: '".$qinfo["groupsort"]."'\n";

		if ( @$qinfo["sortmode"] == SPH_SORT_EXPR )
			$str .= "Sort: expr: ".$qinfo["sortby"]."\n";

		$str .= "Query '$result[query]': retrieved $result[total_found] of $result[total] matches in $result[time] sec.\n";
		if ( $result["error"] )
			$str .= "Error: $result[error]\n";
		if ( $result["warning"] )
			$str .= "Warning: $result[warning]\n";

		$array_result = @$result["resarray"];

		// format keywords
		if ( isset($result["words"]) && is_array($result["words"]) )
		{
			$str .= "Word stats:\n";
			foreach ( $result ["words"] as $word => $word_result )
			{
				$hits = $word_result ["hits"];
				$docs = $word_result ["docs"];
				$str .= "\t'$word' found $hits times in $docs documents\n";
			}
		}

		// format attribute types
		if ( @$opts["format_attrs"] )
		{
			$typenames = array (
				SPH_ATTR_INTEGER => "int",
				SPH_ATTR_TIMESTAMP=> "timestamp",
				SPH_ATTR_ORDINAL => "ordinal",
				SPH_ATTR_BOOL => "bool",
				SPH_ATTR_FLOAT => "float",
				SPH_ATTR_BIGINT => "bigint",
				SPH_ATTR_STRING => "string",
				SPH_ATTR_MULTI | SPH_ATTR_INTEGER => "mva" );

			$n = 1;
			$str .= "Result set attributes:\n";
			foreach ( $result["attrs"] as $name=>$type )
			{

				$typename = "type-$type";
				if ( $typenames[$type] )
					$typename = $typenames[$type];

				$str .= "\tattr $n: $typename $name\n";
				$n++;
			}
		}

		// check our table for well-known id column names
		$idcol = "";
		if ( $this->IsNeedDB() )
			$r = mysql_query ( "DESC test_table", $this->_connection );
		else
			$r = false;
		if ( $r )
		{
			while ( $row = mysql_fetch_assoc($r) )
			{
				$idcand = strtolower ( $row["Field"] );
				if ( array_search ( $idcand, array ( "id", "document_id" ) ) )
				{
					$idcol = $idcand;
					break;
				}
			}
		}

		// format matches
		$str .= "\n";
		if ( isset($result["matches"]) && is_array($result["matches"]) )
		{
			$n = 1;
			$str .= "Matches:";
			foreach ( $result ["matches"] as $doc => $docinfo )
			{
				$doc_id = $array_result ? $docinfo["id"] : $doc;
				$weight = $docinfo["weight"];

				$str .= "\n$n. doc_id=$doc_id, weight=$weight";
				$n++;

				// only format specified attrs if requested
				if ( !empty ( $this->_query_attributes ) )
				{
					foreach ( $this->_query_attributes as $attr )
						if ( isset($docinfo ["attrs"][$attr]) )
					{
						$val = $docinfo["attrs"][$attr];
						if ( is_array ( $val ) )
							$val = join ( " ", $val );
						$str .= " $attr=$val";
					}
					continue;
				}

				// fetch and format fields from db by default
				if ( $idcol )
				{
					if ( $this->IsNeedDB() )
						$query_res = mysql_query ( "select * from test_table where document_id = $doc_id",
							$this->_connection);
					else
						$query_res = false;
					if ( $query_res )
					{
						$row = mysql_fetch_assoc ( $query_res );
						if ( $row )
							foreach ( $row as $col_name => $col_content )
								if ( array_search ( $col_name, $result["fields"] )!==false )
									$str .= " $col_name=\"$col_content\"";
					}
				}

				// format attrs
				foreach ( $docinfo["attrs"] as $attr=>$val )
				{
					if ( is_array($val) )
						$val = join ( ",", $val );
					$str .= " $attr=\"$val\"";
				}
			}
			$str .= "\n\n";
		}
		return $str . "\n";
	}

	/// format and write a single result set into log file
	function WriteQuery ( $fp, $nquery, $result )
	{
		$res_fmt = $this->FormatResultSet ( $nquery, $result );
		fwrite ( $fp, $res_fmt );
	}

	/// write all the result sets
	function WriteResults ( $fp )
	{
		if ( $this->IsQueryTest () )
		{
			$nquery = 1;
			foreach ( $this->_results as $result )
				$this->WriteQuery ( $fp, $nquery++, $result );
		}
		else
			$this->WriteCustomTestResults ( $fp );
	}

	/// write difference from the reference result sets
	function WriteReferenceResultsDiff ( $fp )
	{
		global $windows;

		$nquery = 0;
		if ( !is_array ( $this->_results_model [ $this->SubtestNo() ] ) )
			return;

		foreach ( $this->_results_model [ $this->SubtestNo() ] as $ref )
		{
			$cur = $this->_results[$nquery];
			if ( $this->CompareResultSets ( $ref, $cur ) )
			{
				$nquery++;
				continue;
			}

			$opts = array();
			if ( isset($cur["attrs"]) || isset($ref["attrs"]) )
				if ( @$cur["attrs"]!=@$ref["attrs"] )
					$opts["format_attrs"] = 1;

			$result_f_cur = $this->FormatResultSet ( $nquery+1, $this->_results[$nquery], $opts );
			$result_f_ref = $this->FormatResultSet ( $nquery+1, $ref, $opts );
			file_put_contents ( "current", $result_f_cur );
			file_put_contents ( "reference", $result_f_ref );
			system ( "diff --unified=3 reference current > diffed.txt" );

			$diffed = file_get_contents ( "diffed.txt" );
			unlink ( "current" );
			unlink ( "reference" );
			unlink ( "diffed.txt" );

			$nquery++;
			fwrite ( $fp, "=== query $nquery diff start ===\n" );
			fwrite ( $fp, $diffed );
			fwrite ( $fp, "=== query $nquery diff end ===\n" );
		}

		$nref = count ( array_keys ( $this->_results_model [ $this->SubtestNo() ] ) );
		$nres = count ( array_keys ( $this->_results ) );
		if ( $nres > $nref )
		{
			$delta = $nres - $nref;
			fwrite ( $fp, "$delta result set(s) missing from model!\n" );
		}
	}

	function EraseIndexFiles ( $path )
	{
		$dh = glob ( "./$path.*" );
		foreach ( $dh as $entry )
		{
			if ( is_file ($entry) )
				unlink ($entry);
		}
	}

	function WriteConfig ( $filename, $agentid, &$msg, $collectdata = true )
	{
		global $g_locals;
		$fp = fopen ( $filename, 'w' );
		if ( !$fp )
		{
			$msg = "Can't open file $filename for writing";
			return FALSE;
		}

		$this->Dump ( $this->_config, $fp, false, $agentid );
		fclose ( $fp );

		$fp = fopen ( $filename, 'r' );
		if ( !$fp )
		{
			$msg = "Can't open file $filename for reading";
			return FALSE;
		}

		$config = fread ( $fp, filesize ( $filename ) );
		fclose ( $fp );

		// for rt case - extract the schema from the config
		// and make the new config, making the index as rt instead
		if ( $this->IsRt() )
		{
			$pattern = "/.*?(?<body>source\s+(?<srcname>\S*?)(\s*\:\s*(?<parent>\S*?))?\s*\{(?<content>.*?)\})(?<epilog>.*?)/s";
			preg_match_all ( $pattern, $config, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE );
			$schemas = array();
			$shift = 0;
			$newconfig = "";

			// parse sources
			foreach ( $matches as $match )
			{
				$lines = explode("\n", $match['content'][0]);
				$insert_schema = array();
				$insert_types = array();
				if ( $match['parent'][0] != "" )
					$insert_types = $schemas[$match['parent'][0]]['types'];

				$sql_query_pre = array();
				$sql_query = "";

				foreach ( $lines as $line )
				{
					// skip comment lines (if any)
					if ( preg_match ( "/\s*#/" , $line ) > 0 )
						continue;

					// extract config key/value pairs
					$eq = strpos ( $line,"=" );
					if ($eq == 0)
						continue;
					$key = strtolower ( trim ( substr($line,0,$eq), " \t" ) );
					$value = trim ( substr($line,$eq+1), " \t" );

					// handle known keys
					switch ( $key )
					{
						case "type":
							if ( $value != "mysql" )
							{
								$msg = "non-mysql source (type=$value), skipping...";
								return FALSE;
							}
							break;

						case "sql_attr_uint":		$insert_types[$value] = "rt_attr_uint"; break;
						case "sql_attr_bigint":		$insert_types[$value] = "rt_attr_bigint"; break;
						case "sql_attr_float":		$insert_types[$value] = "rt_attr_float"; break;
						case "sql_attr_timestamp":	$insert_types[$value] = "rt_attr_timestamp"; break;
						// case "sql_attr_multi" TBD
						// case "sql_attr_string" TBD

						case "sql_query_pre":		$sql_query_pre[] = $value; break;
						case "sql_query":			$sql_query = $value; break;
					}
				}

				// query is kinda mandatory
				if ( !$sql_query )
				{
					$msg = "missing sql_query";
					return false;
				}

				// now let's connect to MySQL, run the query, and fetch the values
				$conn = ConnectDB();
				if ( !$conn )
				{
					$msg = "can't connect or select the database";
					return false;
				}

				// gotta run pre-queries first!
				foreach ( $sql_query_pre as $q )
				{
					if ( @mysql_query ( $q, $conn ) )
						continue;

					$msg = sprintf ( "sql_query_pre failed (query=%s, error=%s)", $q, mysql_error ( $conn ) );
					mysql_close ( $conn );
					return false;
				}

				// run main query
				$res = @mysql_query ( $sql_query, $conn );
				if ( !$res )
				{
					$msg = sprintf ( "sql_query failed (query=%s, error=%s)", $sql_query, mysql_error ( $conn ) );
					$msg = "sql_query can't fetch test data: " . mysql_error ( $conn );
					mysql_close ( $conn );
					return false;
				}

				// fetch fields
				$insert_schema = array ( "id" => 0 );
				for ( $i=1; $i < mysql_num_fields($res); $i++ )
					$insert_schema [ mysql_fetch_field ( $res, $i )->name ] = $i;

				// fetch data
				$insert_values = array();
				while ( $row = mysql_fetch_row($res) )
					$insert_values[] = array_values ( $row );

				// cleanup
				mysql_free_result ( $res );
				mysql_close ( $conn );

				// store
				$schema = array();
				$schema['types'] = $insert_types;
				if ( $match['parent'][0] != "" )
					$schema['orders'] = $schemas[$match['parent'][0]]['orders'];
				else
					$schema['orders'] = $insert_schema;
				$schema['values'] = $insert_values;
				$schema['sqlport'] = $this->_sd_sphinxql_port;

				$schemas[$match['srcname'][0]] = $schema;
				$srclen = $match['epilog'][1] - $match['body'][1];
				$config = substr_replace ( $config, "", $match['body'][1]-$shift,$srclen );
				$shift += $srclen;
			}

			$pattern = "/.*?(?<body>index\s+(?<idxname>\S*?)(\s*\:\s*(?<parent>\S*?))?\s*\{(?<content>.*?)\})(?<epilog>.*?)/s";
			preg_match_all ( $pattern, $config, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE );
			$shift = 0;
			// parse indexes
			$indexes = array();
			foreach ( $matches as $match )
			{
				$idx = "index ".$match['idxname'][0];
				if ( $match['parent'][0] != "" )
					$idx .= " : ".$match['parent'][0];
				$idx .= "\n{\n";

				$lines = explode("\n", $match['content'][0]);
				$justcopy = false;
				$rtcopy = false;
				$idxbody = "";
				foreach ($lines as $line)
				{
					// skip comment lines (if any)
					if ( preg_match ( "/\s*#/" , $line ) > 0 )
						continue;
					$eq = strpos ( $line,"=" );
					if ($eq == 0)
						continue;
					$key = strtolower ( trim ( substr($line,0,$eq), " \t" ) );
					$value = trim ( substr($line,$eq+1), " \t" );
					switch ( $key )
					{
						case "type":
							if ($value=="rt")
								$rtcopy = true;
							else
								$justcopy = true;
							break;
						case "source";
							{
								$idxbody .= "\ttype\t= rt\n";
								if ( $collectdata )
									$indexes[$match['idxname'][0]] = $schemas[$value];
								foreach ( array_keys( $schemas[$value]['orders'] ) as $key )
									if ( $key != "id" && $key != "document_id" )
									{
										if ( array_key_exists ( $key, $schemas[$value]['types'] ) )
											$idxbody .= "\t".$schemas[$value]['types'][$key]."\t= $key\n";
										else
											$idxbody .= "\trt_field\t= $key\n";
									}
								break;
							}
						case "path": $this->EraseIndexFiles($value);
							if ($rtcopy)
								$justcopy = true;
							// no break!
						default:
							$idxbody .= "\t$key\t= $value\n";
					}
					if ( $justcopy ) // explicitly defined type, don't transform to rt.
					{
						$idxbody = $match['content'][0];
						break;
					}
				}
				$idx .= "$idxbody\n}\n";
				$srclen = $match['epilog'][1] - $match['body'][1];
				$config = substr_replace ($config, $idx, $match['body'][1]-$shift,$srclen );
				$shift += $srclen-strlen($idx);
			}
			if ( $collectdata )
				foreach ($indexes as $key => $value)
					$this->_indexdata[$key] = $value;
			$fp = fopen ( $filename, 'w' );
			if ( !$fp )
			{
				$msg = "Can't open $filename for writing";
				return FALSE;
			}
			fwrite ( $fp, $config );
			fclose ( $fp );
		}
		else // for rt indexes we need to clean up all index files before the run.
		{
			$pattern = "/.*?index\s+\S*?(\s*\:\s*\S*?)?\s*\{(.*?)\}.*?/s";
			preg_match_all ( $pattern, $config, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE );
			// parse indexes
			$indexes = array();
			foreach ( $matches as $match )
			{
				$lines = explode("\n", $match[2][0]);
				$path = "";
				$isrt = false;
				foreach ($lines as $line)
				{
					// skip comment lines (if any)
					if ( preg_match ( "/\s*#/" , $line ) > 0 )
						continue;
					$eq = strpos ( $line,"=" );
					if ($eq == 0)
						continue;
					$key = strtolower ( trim ( substr($line,0,$eq), " \t" ) );
					$value = trim ( substr($line,$eq+1), " \t" );
					switch ( $key )
					{
						case "type":
							if ($value=="rt")
								$isrt = true;
							break;
						case "path":
							$path = $value;
					}
					if ( $isrt && $path!="" )
					{
						$this->EraseIndexFiles($path);
						break;
					}
				}
			}
		}
		return TRUE;
	}

	function InsertIntoIndexer ( &$error )
	{
		global $sd_address, $sd_sphinxql_port, $action_retries, $action_wait_timeout;
		$address = $sd_address;
		if ($address == "localhost")
			$address = "127.0.0.1";

		$cn = false;
		$port = 0;
		foreach ( $this->_indexdata as $name => $data )
		{
			if ( $port != $data["sqlport"] )
			{
				$port = $data["sqlport"];
				$connect_string = "$address:$port";
				if ( $cn !== false )
					mysql_close ( $cn );
				$cn = @mysql_connect ( $connect_string );
			}
			if ( $cn === false )
				return false;

			$cols = join ( ", ", array_keys ( $data["orders"] ) );
			$prefix = "INSERT INTO $name ($cols) VALUES ";

			$accum = "";
			foreach ($data['values'] as $row)
			{
				$query = "";
				foreach ( $row as $column )
				{
					if ( $query!="" )
						$query .=",";
					$query .="'".$column."'";
				}
				
				if ( ( strlen ($accum) + strlen ($query) ) > 1024000 ) ///<checkit!
				{
					$result = @mysql_query ( $prefix.$accum, $cn );
					if ( $result === false )
					{
						$error = mysql_error ( $cn );
						return false;
					}
					$accum="";
				}

				if ( $accum != "" )
					$accum .=",";
				$accum .= "($query)";
			}
			// final chunk;
			if ( $accum !="" )
			{
				$result = @mysql_query ( $prefix.$accum, $cn );
				if ( $result === false )
				{
					$error = mysql_error ( $cn );
					return false;
				}
			}
		}
		if ( $cn )
			mysql_close ( $cn );
		return true;
	}

	function WriteDiff ( $fp )
	{
		$this->Dump ( $this->_config, $fp, true, "all" );
	}


	function WriteModel ( $filename )
	{
		if ( IsModelGenMode () )
			file_put_contents ( $filename, serialize ( $this->_results_model ) );
	}


	function WriteSearchdSettings ( $fp )
	{
		global $sd_log, $sd_query_log, $sd_read_timeout, $sd_max_children, $sd_pid_file, $sd_max_matches;

		if ( $this->_compat098 )
		{
			fwrite ( $fp, "\taddress	= {$this->_sd_address}\n" );
			fwrite ( $fp, "\tport		= {$this->_sd_port}\n" );
		}
		else
		{
			fwrite ( $fp, "\tlisten		= {$this->_sd_address}:{$this->_sd_port}\n" );
			fwrite ( $fp, "\tlisten		= {$this->_sd_address}:{$this->_sd_sphinxql_port}:mysql41\n" );
		}
		fwrite ( $fp, "\tlog			= $sd_log\n" );
		fwrite ( $fp, "\tquery_log		= $sd_query_log\n" );
		fwrite ( $fp, "\tread_timeout	= $sd_read_timeout\n" );
		fwrite ( $fp, "\tmax_children	= $sd_max_children\n" );
		fwrite ( $fp, "\tpid_file		= ".$this->_sd_pid_file."\n" );
		fwrite ( $fp, "\tmax_matches	= $sd_max_matches\n" );
		if ( $this->IsRt() )
			fwrite ( $fp, "\tworkers		= threads\n" );
			
	}

	function WriteSqlSettings ( $fp, $attributes )
	{
		global $g_locals;

		fwrite ( $fp, "\tsql_host		= " . $g_locals['db-host'] . "\n" );
		fwrite ( $fp, "\tsql_user		= " . $g_locals['db-user'] . "\n" );
		fwrite ( $fp, "\tsql_pass		= " . $g_locals['db-password'] . "\n" );
		fwrite ( $fp, "\tsql_port		= " . $g_locals['db-port'] . "\n" );

		$node = $attributes->getNamedItem('sql_db');
		fprintf ( $fp, "\tsql_db		= %s\n", $node ? $node->nodeValue : $g_locals['db-name'] );
	}

	function Dump ( $node, $fp, $dynamic_only, $agentid )
	{
		global $index_data_path, $agent_address, $agent_port;

		$nodename = strtolower ( $node->nodeName );

		if ( !$dynamic_only )
			switch ( $nodename )
		{
			case "#text":				fwrite ( $fp, $node->nodeValue ); return;
			case "static":				fwrite ( $fp, $node->nodeValue ); return;
			case "searchd_settings":	$this->WriteSearchdSettings ( $fp ); return;
			case "sql_settings":		$this->WriteSqlSettings ( $fp, $node->attributes ); return;
			case "agent_address":		fwrite ( $fp, $agent_address.":".$agent_port ); return;
			case "data_path":			fwrite ( $fp, $index_data_path ); return;
			case "local":				fwrite ( $fp, $this->GetLocal ( $node->nodeValue ) ); return;
			case "test_root":			fwrite ( $fp, dirname(__FILE__) ); return;
			case "this_test":			fwrite ( $fp, $this->_testdir ); return;
		}
		
		if ( $nodename=="variant" )
		{
			fwrite ( $fp, "$node->nodeValue\n" );

		} else if ( $nodename=="dynamic" )
		{
			if ( !is_null($node->id) )
			{
				$variants = ChildrenArray ( $node,"variant" );
				$this->Dump ( $variants[$this->_counters[$node->id]], $fp, $dynamic_only, $agentid );
			}
		} else if ( strpos ( $nodename, "agent" )===0 )
		{
				if ( $agentid==="all" || $nodename=="agent$agentid" )
					foreach ( ChildrenArray($node) as $child )
						$this->Dump ( $child, $fp, $dynamic_only, $agentid );
		} else
		{
				foreach ( ChildrenArray($node) as $child )
					$this->Dump ( $child, $fp, $dynamic_only, $agentid );
		}
	}
}


function HandleFailure ( $config, $report, $error, &$nfailed )
{
	$ret = true;
	if ( !IsModelGenMode() && !$config->ModelSubtestFailed () )
	{
		$nfailed++;
		$ret = false;

		fwrite ( $report, "SUBTEST FAILED, UNEXPECTED ERROR:\n" );
	}

	fwrite ( $report, "$error\n" );
	$config->SubtestFailed ();

	return $ret;
}


function EraseDirContents ( $path )
{
	$fp = opendir ( $path );

	if ( $fp )
	{
		while ( ( $file = readdir ( $fp ) ) !== false )
		{ 
		if ( $file != "." && $file != ".." && !is_dir ( $file ) )
				unlink ( "$path/$file" ); 
		} 

		closedir ( $fp );
	}
}


function CheckConfig ( $config, $path )
{
	global $g_id64, $windows;
	
	if ( $config->Requires("id64") && !$g_id64 )
	{
		printf ( "SKIPPING %s, %s - enable id64 to run this test\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("id32") && $g_id64 )
	{
		printf ( "SKIPPING %s, %s - disable id64 to run this test\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("non-windows") && $windows )
	{
		printf ( "SKIPPING %s, %s - use non-Windows system to run this test\n", $path, $config->Name () );
		return false;
	}

	if ( $config->Requires("non-rt") && $config->IsRt() )
	{
		printf ( "SKIPPING %s, %s - explicitly non-RT test skipped in RT mode\n", $path, $config->Name () );
		return false;
	}

	if ( $config->NeedIndexerEx() && $config->IsRt() )
	{
		printf ( "SKIPPING %s, %s - non-RT test that uses indexer skipped in RT mode\n", $path, $config->Name () );
		return false;
	}

	return true;
}


function RunTest ( $test_dir )
{
	global $indexer_data_path, $agents, $sd_pid_file, $sd_managed_searchd, $g_id64, $windows;

	$model_file = $test_dir."/model.bin";
	$conf_dir 	= $test_dir."/Conf";

	$config = new SphinxConfig;
	if ( !$config->Load ( $test_dir."/test.xml" ) )
		return;

	$config->SetTestDir ( $test_dir );

	$prefix = sprintf ( "testing %s, %s...", $test_dir, $config->Name () );

	if ( !CheckConfig ( $config, $test_dir ) )
		return array ( "tests_total"=>0, "tests_failed"=>0, "tests_skipped"=>1 );

	if ( $config->IsNeedDB() )
	{
		$connection = CreateDB ( $config->DB_Drop(), $config->DB_Create(), $config->DB_Insert(), $config->DB_CustomInsert() );
		if ( $connection === false )
		{
			printf ( "$prefix FAILED, error creating test DB: %s\n", mysql_error() );
			return;
		}
		$config->SetConnection($connection);
	}

	if ( !$config->LoadModel ( $model_file ) )
	{
		printf ( "$prefix FAILED, error loading model\n" );
		return;
	}

	if ( !file_exists ( $conf_dir ) )
		mkdir ( $conf_dir );
	
	$report_path = "$test_dir/report";
	$report_file = "$report_path.txt";
	$report = fopen ( $report_file, "w" );

	$nfailed = 0;
	$error = "";
	$log = ""; // subtest failures log
	$nsubtests = $config->SubtestCount();

	// config to pid hash, instances to stop
	// static is only to workaround PHP braindamage, otherwise $stop gets reset (at least on 5.2.2 under win32)
	static $stop = array();
	do
	{
		// stop them all
		foreach ( $stop as $conf=>$pid )
			StopSearchd ( $conf, $pid );
		$stop = array();

		// do the dew
		$subtest = $config->SubtestNo()+1;
		print ( "$prefix $subtest/$nsubtests\r" );
		$config->WriteReportHeader ( $report );

		$config->SetAgent ( $agents [0] );
		$msg = '';
		if (!$config->WriteConfig ( $conf_dir."/"."config_".$config->SubtestNo ().".conf", "all", $msg, false))
		{
			print ("Interrupted, $msg\n");
			continue;
		}
		$config->WriteConfig ( "config.conf", "all", $msg, $config->NumAgents () < 2 );

		EraseDirContents ( $indexer_data_path );

		if ( $config->IsSkipIndexer()===false )
		{
			// standard run
			if ( !$config->IsRt() )
			{
				$indexer_ret = RunIndexer ( $error, "--all" );
				if ( $indexer_ret==1 )
				{
					if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
						$log .= "\tsubtest $subtest: error running indexer; see $report_file\n";
					continue;

				} else if ( $indexer_ret==2 )
				{
					fwrite ( $report, "$error\n" );
				}
			}

			// additional optional runs (eg for merge tests)
			$indexer_ret = $config->RunIndexerEx ( $error );
			if ( $indexer_ret==1 )
			{
				if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
					$log .= "\tsubtest $subtest: error running indexer; see $report_file\n";
				continue;

			} else if ( $indexer_ret==2 )
			{
				fwrite ( $report, "$error\n" );
			}
		}

		$searchd_error = FALSE;

		if ( $config->NumAgents () == 1 )
		{
			if ( $sd_managed_searchd )
				$searchd_ret = 0;
			else
				$searchd_ret = StartSearchd ( "config.conf", "error.txt", $sd_pid_file, $error );
			
			$stop["config.conf"] = $sd_pid_file;

			if ( $searchd_ret == 1 )
			{
				if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
					$log .= "\tsubtest $subtest: error starting searchd; see $report_file\n";
				
				$searchd_error = TRUE;
			}
			else if ( $searchd_ret==2 )
			{
				fwrite ( $report, "$error\n" );
			}
		}
		else
			for ( $i = $config->NumAgents () - 1; $i >= 0 && !$searchd_error; $i-- )
			{
				static $agent_id = 0;
				$agent_id++;

				$config_file = "config_".$agent_id.".conf";
				$pid_file = "searchd_".$agent_id.".pid";
				$stop[$config_file] = $pid_file;
				$msg = '';
				$config->SetAgent ( $agents [$i] );
				$config->SetPIDFile ( $pid_file );
				if ( !$config->WriteConfig ( $config_file, $i, $msg ) )
					continue;

			if ( $sd_managed_searchd )
				$searchd_ret = 0;
			else
				$searchd_ret = StartSearchd ( $config_file, "error_".$agent_id.".txt", $pid_file, $error );

				if ( $searchd_ret == 1 )
				{
					if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
						$log .= "\tsubtest $subtest: error starting searchd; see $report_file\n";
				
					$searchd_error = TRUE;
		
				}
				else if ( $searchd_ret==2 )
				{
					fwrite ( $report, "$error\n" );
				}

			}

		if ( $searchd_error )
			continue;

		// in case of RT index - run "insert into" instead of indexer
		if ( $config->IsRt () )
			$config->InsertIntoIndexer ( $error );

		if ( $config->IsQueryTest () )
		{
			$error = "";
			if ( ! $config->RunQuery ( "*", $error ) )
			{
				if ( !HandleFailure ( $config, $report, "$error\n", $nfailed ) )
					$log .= "\tsubtest $subtest: query error: $error\n";
				continue;
			}
		}
		else
		{
			if ( ! $config->RunCustomTest ( $error ) )
			{
				if ( !HandleFailure ( $config, $report, "$error\n", $nfailed ) )
					$log .= "\tsubtest $subtest: query error: $error\n";
				continue;
			}
		}

		if ( $config->IsSphinxqlTest () )
		{
			$error = "";
			if ( ! $config->RunQuerySphinxQL ( $error ) )
			{
				if ( !HandleFailure ( $config, $report, "$error\n", $nfailed ) )
					$log .= "\tsubtest $subtest: query error: $error\n";
				continue;
			}
		}

		$mismatch = ( !IsModelGenMode() && !$config->CompareToModel() );
		if ( $mismatch )
		{
			$log .= "\tsubtest $subtest: query results mismatch; see $report_file\n";
			$nfailed++;
		}

		$config->WriteResults ( $report );

		if ( $mismatch )
		{
			fwrite ( $report, "SUBTEST FAILED, RESULTS ARE DIFFERENT FROM THE REFERENCE:\n\n" );
			$config->WriteReferenceResultsDiff ( $report );
		}

		$config->SubtestFinished ();
	}
	while ( $config->CreateNextConfig () );

	foreach ( $stop as $conf=>$pid )
		StopSearchd ( $conf, $pid );

	$total = $config->SubtestNo()+1;
	if ( IsModelGenMode () )
	{
		if ( $config->CheckVariants ( $report_path."_variant.txt" ) )
		{
			$config->WriteModel ( $model_file );
			printf ( "$prefix done; %d/%d subtests run\n", $config->SubtestNo(), $nsubtests );
		}
		else
		{
			printf ( "$prefix done; %d/%d subtests: VARIANT CHECK FAILED\n", $config->SubtestNo(), $nsubtests );
			$nfailed = $total;
		}
	}
	else if ( $nfailed==0 )
		printf ( "$prefix done; %d/%d subtests OK\n", $config->SubtestNo(), $nsubtests );
	else
		printf ( "$prefix done; %d/%d subtests FAILED:\n%s", $nfailed, $nsubtests, $log );

	fclose ( $report );

	return array ( "tests_total"=>$total, "tests_failed"=>$nfailed, "tests_skipped"=>0 );
}

//
// $Id$
//
