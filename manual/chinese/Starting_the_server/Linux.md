# 在Linux中启动Manticore

### 使用systemd启动和停止

安装后，Manticore搜索服务不会自动启动。要启动Manticore，请运行以下命令：

```shell
sudo systemctl start manticore
```

要停止Manticore，请运行以下命令：

```shell
sudo systemctl stop manticore
```

Manticore服务被设置为在启动时运行。您可以通过运行以下命令来检查：

```shell
sudo systemctl is-enabled manticore
```

如果您想禁用Manticore在启动时运行，请运行：

```bash
sudo systemctl disable manticore
```

要使Manticore在启动时运行，请运行：

```bash
sudo systemctl enable manticore
```

`searchd`进程在`systemd`日志中记录启动信息。如果启用了`systemd`日志记录，您可以使用以下命令查看记录的信息：

```shell
sudo journalctl -u manticore
```

### 使用systemd自定义启动标志

`systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS` 允许您指定Manticore搜索守护进程应该使用的自定义启动标志。完整列表请参见 [此处](../Starting_the_server/Manually.md#searchd-command-line-options)。

例如，要以调试日志级别启动Manticore，您可以运行：
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--logdebug'
systemctl restart manticore
```

要撤销此操作，请运行：
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS=''
systemctl restart manticore
```

注意，systemd环境变量在服务器重启时会重置。

### 使用服务启动和停止

Manticore可以通过服务命令启动和停止：

```shell
sudo service manticore start
sudo service manticore stop
```

要在RedHat系统上启用sysV服务启动，请运行：

```shell
chkconfig manticore on
```

要在Debian系统（包括Ubuntu）上启用sysV服务启动，请运行：

```shell
update-rc.d manticore defaults
```

请注意，`searchd`是由`manticore`用户下的初始化系统启动的，服务器创建的所有文件将由此用户拥有。如果`searchd`是在例如root用户下启动的，则文件权限将被更改，这可能会导致再次以服务身份运行`searchd`时出现问题。

<!-- proofread -->
