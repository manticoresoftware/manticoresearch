<?php

/////////////////////////////////////////////////////////////////////////////
// SPHINX "API"
/////////////////////////////////////////////////////////////////////////////

$sphinx_server = "127.0.0.1";
$sphinx_port   = 3312;

function sphinxQuery ( $server, $port, $query, $start=0, $rpp=20, $weights=array() )
{
	$start = (int)$start;
	$rpp = (int)$rpp;

	if ( !($fp = fsockopen($server, $port)) )
		return false;

	// build weights request
	$wreq = pack ( "V", count($weights) );
	foreach ( $weights as $weight )
		$wreq .= pack ( "V", (int)$weight );

	// do query
	fputs ( $fp, pack ( "VVV", $start, $rpp, strlen($query) ) . $query . $wreq );

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

unset ( $argv[0] );
$q = "";
foreach ( $argv as $arg )
	$q .= "$arg ";

$res = sphinxQuery ( $sphinx_server, $sphinx_port, $q, 0, 20, array(100,1) );
print "Query '$q' produced $res[total] matches in $res[time] sec.\n";
print "Query stats:\n";
foreach ( $res["words"] as $word => $info )
	print "    '$word' found $info[hits] times in $info[docs] documents\n";
print "\n";

if ( isset($res["matches"]) )
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