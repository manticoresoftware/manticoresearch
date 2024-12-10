# 用户自定义函数 (UDFs) 和插件

Manticore 可以通过用户自定义函数 (UDFs) 进行扩展，如下所示：

```sql
SELECT id, attr1, myudf (attr2, attr3+attr4) ...
```

你可以动态加载和卸载 UDFs 到 `searchd`，无需重启服务器，并可以在搜索、排序等表达式中使用它们。UDF 的主要特性如下：

- UDFs 可以接受整数（32 位和 64 位）、浮点数、字符串、MVA 或 `PACKEDFACTORS()` 作为参数。
- UDFs 可以返回整数、浮点数或字符串值。
- UDFs 可以在查询设置阶段检查参数的数量、类型和名称，并在必要时抛出错误。

目前不支持聚合函数。换句话说，UDFs 将只针对单个文档调用，并期望为该文档返回一个值。尚不支持编写能够对共享同一 GROUP BY 键的整个文档组计算聚合值（如 `AVG()`）的函数。不过，你可以在内置的聚合函数中使用 UDFs：虽然还不支持 `MYCUSTOMAVG()`，但 `AVG(MYCUSTOMFUNC())` 应该可以正常工作！

UDFs 提供了广泛的应用场景，例如：

- 集成自定义的数学或字符串函数；
- 在 Manticore 中访问数据库或文件；
- 创建复杂的排序函数。

## 插件

插件提供了扩展搜索功能的额外机会。目前，插件可用于计算自定义排序以及对文档和查询进行分词处理。

以下是插件类型的完整列表：

- UDF 插件（本质上是 UDF，但由于它们通过插件加载，也被称为 'UDF 插件'）
- 排序器插件
- 索引时的分词过滤插件
- 查询时的分词过滤插件

本节介绍了编写和管理插件的通用过程；创建不同类型插件的具体细节将在各自的子部分中讨论。

那么，如何编写和使用插件？以下是一个简单的四步指南：

- 创建动态库（.so 或 .dll 文件），通常使用 C 或 C++；
- 使用 [CREATE PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) 命令将插件加载到 `searchd` 中；
- 通过特定的选项调用使用插件（通常通过特定的 OPTIONS 调用）；
- 使用 [DROP PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md) 卸载插件，或使用 [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) 重新加载插件。

请注意，虽然 UDFs 是一流的插件，但它们是通过单独的 [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) 语句安装的。这样可以规范地指定返回类型，而不会牺牲向后兼容性或更改语法。

动态插件在线程和线程池工作者中得到支持。多个插件（和/或 UDFs）可以包含在同一个库文件中。你可以选择将所有项目特定的插件集中在一个大型库中，或者为每个 UDF 和插件创建一个单独的库；这取决于你的需求。

与 UDFs 一样，你应该包含 `src/sphinxudf.h` 头文件。至少，你将需要 `SPH_UDF_VERSION` 常量来实现合适的版本函数。根据具体的插件类型，你可能需要，也可能不需要将插件与 `src/sphinxudf.c` 链接。然而，`sphinxudf.c` 中实现的所有函数都与解包 `PACKEDFACTORS()` blob 有关，而目前没有插件类型能够访问该数据。所以，目前只链接头文件就足够了。（实际上，如果你复制了 UDF 版本号，那么对于某些插件类型，你甚至不需要头文件。）

形式上，插件只是遵循特定命名模式的一组 C 函数。通常，你需要为主要任务定义一个关键函数，但你也可以定义其他函数。例如，要实现名为 "myrank" 的排序器，你必须定义一个返回排序值的 `myrank_finalize()` 函数。不过，你还可以定义 `myrank_init()`、`myrank_update()` 和 `myrank_deinit()` 函数。根据插件类型的不同，已知后缀和调用参数的具体集合会有所不同，但 `_init()` 和 `_deinit()` 是通用的，每个插件都有这些函数。提示：有关已知后缀及其参数类型的快速参考，请参阅 `sphinxplugin.h`，调用原型在文件开头定义。

即使公共接口是用纯 C 语言定义的，我们的插件基本上遵循面向对象的模型。实际上，每个 `_init()` 函数都会接收一个 `void ** userdata` 输出参数，存储在 `(*userdata)` 处的指针值随后会作为第一个参数传递给插件的所有其他函数。因此，你可以将插件视为一个类，每当需要处理请求时实例化该类的对象：`userdata` 指针充当 `this` 指针，函数充当方法，`_init()` 和 `_deinit()` 函数分别作为构造函数和析构函数。

这种轻微的面向对象模型复杂化是因为插件运行在多线程环境中，有些插件需要维护状态。你不能将该状态存储在插件的全局变量中，所以我们传递了一个 `userdata` 参数，这自然导致了面向对象模型。如果你的插件简单且无状态，接口允许你省略 `_init()`、`_deinit()` 以及其他任何函数。

总结一下，下面是一个仅包含三行 C 代码的最简单完整的排序器插件：

```c
// gcc -fPIC -shared -o myrank.so myrank.c
#include "sphinxudf.h"
int myrank_ver() { return SPH_UDF_VERSION; }
int myrank_finalize(void *u, int w) { return 123; }
```

以下是如何使用这个简单的排序器插件：

```sql
mysql> CREATE PLUGIN myrank TYPE 'ranker' SONAME 'myrank.dll';
Query OK, 0 rows affected (0.00 sec)

mysql> SELECT id, weight() FROM test1 WHERE MATCH('test') OPTION ranker=myrank('');
+------+----------+
| id   | weight() |
+------+----------+
|    1 |      123 |
|    2 |      123 |
+------+----------+
2 rows in set (0.01 sec)
```
<!-- proofread -->

