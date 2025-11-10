# 在 Windows 上安装 Manticore

Manticore 可以通过多种方式安装在 Windows 上。我们推荐使用 WSL（Windows 子系统 Linux），因为它支持复制且不需要 Docker。以下是推荐和替代的方法。

### 安装或启用 WSL2

要在 Windows 上安装 Manticore Search，首先需要启用 Windows 子系统 Linux。WSL2 允许你在 Windows 上原生运行 Linux 二进制文件。此方法要求你运行的是 Windows 10 版本 2004 及以上或 Windows 11。

请参阅 [微软官方指南](https://docs.microsoft.com/en-us/windows/wsl/install) 获取安装 WSL2 的分步说明。

### 安装 Manticore

要通过 WSL2 在 Windows 上安装 Manticore，请参考 [Debian 和 Ubuntu 安装](../Installation/Debian_and_Ubuntu.md) 部分。

> 注意：通过 WSL2 安装 Manticore 是推荐的方法，因为它比使用原生 Windows 包提供更好的兼容性。

## 作为原生 Windows 二进制文件安装 Manticore

或者，你可以将 Manticore 作为原生 Windows 二进制文件安装，这需要 Docker 来支持 [Manticore Buddy](../Installation/Manticore_Buddy.md#Manticore-Buddy)，步骤如下：

1. 安装 Docker Desktop 并启动它。
2. 下载 Manticore Search 安装程序（在[安装页面](http://manticoresearch.com/install/)找到链接）并运行。按照安装说明操作。
3. 选择安装目录。
4. 选择你想安装的组件。我们推荐全部安装。
5. Manticore 附带预配置的 `manticore.conf` 文件，处于 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)。无需额外配置。

> 注意：原生 Windows 二进制文件不支持 [复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)。

### 作为 Windows 服务安装

要将 Manticore Search 服务器安装为 Windows 服务，运行：

```bat
\path\to\searchd.exe --install --config \path\to\config --servicename Manticore
```

确保使用配置文件的完整路径，否则 `searchd.exe` 在作为服务启动时将无法定位它。

安装后，可以通过 Microsoft 管理控制台的服务管理单元启动该服务。

启动后，你可以使用 MySQL 命令行界面访问 Manticore：

```bat
mysql -P9306 -h127.0.0.1
```

请注意，在本手册的大多数示例中，我们使用 `-h0` 连接本地主机，但在 Windows 中，必须显式使用 `localhost` 或 `127.0.0.1`。

<!-- proofread -->

