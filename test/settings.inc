<?php

$sd_address 		= "localhost";
$sd_port 			= 6712;
$sd_log				= "searchd.log";
$sd_query_log		= "query.log";
$sd_read_timeout	= 5;
$sd_max_children	= 30;
$sd_pid_file		= "searchd.pid";
$sd_max_matches		=  100000;

$db_host			= "localhost";
$db_user			= "root";
$db_pwd				= "";
$db_name			= "test";
$db_port			= 3306;

$agent_address		= "localhost";
$agent_port			= 6713;

$agents 			= array ( array ( "address" => $sd_address, "port" => $sd_port ),
							  array ( "address" => $agent_address, "port" => $agent_port ) );

$index_data_path	= "data";

$g_model	= false;
$g_id64		= false;
$g_strict	= false;

$windows = isset($_SERVER["WINDIR"]) || isset($_SERVER["windir"]) || isset($_SERVER["HOMEDRIVE"]);
$action_retries			= 200;
$action_wait_timeout	= 50000;

$g_locals = null;
$g_site_defaults = array
(
	'queries'		=> 'queries.txt',
	'qlimit'		=> null,
	'api'			=> '../api/sphinxapi.php',
	'mode'			=> 'aggregate',
	'skip-searchd'	=> false,
	'force-reindex'	=> false,
);

function GetBanner ()
{
	global $g_locals;
	
	exec ( $g_locals['indexer'], $output, $result );
	if ( count($output) == 0 )
	{
		print "ERROR: failed to run the indexer\n";
		exit ( 1 );
	}
	else
		return $output;
}

function GuessIdSize ()
{
	global $g_id64;
	$banner = GetBanner();
	$g_id64 = strstr ( $banner[0], 'id64' ) !== false;
}

function GetVersion ()
{
	$banner = GetBanner();
	return $banner[0];
}

function PublishLocals ( $locals, $benchmark )
{
	global $g_locals, $g_site_defaults, $windows;
	$sources = array();

	if ( @$locals['root'] && !@$locals['bin'] && !@$locals['api'] )
	{
		$locals['bin'] = $locals['root'] . '/src/';
		$locals['api'] = $locals['root'] . '/api/sphinxapi.php';
	}
	
	$text = @file_get_contents ( getenv('HOME') . '/.sphinx' );
	if ( $text )
	{
		eval('$init = array ' . $text . ';');
		$sources[] = $init;
	}
	$sources[] = $g_site_defaults;

	foreach ( $sources as $source )
	{
		foreach ( $source as $key => $value )
		{
			if ( !array_key_exists ( $key, $locals ) )
				$locals[$key] = $value;
		}
	}

	if ( !@$locals['bin'] )
	{
		if ( $windows )
			$locals['bin'] = $benchmark ? '..\\bin\\release\\' : '..\\bin\\debug\\';
		else
			$locals['bin'] = '../src/';
	}

	$ext = $windows ? ".exe" : "";
	foreach ( array ( 'searchd', 'indexer' ) as $key )
	{
		if ( !array_key_exists ( $key, $locals ) )
			$locals[$key] = $locals['bin'] . $key . $ext;
	}

	$g_locals = $locals;
}

?>