# Installing Manticore in Windows

1. Download the [Manticore Search Installer](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.4-230314-1a3a4ea82-x64.exe) and run it. Follow the installation instructions.
2. Choose the directory to install to.
3. Select the components you want to install. We recommend installing all of them.
4. Manticore comes with a preconfigured `manticore.conf` file in [RT mode](../Read_this_first.md#Real-time-mode-vs-plain-mode). No additional configuration is required.

## Installing as a Windows service

To install `searchd` (Manticore Search server) as a Windows service, run:

```bat
\path\to\searchd.exe --install --config \path\to\config --servicename Manticore
```

Make sure to use the full path of the configuration file, otherwise `searchd.exe`  will not be able to locate it when it starts as a service.

After installation, the service can be started from the Services snap-in of the Microsoft Management Console.

Once started, you can access Manticore using the MySQL command line interface:

```bat
mysql -P9306 -h127.0.0.1
```

Note that in most examples in this manual, we use `-h0` to connect to the local host, but in Windows, you must use `localhost` or `127.0.0.1` explicitly.

<!-- proofread -->
