<?php

$sd_managed_searchd	= false;
$sd_skip_indexer = false;
$g_ignore_weights = false;
$g_pick_query = -1;

require_once ( "settings.inc" );

if (!defined("JSON_UNESCAPED_SLASHES") || !defined("JSON_UNESCAPED_UNICODE"))
	die("ubertest needs JSON_UNESCAPED_xxx support; upgrade your PHP to 5.4+");

if (!function_exists("curl_init"))
	die("ERROR: missing required curl_init(); add php_curl.so (.dll on Windows) to your php.ini!");

//////////////////////
// parse command line
//////////////////////

$args = $_SERVER["argv"];
array_shift ( $args );

if ( !is_array($args) || empty($args) )
{
	print ( "Usage: php -f ubertest.php <MODE> [OPTIONS] [TESTDIRS ...]\n" );
	print ( "\nModes are:\n" );
	print ( "g, gen\t\t\tgenerate reference ('model') test results\n" );
	print ( "t, test\t\t\trun tests and compare results to reference\n" );
	print ( "qt\t\t\tsame as test, but skips user-configured slow tests\n" );
	print ( "show\t\tdisplay internal blocks after all settings applied\n" );
	print ( "\nOptions are:\n" );
	print ( "-u, --user <USER>\tuse 'USER' as MySQL user\n" );
	print ( "-p, --password <PASS>\tuse 'PASS' as MySQL password\n" );
	print ( "-i, --indexer <PATH>\tpath to indexer\n" );
	print ( "-s, --searchd <PATH>\tpath to searchd\n" );
	print ( "-b, --bindir <PATH>\tpath to all binaries\n" );
	print ( "-t, --testdir <PATH>\tpath which write to configs when testing\n" );
	print ( "-tt <PATH>\tpath where ubertest will work and create artefacts\n" );
	print ( "--ctest\t\tPrint test report to console (for automatic grabbing)\n" );
	print ( "--strict\t\tterminate on the first failure (for automatic runs)\n" );
	print ( "--strict-verbose\tterminate on the first failure and copy the last report to report.txt (for automatic runs)\n" );
	print ( "--managed\t\tdon't run searchd during test (for debugging)\n" );
	print ( "--skip-indexer\t\tskip DB creation and indexer stages and go directly to queries/custom tests\n");
	print ( "--rt\t\t\ttest RT backend (auto-convert all local indexes)\n" );
	print ( "--columnar\t\t\ttest attrs as columnar\n" );
	print ( "--no-drop-db\t\tKeep test db tables after the test (for debugging)\n");
	print ( "--keep-all\t\tKeep test db and all test data (like generated configs, etc.) after the test (for debugging)\n");
	print ( "--no-demo\t\tJust skip all tests without models. Else - run them, but never fail (for debugging)\n");
	print ( "--no-marks\t\tDon't mark the output of every test in the logs.\n");
	print ( "--valgrind-searchd\t\tRun searchd under valgrind during test.\n");
	print ( "--ignore-weights\tIgnore differences in weights. (Useful for testing that reference database changes are ok.)\n" );
	print ( "--cwd\t\t\tchange directory to ubertest.php location (for git bisect)\n" );
	print ( "\nEnvironment variables are:\n" );
	print ( "DBUSER\t\t\tuse 'USER' as MySQL user\n" );
	print ( "DBPASS\t\t\tuse 'PASS' as MySQL password\n" );
	print ( "\nTests can be specified by full name, or list of IDs, or range of IDs.\n" );
	print ( "\nUsage examples:\n" );
	print ( "php ubertest.php gen\n" );
	print ( "php ubertest.php t --user test --password test\n" );
	print ( "php ubertest.php t test_015\n" );
	print ( "php ubertest.php t 31 37 41 53-64\n" );
	print ( "php ubertest.php show sql_settings\n" );
	print ( "php ubertest.php show searchd_settings\n" );
	print ( "DBPASS=test make check\n" );
	exit ( 0 );
}

$locals = array();
$locals['rt_mode'] = false;
$locals['columnar_mode'] = false;
$locals['testdir'] = '';
$locals['scriptdir'] = '';
$locals['ctest'] = false;

if ( array_key_exists ( "DBUSER", $_ENV ) && $_ENV["DBUSER"] )
	$locals['db-user'] = $_ENV["DBUSER"];

if ( array_key_exists ( "DBPASS", $_ENV ) && $_ENV["DBPASS"] ) 
	$locals['db-password'] = $_ENV["DBPASS"];

