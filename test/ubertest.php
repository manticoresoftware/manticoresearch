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
	print ( "Usage: php -f ubertest.php <MODE> [OPTIONS]\n" );
	print ( "\nModes are:\n" );
	print ( "g, gen\t\t\tgenerate reference ('model') test results\n" );
	print ( "t, test\t\t\trun tests and compare results to reference\n" );
	print ( "\nOptions are:\n" );
	print ( "-u, --user <USER>\tuse 'USER' as MySQL user\n" );
	print ( "-p, --password <PASS>\tuse 'PASS' as MySQL password\n" );
	print ( "-i, --indexer <PATH>\tpath to indexer\n" );
	print ( "-s, --searchd <PATH>\tpath to searchd\n" );
	print ( "\nEnvironment vriables are:\n" );
	print ( "DBUSER\tuse 'USER' as MySQL user\n" );
	print ( "DBPASS\tuse 'PASS' as MySQL password\n" );
	print ( "\nUsage examples:\n" );
	print ( "php ubertest.php gen\n" );
	print ( "php ubertest.php t --user test --password test\n" );
	print ( "DBPASS=test make check\n" );
	exit ( 0 );
}

if ( array_key_exists ( "DBUSER", $_ENV ) && $_ENV["DBUSER"] )
	$db_user = $_ENV["DBUSER"];

if ( array_key_exists ( "DBPASS", $_ENV ) && $_ENV["DBPASS"] ) 
	$db_pwd = $_ENV["DBPASS"];

$run = false;
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


$t = MyMicrotime ();

$tests = array ();
$dh = opendir ( "." );
while ( $entry = readdir($dh) )
{
	if ( substr ( $entry,0,4 )=="test" )
		$tests[] = $entry;
}
sort ( $tests );

foreach ( $tests as $test )
	RunTest ( $test );


// perform cleanup
@unlink ( "config.conf" );
@unlink ( "error.txt" );

$nfile = 0;
while ( file_exists ( "config_$nfile.conf" ) )
{
	@unlink ( "config_$nfile.conf" );
	$nfile++;
}

$nfile = 0;
while ( file_exists ( "error_$nfile.txt" ) )
{
	@unlink ( "error_$nfile.txt" );
	$nfile++;
}

printf ( "%.2f sec elapsed\n", MyMicrotime()-$t );

//
// $Id$
//

?>
