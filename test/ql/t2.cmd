@echo off

start ..\..\bin\release\searchd.exe
C:\Windows\Microsoft.NET\Framework\v4.0.30319\csc.exe /nologo /r:"C:\Program Files\MySQL\MySQL Connector Net 6.8.3\Assemblies\v4.0\MySql.Data.dll" qltest.cs
qltest.exe
start ..\..\bin\release\searchd.exe --stop
