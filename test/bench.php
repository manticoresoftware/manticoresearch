<?php

ini_set ( "display_errors", 1 );
ini_set ( "error_reporting", E_ALL | E_STRICT );

assert_options ( ASSERT_ACTIVE, 1 );
assert_options ( ASSERT_BAIL, 1 );

require_once ( "settings.inc" );

////////////////////////////////////////////////////////////////////////////////

function Fatal ( $message )
{
	print "FATAL: $message.\n";
	exit ( 1 );
}

function sphCompareTime ( $a, $b )
{
	if ( abs ( $a - $b ) < 0.01 )
		return sprintf ( "%.3f", $a );

	$p = $a ? sprintf ( " %+.1f%%", ( $b / $a - 1 ) * 100 ) : '';
	return sprintf ( "%.2f / %.2f%s", $a, $b, $p );
}

function sphCompareKeys ( $sets, $i, $key )
{
	return
		$sets[0][$i][$key] == $sets[1][$i][$key] ?
		$sets[0][$i][$key] :
		$sets[0][$i][$key] . ' / ' . $sets[1][$i][$key];
}

function sphCompareSets ( $sets )
{
	$tag = null;
	$report = array();
	for ( $i=0; $i<count($sets[0]); $i++ )
	{
		if ( array_key_exists ( 'tag', $sets[0][$i] ) && $sets[0][$i]['tag'] != $tag )
		{
			$tag = $sets[0][$i]['tag'];
			$report[] = array ( $tag );
		}
 		$report[] = array (
 			sphCompareKeys ( $sets, $i, 'query' ),
 			sphCompareTime ( $sets[0][$i]['time'], $sets[1][$i]['time'] ),
 			sphCompareKeys ( $sets, $i, 'total' ),
 			sphCompareKeys ( $sets, $i, 'total_found' )
		);
	}
	return $report;
}

function sphCompare ( $sources )
{
	$data = array ( unserialize ( file_get_contents($sources[0]) ),
					unserialize ( file_get_contents($sources[1]) ) );

	$report = array (
		'report' => array(),
		'sources' => $sources,
		'versions' => array ( $data[0]['version'], $data[1]['version'] ),
		'checksums' => $data[0]['hash'] == $data[1]['hash'],
	);

	if ( count($data[0]['results']) != count($data[1]['results']) )
		Fatal ( "unable to produce the report, result set sizes mismatch" );

	$combined = array();
	$skip = array();
	foreach ( $data as &$set )
	{
		$out = array();
		for ( $i=0; $i<count($set['results']); $i++ )
		{
			if ( in_array ( $i, $skip ) ) continue;
			
			$row = &$set['results'][$i];
			if ( $row['total'] == -1 )
			{
				$skip[] = $i;
				continue;
			}

			$tag = $row['tag'];
			if ( !array_key_exists ( $tag, $out ) )
			{
				$out[$tag] = array (
					'query' => $tag,
					'time' => 0,
					'total' => 0,
					'total_found' => 0
				);
			}

			$row['time'] = (float)$row['time'];
			$out[$tag]['time'] += $row['time'];
			$out[$tag]['total'] += $row['total'];
			$out[$tag]['total_found'] += $row['total_found'];
		}
		$combined[] = $out;
	}

	if ( array_keys($combined[0]) != array_keys($combined[1]) )
		Fatal ( "unable to produce the report, tag sets mismatch" );

	$report['aggregate'] = sphCompareSets ( array ( array_values ( $combined[0] ),
													array_values ( $combined[1] ) ) );

	$report['detailed'] = sphCompareSets ( array ( $data[0]['results'],
												   $data[1]['results'] ) );
	
	return $report;
}

////////////////////////////////////////////////////////////////////////////////

