<?php

// *** SPHINX API ***

$sphinx_server = "192.168.0.3";
$sphinx_port   = 3307;

function sphinxQuery($server, $port, $query, $start = 0, $rpp = 20)
{
	$start = (int)$start;
	$rpp = (int)$rpp;

	if (!($fp = fsockopen($server, $port))) return false;
	fputs($fp, pack("VVV", $start, $rpp, strlen($query)) . $query);

	$result = array();
	while (!feof($fp)) {
		$s = trim(fgets($fp, 1024));
		if (substr($s, 0, 6) == "MATCH ") {
			list($dummy, $doc, $weight) = explode(" ", $s);
			$result["matches"][$doc] = $weight;
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

?>