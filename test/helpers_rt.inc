<?php

ini_set ( "display_errors", 1 );
ini_set ( "error_reporting", (E_ALL | E_STRICT) & ~E_USER_DEPRECATED );

function escape ( $string )
{
	$from = array ( '\\', '(',')','|','-','!','@','~','"','&', '/', '^', '$', '=', '<', '\'' );
	$to   = array ( '\\\\', '\(','\)','\|','\-','\!','\@','\~','\"', '\&', '\/', '\^', '\$', '\=', '\<', '\\\'' );
	return str_replace ( $from, $to, $string );
}

function populate_rt ( $id_from, $id_to, $commit_step, $con_daemon, $index_name )
{
	$res = array( 'total'=>0, 'total_found'=>0, 'time'=>0, 'query'=>'', 'tag'=>'', 'error'=>'' );
	$con_sql = @mysql_connect ( "localhost", "root", "", true );
	if ( !$con_sql )
	{
		$res['error'] = 'failed to connect to MySQL server';
		return $res;
	}
	if ( !@mysql_select_db ( "lj", $con_sql ) )
	{
		$res['error'] = 'failed to select db';
		return $res;
	}

	if ( !@mysql_query ( 'SET names utf8', $con_sql ) )
	{
		$res['error'] = 'failed to connect to set utf8';
		return $res;
	}

	$r = @mysql_query ( "SELECT id, UNCOMPRESS(content) body, title, channel_id idd, ts FROM posting where id>=$id_from AND id<=$id_to;", $con_sql );
	if ( !$r )
	{
		$res['error']  = 'faled to fetch';
		return $res;
	}
			
	$data = array();
	while ( $row = @mysql_fetch_assoc ( $r ) )
	{
		$data[] = array ( 'id'=>$row['id'], 'title'=>$row['title'], 'body'=>$row['body'], 'idd'=>$row['idd'], 'ts'=>$row['ts'] );
	}

	$con_daemon->Query ( "SET autocommit=0" );
	$posted = 0;
	$start = MyMicrotime();		
	foreach  ( $data as $elem )
	{
		$id = $elem['id'];
		$title = escape ( $elem['title'] );
		$body = escape ( $elem['body'] );

		$q = "REPLACE INTO $index_name (id, title, body, idd, ts) VALUES ( $id, ' ";
		$q .= $title;			// title
		$q .= " ', ' ";
		$q .= $body;			// body
		$q .= " ', ";
		$q .= $elem['idd'];		// idd
		$q .= ", ";
		$q .= strtotime($elem['ts']);		// ts
		$q .= " ) ";

		$r = ( $con_daemon->Query ( $q ) );
		if ( stristr ( $r, "NOT CONNECTED" ) || stristr ( $r, "ERROR:" ) )
		{
			$res['error']  = $r . '; query: ' . $q;
			$res['total_found'] = $posted;
			return $res;
		}
		$posted++;
		
		if ( ( $posted % $commit_step )==0 )
			$con_daemon->Query ( 'COMMIT' );
	}
	$con_daemon->Query ( 'COMMIT' );
	$con_daemon->Query ( "SET autocommit=1" );
	
	$res['time'] = MyMicrotime() - $start;
	$res['total_found'] = $posted;
	return $res;
}

function delete_from_rt ( $ids, $con_daemon, $index_name )
{
	$res = array( 'total'=>0, 'total_found'=>0, 'time'=>0, 'query'=>'', 'tag'=>'', 'error'=>'' );
	
	$start = MyMicrotime();		
	foreach  ( $ids as $id )
	{
		$q = "DELETE FROM $index_name WHERE id=$id";
		$r = ( $con_daemon->Query ( $q ) );
		if ( stristr ( $r, "NOT CONNECTED" ) || stristr ( $r, "ERROR:" ) )
		{
			$res['error']  = $r . '; query: ' . $q;
			$res['total'] = $id;
			return $res;
		}
	}
	
	$res['time'] = MyMicrotime() - $start;
	$res['total_found'] = count($ids);
	return $res;
}