function sphTextReport ( $report )
{
	global $g_locals;
	$mode = $g_locals['mode'];
	
	$width = array ();
	$header = array ( 'query', 'time', 'total', 'total found' );

	foreach ( $header as $title )
		$width[] = strlen($title);
	foreach ( $report[$mode] as $row )
	{
		if ( count($row)==1 ) continue;
		for ( $i=0; $i<4; $i++ )
		{
			$len = min ( strlen($row[$i]), 40 );
			$width[$i] = max ( $width[$i], $len );
		}
	}

	printf ( "COMPARING: %s - %s\n", $report['versions'][0], $report['sources'][0] );
	printf ( "           %s - %s\n", $report['versions'][1], $report['sources'][1] );

	if ( !$report['checksums'] )
		printf ( "WARNING: checksum mismatch, results might be incorrect\n\n" );

	if ( $mode=='aggregate' ) printf("\n");
	$restart = true;
	foreach ( $report[$mode] as $row )
	{
		if ( count($row)==1 )
		{
			printf("\n");
			$w = str_repeat ( '=', ( array_sum($width) + 9 - strlen($row[0]) ) / 2 );
			printf ( " %s %s %s\n", $w, $row[0], $w );
			$restart = true;
		}
		if ( $restart )
		{
			for ( $i=0; $i<4; $i++ )
				printf ( ' %s', str_pad ( $header[$i], $width[$i] + 2, ' ', STR_PAD_BOTH ) );
			printf("\n");
			for ( $i=0; $i<4; $i++ )
				printf ( ' %s', str_repeat ( '-', $width[$i] + 2 ) );
			$restart = false;
			printf("\n");
		}
		if ( count($row)==4 )
			for ( $i=0; $i<4; $i++ )
			{
				$text = $row[$i];
				if ( strlen($text) > 40 )
					$text = substr ( $text, 0, 37 ) . '...';
				if ( $i>1 )
					$text = number_format ( $text, 0, '', ' ' );
				printf ( ' %s', str_pad ( $text, $width[$i] + 2, ' ', STR_PAD_LEFT ) );
			}
		printf("\n");
	}

	if ( !count ( $report[$mode] ) )
		printf ( "empty report.\n" );

}

////////////////////////////////////////////////////////////////////////////////

/// returns results file name on success; false on failure
function sphBenchmark ( $name, $locals, $force_reindex )
{
	// load config
	$config = new SphinxConfig ( $locals );
	if ( !( $config->Load ( "bench/$name.xml" ) && CheckConfig ( $config, $name ) ) )
		return false;

	// temporary limitations
	assert ( $config->SubtestCount()==1 );
	assert ( $config->IsQueryTest() );

	// find unused output prefix
	$i = 0;
	for ( ; file_exists("bench-results/$name.$i.bin"); $i++ );
	$output = "bench-results/$name.$i";
	
	printf ( "benchmarking: %s\n", $config->Name() );

	// grab index names and paths
	$config->EnableCompat098 ();
	$config->WriteConfig ( 'config.conf', 'all' );
	$indexes = array();
	$text = file_get_contents('config.conf');
	preg_match_all ( '/index\s+(\S+)\s+{[^}]+path\s*=\s*(.*)[^}]+}/m', $text, $matches );
	for ( $i=0; $i<count($matches[1]); $i++ )
		$indexes[$matches[1][$i]] = $matches[2][$i];
	
	// checksum/reindex as needed
	$hash = null;
	foreach ( $indexes as $name => $path )
	{
		printf ( "index: %s - ", $name );
		if ( !is_readable ( "$path.spa" ) || !is_readable ( "$path.spi" ) || $force_reindex )
		{
			printf ( "indexing... " );
			$tm = MyMicrotime();
			$result = RunIndexer ( $error, $name );
			$tm = MyMicrotime() - $tm;
			if ( $result==1 )
			{
				printf ( "\nerror running the indexer:\n%s\n", $error );
				return false;
			}
			else if ( $result==2 )
				printf ( "done in %s, there were warnings:\n%s\n", sphFormatTime($tm), $error );
			else
				printf ( "done in %s - ", sphFormatTime($tm) );
		}
		$hash = array ( 'spi' => md5_file ( "$path.spi" ),
						'spa' => md5_file ( "$path.spa" ) );
		printf ( "%s\n", $hash['spi'] );
	}

	// start searchd
	if ( !$locals['skip-searchd'] )
	{
		$result = StartSearchd ( 'config.conf', "$output.searchd.txt", 'searchd.pid', $error );
		if ( $result==1 )
		{
			printf ( "error starting searchd:\n%s\n", $error );
			return false;
		}
		else if ( $result==2 )
			printf ( "searchd warning: %s\n", $error );
	}

	// run the benchmark
	if ( $config->RunQuery ( '*', $error, 'warming-up:' ) &&
		 $config->RunQuery ( '*', $error, 'profiling:' ) )
	{
		$report = array (
			'results' => array(),
			'time' => time(),
			'hash' => $hash,
			'version' => GetVersion()
		);
		$i = 0; $q = null;
		foreach ( $config->Results() as $result )
		{
			if ( $result[0] !== $q )
			{
				$i = 0;
				$q = $result[0];
			}
			$query = $config->GetQuery ( $q );
			$report['results'][] =
				array ( 'total'			=> $result[1],
						'total_found'	=> $result[2],
						'time'			=> $result[3],
						'query'			=> $query['query'][$i++],
						'tag'			=> $query['tag'] );
			
		}
		file_put_contents ( "$output.bin", serialize ( $report ) );
		printf ( "results saved to: $output.bin\n" );
	}
	else
		printf ( "\nfailed to run queries:\n%s\n", $error );

	// shutdown
	StopSearchd ( 'config.conf', 'searchd.pid' );

	// all good
	return $output;
}

