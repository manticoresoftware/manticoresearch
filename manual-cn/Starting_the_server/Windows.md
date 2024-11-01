# 在 Windows 上启动 Manticore

在 Windows 上，如果你希望 Manticore 在启动时自动运行，你可以将其安装为 Windows 服务。你可以按照 [Manticore 作为 Windows 服务](../Installation/Windows.md) 指南中的说明，将 Manticore 安装为服务。

一旦 Manticore 安装为服务后，你可以通过控制面板启动和停止服务，或者通过命令行使用 `sc.exe` 命令来启动和停止它。

```shell
sc.exe start Manticore
```

```shell
sc.exe stop Manticore
```
或者，如果你没有将 Manticore 安装为 Windows 服务，你可以通过运行以下命令从命令行启动它：

```shell
.\bin\searchd -c manticore.conf
```
此命令假设 Manticore 的二进制文件和配置文件位于当前目录中。
<!-- proofread -->