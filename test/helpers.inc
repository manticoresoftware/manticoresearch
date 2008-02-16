<?php

//
// $Id$
//

require_once ( "../api/sphinxapi.php" );

$windows = isset($_SERVER["WINDIR"]) || isset($_SERVER["windir"]) || isset($_SERVER["HOMEDRIVE"]);


function MyMicrotime ()
{
	$q = @gettimeofday();
	return (float)($q["usec"] / 1000000) + $q["sec"];
}


function CreateDB ( $db_drop, $db_create, $db_insert )
{
	global $db_host, $db_user, $db_pwd, $db_name, $db_port;
	return
		@mysql_connect ( $db_host.":".$db_port, $db_user, $db_pwd ) &&
		@mysql_query ( "CREATE DATABASE IF NOT EXISTS $db_name" ) &&
		@mysql_select_db ( $db_name ) &&
		@mysql_query ( $db_drop ) &&
		@mysql_query ( $db_create ) &&
		@mysql_query ( $db_insert );
}


function RunIndexer ( &$error )
{
	global $indexer_path;

	$retval = 0;
	exec ( "$indexer_path --quiet --config config.conf --all", $error, $retval );

	$error = join ( "\n", $error );
	return ( $retval==0 && !empty($error) ) ? 2 : $retval;
}


function StartSearchd ( $config_file, $error_file, $pidfile, &$error )
{
	global $windows, $searchd_path;

	$retval = 0;

	if ( $windows )
	{
		$process = popen ("start /min $searchd_path --config $config_file --pidfile > $error_file", "r" );
		pclose ( $process );

		for ( $i=0; $i<10 && !file_exists($pidfile); $i++ )
			usleep ( 50000 );
	}
	else
		system ( "$searchd_path --config $config_file > $error_file", $retval );

	if ( $retval == 0 )
	{
		$error = "";
		$log_raw = explode ( '\n', file_get_contents ( $error_file ) );
		foreach ( $log_raw as $msg )
		{
			$warn = stristr ( $msg, 'WARNING' );
			if ( $warn )
				$error .= $warn;

			$warn = stristr ( $warn, 'ERROR' );
			if ( $warn )
				$error .= $warn;
		}

		if ( empty ( $error ) )
			return 0;
		else
			return 2;
	}

	return $retval;
}


