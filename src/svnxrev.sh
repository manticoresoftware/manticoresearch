#!/bin/bash

if [ -e $1/.svn ] ; then 
    svn info --xml $1 | perl $1/src/svnxrev.pl $1/src/sphinxversion.h
fi