////////////////////////////////////////////////////////////////////////////////
 
/// pick $count freshest run files for $bench benchmark
function PickFreshest ( $bench, $count )
{
	// traverse results dir for $bench.RUNID.bin
	$found = array();
	$dh = opendir ( "bench-results" );
	if ( $dh )
	{
		while ( $entry = readdir ( $dh ) )
		{
			if ( substr ( $entry, 0, 1+strlen($bench) )!==$bench."." )
				continue;
			if ( substr ( $entry, -4 )!==".bin" )
				continue;

			$index = (int)substr ( $entry, 1+strlen($bench) );
			$found[$index] = "bench-results/$entry";
		}
		closedir ( $dh );
	}

	if ( !$found )
		return null;

	ksort ( $found );
	return array_slice ( array_values ( $found ), -$count );
}


/// lookup run file by name, doing some common guesses
function LookupRun ( $name )
{
	// try full BENCH.RUNID guesses first
	$found = null;

	foreach ( array ( $name, "$name.bin", "bench-results/$name", "bench-results/$name.bin" ) as $guess )
	{
		if ( is_readable($guess) )
		{
			$found = $guess;
			break;
		}
	}

	// try pick the freshest one by bench name next
	if ( !$found )
		list($found) = PickFreshest ( $name, 1 );

	// still not found? too bad
	if ( !$found )
		Fatal ( "no run files for '$name' found" );

	return $found;
}


function BenchPrintHelp ( $path )
{
	print <<<EOT
Usage: php $path <COMMAND> [OPTIONS]

Commands are:
  b, benchmark BENCH	benchmark (and store run result)
  bv BENCH		benchmark, view run result
  bb BENCH		(forcibly) build index, benchmark
  bbv BENCH		(forcibly) build index, benchmark, view run result
  c, compare BENCH	compare two latest run results of given benchmark
  c, compare BENCH RUNID1 RUNID2
  c, compare BENCH.RUNID1 BENCH.RUNID2
  			compare two given run results
  t, try BENCH		benchmark, view, but do not store run result
  v, view BENCH.RUNID	view given run result

Examples:
  bench.php b mytest	run 'mytest' benchmark
  bench.php v mytest	view latest 'mytest' run result
  bench.php v mytest.4	view 4th 'mytest' run result
  bench.php c mytest 3 7
  bench.php c mytest.3 mytest.7
  			compare runs 3 and 7 of 'mytest' benchmark

EOT;
}


function AvailableBenchmarks ()
{
	$dh = opendir ( "bench" );
	if ( !$dh )
		return;

	$avail = array ();
	while ( $entry = readdir ( $dh ) )
		if ( substr ( $entry, -4 )===".xml" )
			$avail[] = substr ( $entry, 0, -4 );

	closedir ( $dh );
	return join ( ", or ", $avail );
}


