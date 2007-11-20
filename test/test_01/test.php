<?php

require_once ( "helpers.php" );

function RunTest_01 ()
{
	$test_dir 	= "test_01/";
	$model_file = $test_dir."model.bin";
	$conf_dir 	= $test_dir."Conf";

	printf ( "Running infix_fields / prefix_fields test...\n" );

	$db_link = CreateDB ( $test_dir."db_drop.sql", $test_dir."db_create.sql", $test_dir."db_insert.sql" );
	if ( ! $db_link )
		die ( "Error creating test database\n" );

	$config = new SphinxConfig;
	$config->Load ( $test_dir."test.xml" );
	if ( ! $config->LoadModel ( $model_file ) )
		die ( "Error loading model\n" );
		
	if ( ! file_exists ( $conf_dir ) )
		mkdir ( $conf_dir );

	$report = fopen ( $test_dir."report.txt", "w" );

	$nfailed = 0;

	while ( $config->CreateNextConfig ( "config.conf" ) )
	{
		StopSearchd ();

		printf ( "\r\tRunning subtest %d...", $config->SubtestNo () + 1 );

		$config->Write ( $conf_dir."/"."config_".$config->SubtestNo ().".conf" );

		$error = "";
		if ( RunIndexer () != 0 )
		{
			print ( "Error running indexer... FAILED\n" );
			$nfailed++;
			$config->SubtestFinished ();
			continue;
		}

		if ( StartSearchd () != 0 )
		{
			print ( "Error starting searchd... FAILED\n" );
			$nfailed++;
			$config->SubtestFinished ();
			continue;
		}

		$error = "";
		if ( ! $config->RunQuery ( "*", $error ) )
		{
			print ( "Error running query: $error... FAILED\n" );
			$nfailed++;
			$config->SubtestFinished ();
			continue;
		}

		if ( ! IsModelGenMode () && ! $config->CompareToModel () )
		{
			print ( "FAILED\n" );
			$nfailed++;
		}

		$config->WriteResults ( $report );
		$config->SubtestFinished ();
	}

	StopSearchd ();

	fclose ( $report );

	printf ( "\r\t%d subtests total, %d failed\n", $config->SubtestNo () + 1, $nfailed );

	mysql_close ( $db_link );

	$config->WriteModel ( $model_file );

	$test_result = new TestResult;
	$test_result->_tests_total = $config->SubtestNo () + 1;
	$test_result->_tests_failed = $nfailed;

	return $test_result;
}

?>