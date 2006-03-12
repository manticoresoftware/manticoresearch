<?php

require ( "sphinxapi.php" );


$docs = array
(
	"this is my test text to be highlighted",
	"this is another test text to be highlighted",
);
$words = "test text";
$index = "test1";
$opts = array
(
	"before_match"		=> "<b>",
	"after_match"		=> "</b>",
	"chunk_separator"	=> " ... ",
	"limit"				=> 400,
	"around"			=> 15
);


$cl = new SphinxClient ();
$res = $cl->BuildExcerpts ( $docs, $index, $words );
if ( !$res )
{
	die ( "ERROR: " . $cl->GetLastError() . ".\n" );
} else
{
	foreach ( $res as $entry )
	{
		$n++;
		print "n=$n, res=$entry\n";
	}
}

?>