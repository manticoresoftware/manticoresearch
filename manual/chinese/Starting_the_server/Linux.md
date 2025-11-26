# 在 Linux 中启动 Manticore

### 使用 systemd 启动和停止

安装后，Manticore Search 服务不会自动启动。要启动 Manticore，请运行以下命令：

```shell
sudo systemctl start manticore
```

要停止 Manticore，请运行以下命令：

```shell
sudo systemctl stop manticore
```

Manticore 服务设置为开机启动。您可以通过运行以下命令检查：

```shell
sudo systemctl is-enabled manticore
```

如果您想禁用 Manticore 开机启动，请运行：

```bash
sudo systemctl disable manticore
```

要使 Manticore 开机启动，请运行：

```bash
sudo systemctl enable manticore
```

`searchd` 进程会将启动信息记录在 `systemd` 日志中。如果启用了 `systemd` 日志记录，您可以使用以下命令查看记录的信息：

```shell
sudo journalctl -u manticore
```

### 使用 systemd 自定义启动参数

`systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS` 允许您指定 Manticore Search 守护进程启动时应使用的自定义启动参数。完整列表请参见[这里](../Starting_the_server/Manually.md#searchd-command-line-options)。

例如，要以调试日志级别启动 Manticore，您可以运行：
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--logdebug'
systemctl restart manticore
```

要撤销该操作，运行：
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS=''
systemctl restart manticore
```

注意，systemd 环境变量在服务器重启时会被重置。

### 使用 service 启动和停止

可以使用 service 命令启动和停止 Manticore：

```shell
sudo service manticore start
sudo service manticore stop
```

在 RedHat 系统上启用 sysV 服务开机启动，请运行：

```shell
chkconfig manticore on
```

在 Debian 系统（包括 Ubuntu）上启用 sysV 服务开机启动，请运行：

```shell
update-rc.d manticore defaults
```

请注意，`searchd` 由 init 系统以 `manticore` 用户身份启动，服务器创建的所有文件都将归该用户所有。如果 `searchd` 以例如 root 用户身份启动，文件权限将被更改，这可能导致再次以服务方式运行 `searchd` 时出现问题。

<!-- proofread -->

