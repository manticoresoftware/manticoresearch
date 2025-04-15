# 在 Windows 上安装 Manticore

Manticore 可以通过几种方式在 Windows 上安装。我们推荐使用 WSL（Windows 子系统用于 Linux），因为它支持复制，并且不需要 Docker。以下是推荐的和替代的方法。

### 安装或启用 WSL2

要在 Windows 上安装 Manticore Search，您首先需要启用 Windows 子系统用于 Linux。WSL2 允许您在 Windows 上原生运行 Linux 二进制文件。要使此方法有效，您需要运行 Windows 10 版本 2004 或更高版本，或 Windows 11。

请按照 [官方 Microsoft 指南](https://docs.microsoft.com/en-us/windows/wsl/install) 进行逐步操作以安装 WSL2。

### 安装 Manticore

要通过 WSL2 在 Windows 上安装 Manticore，请参阅 [Debian 和 Ubuntu 安装](../../Installation/Debian_and_Ubuntu.md) 部分。

> 注意：通过 WSL2 安装 Manticore 是推荐的方法，因为与使用原生 Windows 包相比，它提供了更好的兼容性。

## 作为原生 Windows 二进制文件安装 Manticore

另外，您可以将 Manticore 作为原生 Windows 二进制文件安装，这需要 Docker 来支持 [Manticore Buddy](../../Installation/Manticore_Buddy.md#Manticore-Buddy)，请按照以下步骤操作：

1. 安装 Docker Desktop 并启动它。
2. 下载 Manticore Search 安装程序（可以在 [安装页面](http://manticoresearch.com/install/) 找到链接）并运行它。按照安装说明进行操作。
3. 选择要安装的目录。
4. 选择您想安装的组件。我们推荐安装所有组件。
5. Manticore 自带一个预配置的 `manticore.conf` 文件，处于 [实时模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 中。无需额外配置。

> 注意：原生 Windows 二进制文件不支持 [复制](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication)。

### 作为 Windows 服务安装

要将 Manticore Search 服务器安装为 Windows 服务，请运行：

```bat
\path\to\searchd.exe --install --config \path\to\config --servicename Manticore
```

确保使用配置文件的完整路径，否则 `searchd.exe` 在作为服务启动时将无法找到它。

安装后，可以从 Microsoft 管理控制台的服务管理单元中启动该服务。

启动后，您可以使用 MySQL 命令行界面访问 Manticore：

```bat
mysql -P9306 -h127.0.0.1
```

请注意，在本手册的大多数示例中，我们使用 `-h0` 连接到本地主机，但在 Windows 中，您必须明确使用 `localhost` 或 `127.0.0.1`。

<!-- proofread -->
