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

Manticore 服务设置为开机启动。你可以通过运行以下命令来检查：

```shell
sudo systemctl is-enabled manticore
```

如果你想禁止 Manticore 在开机时启动，运行：

```bash
sudo systemctl disable manticore
```

要使 Manticore 在开机时启动，运行：

```bash
sudo systemctl enable manticore
```

`searchd` 进程将启动信息记录在 `systemd` 日志中。如果启用了 `systemd` 日志记录，你可以使用以下命令查看日志信息：

```shell
sudo journalctl -u manticore
```

### 使用 systemd 自定义启动参数

`systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS` 允许你指定自定义启动参数，用于启动 Manticore Search 守护进程。完整列表请参见 [这里](../Starting_the_server/Manually.md#searchd-command-line-options)。

例如，要以调试日志级别启动 Manticore，你可以运行：
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--logdebug'
systemctl restart manticore
```

要撤销该设置，运行：
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS=''
systemctl restart manticore
```

注意，systemd 环境变量在服务器重启时会被重置。

### 使用 service 启动和停止

Manticore 可以使用 service 命令来启动和停止：

```shell
sudo service manticore start
sudo service manticore stop
```

要在 RedHat 系统上启用 sysV 服务开机自启动，运行：

```shell
chkconfig manticore on
```

要在 Debian 系统（包括 Ubuntu）上启用 sysV 服务开机自启动，运行：

```shell
update-rc.d manticore defaults
```

请注意，`searchd` 由 init 系统在 `manticore` 用户下启动，服务器创建的所有文件都归该用户所有。如果 `searchd` 由例如 root 用户启动，文件权限会被更改，这可能导致再次以服务方式运行 `searchd` 时出现问题。

<!-- proofread -->

