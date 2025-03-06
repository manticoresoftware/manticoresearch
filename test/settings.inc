<?php

/*
CTEST_RESOURCE_GROUP_0=datadir,mysql
CTEST_RESOURCE_GROUP_COUNT=1
CTEST_RESOURCE_GROUP_0_MYSQL=id:test1,slots:1
CTEST_RESOURCE_GROUP_0_DATADIR=id:data,slots:1
 */

$dbname = "test";
$db = getenv('CTEST_RESOURCE_GROUP_0_MYSQL');
if ( $db )
	$dbname = explode(":", explode(",", $db)[0])[1];

$data_slot =  ['base-port' => 6706, 'name-base' => "searchd", 'config-base' => "config", 'query-log' => "query.log", 'data-path' => "data", 'error' => "error" ];
$datadir = getenv('CTEST_RESOURCE_GROUP_0_DATADIR');
if ($datadir) {
	$dataname = explode(":", explode(",", $datadir)[0])[1];
	if ( $dataname!="data" ) {
		if (strlen($dataname) < 4)
			$prefix = $dataname;
		else if (substr($dataname, 0, 4) == "data")
			$prefix = substr($dataname, 4);
		$iprefix = (int)$prefix;
		$x = "_{$prefix}x";
		$data_slot =  ['base-port' => 6706+$iprefix*100, 'name-base' => "searchd$x", 'config-base' => "config$x", 'query-log' => "query$x.log", 'data-path' => "data$x", 'error' => "error$x" ];
	}
}


$sd_sphinxql_port	= $data_slot['base-port']; //6706
$agent_port_sql		= $sd_sphinxql_port+1; // 6707,6708
$sd_sphinxql_port_vip = $agent_port_sql+2; //6709;
$agent_port_sql_vip	= $sd_sphinxql_port_vip+1;//6710; // 6711
$sd_port 			= $agent_port_sql_vip+2;//6712;
$agent_port			= $sd_port+1;//6713; // 6714
$sd_replication_port = $agent_port_sql_vip+10;//6720; // 6740, 6760
$sd_http_port		= $sd_replication_port+60;//6780;
$agent_port_http	= $sd_http_port+1;//6781; // +6782
$searchd_name_base	= $data_slot['name-base']; //"searchd.log";
$config_base		= $data_slot['config-base']; //"config.log";
$query_log			= $data_slot['query-log']; //"query.log";
$index_data_prefix	= $data_slot['data-path']; //"data";
$error_base			= $data_slot['error']; // "error";
$searchd_log		= "$searchd_name_base.log"; //"searchd.log";
$searchd_pid		= "$searchd_name_base.pid"; //"searchd.pid";

$sd_split_logs		= false; // if we want one log per agent, its easier to investigate problems!
$sd_network_timeout	= 5;
$sd_max_children	= 100;
$sd_address 		= "127.0.0.1";

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
	'db-name'			=> $dbname,
	'db-port'			=> 3306
);


if ( array_key_exists ( "SPHINXDAEMON", $_ENV ) && $_ENV["SPHINXDAEMON"] )
	$sd_address = $_ENV["SPHINXDAEMON"];
else if ( array_key_exists ( "SPHINXDAEMON", $_SERVER ) && $_SERVER["SPHINXDAEMON"] )
	$sd_address = $_SERVER["SPHINXDAEMON"];
$agent_address		= $sd_address;

$agents 			= array (
	array ( "address" => $sd_address, "port" => $sd_port, "sqlport" => $sd_sphinxql_port, "sqlport_vip" => $sd_sphinxql_port_vip, "replication_port" => $sd_replication_port, "http_port" => $sd_http_port ),
	array ( "address" => $agent_address, "port" => $agent_port, "sqlport" => $agent_port_sql, "sqlport_vip" => $agent_port_sql_vip, "replication_port" => $sd_replication_port+20, "http_port" => $agent_port_http ),
	array ( "address" => $agent_address, "port" => $agent_port+1, "sqlport" => $agent_port_sql+1, "sqlport_vip" => $agent_port_sql_vip+1, "replication_port" => $sd_replication_port+40, "http_port" => $agent_port_http+1 ) );

