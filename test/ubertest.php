<?php

//
// $Id$
//

$sd_managed_searchd	= false;
$sd_skip_indexer = false;
$g_ignore_weights = false;

require_once ( "settings.inc" );

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
	print ( "\nOptions are:\n" );
	print ( "-u, --user <USER>\tuse 'USER' as MySQL user\n" );
	print ( "-p, --password <PASS>\tuse 'PASS' as MySQL password\n" );
	print ( "-i, --indexer <PATH>\tpath to indexer\n" );
	print ( "-s, --searchd <PATH>\tpath to searchd\n" );
	print ( "--strict\t\tterminate on the first failure (for automatic runs)\n" );
	print ( "--managed\t\tdon't run searchd during test (for debugging)\n" );
	print ( "--skip-indexer\t\tskip DB creation and indexer stages and go directly to queries/custom tests\n");
	print ( "--rt\t\t\ttest RT backend (auto-convert all local indexes)\n" );
	print ( "\nEnvironment vriables are:\n" );
	print ( "DBUSER\t\t\tuse 'USER' as MySQL user\n" );
	print ( "DBPASS\t\t\tuse 'PASS' as MySQL password\n" );
	print ( "\nTests can be specified by full name, or list of IDs, or range of IDs.\n" );
	print ( "\nUsage examples:\n" );
	print ( "php ubertest.php gen\n" );
	print ( "php ubertest.php t --user test --password test\n" );
	print ( "php ubertest.php t test_015\n" );
	print ( "php ubertest.php t 31 37 41 53-64\n" );
	print ( "DBPASS=test make check\n" );
	exit ( 0 );
}

$locals = array();
$locals['rt_mode'] = false;

if ( array_key_exists ( "DBUSER", $_ENV ) && $_ENV["DBUSER"] )
	$locals['db-user'] = $_ENV["DBUSER"];

if ( array_key_exists ( "DBPASS", $_ENV ) && $_ENV["DBPASS"] ) 
	$locals['db-password'] = $_ENV["DBPASS"];

$run = false;
$test_dirs = array();
$test_range = array();
for ( $i=0; $i<count($args); $i++ )
{
	$arg = $args[$i];

	if ( false );
	else if ( $arg=="g" || $arg=="gen" )			{ $g_model = true; $run = true; }
	else if ( $arg=="t" || $arg=="test" )			{ $g_model = false; $run = true; }
	else if ( $arg=="--managed" )					$sd_managed_searchd = true;
	else if ( $arg=="--skip-indexer")				$sd_skip_indexer = true;
	else if ( $arg=="-u" || $arg=="--user" )		$locals['db-user'] = $args[++$i];
	else if ( $arg=="-p" || $arg=="--password" )	$locals['db-password'] = $args[++$i];
	else if ( $arg=="-i" || $arg=="--indexer" )		$locals['indexer'] = $args[++$i];
	else if ( $arg=="-s" || $arg=="--searchd" )		$locals['searchd'] = $args[++$i];
	else if ( $arg=="--rt" )						$locals['rt_mode'] = true;
	else if ( $arg=="--strict" )					$g_strict = true;
	else if ( $arg=="--ignore-weights" )			$g_ignore_weights = true;
	else if ( is_dir($arg) )						$test_dirs[] = $arg;
	else if ( preg_match ( "/^(\\d+)-(\\d+)$/", $arg, $range ) )
	{
		$test_range = array ( $range[1], $range[2] ); // from, to

	} else if ( is_dir(sprintf("test_%03d", $arg)))
	{
		$test_dirs[] = sprintf("test_%03d", $arg);

	} else
	{
		print ( "ERROR: unknown option '$arg'; run with no arguments for help screen.\n" );
		exit ( 1 );
	}
}
if ( !$run )
{
	print ( "ERROR: no run mode defined; run with no arguments for help screen.\n" );
	exit ( 1 );
}

PublishLocals ( $locals, false );
GuessIdSize();

if ( $g_locals["malloc-scribble"] )
{
	print ( "Malloc scribbling enabled.\n" );
	putenv ( "MallocLogFile=/dev/null" );	
	putenv ( "MallocScribble=1" );
	putenv ( "MallocPreScribble=1" );
	putenv ( "MallocGuardEdges=1" );
}

require_once ( "helpers.inc" );

/////////////
// run tests
/////////////

if ( IsModelGenMode () )
	print ( "GENERATING REFERENCE TEST RESULTS\n\n" );
else
	print ( "PERFORMING AUTOMATED TESTING\n\n" );

$testconn = ConnectDB();
if ( !$testconn )
{
	print ( "ERROR: failed to connect to MySQL: " . mysql_error() . "\n" );
	exit ( 1 );
} else
{
	mysql_close ( $testconn );
}

$t = MyMicrotime ();

// build test lists
$tests = array ();
$dh = opendir ( "." );
while ( $entry=readdir($dh) )
{
	if ( substr ( $entry, 0, 5 )!="test_" )
		continue;
	$test_id = (int) substr ( $entry, 5 );

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
$total_skipped = 0;
$failed_tests = array();
foreach ( $tests as $test )
{
	if ( $windows && !$sd_managed_searchd )
	{
		// avoid an issue with daemons stuck in exit(0) for some seconds
		$sd_port += 10;
		$agent_port += 10;
		$agent_port_sql += 10;
		$agents	= array (
			array ( "address" => $sd_address, "port" => $sd_port, "sqlport" => $sd_sphinxql_port ),
			array ( "address" => $agent_address, "port" => $agent_port, "sqlport" => $agent_port_sql ),
			array ( "address" => $agent_address, "port" => $agent_port+1, "sqlport" => $agent_port_sql+1 )
		);
	}

	if ( file_exists ( $test."/test.xml" ) )
	{
		$total_tests++;
		$res = RunTest ( $test );

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
				break;
		}
	}
	elseif ( file_exists ( $test."/test.inc" ) )
	{
		$run_func = create_function ( '$test_path', file_get_contents ( $test."/test.inc" ) );

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
@unlink ( "config.conf" );
@unlink ( "error.txt" );

$nfile = 1;
while ( file_exists ( "config_$nfile.conf" ) )
{
	@unlink ( "config_$nfile.conf" );
	$nfile++;
}

$nfile = 1;
while ( file_exists ( "error_$nfile.txt" ) )
{
	@unlink ( "error_$nfile.txt" );
	$nfile++;
}

// summarize
if ( $total_tests_failed )
{
	printf ( "\nTo re-run failed tests only:\nphp ubertest.php t %s\n", join ( " ", $failed_tests ) );
	printf ( "\n%d of %d tests and %d of %d subtests failed, %d tests skipped, %.2f sec elapsed\nTHERE WERE FAILURES!\n",
		$total_tests_failed, $total_tests,
		$total_subtests_failed, $total_subtests,$total_skipped,
		MyMicrotime()-$t );
	exit ( 1 );
} else
{
	printf ( "\n%d tests and %d subtests succesful, %d tests skipped, %.2f sec elapsed\nALL OK\n",
		$total_tests, $total_subtests, $total_skipped,
		MyMicrotime()-$t );
	exit ( 0 );
}

//
// $Id$
//

?>
