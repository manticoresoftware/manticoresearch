# 在 Windows 上启动 Manticore

在 Windows 上，如果您希望 Manticore 在启动时自动运行，可以将其安装为 Windows 服务。您可以按照 [Manticore 作为 Windows 服务](../Installation/Windows.md) 指南中的说明，将 Manticore 安装为服务。

一旦将 Manticore 安装为服务，您可以通过控制面板或使用 sc.exe 命令在命令行中启动和停止它。

```shell
sc.exe start Manticore
```

```shell
sc.exe stop Manticore
```
另外，如果您没有将 Manticore 安装为 Windows 服务，也可以通过运行以下命令从命令行启动它：

```shell
.\bin\searchd -c manticore.conf
```
此命令假设您已经将 Manticore 的二进制文件和配置文件放在当前目录中。
<!-- proofread -->