$g_guesscached = false;
$g_model	= false;
$g_re2		= false;
$g_icu		= false;
$g_jieba	= false;
$g_odbc     = false;
$g_columnar_loaded = false;
$g_strict	= false;
$g_skipdemo = false;
$g_usemarks = true; // that we mark the output of every test in the searchd.log and query.log
$g_strictverbose = false;
$g_ssl = false;
$g_secondary_loaded = false;
$g_knn_loaded = false;
$g_tzdata_loaded = false;
$g_zlib		= false;

$windows = isset($_SERVER["WINDIR"]) || isset($_SERVER["windir"]) || isset($_SERVER["HOMEDRIVE"]);
$action_retries			= 100;
$valgrind_action_retries	= 200;
$action_wait_timeout	= 50000;

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

function LoadCachedGuesses ()
{
	global $g_locals;
	$guessfile = $g_locals['scriptdir'].'guess.txt';

	if ( file_exists ( $guessfile ))
	{
		$text = @file_get_contents ( $guessfile );
		eval ( $text );
	}
}

function CacheGuesses ()
{
	global $g_locals;
	global $g_re2, $g_icu, $g_jieba, $g_odbc, $g_repli, $g_ssl, $g_columnar_loaded, $g_secondary_loaded, $g_knn_loaded, $g_tzdata_loaded, $g_zlib;

	$guessfile = $g_locals['scriptdir'].'guess.txt';

	$text = "global \$g_guesscached; \$g_guesscached = true;\n";
	if ( $g_re2 ) $text .= "global \$g_re2; \$g_re2 = true;\n";
	if ( $g_icu ) $text .= "global \$g_icu; \$g_icu = true;\n";
	if ( $g_jieba ) $text .= "global \$g_jieba; \$g_jieba = true;\n";
	if ( $g_odbc ) $text .= "global \$g_odbc; \$g_odbc = true;\n";
	if ( $g_repli ) $text .= "global \$g_repli; \$g_repli = true;\n";
	if ( $g_ssl ) $text .= "global \$g_ssl; \$g_ssl = true;\n";
	if ( $g_columnar_loaded ) $text .= "global \$g_columnar_loaded; \$g_columnar_loaded = true;\n";
    if ( $g_secondary_loaded ) $text .= "global \$g_secondary_loaded; \$g_secondary_loaded = true;\n";
    if ( $g_knn_loaded ) $text .= "global \$g_knn_loaded; \$g_knn_loaded = true;\n";
    if ( $g_tzdata_loaded ) $text .= "global \g_tzdata_loaded; \$g_tzdata_loaded = true;\n";
    if ( $g_zlib ) $text .= "global \$g_zlib; \$g_zlib = true;\n";

	file_put_contents ( $guessfile, $text );
}

function testdir ($path)
{
	global $g_locals;
	if ( !$path || $path=='' )
		return $g_locals['testdir'];

	# linux absolute starts with /, windows have drive letter with :.
	if ( substr ($path, 0, 1) === '/' || substr ($path, 1, 1) === ':' )
		return $path;

	return $g_locals['testdir'].$path;
}

function config_conf()
{
	global $config_base;
	return "$config_base.conf";
}

function testdir_config()
{
	return testdir(config_conf());
}

function error_txt()
{
	global $error_base;
	return "$error_base.txt";
}

function searchd_pid()
{
	global $searchd_pid;
	return $searchd_pid;
}

function scriptdir ($path)
{
	global $g_locals;
	if ( !$path || $path=='' )
		return $g_locals['scriptdir'];

	# linux absolute starts with /, windows have drive letter with :.
	if ( substr ($path, 0, 1) === '/' || substr ($path, 1, 1) === ':' )
		return $path;

	return $g_locals['scriptdir'].$path;
}

function resdir ($testdir)
{
	$testid = substr ( $testdir, 5 );
	global $ctest;
	$prefix = "test_";
	if ( $ctest )
	{
		if ( IsColumnar() )
			$prefix = "col_";
		elseif ( IsRt() )
			$prefix = "rt_";
	}
	return scriptdir ( $prefix.$testid );
}

