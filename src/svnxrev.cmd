@echo off
setlocal enabledelayedexpansion

set REVFILE=%1\src\sphinxversion.h
if exist %1\.svn (
	svn info --xml %1 | perl %1\src\svnxrev.pl %REVFILE%
) else if exist %1\.hg (
	if exist %REVFILE% for /f "delims=" %%a in (%REVFILE%) do set oldfile=%%a
	for /f %%i in ('hg id') do set newfile=#define SPH_SVN_TAGREV "%%i"
	if "!newfile!" neq "!oldfile!" echo !newfile!> %REVFILE%
) else if exist %1\.git (
	perl %1\src\gitxrev.pl %REVFILE%
)
