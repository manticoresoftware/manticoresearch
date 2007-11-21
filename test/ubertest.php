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

require_once ( "helpers.php" );

if ( IsModelGenMode () )
	print ( "Model generation is ON\n" );

require_once ( "test_00/test.php" );
require_once ( "test_01/test.php" );

RunTest_00 ();
RunTest_01 ();

?>