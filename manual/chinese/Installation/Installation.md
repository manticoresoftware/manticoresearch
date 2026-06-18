# 安装

<!-- example installation expanded -->

<!-- request RHEL, Centos, Alma, Amazon, Oracle, Debian, Ubuntu, Mint, MacOS -->

``` bash
curl https://manticoresearch.com | sh
```

或者，如果你更喜欢 `wget`：

```bash
wget -O - https://manticoresearch.com | sh
```

如果您需要单独的安装包，请在[这里](https://manticoresearch.com/install/#separate-packages)查找。

有关在基于 RPM 的 Linux 上安装的更多细节，请参见[这里](../Installation/RHEL_and_Centos.md)。

有关在基于 DEBIAN 的 Linux 上安装的更多细节，请参见[这里](../Installation/Debian_and_Ubuntu.md)。

有关在 macOS 上安装的更多细节，请参见[下方](../Installation/MacOS.md)。

有关快速安装器的更多细节，请运行

``` bash
curl https://manticoresearch.com | sh -s help
```

或者，如果你更喜欢 `wget`：

```bash
wget -O - https://manticoresearch.com | sh -s help
```

<!-- request Windows -->

1. 下载 [Manticore Search 安装程序](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.4-230314-1a3a4ea82-x64.exe) 并运行，按照安装提示操作。
2. 选择安装目录。
3. 选择您想安装的组件。我们推荐全部安装。
4. Manticore 附带一个预配置的 `manticore.conf` 文件，处于 [RT 模式](https://manual.manticoresearch.com/Read_this_first.md#Real-time-mode-vs-plain-mode)。无需额外配置。

有关安装的更多详情，请参见[下文](../Installation/Windows.md#Installing-Manticore-on-Windows)。

<!-- request Docker -->

一个用于沙箱测试的单行命令（不推荐用于生产环境）：
``` bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

此命令运行 Manticore 容器并等待其启动。启动后，它会开启一个 MySQL 客户端会话。当您退出 MySQL 客户端时，Manticore 容器停止运行并被删除，不会留下任何数据。关于如何在生产环境中使用 Manticore，请参见下一节。

用于生产环境：
``` bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

此配置将启用 Manticore 列存库和 Manticore Buddy，并使用 `./data/` 作为指定数据目录，把 Manticore 运行在 9306 端口用于 MySQL 连接，9308 端口用于其他所有连接。

更多关于生产环境使用信息，请阅读[文档](https://github.com/manticoresoftware/docker#production-use)。

<!-- request Kubernetes -->

``` bash
helm repo add manticoresearch https://helm.manticoresearch.com
# Update values.yaml if needed
helm install manticore -n manticore --create-namespace manticoresearch/manticoresearch
```

您可以在[文档](https://github.com/manticoresoftware/manticoresearch-helm#installation)中找到有关安装 Helm Chart 的更多信息。

<!-- end -->

<!-- proofread -->
