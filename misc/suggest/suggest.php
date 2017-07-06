<?php

define ( "FREQ_THRESHOLD", 40 );
define ( "SUGGEST_DEBUG", 0 );
define ( "LENGTH_THRESHOLD", 2 );
define ( "LEVENSHTEIN_THRESHOLD", 2 );
define ( "TOP_COUNT", 10 );

// error_reporting ( E_ALL ^ E_NOTICE );
mb_internal_encoding ( "utf-8" );

require ( "../../api/sphinxapi.php" );

/// build a list of trigrams for a given keywords
function BuildTrigrams ( $keyword )
{
	$t = "__" . $keyword . "__";

	$trigrams = "";
	for ( $i=0; $i<mb_strlen($t)-2; $i++ )
		$trigrams .= mb_substr ( $t, $i, 3 ) . " ";

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
	freq		INTEGER NOT NULL,
	UNIQUE(keyword)
);

" );

	$n = 0;
	$m = 0;
	while ( $line = fgets ( $in, 1024 ) )
	{
		list ( $keyword, $freq ) = preg_split ( "/[\s,]+/", trim ( $line ) );

		if ( $freq<FREQ_THRESHOLD || strstr ( $keyword, "_" )!==false || strstr ( $keyword, "'" )!==false )
			continue;

		$trigrams = BuildTrigrams ( $keyword );

		if ( !$m )
			print "INSERT INTO suggest VALUES\n";
		else
			print ",\n";

		$n++;
		fwrite ( $out, "( 0, '$keyword', '$trigrams', $freq )" );

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

	$delta = LENGTH_THRESHOLD;
	$cl = new SphinxClient ();
	$cl->SetMatchMode ( SPH_MATCH_EXTENDED2 );
	$cl->SetRankingMode ( SPH_RANK_WORDCOUNT );
	$cl->SetFilterRange ( "len", $len-$delta, $len+$delta );
	$cl->SetSelect ( "*, @weight+$delta-abs(len-$len) AS myrank" );
	$cl->SetSortMode ( SPH_SORT_EXTENDED, "myrank DESC, freq DESC" );
  	$cl->SetArrayResult ( true );

  	// pull top-N best trigram matches and run them through Levenshtein
	$cl->SetLimits ( 0, TOP_COUNT );
	$res = $cl->Query ( $query, "suggest" );

	if ( !$res || !$res["matches"] )
		return false;

	if ( SUGGEST_DEBUG )
	{
		print "--- DEBUG START ---\n";

		foreach ( $res["matches"] as $match )
		{
			$w = $match["attrs"]["keyword"];
			$myrank = @$match["attrs"]["myrank"];
			if ( $myrank )
				$myrank = ", myrank=$myrank";

			// FIXME? add costs?
			// FIXME! does not work with UTF-8.. THIS! IS!! PHP!!!
			$levdist = levenshtein ( $keyword, $w );

			print "id=$match[id], weight=$match[weight], freq={$match[attrs][freq]}{$myrank}, word=$w, levdist=$levdist\n";
		}

		print "--- DEBUG END ---\n";
	}

	// further restrict trigram matches with a sane Levenshtein distance limit
	foreach ( $res["matches"] as $match )
	{
		$suggested = $match["attrs"]["keyword"];
		if ( levenshtein ( $keyword, $suggested )<=LEVENSHTEIN_THRESHOLD )
			return $suggested;
	}
	return $keyword;
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
