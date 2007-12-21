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


class TestResult
{
	public	$_tests_total;
	public	$_tests_failed;

	function TestResult ()
	{
		$_tests_total = 0;
		$_tests_failed = 0;
	}
}


function CreateDB ( $db_drop, $db_create, $db_insert )
{
	global $db_host, $db_user, $db_pwd, $db_name, $db_port;
	return
		@mysql_connect ( $db_host.":".$db_port, $db_user, $db_pwd ) &&
		@mysql_query ( "DROP DATABASE $db_name" ) &&
		@mysql_query ( "CREATE DATABASE $db_name" ) &&
		@mysql_select_db ( $db_name ) &&
		@mysql_query ( $db_drop ) &&
		@mysql_query ( $db_create ) &&
		@mysql_query ( $db_insert );
}


function RunIndexer ( &$error )
{
	global $windows, $indexer_path;

	$retval = 0;

	system ( "$indexer_path --quiet --config config.conf --all > error.txt", $retval );
	$error = file_get_contents ( "error.txt" );

	if ( $retval == 0  )
	{
		if ( empty ( $error ) )
			return 0;
		else
			return 2;
	}

	return $retval;
}


function StartSearchd ( $config_file, $error_file, $pidfile, &$error )
{
	global $windows, $searchd_path;

	$retval = 0;

	if ( $windows )
	{
		$process = popen ("start /min $searchd_path --config $config_file --pidfile > $error_file", "r" );
		pclose ( $process );

		for ( $i = 0; $i < 10; $i++ )
		{
			if ( !file_exists ( $pidfile ) )
				usleep ( 50000 );
			else
				break;
		}
	}
	else
		system ( "$searchd_path --config $config_file > $error_file", $retval );

	if ( $retval != 0 )
		$error = file_get_contents ( $error_file );

	return $retval;
}


function StopSearchd ( $config, $pidfile )
{
	global $windows, $searchd_path;

	$retval = 0;

	if ( file_exists ( $pidfile ) )
	{
		if ( $windows )
   			system ( "$searchd_path --config $config --stop > NUL", $retval );
		else 
			system ( "$searchd_path --config $config --stop > /dev/null", $retval );

		while ( file_exists ( $pidfile ) )
			usleep ( 50000 );
	}

	return $retval;
}



function IsModelGenMode ()
{
	global $g_model;
	return $g_model;
}


