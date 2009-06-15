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

function CreateDB ( $db_drop, $db_create, $db_insert )
{
	global $db_host, $db_user, $db_pwd, $db_name, $db_port;
	if ( !@mysql_connect ( $db_host.":".$db_port, $db_user, $db_pwd ) ||
		!@mysql_query ( "CREATE DATABASE IF NOT EXISTS $db_name" ) ||
		!@mysql_select_db ( $db_name ) )
		return false;

	foreach ( $db_drop as $q )
		if ( !@mysql_query ( $q ) )
			return false;

	foreach ( $db_create as $q )
		if ( !@mysql_query ( $q ) )
			return false;

	foreach ( $db_insert as $q )
		if ( !@mysql_query ( $q ) )
			return false;

	return true;
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
		$process = popen ("start /min cmd /c \"$path --config $config_file --pidfile > $error_file\"", "r" );
		pclose ( $process );
	}
	else
		system ( "$path --config $config_file > $error_file", $retval );

	for ( $i=0; $i<$action_retries && !file_exists($pidfile); $i++ )
		usleep ( $action_wait_timeout );

	if ( !file_exists($pidfile) )
	{
		$error = "PID file ($pidfile) was not created";
		return 1;
	}

	// we've got a pid file; but lets check the log file for startup errors
	for ( $i=0; $i<$action_retries && !file_exists($error_file); $i++ )
		usleep ( $action_wait_timeout );

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

	if ( $retval==0 && !empty($error) )
		$retval = 2; // no errors, but there were warnings

	return $retval;
}


