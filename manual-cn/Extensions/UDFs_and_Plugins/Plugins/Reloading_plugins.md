# RELOADING PLUGINS

```sql
RELOAD PLUGINS FROM SONAME 'plugin_library'
```

此命令会重新加载指定库中的所有插件（包括 UDF、ranker 等）。重新加载过程具有以下特点：

1. 确保所有插件都成功更新为它们的新版本；
2. 更新是原子性的，这意味着所有插件会同时被替换，确保在查询中使用来自重新加载库的多个函数时，绝不会混合使用旧版本和新版本。

在 `RELOAD` 过程中，插件集是保证一致的；要么全部是旧版本，要么全部是新版本。

此外，重新加载过程是无缝的，因为在插件重新加载期间，总有某个版本的插件可供并发查询使用，不会出现暂时的中断。这比使用一对 `DROP` 和 `CREATE` 语句来重新加载插件要好，后者在 `DROP` 和随后的 `CREATE` 之间会有一个短暂的时间窗口，期间查询引用的插件会不可用，因此会导致查询失败。

如果重新加载失败，`RELOAD PLUGINS` 不会做任何更改，保留旧插件并报告错误。

在 Windows 上，覆盖或删除当前正在使用的 DLL 库可能会有问题。不过，您仍然可以重命名它，然后将新版本放到原来的名称下，之后 `RELOAD` 命令将会生效。成功重新加载后，您还可以删除重命名的旧库。

```sql
mysql> RELOAD PLUGINS FROM SONAME 'udfexample.dll';
Query OK, 0 rows affected (0.00 sec)
```
<!-- proofread -->