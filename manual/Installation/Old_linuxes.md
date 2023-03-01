# Via Homebrew on old Linuxes

The Manticore team typically does not provide new version packages for old Linuxes that have reached their end of life. If:

1. you are still using one
2. and you cannot find an official `.deb` / `.apt` package on https://repo.manticoresearch.com/

you can try using [Homebrew](https://brew.sh/) to install Manticore Search:

```bash
brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra
```

and then start it:

```bash
brew services start manticoresearch
```
<!-- proofread -->