function Main ( $argv )
{
	if ( count($argv)==1 )
	{
		BenchPrintHelp ( $argv[0] );
		return 0;
	}

	$mode = null;
	$files = array();
	$locals = array();

	// parse arguments
	$force_reindex = false;
	$view_results = false;
	$unlink_run = false;

	for ( $i=1; $i<count($argv); $i++ )
	{
		$opt = $argv[$i];
		if ( substr ( $opt, 0, 2 ) == '--' )
		{
			$values = explode ( '=', substr ( $opt, 2 ), 2 );
			if ( count($values)==2 )
				$locals[$values[0]] = $values[1];
		}
		else if ( $opt[0] == '-' && $i < ( count($argv) - 1 ) )
		{
			$locals [ substr ( $opt, 1 ) ] = $argv[++$i];
		}
		else if ( $mode )
		{
			$files[] = $opt;
		}
		else
		{
			switch ( $opt )
			{
				case 'b':
				case 'benchmark':
					$mode = 'benchmark';
					break;

				case 'bb':
					$mode = 'benchmark';
					$force_reindex = true;
					break;

				case 'bv':
					$mode = 'benchmark';
					$view_results = true;
					break;

				case 'bbv':
					$mode = 'benchmark';
					$force_reindex = true;
					$view_results = true;
					break;

				case 'c':
				case 'compare':
					$mode = 'compare';
					break;

				case 't':
				case 'try':
					$mode = 'benchmark';
					$view_results = true;
					$unlink_run = true;
					break;

				case 'v':
				case 'view':
					$mode = 'view';
					break;

				default:
					Fatal ( "unknown mode '$opt' (run bench.php without arguments for help)" );
			}
		}
	}

	global $g_locals;
	PublishLocals ( $locals, true );
	require_once ( "helpers.inc" );

	// run the command
	if ( $mode=='benchmark' )
	{
		if ( count($files)==0 )
		{
			$avail = AvailableBenchmarks ();
			if ( $avail )
				$avail = " ($avail)";

			Fatal ( "benchmark command requires a benchmark name$avail" );
		}

		if ( count($files)!=1 )
			Fatal ( "benchmark command requires exactly one name" );

		$path = "bench/$files[0].xml";
		if ( !is_readable($path) )
			Fatal ( "benchmark scenario '$path' is not readable" );

		$res = sphBenchmark ( $files[0], $g_locals, $force_reindex );
		if ( !$res )
			return 1;

		if ( $view_results )
			sphTextReport ( sphCompare ( array ( "$res.bin", "$res.bin" ) ) );

		if ( $unlink_run )
		{
			printf ( "\nunlinked %s\n", $res );
			unlink ( "$res.bin" );
			unlink ( "$res.searchd.txt" );
		}
	}
	else if ( $mode=='compare' )
	{
		switch ( count($files) )
		{
			case 1:
				// pick two freshest by name
				$runs = PickFreshest ( $files[0], 2 );
				if ( count($runs)!=2 )
					Fatal ( "not enough run files for '$files[0]' (needed 2, got ".count($runs).")" );
				break; 

			case 2:
				// explicit run names given
				$runs = array ( LookupRun ( $files[0] ), LookupRun ( $files[1] ) );
				break;

			case 3:
				// explicit run names, shortcut syntax
				$runs = array (
					LookupRun ( "$files[0].$files[1]" ),
					LookupRun ( "$files[0].$files[2]" ) );
				break;

			default:
				Fatal ( "invalid compare syntax (expected 1 to 3 args, got ".count($files).")" );
		}
		sphTextReport ( sphCompare ( $runs ) );
	}
	else if ( $mode=='view' )
	{
		if ( count($files)!=1 )
		{
			Fatal ( "view command requires exactly one argument" );
		}

		$run = LookupRun ( $files[0] );
		sphTextReport ( sphCompare ( array ( $run, $run ) ) );
	}
	else
	{
		BenchPrintHelp ( $argv[0] );
	}
}

Main ( $argv );
exit ( 0 );
