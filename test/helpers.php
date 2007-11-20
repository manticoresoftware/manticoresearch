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


function CreateDB ( $db_drop_path, $db_create_path, $db_insert_path )
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

	$query = file_get_contents ( $db_drop_path );
	if ( $query == FALSE )
		return FALSE;

	$result = mysql_query ( $query, $link );
	if ( $result == FALSE )
		return FALSE;

	$query = file_get_contents ( $db_create_path );
	if ( $query == FALSE )
		return FALSE;

	$result = mysql_query ( $query, $link );
	if ( $result == FALSE )
		return FALSE;

	$query = file_get_contents ( $db_insert_path );
	if ( $query == FALSE )
		return FALSE;

	$result = mysql_query ( $query, $link );
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
			}

			fwrite ( $handle, "\n" );
			$n++;
		}

		fwrite ( $handle, "\n" );
	
		++$nquery;
	}
}


function RunIndexer ()
{
	global $windows;

	$retval = 0;

	if ( $windows )
		system ( "indexer --config config.conf --all > NUL", $retval );
	else
		system ( "./indexer --config config.conf --all > /dev/null", $retval );

	return $retval;
}


function StartSearchd ()
{
	global $windows;

	$retval = 0;

	if ( $windows )
		system ( "net start searchd > NUL", $retval );
	else
		system ( "./searchd --config config.conf > /dev/null", $retval );

	return $retval;
}


function StopSearchd ()
{
	global $sd_pid_file, $windows;

	$retval = 0;

	if ( $windows )
		system ( "net stop searchd > NUL", $retval );
	else
	{
		if ( file_exists ( $sd_pid_file ) )
		{
			system ( "./searchd --config config.conf --stop > /dev/null", $retval );
			while ( file_exists ( $sd_pid_file ) )
				usleep ( 50000 );
		}
	}

	return $retval;
}



function IsModelGenMode ()
{
	global $argc, $argv;

	return $argc == 2 && ( $argv [1] == "-g" || $argv [1] == "--gen" );
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


	function CreateNextConfig ( $filename )
	{
		if ( $this->GenNextCfg ( 0 ) )
		{
			$this->Write ( $filename );
			return TRUE;
		}
		else
			return FALSE;
	}


	function SubtestFinished ()
	{
		$this->_subtest++;
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


	function WriteResults ( $handle )
	{
		fprintf ( $handle, "==== Run:  %d ====\n", $this->SubtestNo () + 1 );
		fwrite ( $handle, "Settings:\n" );
		$this->WriteDiff ( $handle );
		fwrite ( $handle, "\n" );
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
		switch ( strtolower ( $node->nodeName ) )
		{
			case "static":
				if ( $dynamic_only )
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

?>
