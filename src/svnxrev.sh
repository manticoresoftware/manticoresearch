#!/bin/sh
## As standalone run from this directory as "./svnxrev.sh .."
if [ -d $1/.svn ] ; then 
    svn info --xml $1 | perl $1/src/svnxrev.pl $1/src/sphinxversion.h
elif [ -d $1/.hg ] ; then
    ddr=`pwd`
    cd $1
	target="src/sphinxversion.h"
	startrev=`hg id -n | sed 's/\\+//'`
	rev="$startrev"
	svnrev=`hg log -r$rev --template "{desc}" | grep ^\\\\[svn | sed 's/\\[svn r//; s/\\].*//'`
#	while [ "z" = "z$svnrev" ] ; do
#		revplus=`hg log -r$rev --template "{rev}"`
#		rev=`expr $revplus - 1`
#		svnrev=`hg log -r$rev --template "{desc}" | grep ^\\\\[svn | sed 's/\\[svn r//; s/\\].*//'`
#	done
	rsvnrev="r$svnrev"
	node=`hg log -r$startrev --template "{node}"`
	branch=`hg log -r$startrev --template "{branches}"`
	[ "z$branch" = "ztrunk" ] && branch=""
	[ ! "z$startrev" = "z$rev" ]  && rsvnrev=r"$svnrev/$node"

	printf "#define SPH_SVN_TAG \"%s\"\n" $branch > $target.tmp
	printf "#define SPH_SVN_REV %s\n" $svnrev >> $target.tmp
	printf "#define SPH_SVN_REVSTR \"%s\"\n" $svnrev >> $target.tmp
	printf "#define SPH_SVN_TAGREV \"%s\"\n" $rsvnrev >> $target.tmp

	# dont touch it ALL the time
	if [ -f $target ]; then
		old=`cat $target`
		new=`cat $target.tmp`
		if [ "z$old" != "z$new" ] ; then
			mv -f $target.tmp $target
		else
			rm -f $target.tmp
		fi
	else
		mv -f $target.tmp $target
	fi
    cd "$ddr"
    exit 0
fi

if [ ! -f ./sphinxversion.h ] ; then
    cat *.cpp *.h | perl srcxrev.pl > sphinxversion.h
fi
