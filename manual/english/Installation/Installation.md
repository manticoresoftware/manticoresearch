# Installation

<!-- example installation expanded -->

<!-- request RHEL, Centos, Alma, Amazon, Oracle -->

``` bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-extra
```

If you are upgrading from an older version, it is recommended to remove your old packages first to avoid conflicts caused by the updated package structure:
```bash
sudo yum --setopt=tsflags=noscripts remove manticore*
```
It won't remove your data. If you made changes to the configuration file, it will be saved to `/etc/manticoresearch/manticore.conf.rpmsave`.

If you are looking for separate packages, please find them [here](https://manticoresearch.com/install/#separate-packages).

For more details on the installation, see [below](../Installation/RHEL_and_Centos.md).

<!-- request Debian, Ubuntu, Mint -->

``` bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-extra
```

If you are upgrading to Manticore 6 from an older version, it is recommended to remove your old packages first to avoid conflicts caused by the updated package structure:
```bash
sudo apt remove manticore*
```
It won't remove your data or configuration file.

If you are looking for separate packages, please find them [here](https://manticoresearch.com/install/#separate-packages).

For more details on the installation, see [below](../Installation/Debian_and_Ubuntu.md).

<!-- request MacOS -->

``` bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

Please find more details on the installation [below](../Installation/MacOS.md).

<!-- request Windows -->

1. Download the [Manticore Search Installer](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.4-230314-1a3a4ea82-x64.exe) and run it. Follow the installation instructions.
2. Choose the directory to install to.
3. Select the components you want to install. We recommend installing all of them.
4. Manticore comes with a preconfigured `manticore.conf` file in [RT mode](https://manual.manticoresearch.com/Read_this_first.md#Real-time-mode-vs-plain-mode). No additional configuration is required.

For more details on the installation, see [below](../Installation/Windows.md#Installing-Manticore-on-Windows).

<!-- request Docker -->

One-liner for a sandbox (not recommended for production use):
``` bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

This runs Manticore container and waits for it to boot up. After starting, it launches a MySQL client session. When you exit the MySQL client, the Manticore container ceases operation and gets deleted, leaving no stored data behind. For details on how to utilize Manticore in a live production setting, refer to the following section.

For production use:
``` bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

This setup will enable the Manticore Columnar Library and Manticore Buddy, and run Manticore on ports 9306 for MySQL connections and 9308 for all other connections, using `./data/` as the designated data directory.

Read more about production use [in the documentation](https://github.com/manticoresoftware/docker#production-use).

<!-- request Kubernetes -->

``` bash
helm repo add manticoresearch https://helm.manticoresearch.com
# Update values.yaml if needed
helm install manticore -n manticore --create-namespace manticoresearch/manticoresearch
```

You can find more information about installing the Helm chart [in the documentation](https://github.com/manticoresoftware/manticoresearch-helm#installation).

<!-- end -->

<!-- proofread -->
