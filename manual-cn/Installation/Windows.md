# 在 Windows 上安装 Manticore

1. 安装 Docker Desktop并启动它。
2. 下载 [Manticore Search 安装程序](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.3.6-24080214-593045790-x64.exe)，并按照安装说明进行操作。
3. 选择安装目录。
4. 选择要安装的组件。建议安装所有组件。
5. Manticore 自带一个预配置的 `manticore.conf` 文件，默认为 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)，无需额外配置。

## 作为 Windows 服务安装

要将 `searchd`（Manticore Search 服务器）作为 Windows 服务安装，请运行：

```bat
\path\to\searchd.exe --install --config \path\to\config --servicename Manticore
```

确保使用配置文件的完整路径，否则 `searchd.exe` 启动服务时无法找到该文件。

安装后，可以通过 Microsoft 管理控制台的服务管理器启动该服务。

启动后，可以使用 MySQL 命令行接口访问 Manticore：

```bat
mysql -P9306 -h127.0.0.1
```

请注意，在本手册的大多数示例中，我们使用 `-h0` 连接本地主机，但在 Windows 中，必须显式使用 `localhost` 或 `127.0.0.1`。

<!-- proofread -->
