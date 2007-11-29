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

$g_model			= false;

require_once ( "helpers.php" );

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
	print ( "Usage: php -f ubertest.php <MODE> [OPTIONS]\n\n" );
	print ( "Modes are:\n" );
	print ( "g, gen\t\t\tgenerate reference ('model') test results\n" );
	print ( "t, test\t\t\trun tests and compare results to reference\n" );
	print ( "\nOptions are:\n" );
	print ( "-u, --user <USER>\tuse 'USER' as MySQL user\n" );
	print ( "-P, --password <PASS>\tuse 'PASS' as MySQL password\n" );
	print ( "-i, --indexer <PATH>\tpath to indexer\n" );
	print ( "-s, --searchd <PATH>\tpath to searchd\n" );
	print ( "\nExamples:\n" );
	print ( "php ubertest.php gen\n" );
	print ( "php ubertest.php t --user test --password test\n" );
	exit ( 0 );
}

$run = false;
for ( $i=0; $i<count($args); $i++ )
{
	$arg = $args[$i];

	if ( false );
	else if ( $arg=="g" || $arg=="gen" )			{ $g_model = true; $run = true; }
	else if ( $arg=="t" || $arg=="test" )			{ $g_model = false; $run = true; }
	else if ( $arg=="-u" || $arg=="--user" )		$db_user = $args[++$i];
	else if ( $arg=="-P" || $arg=="--password" )	$db_pwd = $args[++$i];
	else if ( $arg=="-i" || $arg=="--indexer" )		$indexer_path = $args[++$i];
	else if ( $arg=="-s" || $arg=="--searchd" )		$searchd_path = $args[++$i];
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

/////////////
// run tests
/////////////

if ( IsModelGenMode () )
	print ( "GENERATING REFERENCE TEST RESULTS\n\n" );
else
	print ( "PERFORMING AUTOMATED TESTING\n\n" );


$dh = opendir ( "." );
while ( $entry = readdir($dh) )
{
	if ( substr ( $entry,0,4 )!="test" )
		continue;

	RunTest ( $entry );
}

unlink ( "error.txt" );
?>