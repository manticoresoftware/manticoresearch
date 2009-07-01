<?php

ini_set ( "display_errors", 1 );
ini_set ( "error_reporting", E_ALL | E_STRICT );

assert_options ( ASSERT_ACTIVE, 1 );
assert_options ( ASSERT_BAIL, 1 );

require_once ( "settings.inc" );

////////////////////////////////////////////////////////////////////////////////

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
	{
		printf ( "unable to produce the report, result set sizes mismatch\n" );
		exit ( 1 );
	}

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
	{
		printf ( "unable to produce the report, tag sets mismatch\n" );
		exit ( 1 );
	}

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
	return "$output.bin";
}

////////////////////////////////////////////////////////////////////////////////
 
function sphFindBenchmarkResults ( $files )
{
	$results = array();
	foreach ( $files as $file )
	{
		$found = null;
		$variants = array (
			"$file",
			"$file.bin",
			"bench-results/$file",
			"bench-results/$file.bin"
		);
		foreach ( $variants as $variant )
			if ( is_readable($variant) )
			{
				$found = $variant;
				break;
			}
		if ( !$found )
		{
			printf ( "%s: file not found\n", $file );
			return null;
		}
		$results[] = $found;
	}
	return $results;
}

function BenchPrintHelp ( $path )
{
	print
		"Usage: php $path [locals] benchmark <name>\n" .
		"       php $path compare <files>\n";
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

				default:
					printf ( "FATAL: unknown mode '$opt'\n\n" );
					BenchPrintHelp ( $argv[0] );
					return 1;
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

			printf ( "benchmark mode requires a test case name$avail.\n" );
			return 1;
		}
		if ( count($files)!=1 )
		{
			printf ( "benchmark mode requires just one test case name\n" );
			return 1;
		}
		$path = "bench/$files[0].xml";
		if ( !is_readable($path) )
		{
			printf ( "%s: is not readable\n", $path );
			return 1;
		}

		$res = sphBenchmark ( $files[0], $g_locals, $force_reindex );
		if ( !$res )
			return 1;

		if ( $view_results )
			sphTextReport ( sphCompare ( array ( $res, $res ) ) );

		return 0;
	}
	else if ( $mode=='compare' )
	{
		if ( count($files)!=2 )
		{
			printf ( "compare mode requires two files\n" );
			return 1;
		}
		$results = sphFindBenchmarkResults ( $files );
		if ( !$results ) return 1;
		sphTextReport ( sphCompare ( array ( $results[0], $results[1] ) ) );
	}
	else if ( $mode=='view' )
	{
		if ( count($files)!=1 )
		{
			printf ( "view mode requires a single results file\n" );
			return 1;
		}
		$results = sphFindBenchmarkResults ( $files );
		if ( !$results ) return 1;
		sphTextReport ( sphCompare ( array ( $results[0], $results[0] ) ) );
	}
	else
	{
		BenchPrintHelp ( $argv[0] );
		return 0;
	}
}

exit ( Main($argv) );

?>