function GuessFeature ($config, $testmsg, $matchmsg = null)
{
    global $g_locals;

    $arg_quiet = '--quiet';
    if (null!==$matchmsg)
    {
        $arg_quiet = '';
    }

    exec ( $g_locals['indexer'].' '.$arg_quiet.' --config '.$config.' --all', $output, $result );
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
    if (null!==$matchmsg)
    {
        $flag = false;
        foreach ( $output as $str )
        {
            if ( strstr ( $str, $matchmsg ) )
            {
                $flag = true;
                break;
            }
        }
    }
    return $flag;
}

function GuessRE2 ()
{
	global $g_re2;
    $g_re2 = GuessFeature ('test_re.conf','no regexp support compiled');
}

function GuessICU ()
{
	global $g_icu;
    $g_icu = GuessFeature ( 'test_icu.conf','no ICU support compiled');
}

function GuessJieba ()
{
	global $g_jieba;
    $g_jieba = GuessFeature ( 'test_jieba.conf','no Jieba support compiled');
}

function GuessODBC ()
{
    global $g_odbc;
    $g_odbc = GuessFeature ( 'test_odbc.conf','failed to configure some of the sources');
}

function GuessColumnar()
{
    global $g_columnar_loaded;
    $g_columnar_loaded = GuessFeature ( 'test_columnar.conf','columnar library not loaded');
}

function GuessSecondary()
{
    global $g_secondary_loaded;
    $g_secondary_loaded = GuessFeature ( 'test_columnar.conf','secondary library not loaded', ' (secondary ');
}

function GuessKNN()
{
    global $g_knn_loaded;
    $g_knn_loaded = GuessFeature ( 'test_columnar.conf','knn library not loaded', ' (knn ');
}

function GuessTzData()
{
	global $g_tz_data_loaded;
	$g_tz_data_loaded = strpos( file_get_contents( testdir("searchd.log"), 'Error initializing time zones' ) ) === false;
}

function GetVersion ()
{
	$banner = GetBanner();
	return $banner[0];
}

function GuessReplication()
{
	$repli_on = 'DWITH_GALERA=1';
	global $g_repli;

	$banner = GetBanner();
	foreach ( $banner as $line )
	{
		if ( strpos ( $line, $repli_on ) !== false )
		{
			$g_repli = true;
			break;
		}
	}
}

function GuessSSL ()
{
	global $g_ssl;
	if ( !defined('OPENSSL_VERSION_NUMBER') )
	{
		$g_ssl = false;
		return;
	}

	$banner = GetBanner();
	foreach ( $banner as $line )
	{
		if ( strpos ( $line, 'DWITH_SSL=1' ) !== false )
		{
			$g_ssl = true;
			break;
		}
	}
}

function GuessZlib()
{
    global $g_zlib;
    $zlib_on = 'DWITH_ZLIB=1';

	$banner = GetBanner();
	foreach ( $banner as $line )
	{
		if ( strpos ( $line, $zlib_on ) !== false )
		{
			$g_zlib = true;
			break;
		}
	}
}

function PublishLocals ( &$locals, $benchmark )
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

	if ( $windows ) {
		$ext = ".exe";
		$pref = $benchmark ? "RelWithDebInfo/" : "Debug/";
	} else
	{
		$ext = '';
		$pref = '';
	}

	foreach ( array ( 'searchd', 'indexer', 'spelldump' ) as $key )
	{
		$KEY = getenv(strtoupper($key));
		if ( !array_key_exists ( $key, $locals ) )
			$locals[$key] = $KEY ? $KEY : $locals['bin'] . $pref . $key . $ext;
	}

	$g_locals = $locals;


	if ( isset($g_locals["icu_data_dir"]))
		putenv("ICU_DATA_DIR=$g_locals[icu_data_dir]");

	if ( isset($g_locals["jieba_data_dir"]))
    	putenv("JIEBA_DATA_DIR=$g_locals[jieba_data_dir]");

	if ( isset($g_locals["full_share_dir"]))
		putenv("FULL_SHARE_DIR=$g_locals[full_share_dir]");
}

?>
