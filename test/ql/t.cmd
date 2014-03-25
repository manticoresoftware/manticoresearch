@echo off

start ..\..\bin\release\searchd.exe
call javac -cp .;mysql-connector-java-5.1.29\mysql-connector-java-5.1.29-bin.jar -d . qltest.java
call java -cp .;mysql-connector-java-5.1.29\mysql-connector-java-5.1.29-bin.jar qltest
start ..\..\bin\release\searchd.exe --stop