$run = false;
$test_dirs = array();
$test_range = array();
$user_skip = false;
$force_guess = true;
$show = false;
$showpar = array();
$ctest = false;

for ( $i=0; $i<count($args); $i++ )
{
	$arg = $args[$i];

	if ( false );
	else if ( $arg=="g" || $arg=="gen" )			{ $g_model = true; $run = true; }
	else if ( $arg=="t" || $arg=="test" )			{ $g_model = false; $run = true; }
	else if ( $arg=="qt" )							{ $g_model = false; $run = true; $user_skip = true; }
	else if ( $arg=="show" )						{ $run = false; $show = true; }
	else if ( $arg=="--managed" )					$sd_managed_searchd = true;
	else if ( $arg=="--skip-indexer")				$sd_skip_indexer = true;
	else if ( $arg=="-u" || $arg=="--user" )		$locals['db-user'] = $args[++$i];
	else if ( $arg=="-p" || $arg=="--password" )	$locals['db-password'] = $args[++$i];
	else if ( $arg=="-i" || $arg=="--indexer" )		$locals['indexer'] = $args[++$i];
	else if ( $arg=="-s" || $arg=="--searchd" )		$locals['searchd'] = $args[++$i];
	else if ( $arg=="-b" || $arg=="--bindir" )		$locals['bin'] = $args[++$i];
	else if ( $arg=="-t" || $arg=="--testdir" )		$locals['testdir'] = $args[++$i];
	else if ( $arg=="-tt" )							$locals['scriptdir'] = $args[++$i];
	else if ( $arg=="--ctest" )						{ $locals['ctest'] = true; $ctest = true; $force_guess = false; }
	else if ( $arg=="--rt" )						$locals['rt_mode'] = true;
	else if ( $arg=="--columnar" )					$locals['columnar_mode'] = true;
	else if ( $arg=="--test-thd-pool" )				$locals['use_pool'] = true;
	else if ( $arg=="--strict" )					$g_strict = true;
	else if ( $arg=="--strict-verbose" )			{ $g_strict = true; $g_strictverbose = true; }
	else if ( $arg=="--valgrind-searchd" )			$locals['valgrindsearchd'] = true;
	else if ( $arg=="--ignore-weights" )			$g_ignore_weights = true;
	else if ( $arg=="--no-drop-db" )				$locals['no_drop_db'] = true;
	else if ( $arg=="--keep-all" )					$locals['keep_all'] = true;
	else if ( $arg=="--no-demo" )					$g_skipdemo = true;
	else if ( $arg=="--no-marks" )					$g_usemarks = false;
	else if ( $arg=="--cwd" )						chdir ( DIRNAME ( __FILE__ ) );
	else if ( $arg=="--sd_extra" )					$locals['extra_searchd_options'] = $args[++$i];
	else if ( is_dir($arg) )						$test_dirs[] = $arg;
	else if ( preg_match ( "/^(\\d+)-(\\d+)$/", $arg, $range ) )
	{
		$test_range = array ( $range[1], $range[2] ); // from, to

	} else if ( preg_match ( "/^(\\d+):(\\d+)$/", $arg, $range ) )
	{
		// FIXME! lockdown gen model, only keep test mode
		// FIXME! lockdown $test_dirs from here, and check it for emptiness
		// ie. make sure there are NO other test arguments when this syntax is in use
		$test_dirs = array ( sprintf ( "test_%03d", $range[1] ) );
		$g_pick_query = (int)$range[2];

	} else if ( is_dir(sprintf("test_%03d", $arg)))
	{
		$test_dirs[] = sprintf("test_%03d", $arg);

	} else if ( $show )
	{
		$showpar[] = $arg;
	} else
	{
		print ( "ERROR: unknown option '$arg'; run with no arguments for help screen.\n" );
		exit ( 1 );
	}
}
if ( !$run && !$show )
{
	print ( "ERROR: no run mode defined; run with no arguments for help screen.\n" );
	exit ( 1 );
}

$VLG = getenv('VLG');

$python = getenv('python');
if (!$python)
  $python = file_exists("/usr/bin/python3")?"/usr/bin/python3":"/usr/bin/python";

if (!$windows && !is_executable($python) && !$ctest)
	die("ubertest needs python support; install python\n");

$cygwin = false;
if ( $locals['scriptdir']!=$locals['testdir'] )
{

	$fscript = @file_get_contents($locals['scriptdir']. "cmake_install.cmake");
	$ftest = @file_get_contents($locals['testdir']. "cmake_install.cmake");

	if ( $fscript && !$ftest)
	{
		$cygwin = true;
//		print ("Is +++ cygwin\n\n");
	}
	else
	{
		$locals['scriptdir'] = $locals['testdir'];
//		print ("Is NOT cygwin\n\n");
	}
}