function GetTreeRoot ( $node, $name )
{
	if ( strtolower ( $node->nodeName ) == $name )
		return $node;

	for ( $i = 0; !is_null ( $node->childNodes ) && $i < $node->childNodes->length; $i++ )
	{
		$result = GetTreeRoot ( $node->childNodes->item ( $i ), $name );
		if ( ! is_null ( $result ) )
			return $result;
	}

	return NULL;
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
	private $_xml;
	private	$_sd_address;
	private	$_sd_port;
	private	$_sd_pid_file;
	private $_num_agents;
	private $_subtest;
	private $_subtestcount;
	private $_results;
	private $_results_model;
	
	
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

		$num_variants = 0;
	   	for ( $j = 0; $j < $this->_dynamic_entries [$i]->childNodes->length; $j++ )
			if ( strtolower ( $this->_dynamic_entries [$i]->childNodes->item ( $j )->nodeName ) == "variant" )
				$num_variants++;
	
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
		if ( empty ( $node ) )
			return;

		for ( $i = 0; !is_null ( $node->childNodes ) && $i < $node->childNodes->length; $i++ )
		{
			$child = $node->childNodes->item ( $i );
		
			if ( $child->nodeType == XML_ELEMENT_NODE )
				array_push ( $array, $child->nodeValue );
		}
	}


	function GatherNodes ( $node )
	{
		if (    $node->nodeType != XML_TEXT_NODE && $node->nodeType != XML_DOCUMENT_NODE
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
		$this->_xml = new DOMDocument ("1.0", "utf-8");
		$this->_xml->load ( $config_file );
		$this->GatherNodes ( GetTreeRoot ( $this->_xml, "config" ) );
		$this->GatherEntities ( GetTreeRoot ( $this->_xml, "query" ), $this->_queries );
		$this->GatherEntities ( GetTreeRoot ( $this->_xml, "query_attributes" ), $this->_query_attributes );

		$name = GetTreeRoot ( $this->_xml, "name" );
		if ( $name )
			$this->_name = $name->nodeValue;

		$db_create = GetTreeRoot ( $this->_xml, "db_create" );
		if ( $db_create )
			$this->_db_create = $db_create->nodeValue;

		$db_drop = GetTreeRoot ( $this->_xml, "db_drop" );
		if ( $db_drop )
			$this->_db_drop = $db_drop->nodeValue;

		$db_insert = GetTreeRoot ( $this->_xml, "db_insert" );
		if ( $db_insert )
			$this->_db_insert = $db_insert->nodeValue;

		$query_settings = GetTreeRoot ( $this->_xml, "query_settings" );
		if ( $query_settings )
			$this->_query_settings = $query_settings->nodeValue;

		$num_agents = GetTreeRoot ( $this->_xml, "num_agents" );
		if ( $num_agents )
			$this->_num_agents = $num_agents->nodeValue;

		
		// precalc subtests count
		$this->_subtestcount = 1;
	   	foreach ( $this->_dynamic_entries as $entry )
	   	{
	   		$nodes = $entry->childNodes;
	   		$variants = 0;

	   		for ( $j=0; $j<$nodes->length; $j++ )
				if ( strtolower ( $nodes->item($j)->nodeName )=="variant" )
					$variants++;
	
			$this->_subtestcount *= max ( $variants, 1 );
		}
	}


	function RunQuery ( $index, &$error )
	{
		global $sd_address, $sd_port;

		$query_results = array ();

		foreach ( $this->_queries as $query )
		{
			$bOk = FALSE;
			for ( $i = 0; $i < 3 && !$bOk; $i++ )
			{
				$cl = new SphinxClient ();
				
				$cl->SetServer ( $sd_address, $sd_port );

				$results = 0;
	
				if ( empty ( $this->_query_settings ) )
	            	$results = $cl->Query ( $query, $index );
				else
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
			}
			if ( is_array($query2[$i]) )
			{
				unset ( $query2[$i]["time"] );
				unset ( $query2[$i]["warning"] );
				unset ( $query2[$i]["status"] );
			}
		}
	
		return $query1==$query2;
	}


	function WriteReportHeader ( $handle )
	{
		fprintf ( $handle, "==== Run %d ====\n", $this->SubtestNo () + 1 );
		fwrite ( $handle, "Settings:\n" );
		$this->WriteDiff ( $handle );
		fwrite ( $handle, "\n" );

		if ( !empty ( $this->_query_settings ) )
			fprintf ( $handle, "Query settings:\n%s\n", $this->_query_settings );
	}


	function FormatResultSet ( $nquery, $result )
	{
		$str = "--- Query $nquery ---\n";

		$txt_q = $result["query"];
		$txt_found = $result ["total_found"];
		$txt_total = $result ["total"];
		$txt_time = $result ["time"];

		$str .= "Query '$txt_q': retrieved $txt_found of $txt_total matches in $txt_time sec.\n";

		if ( $result["warning"] )
			$str .= "Warning: " . $result["warning"] . "\n";

		if ( array_key_exists ( "words", $result ) && is_array( $result ["words"] ) )
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

		if ( array_key_exists ( "matches", $result ) && is_array ( $result ["matches"] ) )
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
						if ( array_key_exists ( $attribute, $docinfo ["attrs"] ) )
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
	function WriteQuery ( $handle, $nquery, $result )
	{
		$res_fmt = $this->FormatResultSet ( $nquery, $result );
		fwrite ( $handle, $res_fmt );
	}

	/// write all the result sets
    function WriteResults ( $handle )
    {
        $nquery = 1;
        foreach ( $this->_results as $result )
			$this->WriteQuery ( $handle, $nquery++, $result );
	}


	/// write difference from the reference result sets
	function WriteReferenceResultsDiff ( $handle )
	{
		$nquery = 0;
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
			system ( "diff -u3 reference current > diffed.txt" );
			$diffed = file_get_contents ( "diffed.txt" );
			unlink ( "current" );
			unlink ( "reference" );
			unlink ( "diffed.txt" );

			fwrite ( $handle, "=== diff start ===\n" );
			fwrite ( $handle, $diffed );
			fwrite ( $handle, "=== diff end ===\n" );

			$nquery++;
		}
	}


	function Write ( $filename )
	{
		$handle = fopen ( $filename, 'w' );
		if ( !$handle )
			return FALSE;

		$this->Dump ( GetTreeRoot ( $this->_xml, "config" ), $handle, false );
	
		fclose ( $handle );
	
		return TRUE;
	}


	function WriteDiff ( $handle )
	{
		$this->Dump ( GetTreeRoot ( $this->_xml, "config" ), $handle, true );
	}


	function WriteModel ( $filename )
	{
		if ( IsModelGenMode () )
			file_put_contents ( $filename, serialize ( $this->_results_model ) );
	}


	function WriteSearchdSettings ( $handle )
	{
		global $sd_log, $sd_query_log, $sd_read_timeout, $sd_max_children, $sd_pid_file, $sd_max_matches;

		fwrite ( $handle, "\taddress			= ".$this->_sd_address."\n" );
		fwrite ( $handle, "\tport			= ".$this->_sd_port."\n" );
		fwrite ( $handle, "\tlog				= $sd_log\n" );
		fwrite ( $handle, "\tquery_log		= $sd_query_log\n" );
		fwrite ( $handle, "\tread_timeout	= $sd_read_timeout\n" );
		fwrite ( $handle, "\tmax_children	= $sd_max_children\n" );
		fwrite ( $handle, "\tpid_file		= ".$this->_sd_pid_file."\n" );
		fwrite ( $handle, "\tmax_matches		= $sd_max_matches\n" );
	}

	function WriteSqlSettings ( $handle )
	{
		global $db_host, $db_user, $db_pwd, $db_name, $db_port;

		fwrite ( $handle, "\tsql_host		= $db_host\n" );
		fwrite ( $handle, "\tsql_user		= $db_user\n" );
		fwrite ( $handle, "\tsql_pass		= $db_pwd\n" );
		fwrite ( $handle, "\tsql_db			= $db_name\n" );
		fwrite ( $handle, "\tsql_port		= $db_port\n" );
	}


	function Dump ( $node, $file_handle, $dynamic_only )
	{
		global $index_data_path, $agent_address, $agent_port;

		switch ( strtolower ( $node->nodeName ) )
		{
			case "static":
				if ( $dynamic_only )
					break;

				fwrite ( $file_handle, "$node->nodeValue" );
				break;

			case "variant":
				fwrite ( $file_handle, "$node->nodeValue\n" );
				break;

			case "searchd_settings":
				if ( $dynamic_only )
					break;

				$this->WriteSearchdSettings ( $file_handle );
				break;

			case "sql_settings":
				if ( $dynamic_only )
					break;

				$this->WriteSqlSettings ( $file_handle );
				break;
			
			case "agent_address":
				if ( $dynamic_only )
					break;

				fwrite ( $file_handle, $agent_address.":".$agent_port );
				break;
			
			case "data_path":
				if ( $dynamic_only )
					break;

				fwrite ( $file_handle, $index_data_path );
				break;

			case "dynamic":
				$key = $node->id;
				if ( is_null ( $key ) )
					break;
			
				$nvariant = 0;

				for ( $i = 0; !is_null ( $node->childNodes ) && $i < $node->childNodes->length; $i++ )
					if ( strtolower ( $node->childNodes->item ( $i )->nodeName ) == "variant" )
					{
						if ( $nvariant == $this->_counters [$key] )
						{
							$this->Dump ( $node->childNodes->item ( $i ), $file_handle, $dynamic_only );
							break;
						}

						$nvariant++;
					}
				break;

			default:
				for ( $i = 0; !is_null ( $node->childNodes ) && $i < $node->childNodes->length; $i++ )
					$this->Dump ( $node->childNodes->item ( $i ), $file_handle, $dynamic_only );
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
	$handle = opendir ( $path );

	if ( $handle )
	{
    	while ( ( $file = readdir ( $handle ) ) !== false )
		{ 
        	if ( $file != "." && $file != ".." && !is_dir ( $file ) )
				unlink ( "$path/$file" ); 
        } 

	    closedir ( $handle );
    }
}


function RunTest ( $test_dir )
{
	global $indexer_data_path, $agents, $sd_pid_file;

	$test_dir = $test_dir."/";

	$model_file = $test_dir."model.bin";
	$conf_dir 	= $test_dir."Conf";

	$config = new SphinxConfig;
	$config->Load ( $test_dir."test.xml" );

	$prefix = sprintf ( "testing %s...", $config->Name () );

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

	$report_file = $test_dir."report.txt";
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
		$config->Write ( $conf_dir."/"."config_".$config->SubtestNo ().".conf" );
		$config->Write ( "config.conf" );

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
			if ( StartSearchd ( "config.conf", "error.txt", $sd_pid_file, $error ) != 0 )
			{
				if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
					$log .= "\tsubtest $subtest: error starting searchd; see $report_file\n";
				
				$searchd_error = TRUE;
			}
		}
		else
			for ( $i = $config->NumAgents () - 1; $i >= 0  && !$searchd_error; $i-- )
			{
				$config_file = "config_$i.conf";
				$pid_file = "searchd$i.pid";

				$config->SetAgent ( $agents [$i] );
				$config->SetPIDFile ( $pid_file );
				$config->Write ( $config_file );

				if ( StartSearchd ( $config_file, "error_$i.txt", $pid_file, $error ) != 0 )
				{
					if ( !HandleFailure ( $config, $report, $error, $nfailed ) )
						$log .= "\tsubtest $subtest: error starting searchd; see $report_file\n";
				
					$searchd_error = TRUE;
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

	$test_result = new TestResult;
	$test_result->_tests_total = $config->SubtestNo () + 1;
	$test_result->_tests_failed = $nfailed;

	return $test_result;
}

//
// $Id$
//

?>
