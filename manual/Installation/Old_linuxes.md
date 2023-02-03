# Via Homebrew in old Linuxes

Manticore team usually doesn't provide new version packages for old Linuxes (after end of life). If:
1. you are still using one
2. and you can't find an official `.deb` / `.apt` package on https://repo.manticoresearch.com/

you can try to use [Homebrew](https://brew.sh/) for installing Manticore Search:

```bash
brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra
```

and then start it:

```bash
brew services start manticoresearch
```
