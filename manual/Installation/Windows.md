# Installing Manticore on Windows

Manticore can be installed on Windows in several ways. We recommend using WSL (Windows Subsystem for Linux) because it supports replication and does not require Docker. Below are the recommended and alternative methods.

### Installing or enabling WSL2

To install Manticore Search on Windows, you'll first need to enable Windows Subsystem for Linux. WSL2 lets you run Linux binaries natively on Windows. For this method to work, you'll need to be running Windows 10 version 2004 and higher or Windows 11.

Follow the [official Microsoft guide](https://docs.microsoft.com/en-us/windows/wsl/install) for step-by-step instructions to install WSL2.

### Installing Manticore

To install Manticore on Windows via WSL2, refer to the [Debian and Ubuntu Installation](../../Installation/Debian_and_Ubuntu.md) section.

> NOTE: Installing Manticore via WSL2 is the recommended method as it offers better compatibility compared to using native Windows packages.

## Installing Manticore as native Windows binaries

Alternatively, you can install Manticore as native Windows binaries, which require Docker for [Manticore Buddy](../../Installation/Manticore_Buddy.md#Manticore-Buddy), by following these steps:

1. Install Docker Desktop and start it.
2. Download the [Manticore Search Installer](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-7.4.6-25022808-b2ff82920-x64.exe) and run it. Follow the installation instructions.
3. Choose the directory to install to.
4. Select the components you want to install. We recommend installing all of them.
5. Manticore comes with a preconfigured `manticore.conf` file in [RT mode](../Read_this_first.md#Real-time-mode-vs-plain-mode). No additional configuration is required.

> NOTE: The native Windows binaries do not support [replication](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication).

### Installing as a Windows service

To install Manticore Search server as a Windows service, run:

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
