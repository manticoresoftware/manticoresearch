# Installing Manticore in Windows

To install Manticore in Windows you need to download zip archive and unpack it into a folder.

1. Download [Manticore bundle](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-5.0.2-220530-348514c86-main.zip) and [Manticore Columnar Library](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-columnar-lib-1.15.4-220522-2fef34e-x64.zip)
2. Create a folder, e.g. `C:\Manticore`
3. Move the archives into it and extract files from them:
   * `unzip manticore-5.0.2-220530-348514c86-main.zip`
   * `unzip manticore-columnar-lib-1.15.4-220522-2fef34e-x64.zip`

The archive comes with a sample configuration file in `manticore.conf.in`.
The configuration contains placeholders `@LOGDIR@`, `@RUNDIR@` and `@CONFDIR@` that need to be replaced with your:
* logs dir path
* pid files directory path
* and data directory location

respectively. The archive also includes empty folders `var/log` and `var/data`, so if you like - you can use them.

System variable `FULL_SHARE_DIR` should be also set to the full path of the `share` folder. That allows Manticore to work with installed `ICU`, stop words and columnar and secondary libraries from the `share` folder.

To install the `searchd` (Manticore search server) as a Windows service run:

```bat
C:\Manticore\bin\searchd --install --config C:\Manticore\manticore.conf.in --servicename Manticore
```

Make sure to use the full path of the configuration file, otherwise searchd.exe will not be able to know its location when it's started as service.

After installation the service can be started from the Services snap-in of the Microsoft Management Console.

Once started you can access Manticore using the mysql command line interface:

```bat
mysql -P9306 -h127.0.0.1
```

Note that in most examples in this manual we use `-h0`, but in Windows you need to use `localhost` or `127.0.0.1` for the local host.
