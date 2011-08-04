<?php

define ( "FREQ_THRESHOLD", 40 );
define ( "SUGGEST_DEBUG", 0 );

require ( "../../api/sphinxapi.php" );

/// build a list of trigrams for a given keywords
function BuildTrigrams ( $keyword )
{
	$t = "__" . $keyword . "__";

	$trigrams = "";
	for ( $i=0; $i<strlen($t)-2; $i++ )
		$trigrams .= substr ( $t, $i, 3 ) . " ";

	return $trigrams;
}


/// create SQL dump of the dictionary from Sphinx stopwords file
/// expects open files as parameters
function BuildDictionarySQL ( $out, $in )
{
	fwrite ( $out, "DROP TABLE IF EXISTS suggest;

CREATE TABLE suggest (
	id			INTEGER PRIMARY KEY AUTO_INCREMENT NOT NULL,
	keyword		VARCHAR(255) NOT NULL,
	trigrams	VARCHAR(255) NOT NULL,
	freq		INTEGER NOT NULL
);

" );

	$n = 0;
	$m = 0;
	while ( $line = fgets ( $in, 1024 ) )
	{
		list ( $keyword, $freq ) = split ( " ", trim ( $line ) );

		if ( $freq<FREQ_THRESHOLD || strstr ( $keyword, "_" )!==false || strstr ( $keyword, "'" )!==false )
			continue;

		$trigrams = BuildTrigrams ( $keyword );

		if ( !$m )
			print "INSERT INTO suggest VALUES\n";
		else
			print ",\n";

		$n++;
		fwrite ( $out, "( $n, '$keyword', '$trigrams', $freq )" );

		$m++;
		if ( ( $m % 10000 )==0 )
		{
			print ";\n";
			$m = 0;
		}
	}

	if ( $m )
		fwrite ( $out, ";" );
}


/// search for suggestions
function MakeSuggestion ( $keyword )
{
	$trigrams = BuildTrigrams ( $keyword );
	$query = "\"$trigrams\"/1";
	$len = strlen($keyword);

	$cl = new SphinxClient ();
	$cl->SetMatchMode ( SPH_MATCH_EXTENDED2 );
	$cl->SetRankingMode ( SPH_RANK_WORDCOUNT );
	$cl->SetFilterRange ( "len", $len-2, $len+2 );
	$cl->SetSelect ( "*, @weight+2-abs(len-$len) AS myrank" );
	$cl->SetSortMode ( SPH_SORT_EXTENDED, "myrank DESC, freq DESC" );
  	$cl->SetArrayResult ( true );

	$res = $cl->Query ( $query, "suggest" );

	if ( !$res || !$res["matches"] )
		return false;

	if ( SUGGEST_DEBUG )
	{
		print "--- DEBUG START ---\n";

		foreach ( $res["matches"]  as $match )
		{
			$r = mysql_query ( "SELECT keyword FROM suggest WHERE id=".$match["id"] ) or die ( "mysql_query() failed: ".mysql_error() );
			$row = mysql_fetch_row ( $r );
			$w = $row[0];

			$myrank = @$match["attrs"]["myrank"];
			if ( $myrank ) $myrank = ", myrank=$myrank";

			print "id=$match[id], weight=$match[weight], freq={$match[attrs][freq]}{$myrank}, word=$w\n";
		}

		print "--- DEBUG END ---\n";
	}

	$id = (int)$res["matches"][0]["id"];
	$r = mysql_query ( "SELECT keyword FROM suggest WHERE id=$id" ) or die ( "mysql_query() failed: ".mysql_error() );
	$row = mysql_fetch_row ( $r );
	return $row[0];
}

/// main
if ( $_SERVER["argc"]<2 )
{
	die ( "usage:\n"
		. "php suggest.php --builddict\treads stopwords from stdin, prints SQL dump of the dictionary to stdout\n"
		. "php suggest.php --query WORD\tqueries Sphinx, prints suggestion\n" );
}

if ( $_SERVER["argv"][1]=="--builddict" )
{
	$in = fopen ( "php://stdin", "r" );
	$out = fopen ( "php://stdout", "w+" );
	BuildDictionarySQL ( $out, $in );
}

if ( $_SERVER["argv"][1]=="--query" )
{
	mysql_connect ( "localhost", "root", "" ) or die ( "mysql_connect() failed: ".mysql_error() );
	mysql_select_db ( "test" ) or die ( "mysql_select_db() failed: ".mysql_error() );

	$keyword = $_SERVER["argv"][2];
	printf ( "keyword: %s\nsuggestion: %s\n", $keyword, MakeSuggestion($keyword) );
}