$index_script_path = $locals['scriptdir'].$index_data_path;
if (!file_exists ($index_script_path))
	mkdir ($index_script_path);

$index_data_path = $locals['testdir'].$index_data_path;

PublishLocals ( $locals, false );

$sd_log				= testdir("searchd.log");
$ss_log				= scriptdir("searchd.log");
$sd_query_log		= testdir("query.log");
$ss_query_log		= scriptdir("query.log");
$sd_pid_file		= testdir("searchd.pid");
$ss_pid_file		= scriptdir("searchd.pid");

require_once ( "helpers.inc" );

if ( $show )
{
	$config = new SphinxConfig ( $g_locals );
	foreach ( $showpar as $nodename )
		$config->show_settings ($nodename);
	exit (0);
}

global $g_guesscached;

if ( $force_guess )
	$g_guesscached = false;
else
	LoadCachedGuesses();

if ( !$g_guesscached ) {
	GuessRE2();
	GuessICU();
	GuessODBC();
	GuessReplication();
	GuessSSL();
	GuessColumnar();
	GuessSecondary();
	GuessKNN();
	if ( !$force_guess )
		CacheGuesses();
}

if ( $g_locals["malloc-scribble"] )
{
	print ( "Malloc scribbling enabled.\n" );
	putenv ( "MallocLogFile=/dev/null" );	
	putenv ( "MallocScribble=1" );
	putenv ( "MallocPreScribble=1" );
	putenv ( "MallocGuardEdges=1" );
}

/////////////
// run tests
/////////////

if ( IsModelGenMode () )
	print ( "GENERATING REFERENCE TEST RESULTS\n\n" );
else
	print ( "PERFORMING AUTOMATED TESTING\n\n" );

$testconn = ConnectDB();
if ( mysqli_connect_error() )
{
	print ( "ERROR: failed to connect to MySQL: " . mysqli_connect_error() . "\n" );
	exit ( 1 );
} else
{
	$testconn->close();
}

$t = MyMicrotime ();

// build test lists
$tests = array ();
$dh = opendir ( "." );
$user_skipped = 0;
while ( $entry=readdir($dh) )
{
	if ( substr ( $entry, 0, 5 )!="test_" )
		continue;
	$test_id = (int) substr ( $entry, 5 );

	if ( $user_skip
		&& isset ( $g_locals["skip-tests"] )
		&& in_array ( $test_id, $g_locals["skip-tests"] ) )
	{
		$user_skipped++;
		continue;
	}

	if ( ( empty($test_dirs) && empty($test_range) )
		|| in_array ( $entry, $test_dirs )
		|| ( $test_range && $test_id>=$test_range[0] && $test_id<=$test_range[1] ) )
	{
		$tests[] = $entry;
	}
}
sort ( $tests );

// full name to short alias
function ShortTestName ( $full )
{
	if ( substr ( $full,0,5 )=="test_" )
		return substr ( $full, 5 );
	return $full;
}

// run tests
$total_tests = 0;
$total_tests_failed = 0;
$total_subtests = 0;
$total_subtests_failed = 0;
$total_skipped = $user_skipped;
$failed_tests = array();

$test_num = 0;
$sd_port_ref = $sd_port;
$agent_port_ref = $agent_port;
$agent_port_sql_ref = $agent_port_sql;
$agent_port_sql_vip_ref = $agent_port_sql_vip;
$agent_port_http_ref = $agent_port_http;
$sd_sphinxql_port_ref = $sd_sphinxql_port;
$sd_sphinxql_port_vip_ref = $sd_sphinxql_port_vip;
$sd_http_port_ref = $sd_http_port;

$name_err_all = $locals['scriptdir'] . "error_all.txt";
$name_err = $locals['scriptdir'] . "error.txt";

