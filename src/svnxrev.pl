#!/usr/bin/perl

# svnxrev, an utility to extract SVN working copy revision information
# from svn info --xml output
#
# svnversion is nice, but lacks options to extract tag/branch name,
# which is something i would really like to mention in builds
#
# usage: svn info --xml WORKING-COPY-ROOT | perl svnxrev.pl [OUTPUT-HEADER-NAME]

$UTILITY = "svnxrev";				# that's my name
$PROJECT = "sphinx";				# that's expected project name
$PREFIX = "SPH";					# that's the prefix for defines
$OUTPUT = "sphinxversion.h";		# that's where i will write the result

$OUTPUT = $ARGV[0] if ( $#ARGV==0 );

undef $/;
$info = <STDIN>;

die ( "$UTILITY: failed to extract repository url" ) if (!( $info =~ /<url>(.*?)<\/url>/ ));
$url = $1;

die ( "$UTILITY: failed to extract commit revision" ) if (!( $info =~ /<commit\s+.*?revision=\"(\d+)\"/ ));
$rev = $1;

exit ( 0 ) if (!( $url =~ s/svn:\/\/.*?\/$PROJECT\/// )); # for cases when we're imported to 3rd party SVN repo
die ( "$UTILITY: unexpected project directory layout (url=$url)" ) if (!( $url =~ /^(?:trunk|tags\/(.*)|branches\/(.*))$/ ));
$tag = $1.$2;

$tagrev = length($tag)
	? "$tag-r$rev"
	: "r$rev";

$result = <<EOT;
#define ${PREFIX}_SVN_TAG "$tag"
#define ${PREFIX}_SVN_REV $rev
#define ${PREFIX}_SVN_TAGREV "$tagrev"
EOT

if ( open FH, "<$OUTPUT" )
{
	$current = <FH>;
	close ( FH );
	if ( $current eq $result )
	{
		print "$UTILITY: build tag $tagrev unchanged\n";
		exit 0;
	}
}

open ( FH, "+>$OUTPUT" ) or die ( "$UTILITY: failed to write output file (name=$OUTPUT)" );
print FH $result;
close ( FH );

print "$UTILITY: extracted build tag: $tagrev\n";