function StopSearchd ( $config, $pidfile )
{
	global $g_locals, $action_retries, $action_wait_timeout;

	if ( file_exists($pidfile) )
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
	private $_counters;
	private $_dynamic_entries;
	private $_queries;
	private $_query_settings;
	private $_query_attributes;
	private $_indexer_runs;
	private $_custom_test;
	private	$_sd_address;
	private	$_sd_port;
	private	$_sd_pid_file;
	private $_num_agents;
	private $_subtest;
	private $_subtestcount;
	private $_results;
	private $_results_model;
	private $_prereqs;
	private $_config;				///< config DOM node


	function SphinxConfig ()
	{
		global $sd_address, $sd_port, $sd_pid_file;

		$this->_counters 		= array ();
		$this->_dynamic_entries = array ();
		$this->_queries 		= array ();
		$this->_results			= array ();
		$this->_results_model	= array ();
		$this->_query_attributes = array ();
		$this->_indexer_runs	= array ();
		$this->_db_create		= array ();
		$this->_db_drop			= array ();
		$this->_db_insert		= array ();
		$this->_num_agents		= 1;
		$this->_subtest 		= 0;
		$this->_subtestcount	= 0;
		$this->_sd_address		= $sd_address;
		$this->_sd_port			= $sd_port;
		$this->_sd_pid_file		= $sd_pid_file;
		$this->_custom_test		= "";
		$this->_compat098		= false;
		$this->_skip_indexer	= false;
	}

	function EnableCompat098 ()		{ $this->_compat098 = true; }	
	function SubtestNo ()			{ return $this->_subtest; }
	function SubtestCount ()		{ return $this->_subtestcount; }
	function Name ()				{ return $this->_name; }
	function DB_Drop ()				{ return $this->_db_drop; }
	function DB_Create ()			{ return $this->_db_create; }
	function DB_Insert ()			{ return $this->_db_insert; }
	function NumAgents ()			{ return $this->_num_agents; }
	function Requires ( $name )		{ return isset($this->_prereqs[$name]); }
	function IsQueryTest ()			{ return strlen ( $this->_custom_test ) == 0;	}
	function Results ()				{ return $this->_results; }
	function GetQuery ( $i )		{ return $this->_queries[$i]; }
	function IsSkipIndexer ()		{ return $this->_skip_indexer; }

	function GetLocal ( $key )
	{
		global $g_locals;
		if ( !array_key_exists ( $key, $g_locals ) )
		{
			printf ( "Unbound local variable: %s\n", $key );
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
			$node->id =  count ( $this->_dynamic_entries );
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
		preg_match_all ( '/([^\s]+):(\d+)/', $weights, $matches, PREG_SET_ORDER  );
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

				// add query
				if ( $q->getAttribute("source") )
				{
					$source = $q->getAttribute("source");
					if ( substr ( $source, 0, 6 ) == "local:" )
						$source = $this->GetLocal ( substr ( $source, 6 ) );
					if ( !is_readable($source) )
					{
						printf ( "%s: source not found.\n", $source );
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
				for ( $i = 0; $i < $action_retries && !$bOk; $i++ )
				{
					$cl = new SphinxClient ();
					$cl->SetServer ( $sd_address, $sd_port );

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
						
						$results = $cl->Query ( $query, $my_index );
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
						$bOk = TRUE;
						if ( $compact )
						{
							$results = array ( $n,
											   $results['total'],
											   $results['total_found'],
											   $results['time'] );
						}
						else
							$results ["query"] = $query;

						array_push ( $query_results, $results );
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

		if ( IsModelGenMode () )
			array_push ( $this->_results_model, $query_results );

		if ( $benchmark )
			printf ( " - done in %s\n", sphFormatTime ( MyMicrotime() - $start ) );

		return TRUE;
	}

	
	function RunCustomTest ( & $error )
	{
		global $sd_address, $sd_port, $action_retries, $action_wait_timeout;

		$bOk = false;
		$results = false;

		for ( $i = 0; $i < $action_retries && !$bOk; $i++ )
		{
    		$cl = new SphinxClient ();
			$cl->SetServer ( $sd_address, $sd_port );

			$results = false;
			$run_func = create_function( '$client, &$results', $this->_custom_test );
			$run_func ( $cl, $results ); 

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


	function WriteReportHeader ( $fp )
	{
		fprintf ( $fp, "==== Run %d ====\n", $this->SubtestNo () + 1 );
		fwrite ( $fp, "Settings:\n" );
		$this->WriteDiff ( $fp );
		fwrite ( $fp, "\n" );

		if ( !empty ( $this->_query_settings ) )
			fprintf ( $fp, "Query settings:\n%s\n", $this->_query_settings );
	}


	function FormatResultSet ( $nquery, $result )
	{
		if ( !$this->IsQueryTest () )
			return var_export ( $result, true )."\n";

		$qinfo = $this->_queries[$nquery-1];
		if ( array_key_exists ( "index", $qinfo ) && $qinfo ["index"] != '*' )
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

		$str .= "\n";
		if ( isset($result["matches"]) && is_array($result["matches"]) )
		{
			$n = 1;
			$str .= "Matches:\n";
			foreach ( $result ["matches"] as $doc => $docinfo )
			{
				if ( $array_result )
					$doc_id = $docinfo ["id"];
				else
					$doc_id = $doc;

				$weight = $docinfo ["weight"];

				$str .= "$n. doc_id=$doc_id, weight=$weight";

				if ( empty ( $this->_query_attributes ) )
				{
					$query_res = mysql_query ( "select * from test_table where document_id = $doc_id" );

					if ( $query_res === FALSE )
						$str .= "\n";
					else
					{
						while ( $row = mysql_fetch_array ( $query_res, MYSQL_ASSOC ) )
						{
							foreach ( $row as $col_name => $col_content )
							{
								if ( array_search ( $col_name, $result ["fields"] ) !== FALSE )
								   	$str .= " $col_name=\"$col_content\"";
							}
				    	}

						foreach ( $docinfo ["attrs"] as $attr => $value )
						{
							if ( is_array ( $value ) )
							{
								$str .= " $attr=\"";
								foreach ( $value as $v )
									$str .= $v." ";

								$str .= "\"";
							}
							else
						   		$str .= " $attr=\"$value\"";
						}

						$str .= "\n";
					}
				}
				else
				{
					foreach ( $this->_query_attributes as $attribute )
						if ( isset($docinfo ["attrs"][$attribute]) )
						{
							$attrtmp = $docinfo ["attrs"][$attribute];
							if ( is_array ( $attrtmp ) )
							{
								$str .= " $attribute=";
								foreach ( $attrtmp as $valuetmp )
									$str .= " $valuetmp";
							}
							else
								$str .= " $attribute=$attrtmp";
						}

					$str .= "\n";
				}

				$n++;
			}

			$str .= "\n";
		}

		$str .= "\n";
	
		return $str;
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
			if ( $this->CompareResultSets ( $ref, $this->_results[$nquery] ) )
			{
				$nquery++;
				continue;
			}

			$result_f_cur = $this->FormatResultSet ( $nquery + 1, $this->_results[$nquery] );
			$result_f_ref = $this->FormatResultSet ( $nquery + 1, $ref );
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
	}


	function WriteConfig ( $filename, $agentid )
	{
		$fp = fopen ( $filename, 'w' );
		if ( !$fp )
			return FALSE;

		$this->Dump ( $this->_config, $fp, false, $agentid );
	
		fclose ( $fp );
	
		return TRUE;
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
			fwrite ( $fp, "\tlisten		= {$this->_sd_address}:{$this->_sd_port}\n" );
		fwrite ( $fp, "\tlog			= $sd_log\n" );
		fwrite ( $fp, "\tquery_log		= $sd_query_log\n" );
		fwrite ( $fp, "\tread_timeout	= $sd_read_timeout\n" );
		fwrite ( $fp, "\tmax_children	= $sd_max_children\n" );
		fwrite ( $fp, "\tpid_file		= ".$this->_sd_pid_file."\n" );
		fwrite ( $fp, "\tmax_matches	= $sd_max_matches\n" );
	}

	function WriteSqlSettings ( $fp, $attributes )
	{
		global $db_host, $db_user, $db_pwd, $db_name, $db_port;

		fwrite ( $fp, "\tsql_host		= $db_host\n" );
		fwrite ( $fp, "\tsql_user		= $db_user\n" );
		fwrite ( $fp, "\tsql_pass		= $db_pwd\n" );
		fwrite ( $fp, "\tsql_port		= $db_port\n" );

		$node = $attributes->getNamedItem('sql_db');
		fprintf ( $fp, "\tsql_db		= %s\n", $node ? $node->nodeValue : $db_name );
	}

	function Dump ( $node, $fp, $dynamic_only, $agentid )
	{
		global $index_data_path, $agent_address, $agent_port;

		if ( !$dynamic_only )
			switch ( strtolower ( $node->nodeName ) )
		{
			case "#text":				fwrite ( $fp, $node->nodeValue ); return;
			case "static":				fwrite ( $fp, $node->nodeValue ); return;
			case "searchd_settings":	$this->WriteSearchdSettings ( $fp ); return;
			case "sql_settings":		$this->WriteSqlSettings ( $fp, $node->attributes ); return;
			case "agent_address":		fwrite ( $fp, $agent_address.":".$agent_port ); return;
			case "data_path":			fwrite ( $fp, $index_data_path ); return;
			case "local":				fwrite ( $fp, $this->GetLocal ( $node->nodeValue ) ); return;
		}

		$nodename = strtolower ( $node->nodeName );
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
		printf ( "SKIPPING %s, %s - disable id64 to run this test\n", $test_dir, $config->Name () );
		return $res_skipped;
	}

	if ( $config->Requires("non-windows") && $windows )
	{
		printf ( "SKIPPING %s, %s - use non-Windows system to run this test\n", $path, $config->Name () );
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

	$prefix = sprintf ( "testing %s, %s...", $test_dir, $config->Name () );

	if ( !CheckConfig ( $config, $test_dir ) )
		return array ( "tests_total"=>0, "tests_failed"=>0 );

	if ( !CreateDB ( $config->DB_Drop(), $config->DB_Create(), $config->DB_Insert() ) )
	{
		printf ( "$prefix FAILED, error creating test DB: %s\n", mysql_error() );
		return;
	}

	if ( !$config->LoadModel ( $model_file ) )
	{
		printf ( "$prefix FAILED, error loading model\n" );
		return;
	}

	if ( !file_exists ( $conf_dir ) )
		mkdir ( $conf_dir );

	$report_file = $test_dir."/report.txt";
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
		$config->WriteConfig ( $conf_dir."/"."config_".$config->SubtestNo ().".conf", "all" );
		$config->WriteConfig ( "config.conf", "all" );

		EraseDirContents ( $indexer_data_path );

		if ( $config->IsSkipIndexer()===false )
		{
			// standard run
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
			for ( $i = $config->NumAgents () - 1; $i >= 0  && !$searchd_error; $i-- )
			{
				static $agent_id = 0;
				$agent_id++;

				$config_file = "config_".$agent_id.".conf";
				$pid_file = "searchd_".$agent_id.".pid";
				$stop[$config_file] = $pid_file;

				$config->SetAgent ( $agents [$i] );
				$config->SetPIDFile ( $pid_file );
				$config->WriteConfig ( $config_file, $i );

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

	fclose ( $report );
	mysql_close ();

	if ( IsModelGenMode () )
		printf ( "$prefix done; %d/%d subtests run\n", $config->SubtestNo(), $nsubtests );
	else if ( $nfailed==0 )
		printf ( "$prefix done; %d/%d subtests OK\n", $config->SubtestNo(), $nsubtests );
	else
		printf ( "$prefix done; %d/%d subtests FAILED:\n%s", $nfailed, $nsubtests, $log );

	$config->WriteModel ( $model_file );

	return array ( "tests_total"=>$config->SubtestNo()+1, "tests_failed"=>$nfailed );
}

//
// $Id$
//

?>
