# 安装

<!-- example installation expanded -->

<!-- request RHEL, Centos, Alma, Amazon, Oracle -->

``` bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-extra
```

如果您正在从旧版本升级，建议您首先删除旧软件包，以避免因更新的软件包结构而导致的冲突：
```bash
sudo yum --setopt=tsflags=noscripts remove manticore*
```
这不会删除您的数据。如果您对配置文件进行了更改，它将保留在 `/etc/manticoresearch/manticore.conf.rpmsave` 中。

如果您正在寻找单独的软件包，请在 [这里](https://manticoresearch.com/install/#separate-packages) 查找。

有关安装的更多详细信息，请参见 [下面](../Installation/RHEL_and_Centos.md)。

<!-- request Debian, Ubuntu, Mint -->

``` bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-extra
```

如果您正在从旧版本升级到 Manticore 6，建议您首先删除旧软件包，以避免因更新的软件包结构而导致的冲突：
```bash
sudo apt remove manticore*
```
这不会删除您的数据或配置文件。

如果您正在寻找单独的软件包，请在 [这里](https://manticoresearch.com/install/#separate-packages) 查找。

有关安装的更多详细信息，请参见 [下面](../Installation/Debian_and_Ubuntu.md)。

<!-- request MacOS -->

``` bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

有关安装的更多详细信息，请参见 [下面](../Installation/MacOS.md)。

<!-- request Windows -->

1. 下载 [Manticore Search Installer](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.4-230314-1a3a4ea82-x64.exe) 并运行。按照安装说明操作。
2. 选择要安装的目录。
3. 选择您想要安装的组件。我们建议安装它们全部。
4. Manticore带有预配置的 `manticore.conf` 文件，支持 [RT 模式](https://manual.manticoresearch.com/Read_this_first.md#Real-time-mode-vs-plain-mode)。无需额外配置。

有关安装的更多详细信息，请参见 [下面](../Installation/Windows.md#Installing-Manticore-on-Windows)。

<!-- request Docker -->

用于沙箱的单行命令（不建议用于生产环境）：
``` bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

这将运行 Manticore 容器并等待其启动。启动后，会启动一个 MySQL 客户端会话。当您退出 MySQL 客户端时，Manticore 容器将停止运行并被删除，不会留存任何存储的数据。有关如何在生产环境中使用 Manticore 的详细信息，请查看以下部分。

对于生产使用：
``` bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

该设置将启用 Manticore 列库和 Manticore Buddy，并在 9306 端口运行 Manticore 以进行 MySQL 连接，在 9308 端口运行以进行所有其他连接，使用 `./data/` 作为指定的数据目录。

有关生产使用的更多信息，请参见 [在文档中](https://github.com/manticoresoftware/docker#production-use)。

<!-- request Kubernetes -->

``` bash
helm repo add manticoresearch https://helm.manticoresearch.com
# Update values.yaml if needed
helm install manticore -n manticore --create-namespace manticoresearch/manticoresearch
```

您可以在 [文档中](https://github.com/manticoresoftware/manticoresearch-helm#installation) 找到有关安装 Helm 图表的更多信息。

<!-- end -->

<!-- proofread -->
