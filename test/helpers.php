<?php

require_once ( "../api/sphinxapi.php" );

$windows = isset($_SERVER["WINDIR"]) || isset($_SERVER["windir"]) || isset($_SERVER["HOMEDRIVE"]);

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

	$link = mysql_connect ( $db_host.":".$db_port, $db_user, $db_pwd );
	if ( $link == FALSE )
		return FALSE;

	mysql_query ( "DROP DATABASE $db_name", $link );

	$result = mysql_query ( "CREATE DATABASE $db_name", $link );
	if ( $result == FALSE )
		return FALSE;

	$result = mysql_select_db ( $db_name, $link );
	if ( $result == FALSE )
		return FALSE;

	$result = mysql_query ( $db_drop, $link );
	if ( $result == FALSE )
		return FALSE;

	$result = mysql_query ( $db_create, $link );
	if ( $result == FALSE )
		return FALSE;

	$result = mysql_query ( $db_insert, $link );
	if ( $result == FALSE )
		return FALSE;

	return $link;
}


function WriteQueryResults ( $results, $handle )
{
	$nquery = 1;
	foreach ( $results as $result )
	{
		fwrite ( $handle, "--- Query $nquery ---\n" );
		$txt_q = $result["query"];
		$txt_found = $result ["total_found"];
		$txt_total = $result ["total"];
		$txt_time = $result ["time"];

		fwrite ( $handle, "Query '$txt_q': retrieved $txt_found of $txt_total matches in $txt_time sec.\n" );

		if ( array_key_exists ( "words", $result ) && is_array( $result ["words"] ) )
		{
			fwrite ( $handle, "Word stats:\n" );

			foreach ( $result ["words"] as $word => $word_result )
			{
				$hits = $word_result ["hits"];
				$docs = $word_result ["docs"];
				fwrite ( $handle, "\t'$word' found $hits times in $docs documents\n" );
			}
		}

		fwrite ( $handle, "\n" );

		if ( array_key_exists ( "matches", $result ) && is_array ( $result ["matches"] ) )
		{
			$n = 1;
			fwrite ( $handle, "Matches:\n" );
			foreach ( $result ["matches"] as $doc => $docinfo )
			{
				$weight = $docinfo ["weight"];

				fwrite ( $handle, "$n. doc_id=$doc, weight=$weight" );

				$query_res = mysql_query ( "select * from test_table where document_id = $doc" );

				if ( $query_res != FALSE )
				{
					while ( $row = mysql_fetch_array ( $query_res, MYSQL_ASSOC ) )
					{
						foreach ( $row as $col_name => $col_content )
						{
							if ( array_search ( $col_name, $result ["fields"] ) !== FALSE )
							   	fwrite ( $handle, " $col_name=\"$col_content\"" );
						}

						fwrite ( $handle, "\n" );
				    }
				}
				$n++;
			}

			fwrite ( $handle, "\n" );
		}

		fwrite ( $handle, "\n" );
	
		++$nquery;
	}
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


function StartSearchd ( &$error )
{
	global $windows, $searchd_path;

	$retval = 0;

	if ( $windows )
	{
		$process = popen ("start $searchd_path --config config.conf --pidfile > error.txt", "r" );
		pclose ( $process );
	}
	else
		system ( "$searchd_path --config config.conf > error.txt", $retval );

	if ( $retval != 0 )
		$error = file_get_contents ( "error.txt" );

	return $retval;
}


function StopSearchd ()
{
	global $sd_pid_file, $windows, $searchd_path;

	$retval = 0;

	if ( $windows )
		system ( "$searchd_path --config config.conf --stop > NUL", $retval );
	else
	{
		if ( file_exists ( $sd_pid_file ) )
		{
			system ( "$searchd_path --config config.conf --stop > /dev/null", $retval );
			while ( file_exists ( $sd_pid_file ) )
				usleep ( 50000 );
		}
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
	private $_xml;
	private $_subtest;
	private $_results;
	private $_results_model;
	
	
	function SphinxConfig ()
	{
		$this->_counters 		= array ();
		$this->_dynamic_entries = array ();
		$this->_queries 		= array ();
		$this->_results			= array ();
		$this->_results_model	= array ();
		$this->_subtest 		= 0;
	}

	
	function SubtestNo ()
	{
		return $this->_subtest;
	}


	function Name ()
	{
		return $this->_name;
	}


	function DB_Drop ()
	{
		return $this->_db_drop;
	}


	function DB_Create ()
	{
		return $this->_db_create;
	}


	function DB_Insert ()
	{
		return $this->_db_insert;
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

		if ( IsModelGenMode () )
			array_push ( $this->_results_model, "failed" );
	}

	
	function ModelSubtestFailed ()
	{
		return $this->_results_model [$this->SubtestNo ()] == "failed";
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


	function GatherQueries ( $node )
	{
		for ( $i = 0; !is_null ( $node->childNodes ) && $i < $node->childNodes->length; $i++ )
		{
			$child = $node->childNodes->item ( $i );
		
			if ( $child->nodeType == XML_ELEMENT_NODE )
				array_push ( $this->_queries, $child->nodeValue );
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
		$this->GatherQueries ( GetTreeRoot ( $this->_xml, "query" ) );

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
            	$res = $cl->Query ( $query, $index );
				if ( $res )
				{
					$bOk = TRUE;
					$res ["query"] = $query;
        			array_push ( $query_results, $res );
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
		foreach ( $query1 as $key => $value )
		{
			if ( is_array ( $value ) && ! is_null ( $value ["time"] ) )
				unset ( $query1 [$key]["time"] );
		}

		foreach ( $query2 as $key => $value )
		{
			if ( is_array ( $value ) && ! is_null ( $value ["time"] ) )
				unset ( $query2 [$key]["time"] );
		}

		return $query1 == $query2;
	}


	function WriteReportHeader ( $handle )
	{
		fprintf ( $handle, "==== Run:  %d ====\n", $this->SubtestNo () + 1 );
		fwrite ( $handle, "Settings:\n" );
		$this->WriteDiff ( $handle );
		fwrite ( $handle, "\n" );
	}


	function WriteResults ( $handle )
	{
		WriteQueryResults ( $this->_results, $handle );
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
		global $sd_address, $sd_port, $sd_log, $sd_query_log, $sd_read_timeout, $sd_max_children, $sd_pid_file, $sd_max_matches;

		fwrite ( $handle, "\taddress			= $sd_address\n" );
		fwrite ( $handle, "\tport			= $sd_port\n" );
		fwrite ( $handle, "\tlog				= $sd_log\n" );
		fwrite ( $handle, "\tquery_log		= $sd_query_log\n" );
		fwrite ( $handle, "\tread_timeout	= $sd_read_timeout\n" );
		fwrite ( $handle, "\tmax_children	= $sd_max_children\n" );
		fwrite ( $handle, "\tpid_file		= $sd_pid_file\n" );
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
		global $index_data_path;

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
	if ( IsModelGenMode () )
		print ( "\n" );
	else
	{
		if ( $config->ModelSubtestFailed () )
			print ( "OK\n" );
		else
		{
			print ( "FAILED\n" );
			$nfailed++;
		}
	}

	fwrite ( $report, "$error\n" );
	$config->SubtestFailed ();
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
	global $indexer_data_path;

	$test_dir = $test_dir."/";

	$model_file = $test_dir."model.bin";
	$conf_dir 	= $test_dir."Conf";

	$config = new SphinxConfig;
	$config->Load ( $test_dir."test.xml" );

	printf ( "Running test '%s'...\n", $config->Name () );

	$db_link = CreateDB ( $config->DB_Drop (), $config->DB_Create (), $config->DB_Insert () );
	if ( ! $db_link )
		die ( "Error creating test database\n" );

	if ( ! $config->LoadModel ( $model_file ) )
		die ( "Error loading model\n" );

	if ( ! file_exists ( $conf_dir ) )
		mkdir ( $conf_dir );

	$report = fopen ( $test_dir."report.txt", "w" );

	$nfailed = 0;
	$error = "";

	do
	{
		printf ( "\r\tRunning subtest %d...", $config->SubtestNo () + 1 );
		$config->WriteReportHeader ( $report );

		StopSearchd ();

		$config->Write ( $conf_dir."/"."config_".$config->SubtestNo ().".conf" );
		$config->Write ( "config.conf" );

		EraseDirContents ( $indexer_data_path );

		$indexer_ret = RunIndexer ( $error );

		switch ( $indexer_ret )
		{
			case 2:
				fwrite ( $report, "$error\n" );
				break;

			case 1:
				print ( "Error running indexer... " );
				HandleFailure ( $config, $report, $error, $nfailed );
				break;
		}

		if ( $indexer_ret == 1 )
			continue;

		if ( StartSearchd ( $error ) != 0 )
		{
			print ( "Error starting searchd... " );
			HandleFailure ( $config, $report, $error, $nfailed );
			continue;
		}

		$error = "";
		if ( ! $config->RunQuery ( "*", $error ) )
		{
			print ( "Error running query: $error... " );
			HandleFailure ( $config, $report, "$error\n", $nfailed );
			continue;
		}

		if ( ! IsModelGenMode () && ! $config->CompareToModel () )
		{
			print ( "FAILED\n" );
			$nfailed++;
		}

		$config->WriteResults ( $report );
		$config->SubtestFinished ();
	}
	while ( $config->CreateNextConfig () );

	StopSearchd ();

	fclose ( $report );

	if ( IsModelGenMode () )
		printf ( "\n\t%d subtests total\n", $config->SubtestNo () );
	else
		printf ( "\n\t%d subtests total, %d failed\n", $config->SubtestNo (), $nfailed );

	mysql_close ( $db_link );

	$config->WriteModel ( $model_file );

	$test_result = new TestResult;
	$test_result->_tests_total = $config->SubtestNo () + 1;
	$test_result->_tests_failed = $nfailed;

	return $test_result;
}

?>