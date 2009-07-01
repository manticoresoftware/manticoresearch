<?php

mysql_connect ( "localhost", "root", "" ) or die ( "mysql_connect" );
mysql_select_db ( "test" ) or die ( "mysql_select_db");

mysql_query ( "DROP TABLE IF EXISTS fullscan" ) or die ( mysql_error() );
mysql_query ( "CREATE TABLE fullscan
(
	id		INTEGER PRIMARY KEY NOT NULL,
	value	INTEGER NOT NULL,
	text	VARCHAR(255) NOT NULL
)" ) or die ( mysql_error() );

srand ( 0 );
for ( $i=0; $i<100000; $i++ )
{
	for ( $j=0; $j<10; $j++ )
		$r[$j] = rand() % 20;

	if ( $i%100==0 )
		$r[0] = 20;

	$id = $i*10+1;
	mysql_query ( "INSERT INTO fullscan VALUES
		( $id, $r[0], 'dummy' ),
		( $id+1, $r[1], 'dummy' ),
		( $id+2, $r[2], 'dummy' ),
		( $id+3, $r[3], 'dummy' ),
		( $id+4, $r[4], 'dummy' ),
		( $id+5, $r[5], 'dummy' ),
		( $id+6, $r[6], 'dummy' ),
		( $id+7, $r[7], 'dummy' ),
		( $id+8, $r[8], 'dummy' ),
		( $id+9, $r[9], 'dummy' ) " );
}
