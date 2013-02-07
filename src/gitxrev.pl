#!/usr/bin/perl

$OUTPUT = "sphinxversion.h";
$OUTPUT = $ARGV[0] if ( $#ARGV==0 );

$out = `git log -1 --format="commit %h%nbody %b"`;
$gitid = $1 if ( $out =~ /^commit (\w+)/ );
$svnid = $1 if ( $out =~ /git-svn-id: .*?\@(\d+)/ );

$tagrev = $svnid ? "r$svnid" : $gitid;
$result = "#define SPH_SVN_TAGREV \"$tagrev\"";

if ( open FH, "<$OUTPUT" )
{
	$current = <FH>;
	close ( FH );
	if ( $current eq $result )
	{
		print "gitxrev: build tag $tagrev unchanged\n";
		exit 0;
	}
}

open ( FH, "+>$OUTPUT" ) or die ( "gitxrev: failed to write output file (name=$OUTPUT)" );
print FH $result;
close ( FH );

print "gitxrev: extracted build tag: $tagrev\n";
