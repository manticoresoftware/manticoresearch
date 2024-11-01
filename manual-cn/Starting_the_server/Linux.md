# 在 Linux 中启动 Manticore

### 使用 systemd 启动和停止

安装完成后，Manticore Search 服务不会自动启动。要启动 Manticore，运行以下命令：

```shell
sudo systemctl start manticore
```

要停止 Manticore，运行以下命令：

```shell
sudo systemctl stop manticore
```

Manticore 服务被设置为在启动时运行。您可以通过运行以下命令来检查：

```shell
sudo systemctl is-enabled manticore
```

如果您希望禁用 Manticore 在启动时自动启动，请运行：

```bash
sudo systemctl disable manticore
```

要让 Manticore 在启动时自动启动，运行以下命令：

```bash
sudo systemctl enable manticore
```

`searchd` 进程在 `systemd` 日志中记录启动信息。如果启用了 `systemd` 日志记录，您可以通过以下命令查看记录的信息：

```shell
sudo journalctl -u manticore
```

### 使用 systemd 自定义启动标志

`systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS` 允许您指定 Manticore Search 守护进程启动时的自定义启动标志。查看完整列表 [这里](../Starting_the_server/Manually.md#searchd-command-line-options)。

例如，要以调试日志级别启动 Manticore，可以运行：

```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--logdebug'
systemctl restart manticore
```

要撤销此操作，运行：
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS=''
systemctl restart manticore
```

请注意，systemd 的环境变量会在服务器重启时重置。

### 使用 service 启动和停止

Manticore 可以通过 `service` 命令启动和停止：

```shell
sudo service manticore start
sudo service manticore stop
```

要在 RedHat 系统上启用 sysV 服务在启动时运行，运行：

```shell
chkconfig manticore on
```

要在 Debian 系统（包括 Ubuntu）上启用 sysV 服务在启动时运行，运行：

```shell
update-rc.d manticore defaults
```

请注意，`searchd` 是由 init 系统以 `manticore` 用户身份启动的，服务器创建的所有文件都将归该用户所有。如果 `searchd` 以 root 用户身份启动，文件权限将会改变，可能会导致再次以服务方式运行 `searchd` 时出现问题。

<!-- proofread -->
