<?php

/////////////////////////////////////////////////////////////////////////////
// SPHINX "API"
/////////////////////////////////////////////////////////////////////////////

$sphinx_server = "127.0.0.1";
$sphinx_port   = 3312;

function sphinxQuery ( $server, $port, $query, $start=0, $rpp=20, $weights=array(), $any=false, $group=0 )
{
	$start = (int)$start;
	$rpp = (int)$rpp;

	if ( !($fp = fsockopen($server, $port)) )
		return false;

	// build request
	$mreq = pack ( "VVVV", $start, $rpp, $any ? 1 : 0, $group ); // mode/limits part
	$qreq = pack ( "V", strlen($query) ) . $query; // query string part
	$wreq = pack ( "V", count($weights) ); // weights part
	foreach ( $weights as $weight )
		$wreq .= pack ( "V", (int)$weight );

	// do query
	fputs ( $fp, $mreq . $qreq . $wreq );

	$result = array();
	while (!feof($fp)) {
		$s = trim(fgets($fp, 1024));
		if (substr($s, 0, 6) == "MATCH ") {
			list($dummy, $group, $doc, $weight) = explode(" ", $s);
			$result["matches"][$doc] = array ( "weight" => $weight, "group" => $group );
			continue;
		}
		if (substr($s, 0, 6) == "TOTAL ") {
			$result["total"] = substr($s, 6);
			continue;
		}
		if (substr($s, 0, 5) == "TIME ") {
			$result["time"] = substr($s, 5);
			continue;
		}
		if (substr($s, 0, 5) == "WORD ") {
			list($dummy, $word, $docs, $hits) = explode(" ", $s);
			$result["words"][$word]["docs"] = $docs;
			$result["words"][$word]["hits"] = $hits;
			continue;
		}
	}

	fclose($fp);
	return $result;
}

/////////////////////////////////////////////////////////////////////////////

// for very old PHP versions, like at my home test server
if ( is_array($argv) && !isset($_SERVER["argv"]) )
	$_SERVER["argv"] = $argv;
unset ( $_SERVER["argv"][0] );

// build query
if ( !is_array($_SERVER["argv"]) || empty($_SERVER["argv"]) )
	die ( "usage: php -f query.php [--any] <word [word [word [...]]]>\n" );

$args = array();
foreach ( $_SERVER["argv"] as $arg )
	$args[] = $arg;

$q = "";
$any = false;
$group = 0;
for ( $i=0; $i<count($args); $i++ )
{
	if ( $args[$i]=="--any" )
	{
		$any = true;
	} else if ( $args[$i]=="--group" )
	{
		$group = (int)$args[++$i];
	} else
	{
		$q .= $args[$i];
	}
}

// do query
$res = sphinxQuery ( $sphinx_server, $sphinx_port, $q, 0, 20, array(100,1), $any, $group );

// print results
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

?>