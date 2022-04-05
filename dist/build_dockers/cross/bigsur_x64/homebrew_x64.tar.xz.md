# Packaging subset of homebrew

First, make full copy of homebrew (that is located in /opt/homebrew folder on bigsur, or in /usr/local on earlier systems)

Go to Cellar folder, and cleanup it from non-necessary packages.
You need:

* boost
* mysql-client
* openssl
* postgresql
* unixodbc
* zstd

Just keep folders with these names (or, starting with these names). Openssl could be openssl@1.1, or openssl@3 - first one is dependency for mysql-client, second one is standalone. It has sense to keep openssl@1.1 to avoid duplicates of the same lib with two different versions.

The walk thorough 'include', 'lib', and 'opt' folders; cleanup all dead symlinks (as you remove folders from Cellar - references from these folders will became dead).

Move content of the folder to the 'x86_64' folder, and pack to tar.xz archive

```bash
tar -cf - x86_64 | xz -f > /homebrew_x64.tar.xz
```

Prepared archives are also stored on our dev server, in `/work/docker` folder.