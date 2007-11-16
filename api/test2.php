<?php

//
// $Id$
//

require ( "sphinxapi.php" );

$docs = array
(
	"this is my test text to be highlighted, and for the sake of the testing we need to pump its length somewhat",
	"another test text to be highlighted, below limit",
	"test number three, without phrase match",
	"final test, not only without phrase match, but also above limit and with swapped phrase text test as well",
);
$words = "test text";
$index = "test1";
$opts = array
(
	"before_match"		=> "<b>",
	"after_match"		=> "</b>",
	"chunk_separator"	=> " ... ",
	"limit"				=> 60,
	"around"			=> 3,
);

foreach ( array(0,1) as $exact )
{
	$opts["exact_phrase"] = $exact;
	print "exact_phrase=$exact\n";

	$cl = new SphinxClient ();
	$res = $cl->BuildExcerpts ( $docs, $index, $words, $opts );
	if ( !$res )
	{
		die ( "ERROR: " . $cl->GetLastError() . ".\n" );
	} else
	{
		$n = 0;
		foreach ( $res as $entry )
		{
			$n++;
			print "n=$n, res=$entry\n";
		}
		print "\n";
	}
}

//
// $Id$
//

?>