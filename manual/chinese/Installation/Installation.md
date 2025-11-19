# 安装

<!-- example installation expanded -->

<!-- request RHEL, Centos, Alma, Amazon, Oracle -->

``` bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-extra
```

如果您是从旧版本升级，建议先删除旧的软件包，以避免因软件包结构更新而导致的冲突：
```bash
sudo yum --setopt=tsflags=noscripts remove manticore*
```
这不会删除您的数据。如果您对配置文件进行了修改，修改内容将保存到 `/etc/manticoresearch/manticore.conf.rpmsave`。

如果您需要单独的软件包，请在[这里](https://manticoresearch.com/install/#separate-packages)查找。

有关安装的更多详细信息，请参见[下文](../Installation/RHEL_and_Centos.md)。

<!-- request Debian, Ubuntu, Mint -->

``` bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-extra
```

如果您是从旧版本升级到 Manticore 6，建议先删除旧的软件包，以避免因软件包结构更新而导致的冲突：
```bash
sudo apt remove manticore*
```
这不会删除您的数据或配置文件。

如果您需要单独的软件包，请在[这里](https://manticoresearch.com/install/#separate-packages)查找。

有关安装的更多详细信息，请参见[下文](../Installation/Debian_and_Ubuntu.md)。

<!-- request MacOS -->

``` bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

请在[下文](../Installation/MacOS.md)查找有关安装的更多详细信息。

<!-- request Windows -->

1. 下载 [Manticore Search 安装程序](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.4-230314-1a3a4ea82-x64.exe) 并运行。按照安装说明操作。
2. 选择安装目录。
3. 选择您想安装的组件。我们建议安装所有组件。
4. Manticore 附带预配置的 `manticore.conf` 文件，采用[RT 模式](https://manual.manticoresearch.com/Read_this_first.md#Real-time-mode-vs-plain-mode)。无需额外配置。

有关安装的更多详细信息，请参见[下文](../Installation/Windows.md#Installing-Manticore-on-Windows)。

<!-- request Docker -->

用于沙箱的一行命令（不建议用于生产环境）：
``` bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

这会运行 Manticore 容器并等待其启动。启动后，它会启动一个 MySQL 客户端会话。当您退出 MySQL 客户端时，Manticore 容器停止运行并被删除，不会留下任何存储数据。有关如何在生产环境中使用 Manticore 的详细信息，请参见以下部分。

用于生产环境：
``` bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

此设置将启用 Manticore 列存库和 Manticore Buddy，并在端口 9306（MySQL 连接）和 9308（所有其他连接）上运行 Manticore，使用 `./data/` 作为指定的数据目录。

有关生产环境使用的更多信息，请参见[文档](https://github.com/manticoresoftware/docker#production-use)。

<!-- request Kubernetes -->

``` bash
helm repo add manticoresearch https://helm.manticoresearch.com
# Update values.yaml if needed
helm install manticore -n manticore --create-namespace manticoresearch/manticoresearch
```

您可以在[文档](https://github.com/manticoresoftware/manticoresearch-helm#installation)中找到有关安装 Helm chart 的更多信息。

<!-- end -->

<!-- proofread -->

