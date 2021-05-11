# Installing Manticore on MacOS

On MacOS Manticore can be installed in two ways:

## Via Homebrew package manager

Bottles are available for:

* MacOS High Sierra
* MacOS Mojave
* MacOS Catalina

On older versions of MacOS it will attempt to compile from sources automatically.

```bash
brew install manticoresearch
```

For [indexing](../Creating_an_index/Local_indexes/Plain_index.md) the additional dependencies that can be installed using brew are `mysql@5.7`, `libpq`, `unixodbc`.

Afterwards you can start Manticore as a brew service:

```bash
brew services start manticoresearch
```

The configuration is located at `/usr/local/etc/manticore/manticore.conf`.

## From tarball with binaries

Download it [from the website](https://manticoresearch.com/downloads/) and unpack to a folder:

```bash
mkdir manticore
tar -zxvf manticore-3.6.0-210504-96d61d8bf-release-osx10.14.4-x86_64-main.tar.gz -C manticore
cd manticore
bin/searchd  -c manticore.conf
```

Manticore configuration file `manticore.conf` is located in the directory, unpacked from the tarball. 