function StopSearchd ( $config, $pidfile )
{
	global $searchd_path;

	if ( file_exists($pidfile) )
	{
		exec ( "$searchd_path --config $config --stop" );
		while ( file_exists ( $pidfile ) )
			usleep ( 50000 );
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
	private	$_sd_address;
	private	$_sd_port;
	private	$_sd_pid_file;
	private $_num_agents;
	private $_subtest;
	private $_subtestcount;
	private $_results;
	private $_results_model;
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
		$this->_num_agents		= 1;
		$this->_subtest 		= 0;
		$this->_subtestcount	= 0;
		$this->_sd_address		= $sd_address;
		$this->_sd_port			= $sd_port;
		$this->_sd_pid_file		= $sd_pid_file;
	}

	
	function SubtestNo ()			{ return $this->_subtest; }
	function SubtestCount ()		{ return $this->_subtestcount; }
	function Name ()				{ return $this->_name; }
	function DB_Drop ()				{ return $this->_db_drop; }
	function DB_Create ()			{ return $this->_db_create; }
	function DB_Insert ()			{ return $this->_db_insert; }
	function NumAgents ()			{ return $this->_num_agents; }


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


	function Load ( $config_file )
	{
		// load the file
		$xml = new DOMDocument ( "1.0", "utf-8" );
		if ( !$xml->load ( $config_file ) )
			return false;

		// check for proper root node
		if ( !$xml->hasChildNodes() )
			return false;

		$xml = $xml->childNodes->item(0);
		if ( strtolower($xml->nodeName)!="test" )
			return false;

		// extract queries
		$qs = GetFirstChild ( $xml, "queries" );
		if ( $qs )
		{
			// new and cool
			foreach ( ChildrenArray ( $qs, "query" ) as $q )
			{
				$res = array ( "query"=>$q->nodeValue );

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

				// parse filter
				$res["filter"] = $q->getAttribute("filter");
				$res["filtervalue"] = $q->getAttribute("filtervalue" );

				// parse sort mode and get clause
				$sortmode = 0;
				$sortmode_s = $q->getAttribute("sortmode");
				switch ( $sortmode_s )
				{
					case "":			$sortmode_s = "(default)"; break;
					case "expr":		$sortmode = SPH_SORT_EXPR; break;
					default:
						printf ( "$config_file: unknown sorting mode '%s'\n", $sortmode_s );
						return false;
				}
				$res["sortmode"] = $sortmode;
				$res["sortmode_s" ] = $sortmode_s;
				$res["sortby"] = $q->getAttribute("sortby");

				// add query
				$this->_queries[] = $res;
			}

		} else
		{
			// legacy
			$qs = array ();
			$this->GatherEntities ( GetFirstChild ( $xml, "query" ), $qs );
			foreach ( $qs as $q )
				$this->_queries[] = array ( "query"=>$q, "mode"=>0, "mode_s"=>"(default)" );
		}

		// extract my settings
		$this->_config = GetFirstChild ( $xml, "config" );
		$this->GatherNodes ( $this->_config );
		$this->GatherEntities ( GetFirstChild ( $xml, "query_attributes" ), $this->_query_attributes );

		$this->_name			= GetFirstChildValue ( $xml, "name" );
		$this->_db_create		= GetFirstChildValue ( $xml, "db_create" );
		$this->_db_drop			= GetFirstChildValue ( $xml, "db_drop" );
		$this->_db_insert		= GetFirstChildValue ( $xml, "db_insert" );
		$this->_query_settings	= GetFirstChildValue ( $xml, "query_settings" );
		$this->_num_agents		= GetFirstChildValue ( $xml, "num_agents", 1 );

		// precalc subtests count
		$this->_subtestcount = 1;
		foreach ( $this->_dynamic_entries as $entry )
		{
			$variants = count ( ChildrenArray ( $entry, "variant" ) );
			$this->_subtestcount *= max ( $variants, 1 );
		}

		return true;
	}


	function RunQuery ( $index, &$error )
	{
		global $sd_address, $sd_port;

		$query_results = array ();

		foreach ( $this->_queries as $qinfo )
		{
			$query = $qinfo["query"];
			$bOk = FALSE;
			for ( $i = 0; $i < 3 && !$bOk; $i++ )
			{
				$cl = new SphinxClient ();
				$cl->SetServer ( $sd_address, $sd_port );

				$results = 0;
				if ( empty($this->_query_settings) )
				{
					if ( $qinfo["mode"] )		$cl->SetMatchMode ( $qinfo["mode"] );
					if ( $qinfo["sortmode"] )	$cl->SetSortMode ( $qinfo["sortmode"], $qinfo["sortby"] );
					if ( $qinfo["filter"] )		$cl->SetFilter ( $qinfo["filter"], array ( $qinfo["filtervalue"] ) );
	            	$results = $cl->Query ( $query, $index );

            	} else
				{
					$run_func = create_function( '$client, $query, $index, &$results', $this->_query_settings );
					$run_func ( $cl, $query, $index, $results ); 
				}

				if ( $results )
				{
					$bOk = TRUE;
					$results ["query"] = $query;
        			array_push ( $query_results, $results );
            	}
				else
					usleep ( 50000 );
			}

			if ( !$bOk )
			{
				$error = $cl->GetLastError ();
				return FALSE;
			}
		}

		$this->_results = $query_results;

		if ( IsModelGenMode () )
			array_push ( $this->_results_model, $query_results );

		return TRUE;
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

			$this->_results_model = unserialize ( $contents );
		}

		return TRUE;
	}


	function CompareToModel ()
	{
		return $this->CompareResults ( $this->_results, $this->_results_model [$this->SubtestNo ()] );
	}


	function CompareResults ( $query1, $query2 )
	{
		if ( count($query1)!=count($query2) )
			return false;

		for ( $i=0; $i<count($query1); $i++ )
		{
			if ( is_array($query1[$i]) )
			{
				unset ( $query1[$i]["time"] );
				unset ( $query1[$i]["warning"] );
				unset ( $query1[$i]["status"] );
				unset ( $query1[$i]["fields"] );
			}
			if ( is_array($query2[$i]) )
			{
				unset ( $query2[$i]["time"] );
				unset ( $query2[$i]["warning"] );
				unset ( $query2[$i]["status"] );
				unset ( $query2[$i]["fields"] );
			}
		}

		return $query1==$query2;
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
		$qinfo = $this->_queries[$nquery-1];
		$str = "--- Query $nquery (mode=$qinfo[mode_s]) ---\n";
		$str .= "Query '$result[query]': retrieved $result[total_found] of $result[total] matches in $result[time] sec.\n";
		if ( $result["warning"] )
			$str .= "Warning: $result[warning]\n";

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
				$weight = $docinfo ["weight"];

				$str .= "$n. doc_id=$doc, weight=$weight";

				if ( empty ( $this->_query_attributes ) )
				{
					$query_res = mysql_query ( "select * from test_table where document_id = $doc" );

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

							$str .= "\n";
				    	}
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
        $nquery = 1;
        foreach ( $this->_results as $result )
			$this->WriteQuery ( $fp, $nquery++, $result );
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
			if ( $ref==$this->_results[$nquery] )
			{
				$nquery++;
				continue;
			}

			$result_f_cur = $this->FormatResultSet ( $nquery, $this->_results[$nquery] );
			$result_f_ref = $this->FormatResultSet ( $nquery, $ref );

			file_put_contents ( "current", $result_f_cur );
			file_put_contents ( "reference", $result_f_ref );

			if ( $windows )
				system ( "diff -u3 reference current > diffed.txt" );
			else
				system ( "diff reference current > diffed.txt" );

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


	function WriteConfig ( $filename )
	{
		$fp = fopen ( $filename, 'w' );
		if ( !$fp )
			return FALSE;

		$this->Dump ( $this->_config, $fp, false );
	
		fclose ( $fp );
	
		return TRUE;
	}


	function WriteDiff ( $fp )
	{
		$this->Dump ( $this->_config, $fp, true );
	}


	function WriteModel ( $filename )
	{
		if ( IsModelGenMode () )
			file_put_contents ( $filename, serialize ( $this->_results_model ) );
	}


	function WriteSearchdSettings ( $fp )
	{
		global $sd_log, $sd_query_log, $sd_read_timeout, $sd_max_children, $sd_pid_file, $sd_max_matches;

		fwrite ( $fp, "\taddress			= ".$this->_sd_address."\n" );
		fwrite ( $fp, "\tport			= ".$this->_sd_port."\n" );
		fwrite ( $fp, "\tlog				= $sd_log\n" );
		fwrite ( $fp, "\tquery_log		= $sd_query_log\n" );
		fwrite ( $fp, "\tread_timeout	= $sd_read_timeout\n" );
		fwrite ( $fp, "\tmax_children	= $sd_max_children\n" );
		fwrite ( $fp, "\tpid_file		= ".$this->_sd_pid_file."\n" );
		fwrite ( $fp, "\tmax_matches		= $sd_max_matches\n" );
	}

	function WriteSqlSettings ( $fp )
	{
		global $db_host, $db_user, $db_pwd, $db_name, $db_port;

		fwrite ( $fp, "\tsql_host		= $db_host\n" );
		fwrite ( $fp, "\tsql_user		= $db_user\n" );
		fwrite ( $fp, "\tsql_pass		= $db_pwd\n" );
		fwrite ( $fp, "\tsql_db			= $db_name\n" );
		fwrite ( $fp, "\tsql_port		= $db_port\n" );
	}


	function Dump ( $node, $fp, $dynamic_only )
	{
		global $index_data_path, $agent_address, $agent_port;

		if ( !$dynamic_only )
			switch ( strtolower ( $node->nodeName ) )
		{
			case "#text":				fwrite ( $fp, $node->nodeValue ); return;
			case "static":				fwrite ( $fp, $node->nodeValue ); return;
			case "searchd_settings":	$this->WriteSearchdSettings ( $fp ); return;
			case "sql_settings":		$this->WriteSqlSettings ( $fp ); return;
			case "agent_address":		fwrite ( $fp, $agent_address.":".$agent_port ); return;
			case "data_path":			fwrite ( $fp, $index_data_path ); return;
		}

		switch ( strtolower ( $node->nodeName ) )
		{
			case "variant":
				fwrite ( $fp, "$node->nodeValue\n" );
				break;

			case "dynamic":
				if ( !is_null($node->id) )
				{
					$variants = ChildrenArray ( $node,"variant" );
					$this->Dump ( $variants[$this->_counters[$node->id]], $fp, $dynamic_only );
				}
				break;

			default:
				foreach ( ChildrenArray($node) as $child )
					$this->Dump ( $child, $fp, $dynamic_only );
			break;
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


function RunTest ( $test_dir )
{
	global $indexer_data_path, $agents, $sd_pid_file;

	$model_file = $test_dir."/model.bin";
	$conf_dir 	= $test_dir."/Conf";

	$config = new SphinxConfig;
	if ( !$config->Load ( $test_dir."/test.xml" ) )
		return;

	$prefix = sprintf ( "testing %s, %s...", $test_dir, $config->Name () );

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
	do
	{
		$subtest = $config->SubtestNo()+1;
		print ( "$prefix $subtest/$nsubtests\r" );
		$config->WriteReportHeader ( $report );

		$config->SetAgent ( $agents [0] );
		$config->WriteConfig ( $conf_dir."/"."config_".$config->SubtestNo ().".conf" );
		$config->WriteConfig ( "config.conf" );

		EraseDirContents ( $indexer_data_path );

		$indexer_ret = RunIndexer ( $error );

		if ( $indexer_ret==1 )
		{
			if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
				$log .= "\tsubtest $subtest: error running indexer; see $report_file\n";
			continue;

		} else if ( $indexer_ret==2 )
		{
			fwrite ( $report, "$error\n" );
		}

		$searchd_error = FALSE;

		if ( $config->NumAgents () == 1 )
		{
			$searchd_ret = StartSearchd ( "config.conf", "error.txt", $sd_pid_file, $error );

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
				$config_file = "config_$i.conf";
				$pid_file = "searchd$i.pid";

				$config->SetAgent ( $agents [$i] );
				$config->SetPIDFile ( $pid_file );
				$config->WriteConfig ( $config_file );

				
				$searchd_ret = StartSearchd ( $config_file, "error_$i.txt", $pid_file, $error );

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

		$error = "";
		if ( ! $config->RunQuery ( "*", $error ) )
		{
			if ( !HandleFailure ( $config, $report, "$error\n", $nfailed ) )
				$log .= "\tsubtest $subtest: query error: $error\n";
			continue;
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

		if ( $config->NumAgents () == 1 )
			StopSearchd ( "config.conf", $sd_pid_file );
		else
			for ( $i = 0; $i < $config->NumAgents (); $i++ )
				StopSearchd ( "config_$i.conf", "searchd$i.pid" );
	}
	while ( $config->CreateNextConfig () );

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
