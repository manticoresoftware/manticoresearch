# Installing Manticore in Windows

To install Manticore in Windows you need to download the zip archive and unpack it into a folder.

In the following example we'll assume we unpack the zip contents to folder `C:\Manticore`.

```bat
cd C:\Manticore
unzip manticore-3.6.0-210504-96d61d8bf-release-x64-main.zip
```

The zip comes with a sample configuration file in `manticore.conf.in`.
The configuration contains a `@CONFDIR@` string which needs to be replaced with your data directory location. It will be the root directory of `data` and `log` folders (first is used to store indexes, second for logs). The zip package also comes with these folders, so they will be available at the location where you unzipped the package. If you want to use a different location, the two folders must be created there.

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
