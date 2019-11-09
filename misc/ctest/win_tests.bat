start /min cmd /c x://mysql/bin/mysqld.exe --defaults-file=x://mysql//my.ini
:loop
x://mysql/bin/mysql -u test test -e "show tables"
IF %ERRORLEVEL% == 0 GOTO :found
sleep 1
GOTO :loop

:found

cmd /c ctest -V -S misc/ctest/gltest.cmake -DCTEST_SOURCE_DIRECTORY='C:/cygwin64/home/manticore/builds/manticoresearch/dev/' -C Debug
set CTEST_RES=%ERRORLEVEL%

cmd /c x://mysql/bin/mysqladmin.exe --defaults-file=x://mysql//my.ini shutdown

EXIT %CTEST_RES%