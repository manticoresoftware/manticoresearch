# Installing Manticore in Windows

You can install Manticore by either using the provided installer or by manually unpacking the zip archive.

## Using the installer

1. Download [Manticore Search Installer](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-5.0.2-220530-348514c86-main.zip). Run it and follow the installation instructions.
2. It's highly recommended to download [Manticore Columnar Library](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-columnar-lib-1.15.4-220522-2fef34e-x64.zip) to enable Columnar storage and secondary indexes. Unzip the archive and copy `lib_manticore_columnar.dll` and `lib_manticore_secondary.dll` to the folder where `searchd.exe` was installed (`c:\manticore\bin\` by default).
3. Manticore comes with `searchd.conf` preconfigured in the [RT mode](../Read_this_first.md#Real-time-mode-vs-plain-mode). No additional configuration is necessary. Note that the configuration file has several hardcoded paths like `log = c:/manticore/var/log/manticore/searchd.log` pointing to folders inside `c:\manticore`. When installed to a folder other than `c:/manticore`, Manticore corrects these paths automatically on startup, so it's not necessary to modify these paths manually.

## Installing from archive

To install Manticore in Windows you need to download zip archive and unpack it into a folder.

1. Download [Manticore bundle](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-5.0.2-220530-348514c86-main.zip) and [Manticore Columnar Library](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-columnar-lib-1.15.4-220522-2fef34e-x64.zip). Sometimes you may also need Windows redistributable runtime libraries.
2. Create a folder, e.g. `c:\Manticore`
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

## Installing as a Windows service

To install `searchd` (Manticore Search server) as a Windows service run:

```bat
\path\to\searchd --install --config \path\to\config --servicename Manticore
```

Make sure to use a full path of the configuration file, otherwise `searchd.exe` will not be able to know its location when it's started as a service.

After the installation the service can be started from the Services snap-in of the Microsoft Management Console.

Once started you can access Manticore using mysql command line interface:

```bat
mysql -P9306 -h127.0.0.1
```

Note that in most examples in this manual we use `-h0` to make a connection to the local host, but in Windows you need to use `localhost` or `127.0.0.1` explicitly.
