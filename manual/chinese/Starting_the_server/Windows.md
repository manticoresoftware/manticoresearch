# 在 Windows 上启动 Manticore

在 Windows 上，如果您希望 Manticore 在启动时自动运行，可以将其安装为 Windows 服务。您可以按照 [Manticore 作为 Windows 服务](../Installation/Windows.md) 指南中的说明，将 Manticore 安装为服务。

一旦 Manticore 被安装为服务，您可以通过控制面板或者使用 sc.exe 命令行工具启动和停止它。

```shell
sc.exe start Manticore
```

```shell
sc.exe stop Manticore
```
或者，如果您没有将 Manticore 安装为 Windows 服务，也可以通过在命令行运行以下命令来启动它：

```shell
.\bin\searchd -c manticore.conf
```
该命令假设您在当前目录中拥有 Manticore 的可执行文件和配置文件。
<!-- proofread -->

