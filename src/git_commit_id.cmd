@echo off
pushd %CD%
cd %~dp0

if exist %~dp0..\.git (
	git log -1 --format="#define SPH_GIT_COMMIT_ID \"%%h\"" > sphinxversion.h.tmp
	fc sphinxversion.h sphinxversion.h.tmp 1>nul 2>nul
	if errorlevel 1 goto update
	goto noupdate

:update
	del sphinxversion.h 1>nul 2>nul
	rename sphinxversion.h.tmp sphinxversion.h
	goto done

:noupdate
	del sphinxversion.h.tmp
	goto done

:done
	popd
) else (
	if not exist %~dp0\sphinxversion.h (
		echo WARNING: neither .git/ nor src/sphinxversion.h found; generating a dummy version id
		echo #define SPH_GIT_COMMIT_ID "???" > sphinxversion.h
	)
)

