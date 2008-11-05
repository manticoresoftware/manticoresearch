<?php

//
// $Id$
//

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

$g_model			= false;
$g_id64				= false;
$g_strict			= false;

require_once ( "helpers.inc" );

if ( $windows )
{
	$indexer_path = "..\\bin\\debug\\indexer";
	$searchd_path = "..\\bin\\debug\\searchd";
}
else
{
	$indexer_path = "../src/indexer";
	$searchd_path = "../src/searchd";
}

//////////////////////
// parse command line
//////////////////////

$args = $_SERVER["argv"];
array_shift ( $args );

if ( !is_array($args) || empty($args) )
{
	print ( "Usage: php -f ubertest.php <MODE> [OPTIONS] [TESTDIR]\n" );
	print ( "\nModes are:\n" );
	print ( "g, gen\t\t\tgenerate reference ('model') test results\n" );
	print ( "t, test\t\t\trun tests and compare results to reference\n" );
	print ( "\nOptions are:\n" );
	print ( "-u, --user <USER>\tuse 'USER' as MySQL user\n" );
	print ( "-p, --password <PASS>\tuse 'PASS' as MySQL password\n" );
	print ( "-i, --indexer <PATH>\tpath to indexer\n" );
	print ( "-s, --searchd <PATH>\tpath to searchd\n" );
	print ( "--strict\t\tterminate on the first failure (for automatic runs)\n" );
	print ( "\nEnvironment vriables are:\n" );
	print ( "DBUSER\tuse 'USER' as MySQL user\n" );
	print ( "DBPASS\tuse 'PASS' as MySQL password\n" );
	print ( "\nUsage examples:\n" );
	print ( "php ubertest.php gen\n" );
	print ( "php ubertest.php t --user test --password test\n" );
	print ( "php ubertest.php t test_15\n" );
	print ( "DBPASS=test make check\n" );
	exit ( 0 );
}

if ( array_key_exists ( "DBUSER", $_ENV ) && $_ENV["DBUSER"] )
	$db_user = $_ENV["DBUSER"];

if ( array_key_exists ( "DBPASS", $_ENV ) && $_ENV["DBPASS"] ) 
	$db_pwd = $_ENV["DBPASS"];

$run = false;
$test_dirs = array();
for ( $i=0; $i<count($args); $i++ )
{
	$arg = $args[$i];

	if ( false );
	else if ( $arg=="g" || $arg=="gen" )			{ $g_model = true; $run = true; }
	else if ( $arg=="t" || $arg=="test" )			{ $g_model = false; $run = true; }
	else if ( $arg=="-u" || $arg=="--user" )		$db_user = $args[++$i];
	else if ( $arg=="-p" || $arg=="--password" )	$db_pwd = $args[++$i];
	else if ( $arg=="-i" || $arg=="--indexer" )		$indexer_path = $args[++$i];
	else if ( $arg=="-s" || $arg=="--searchd" )		$searchd_path = $args[++$i];
	else if ( is_dir($arg) )						$test_dirs[] = $arg;
	else if ( is_dir("test_$arg") )					$test_dirs[] = "test_$arg";
	else if ( $arg=="--strict" )					$g_strict = true;
	else
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

// guess the size of document IDs

exec ( $indexer_path, $output, $result );
if ( count($output) == 0 )
{
	print "ERROR: failed to run the indexer\n";
	exit ( 1 );
}
else
	$g_id64 = strstr ( $output[0], 'id64' ) !== false;

/////////////
// run tests
/////////////

if ( IsModelGenMode () )
	print ( "GENERATING REFERENCE TEST RESULTS\n\n" );
else
	print ( "PERFORMING AUTOMATED TESTING\n\n" );


$t = MyMicrotime ();

// build test lists
$tests = array ();
$dh = opendir ( "." );
while ( $entry=readdir($dh) )
{
	if ( substr ( $entry,0,4 )!="test" )
		continue;
	if ( !empty($test_dirs) && !in_array ( $entry, $test_dirs ) )
		continue;
	$tests[] = $entry;
}
sort ( $tests );

// run tests
$total_tests = 0;
$total_tests_failed = 0;
$total_subtests = 0;
$total_subtests_failed = 0;
foreach ( $tests as $test )
{
	if ( $windows )
	{
		// avoid an issue with daemons stuck in exit(0) for some seconds
		$sd_port += 10;
		$agent_port += 10;
		$agents	= array (
			array ( "address" => $sd_address, "port" => $sd_port ),
			array ( "address" => $agent_address, "port" => $agent_port ) );
	}

	if ( file_exists ( $test."/test.xml" ) )
	{
		$res = RunTest ( $test );

		if ( !is_array($res) )
			continue; // failed to run that test at all

		$total_tests++;
		$total_subtests += $res["tests_total"];
		if ( $res["tests_failed"] )
		{
			$total_tests_failed++;
			$total_subtests_failed += $res["tests_failed"];
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
	printf ( "\n%d of %d tests and %d of %d subtests failed, %.2f sec elapsed\nTHERE WERE FAILURES!\n",
		$total_tests_failed, $total_tests,
		$total_subtests_failed, $total_subtests,
		MyMicrotime()-$t );
	exit ( 1 );
} else
{
	printf ( "\n%d tests and %d subtests succesful, %.2f sec elapsed\nALL OK\n",
		$total_tests, $total_subtests,
		MyMicrotime()-$t );
	exit ( 0 );
}

//
// $Id$
//

?>
