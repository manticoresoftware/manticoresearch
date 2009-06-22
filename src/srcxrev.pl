#!/usr/bin/perl

while (<>)
{
	next if ( !/\$Id: \S+ (\d+)/ );
	$maxrev = $1 if ( $1>$maxrev );
}
	
print <<EOT;
#define SPH_SVN_TAG ""
#define SPH_SVN_REV $maxrev
#define SPH_SVN_REVSTR "$maxrev"
#define SPH_SVN_TAGREV "r$maxrev"
EOT
