# Via Homebrew in old Linuxes

Manticore team usually doesn't provide new version packages for old Linuxes (after EOL). If:
1. you are still using such
2. and you can't find official .deb / .apt packages on https://repo.manticoresearch.com/

You can try to use [Homebrew](https://brew.sh/) for installing Manticore Search:

```bash
brew install manticoresearch
```

and then start it:

```bash
brew services start manticoresearch
```
