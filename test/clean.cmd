@echo off

if exist "ubertest.php" (
	for /D %%i in (test_*) do (
		if exist "%%i\test.xml" (
			del /q "%%i\report.txt" 2>nul
			rmdir /s /q "%%i\Conf" 2>nul
		)
	)

	del /q "data\*.sp*" 2>nul
	del /q "data\*.mvp" 2>nul
	del /q "data\*.meta" "data\*.lock" "data\*.kill" "data\*.ram" "data\binlog.*" "binlog.*" 2>nul
	del /q "*.log" 2>nul

	del /q "error*.txt" 2>nul
	del /q "config*.conf" 2>nul

	del /q "*.mdmp" 2>nul
)
