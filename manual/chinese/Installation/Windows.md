# 在 Windows 上安装 Manticore

Manticore 可以通过多种方式安装在 Windows 上。我们推荐使用 WSL（Windows Subsystem for Linux），因为它支持复制且不需要 Docker。以下是推荐和替代的方法。

### 安装或启用 WSL2

要在 Windows 上安装 Manticore Search，您首先需要启用 Windows 子系统 Linux。WSL2 允许您在 Windows 上原生运行 Linux 二进制文件。要使用此方法，您需要运行 Windows 10 版本 2004 及更高版本或 Windows 11。

请参考[微软官方指南](https://docs.microsoft.com/en-us/windows/wsl/install)获取安装 WSL2 的逐步说明。

### 安装 Manticore

要通过 WSL2 在 Windows 上安装 Manticore，请参考[Debian 和 Ubuntu 安装](../Installation/Debian_and_Ubuntu.md)部分。

> 注意：通过 WSL2 安装 Manticore 是推荐的方法，因为它相比使用本地 Windows 软件包提供更好的兼容性。

## 作为本地 Windows 二进制安装 Manticore

或者，您也可以作为本地 Windows 二进制文件安装 Manticore，这种方式需要为 [Manticore Buddy](../Installation/Manticore_Buddy.md#Manticore-Buddy) 使用 Docker，步骤如下：

1. 安装 Docker Desktop 并启动它。
2. 下载 Manticore Search 安装程序（可在[安装页面](http://manticoresearch.com/install/)找到链接）并运行。按照安装说明操作。
3. 选择安装目录。
4. 选择要安装的组件。我们建议全部安装。
5. Manticore 附带预配置的 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) `manticore.conf` 文件，无需额外配置。

> 注意：本地 Windows 二进制文件不支持[复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)。

### 作为 Windows 服务安装

要将 Manticore Search 服务器安装为 Windows 服务，请运行：

```bat
\path\to\searchd.exe --install --config \path\to\config --servicename Manticore
```

确保使用配置文件的完整路径，否则 `searchd.exe` 在作为服务启动时将无法找到它。

安装完成后，可以通过 Microsoft Management Console 的服务管理器启动该服务。

服务启动后，您可以通过 MySQL 命令行界面访问 Manticore：

```bat
mysql -P9306 -h127.0.0.1
```

请注意，在本手册的大多数示例中，我们使用 `-h0` 连接到本地主机，但在 Windows 上，您必须显式使用 `localhost` 或 `127.0.0.1`。

<!-- proofread -->