foreach ( $tests as $test )
{
	$res_path = scriptdir($test);
	if ( $windows && !$sd_managed_searchd )
	{
		// avoid an issue with daemons stuck in exit(0) for some seconds
		$sd_port = $sd_port_ref + 10 * ( $test_num % 10 );
		$agent_port = $agent_port_ref + 10 * ( $test_num % 10 );
		$agent_port_sql = $agent_port_sql_ref + 10 * ( $test_num % 10 );
		$agent_port_sql_vip = $agent_port_sql_vip_ref + 10 * ( $test_num % 10 );
		$agent_port_http = $agent_port_http_ref + 10 * ( $test_num % 10 );
		$sd_sphinxql_port = $sd_sphinxql_port_ref + 10 * ( $test_num % 10 );
		$sd_sphinxql_port_vip = $sd_sphinxql_port_vip_ref + 10 * ( $test_num % 10 );
		$sd_http_port = $sd_http_port_ref + 10 * ( $test_num % 10 );
		$test_num += 1;
		$agents	= array (
			array ( "address" => $sd_address, "port" => $sd_port, "sqlport" => $sd_sphinxql_port, "sqlport_vip" => $sd_sphinxql_port_vip, "replication_port"=>0, "http_port" => $sd_http_port ),
			array ( "address" => $agent_address, "port" => $agent_port, "sqlport" => $agent_port_sql, "sqlport_vip" => $agent_port_sql_vip, "replication_port"=>0, "http_port" => $agent_port_http ),
			array ( "address" => $agent_address, "port" => $agent_port+1, "sqlport" => $agent_port_sql+1, "sqlport_vip" => $agent_port_sql_vip+1, "replication_port"=>0, "http_port" => $agent_port_http+1 )
		);
	}


	if (!$g_model)
		if ( !file_exists ( $res_path ) )
			mkdir ( $res_path );

	if ( file_exists ( $test."/test.xml" ) )
	{
		$total_tests++;
		$res = RunTest ( $test, $g_skipdemo, $g_usemarks );
		
		// copy searchd log into a file
		file_put_contents($name_err_all, "\n*** in test $test ***\n", FILE_APPEND);
		if ( file_exists ($name_err) )
		{
			file_put_contents($name_err_all, file_get_contents($name_err), FILE_APPEND);
		}

		if ( !is_array($res) )
		{
			// failed to run that test at all
			$total_tests_failed++;
			$failed_tests[] = ShortTestName ( $test );
			continue;
		}

		$total_subtests += $res["tests_total"];
		$total_skipped += $res["tests_skipped"];
		if ( $res["tests_failed"] )
		{
			$total_tests_failed++;
			$total_subtests_failed += $res["tests_failed"];
			$failed_tests[] = ShortTestName ( $test );
			if ( $g_strict )
			{
				if ( $g_strictverbose )
				{
					$report = file_get_contents ( "$res_path/report.txt" );
					$report.= "\n Test $test failed\n";
					file_put_contents(scriptdir("report.txt"),$report);
					$report = "";
				}
				break;
			}
		}
	}
	elseif ( file_exists ( $test."/test.inc" ) )
	{
		$run_func = function ( $test_path ) use ($test) { return file_get_contents ( $test."/test.inc" ); };

		$total_tests++;
		$total_subtests++;

		if ( !$run_func ( $test ) )
		{
			$total_tests_failed++;
			$total_subtests_failed++;
			$failed_tests[] = ShortTestName ( $test );
		}
	}
}

// cleanup
if ( !array_key_exists ('keep_all', $g_locals) )
{
	@unlink("config.conf");
	@unlink("error.txt");

	$nfile = 1;
	while (file_exists("config_$nfile.conf")) {
		@unlink("config_$nfile.conf");
		$nfile++;
	}

	$nfile = 1;
	while (file_exists("error_$nfile.txt")) {
		// FIXME? not when there were actual errors?
		@unlink("error_$nfile.txt");
		$nfile++;
	}
}
// summarize
if ( $total_tests_failed ) {
	printf("\nTo re-run failed tests only:\nphp ubertest.php t %s\n", join(" ", $failed_tests));
	printf("\n%d of %d tests and %d of %d subtests failed, %d tests skipped, %.2f sec elapsed\nTHERE WERE FAILURES!\n",
		$total_tests_failed, $total_tests,
		$total_subtests_failed, $total_subtests, $total_skipped,
		MyMicrotime() - $t);
	exit (1);
} elseif ( $total_skipped )
{
	printf ( "\n%d tests and %d subtests succesful, %d tests skipped, %.2f sec elapsed\nSKIPPED\n",
		$total_tests, $total_subtests, $total_skipped,
		MyMicrotime()-$t );
	exit ( 42 );
} else
{
	printf ( "\n%d tests and %d subtests succesful, %d tests skipped, %.2f sec elapsed\nALL OK\n",
		$total_tests, $total_subtests, $total_skipped,
		MyMicrotime()-$t );
	exit ( 0 );
}

?>
