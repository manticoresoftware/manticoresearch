<?php

// sql_query = SELECT id, group_id, UNIX_TIMESTAMP(date_added) date_added, title, content FROM documents
// sql_attr_uint = group_id
// sql_attr_timestamp = date_added

mysql_connect ( "localhost", "root", "" ) or die ( "connect" );
mysql_select_db ( "test" ) or die ( "select_db" );

mysql_query ( "DELETE FROM documents" ) or die ( "delete: ".mysql_error() );
for ( $i=1; $i<200; $i++ )
	mysql_query ( "INSERT INTO documents ( id, group_id, group_id2, date_added, title, content )
		VALUES ( $i, $i, $i, NOW(), 'test', 'test$i' )" ) or die ( "insert: ".mysql_error() );

print "ok";