# Starting Manticore in Windows

On Windows if you want Manticore to start at boot, install [Manticore as Windows Service](Installation/Windows.md).

Manticore can be started and stopped from the Control Panel or from the command line:

```shell
sc.exe start Manticore
```

```shell
sc.exe stop Manticore
```

If you don't install Manticore as Windows service, you can start it from the command line:

```shell
.\bin\searchd -c manticore.conf
```
