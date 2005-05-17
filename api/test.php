<?php

//
// $Id$
//

//
// Copyright (c) 2001-2005, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

///////////////////////////////////////////////////////////////////////////////

$searchd_host	= "127.0.0.1";
$searchd_port	= 7812;

///////////////////////////////////////////////////////////////////////////////

require ( "sphinxapi.php" );

// for very old PHP versions, like at my home test server
if ( is_array($argv) && !isset($_SERVER["argv"]) )
	$_SERVER["argv"] = $argv;
unset ( $_SERVER["argv"][0] );

// build query
if ( !is_array($_SERVER["argv"]) || empty($_SERVER["argv"]) )
	die ( "usage: php -f test.php [--any] <word [word [word [...]]]>\n" );

$args = array();
foreach ( $_SERVER["argv"] as $arg )
	$args[] = $arg;

$q = "";
$any = false;
$groups = array();
for ( $i=0; $i<count($args); $i++ )
{
	if ( $args[$i]=="--any" )
	{
		$any = true;
	} else if ( $args[$i]=="--group" )
	{
		$groups[] = (int)$args[++$i];
	} else
	{
		$q .= $args[$i] . " ";
	}
}

// do query
$res = sphinxQuery ( $searchd_host, $searchd_port, $q, 0, 20, array(100,1), $any, $groups );
if ( !$res )
{
	print "Query failed (searchd at $searchd_host:$searchd_port).\n";

} else
{
	print "Query '$q' produced $res[total] matches in $res[time] sec.\n";
	print "Query stats:\n";
	if ( is_array($res["words"]) )
		foreach ( $res["words"] as $word => $info )
			print "    '$word' found $info[hits] times in $info[docs] documents\n";
	print "\n";

	if ( is_array($res["matches"]) )
	{
		$n = 1;
		print "Matches:\n";
		foreach ( $res["matches"] as $doc => $docinfo )
		{
			print "$n. doc_id=$doc, group=$docinfo[group], weight=$docinfo[weight]\n";
			$n++;
		}
	}
}

//
// $Id$
//

?>