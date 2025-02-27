# Manticore Buddy

Manticore Buddy is a sidecar for Manticore Search, written in PHP, that helps with various tasks. The typical workflow is that before returning an error to the user, Manticore Search asks Buddy if it can handle the issue for the daemon. Buddy's PHP code makes it easy to implement high-level features that do not require extremely high performance.

For a deeper understanding of Buddy, check out these articles:
- [Introducing Buddy: the PHP sidecar for Manticore Search](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)
- [Manticore Buddy: challenges and solutions](https://manticoresearch.com/blog/manticoresearch-buddy-challenges-and-solutions/)
- [Manticore Buddy: pluggable design](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/)
- [Manticore Buddy GitHub repository](https://github.com/manticoresoftware/manticoresearch-buddy)

## Manticore Buddy Installation

If you follow the installation instructions above or [on the website](https://manticoresearch.com/install), you don't have to worry about installing or starting Manticore Buddy: it gets installed automatically when you install the package `manticore-extra`, and Manticore Search starts it automatically at launch.

## Disabling Manticore Buddy

To disable Manticore Buddy, please use the [buddy_path](../Server_settings/Searchd.md#buddy_path) setting.
