# Installing Manticore in Windows

1. Download the [Manticore Search Installer](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.0-230206-8de9df201-x64.exe) and run it. Follow the installation instructions.
2. Choose the directory to install to.
3. Select the components you want to install. We recommend installing all of them.
4. Manticore comes with a preconfigured `manticore.conf` file in [RT mode](../Read_this_first.md#Real-time-mode-vs-plain-mode). No additional configuration is required. However, the configuration file has several hardcoded paths, such as `log = C:/manticore/var/log/manticore/searchd.log`, that point to folders inside `C:\manticore`. If you install Manticore to a different folder, Manticore will automatically correct these paths on startup, so there's no need to modify them manually.

## Installing as a Windows service

To install the `searchd` (Manticore Search server) as a Windows service, run the following command:

```bat
\path\to\searchd.exe --install --config \path\to\config --servicename Manticore
```

Make sure to use the full path to the configuration file, or `searchd.exe` won't be able to locate it when it starts as a service.

After installation, you can start the service from the Microsoft Management Console's Services snap-in.

Once started, you can access Manticore using the MySQL command-line interface:

```bat
mysql -P9306 -h127.0.0.1
```

Note that in most examples in this manual, we use `-h0` to connect to the local host, but in Windows, you must use `localhost` or `127.0.0.1` explicitly.

<!-- proofread -->
