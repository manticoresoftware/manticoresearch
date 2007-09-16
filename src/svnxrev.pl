#!/usr/bin/perl

# svnxrev, an utility to extract SVN working copy revision information
# from svn info --xml output
#
# svnversion is nice, but lacks options to extract tag/branch name,
# which is something i would really like to mention in builds

$UTILITY = "svnxrev";				# that's my name
$PROJECT = "sphinx";				# that's expected project name
$PREFIX = "SPH";					# that's the prefix for defines
$OUTPUT = "sphinxversion.h";		# that's where i will write the result

undef $/;
$info = <>;

die ( "$UTILITY: failed to extract repository url" ) if (!( $info =~ /<url>(.*?)<\/url>/ ));
$url = $1;

die ( "$UTILITY: failed to extract commit revision" ) if (!( $info =~ /<commit\s+.*?revision=\"(\d+)\"/ ));
$rev = $1;

die ( "$UTILITY: unexpected project name (project=$PROJECT, url=$url)" ) if (!( $url =~ s/svn:\/\/.*?\/$PROJECT\/// ));
die ( "$UTILITY: unexpected project directory layout (url=$url)" ) if (!( $url =~ /^(?:trunk|tags\/(.*)|branches\/(.*))$/ ));
$tag = $1.$2;

$tagrev = length($tag)
	? "$tag-r$rev"
	: "r$rev";

open ( FH, ">$OUTPUT" ) or die ( "$UTILITY: failed to write output file (name=$OUTPUT)" );
print FH "#define ${PREFIX}_SVN_TAG \"$tag\"\n";
print FH "#define ${PREFIX}_SVN_REV $rev\n";
print FH "#define ${PREFIX}_SVN_TAGREV \"$tagrev\"\n";
close ( FH );

print "$UTILITY: extracted build tag: $tagrev\n";
