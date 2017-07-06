<?php

mysql_connect ( "localhost", "root", "" ) or die ( "mysql_connect" );
mysql_select_db ( "test" ) or die ( "mysql_select_db");

mysql_query ( "DROP TABLE IF EXISTS fullscan" ) or die ( mysql_error() );
mysql_query ( "CREATE TABLE fullscan
(
	id		INTEGER PRIMARY KEY NOT NULL,
	value	INTEGER NOT NULL,
	text	VARCHAR(255) NOT NULL,
	mva	VARCHAR(2048) NOT NULL,
	j		VARCHAR(255) NOT NULL,
	gid	INTEGER NOT NULL,
	tag	INTEGER NOT NULL,
	s1		VARCHAR(255) NOT NULL
)" ) or die ( mysql_error() );

srand ( 0 );
$jnames = array ( 'gid', 'tag', 's1', 's2' );
for ( $i=0; $i<100000; $i++ )
{
	for ( $j=0; $j<10; $j++ )
		$r[$j] = rand() % 20;

	if ( $i%100==0 )
		$r[0] = 20;

	$mva = array ();
	for ( $m=0; $m<10; $m++ )
	{
		$mva[$m] = '';
		for ( $j=0; $j<10; $j++ )
			$mva[$m] .= ( rand() % 100 ) . ',';
			
		$mva[$m] = rtrim ( $mva[$m], ',' );
	}
	

	$jplain = array ();
	$js = array ();
	for ( $m=0; $m<10; $m++ )
	{
		$jplain[$m] = array ( 'gid'=> 9999, 'tag'=> 9999, 's1'=>'' );
		$jrand = array_rand ( $jnames, 3 );
		$js[$m] = "'{";
		for ( $j=0; $j<count($jrand); $j++ )
		{
			$jval = rand() % 1000;
			$jname = $jnames [ $jrand [ $j ] ];
			$js[$m] .= " \"$jname\" :";
			if ( $jname[0]=='s' )
				$js[$m] .= " \"$jval\",";
			else
				$js[$m] .= " $jval,";
			$jplain[$m][$jname] = $jval;
		}
		// remove last ,
		$js[$m] = rtrim ( $js[$m], ',' );
		$js[$m] .= "}'";
	}
	

	$gid0 = $jplain[0]['gid']; $gid1 = $jplain[1]['gid']; $gid2 = $jplain[2]['gid']; $gid3 = $jplain[3]['gid']; $gid4 = $jplain[4]['gid'];
	$gid5 = $jplain[5]['gid']; $gid6 = $jplain[6]['gid']; $gid7 = $jplain[7]['gid']; $gid8 = $jplain[8]['gid']; $gid9 = $jplain[9]['gid'];
	$tag0 = $jplain[0]['tag']; $tag1 = $jplain[1]['tag']; $tag2 = $jplain[2]['tag']; $tag3 = $jplain[3]['tag']; $tag4 = $jplain[4]['tag'];
	$tag5 = $jplain[5]['tag']; $tag6 = $jplain[6]['tag']; $tag7 = $jplain[7]['tag']; $tag8 = $jplain[8]['tag']; $tag9 = $jplain[9]['tag'];
	$s10 = $jplain[0]['s1']; $s11 = $jplain[1]['s1']; $s12 = $jplain[2]['s1']; $s13 = $jplain[3]['s1']; $s14 = $jplain[4]['s1'];
	$s15 = $jplain[5]['s1']; $s16 = $jplain[6]['s1']; $s17 = $jplain[7]['s1']; $s18 = $jplain[8]['s1']; $s19 = $jplain[9]['s1'];
	$id = $i*10+1;
	
	$q = "( $id,     $r[0], 'dummy', '$mva[0]', $js[0], $gid0, $tag0, '$s10' ),
		( $id+1, $r[1], 'dummy', '$mva[1]', $js[1], $gid1, $tag1, '$s11' ),
		( $id+2, $r[2], 'dummy', '$mva[2]', $js[2], $gid2, $tag2, '$s12' ),
		( $id+3, $r[3], 'dummy', '$mva[3]', $js[3], $gid3, $tag3, '$s13' ),
		( $id+4, $r[4], 'dummy', '$mva[4]', $js[4], $gid4, $tag4, '$s14' ),
		( $id+5, $r[5], 'dummy', '$mva[5]', $js[5], $gid5, $tag5, '$s15' ),
		( $id+6, $r[6], 'dummy', '$mva[6]', $js[6], $gid6, $tag6, '$s16' ),
		( $id+7, $r[7], 'dummy', '$mva[7]', $js[7], $gid7, $tag7, '$s17' ),
		( $id+8, $r[8], 'dummy', '$mva[8]', $js[8], $gid8, $tag8, '$s18' ),
		( $id+9, $r[9], 'dummy', '$mva[9]', $js[9], $gid9, $tag9, '$s19' )";
	
	mysql_query ( "INSERT INTO fullscan VALUES $q" ) or die ( mysql_error() );
}
