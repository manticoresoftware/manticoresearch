# Starting Manticore in Windows

On Windows, if you want Manticore to start at boot, you can install it as a Windows Service. You can follow the instructions in the [Manticore as Windows Service](../Installation/Windows.md) guide to install Manticore as a service.

Once Manticore is installed as a service, you can start and stop it from the Control Panel or from the command line using the sc.exe command.

```shell
sc.exe start Manticore
```

```shell
sc.exe stop Manticore
```
Alternatively, if you don't install Manticore as a Windows service, you can start it from the command line by running the following command:

```shell
.\bin\searchd -c manticore.conf
```
This command assumes that you have the Manticore's binary and the configuration file in the current directory.
<!-- proofread -->