# UDFs and Plugins

Manticore 可以通过用户自定义函数 (UDFs，简称 UDF) 进行扩展，如下所示：

```sql
SELECT id, attr1, myudf(attr2, attr3+attr4) ...
```

您可以动态加载和卸载 UDF 到 `searchd` 中，而无需重新启动服务器，并在搜索、排序等时使用它们。UDF 特性的快速总结如下：

* UDF 可以接受整数（包括 32 位和 64 位）、浮点数、字符串、MVA 或 `PACKEDFACTORS()` 参数。
* UDF 可以返回整数、浮点数或字符串值。
* UDF 可以在查询设置阶段检查参数的数量、类型和名称，并引发错误。

我们尚未支持聚合函数。换句话说，您的 UDF 每次只能处理单个文档，并且预期将为该文档返回某个值。编写一个可以在共享相同 GROUP BY 键的整个文档组上计算聚合值（如 AVG()）的函数目前还不可能。然而，您可以在内置聚合函数内使用 UDF：也就是说，尽管 MYCUSTOMAVG() 尚不支持，AVG(MYCUSTOMFUNC()) 应该能正常工作！

UDF 提供了广泛的应用，诸如：

* 融入自定义数学或字符串函数；
* 从 Manticore 内部访问数据库或文件；
* 创建复杂的排名函数。

## Plugins

插件提供了扩展搜索功能的额外机会。目前它们可用于计算自定义排名和标记文档与查询。

以下是插件类型的完整列表：

* UDF 插件（本质上是 UDF，但由于它们是插入的，因此也称为“UDF 插件”）
* 排名器插件
* 索引时标记过滤器插件
* 查询时标记过滤器插件

本节涵盖编写和管理插件的一般过程；关于创建不同类型插件的细节将在各自的子节中讨论。

那么，您如何编写和使用插件？以下是一个快速的四步指南：

* 创建动态库（.so 或 .dll），大多数情况下使用 C 或 C++；
* 使用 [CREATE PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) 将插件加载到 searchd 中；
* 使用插件特定的调用（通常通过特定选项）使用插件。
* 使用 [DROP PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md) 和 [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) 分别卸载或重新加载插件。

请注意，虽然 UDF 是一类插件，但它们是使用单独的 [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) 语句安装的。这允许在不牺牲向后兼容性或更改语法的情况下整齐地指定返回类型。

动态插件在线程和线程池工作者中受到支持。多个插件（和/或 UDF）可以包含在一个单独的库文件中。您可以选择将所有项目特定的插件组合在一个大型库中，或者为每个 UDF 和插件创建一个单独的库；由您决定。

与 UDF 一样，您应该包含 `src/sphinxudf.h` 头文件。至少，您需要 `SPH_UDF_VERSION` 常量来实现适当的版本函数。根据特定的插件类型，您可能需要或不需要将您的插件与 `src/sphinxudf.c` 链接。但是，所有在 `sphinxudf.c` 中实现的函数与解压 `PACKEDFACTORS()` blob 有关，并且没有插件类型可以访问该数据。因此，目前只链接头文件应该足够。（实际上，如果您复制过 UDF 版本号，对于某些插件类型，您甚至不需要头文件。）

从形式上讲，插件仅仅是遵循特定命名模式的一组 C 函数。通常，您需要定义一个关键函数用于主要任务，但您也可以定义附加函数。例如，要实现一个名为“myrank”的排名器，您必须定义一个返回排名值的 `myrank_finalize()` 函数。但是，您也可以定义 `myrank_init()`、`myrank_update()` 和 `myrank_deinit()` 函数。具体的众所周知后缀和调用参数集根据插件类型而异，但 _init() 和 _deinit() 是通用的，每个插件都有它们。提示：要快速参考已知后缀及其参数类型，请参考 `sphinxplugin.h`，在文件开头定义了调用原型。

尽管公共接口是在纯 C 中定义的，但我们的插件本质上遵循 *面向对象模型*。事实上，每个 `_init()` 函数都接收一个 `void ** userdata` 输出参数，存储在 `(*userdata)` 中的指针值随后作为第一个参数传递给所有其他插件函数。因此，您可以将插件视为一个 *类*，每次需要处理请求的该类实例化一个对象时：`userdata` 指针充当 `this` 指针；这些函数充当方法，而 `_init()` 和 `_deinit()` 函数则分别作为构造函数和析构函数。

这个 OOP-in-C 的小复杂性源于插件在多线程环境中运行，有些需要保持状态。您不能在插件中使用全局变量存储该状态，因此我们传递一个 userdata 参数，这自然导致了面向对象模型。如果您的插件简单无状态，接口允许您省略 `_init()`、`_deinit()` 和任何其他函数。

总之，以下是仅用三行 C 代码编写的最简单的完整排名器插件：

```c
// gcc -fPIC -shared -o myrank.so myrank.c
#include "sphinxudf.h"
int myrank_ver() { return SPH_UDF_VERSION; }
int myrank_finalize(void *u, int w) { return 123; }
```

这是如何使用简单排名器插件的示例：

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

