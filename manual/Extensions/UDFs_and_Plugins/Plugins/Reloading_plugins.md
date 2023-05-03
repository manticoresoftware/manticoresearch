# RELOADING PLUGINS

```sql
RELOAD PLUGINS FROM SONAME 'plugin_library'
```

Reloads all plugins (UDFs, rankers, etc.) from a given library. In a sense, the reload process is transactional, ensuring that:
1. all plugins are successfully updated to their new versions;
2. the update is atomic, meaning all plugins are replaced simultaneously. This atomicity ensures that queries using multiple functions from a reloaded library will never mix old and new versions.

During the `RELOAD`, the set of plugins is guaranteed to be consistent; they will either be all old or all new.

The reload process is also seamless, as some version of a reloaded plugin will always be available for concurrent queries, without any temporary disruptions. This is an improvement over using a pair of `DROP` and `CREATE` statements for reloading. With those, there is a brief window between the `DROP` and the subsequent `CREATE` during which queries technically refer to an unknown plugin and will therefore fail.

If there's any failure, `RELOAD PLUGINS` does nothing, retains the old plugins, and reports an error.

On Windows, overwriting or deleting a DLL library currently in use can be problematic. However, you can still rename it, place a new version under the old name, and then `RELOAD` will work. After a successful reload, you'll also be able to delete the renamed old library.

```sql
mysql> RELOAD PLUGINS FROM SONAME 'udfexample.dll';
Query OK, 0 rows affected (0.00 sec)
```
<!-- proofread -->