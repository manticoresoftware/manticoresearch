<?php

$sd_address 		= "127.0.0.1";
$sd_port 			= 6712;
$sd_sphinxql_port	= 6706;
$sd_http_port		= 6780;
$sd_log				= "searchd.log";
$sd_query_log		= "query.log";
$sd_read_timeout	= 5;
$sd_max_children	= 30;
$sd_pid_file		= "searchd.pid";

$agent_address		= "127.0.0.1";
$agent_port			= 6713;
$agent_port_sql		= 6707;

if ( array_key_exists ( "SPHINXDAEMON", $_ENV ) && $_ENV["SPHINXDAEMON"] ) 
	$sd_address = $_ENV["SPHINXDAEMON"];
else if ( array_key_exists ( "SPHINXDAEMON", $_SERVER ) && $_SERVER["SPHINXDAEMON"] )
	$sd_address = $_SERVER["SPHINXDAEMON"];

$agents 			= array (
	array ( "address" => $sd_address, "port" => $sd_port, "sqlport" => $sd_sphinxql_port ),
	array ( "address" => $agent_address, "port" => $agent_port, "sqlport" => $agent_port_sql ),
	array ( "address" => $agent_address, "port" => $agent_port+1, "sqlport" => $agent_port_sql+1 ) );

$index_data_path	= "data";

$g_model	= false;
$g_id64		= false;
$g_re2		= false;
$g_rlp		= false;
$g_odbc     = false;
$g_strict	= false;
$g_skipdemo = false;
$g_usemarks = true; // that we mark the output of every test in the searchd.log and query.log
$g_strictverbose = false;

$windows = isset($_SERVER["WINDIR"]) || isset($_SERVER["windir"]) || isset($_SERVER["HOMEDRIVE"]);
$action_retries			= 20;
$action_wait_timeout	= 50000;

$g_locals = null;
$g_site_defaults = array
(
	'queries'			=> 'queries.txt',
	'qlimit'			=> null,
	'api'				=> '../api/sphinxapi.php',
	'mode'				=> 'aggregate',
	'skip-searchd'		=> false,
	'force-reindex'		=> false,
	'malloc-scribble'	=> false,

	'db-host'			=> '127.0.0.1',
	'db-user'			=> 'root',
	'db-password'		=> '',
	'db-name'			=> 'test',
	'db-port'			=> 3306
);

// localsettings could include dev's own parameters
// which is not to be commited into the public repo (in opposite to this settings.inc)
if ( file_exists ('localsettings.inc') )
	require_once ( 'localsettings.inc' );

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

function GuessFeature ($config, $testmsg)
{
    global $g_locals;

    exec ( $g_locals['indexer'].' --quiet --config '.$config.' --all', $output, $result );
    if ( count($output) == 0 )
    {
        print "ERROR: failed to run the indexer\n";
        exit ( 1 );
    }

    $flag = true;
    foreach ( $output as $str )
    {
        if ( strstr ( $str, $testmsg ) )
        {
            $flag = false;
            break;
        }
    }
    return $flag;
}

function GuessRE2 ()
{
	global $g_re2;
    $g_re2 = GuessFeature ('test_re.conf','no regexp support compiled');
}

function GuessRLP ()
{
	global $g_rlp;
    $g_rpl = GuessFeature ( 'test_rlp.conf','no RLP support compiled');
}

function GuessODBC ()
{
    global $g_odbc;
    $g_odbc = GuessFeature ( 'test_odbc.conf','failed to configure some of the sources');
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
	if ( !$text )
	{
		$path = '';
		if ( $windows )
			$path = $benchmark ? '..\\bin\\release\\' : '..\\bin\\debug\\';
		else
			$path = '../src/';
	
		$text = @file_get_contents ( $path . '/.sphinx' );
	}
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
