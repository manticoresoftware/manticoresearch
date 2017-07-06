#!/bin/sh
scriptdir=`pwd`
gitdir="$scriptdir/../.git"

if [ -d $gitdir ]; then
	if git log -1 --format='#define SPH_GIT_COMMIT_ID "%h"' > sphinxversion.h.tmp; then
		OLD=`cat sphinxversion.h 2>/dev/null`
		NEW=`cat sphinxversion.h.tmp 2>/dev/null`
		if [ "x$OLD" != "x$NEW" ]; then
			echo "NOTE: just updated git commit ID in src/sphinxversion.h"
			mv -f sphinxversion.h.tmp sphinxversion.h
		else
			rm -f sphinxversion.h.tmp
		fi
	else
		echo "ERROR: .git/ folder found but failed to invoke git; please fix"
		exit 1
	fi
else
	if [ ! -f $scriptdir/sphinxversion.h ]; then
		echo "WARNING: neither .git/ nor src/sphinxversion.h found; generating a dummy version id"
		echo "#define SPH_GIT_COMMIT_ID \"???\"" > sphinxversion.h
	fi
fi
