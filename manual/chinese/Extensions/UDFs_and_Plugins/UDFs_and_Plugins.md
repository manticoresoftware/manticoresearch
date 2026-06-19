# UDF 和插件

Manticore 可以通过用户自定义函数（UDF）进行扩展，如下所示：

```sql
SELECT id, attr1, myudf (attr2, attr3+attr4) ...
```

您可以动态加载和卸载 UDF 到 `searchd` 中，而无需重启服务器，并在搜索、排序等表达式中使用它们。UDF 的功能简要总结如下：

* UDF 可以接受整数（32 位和 64 位）、浮点数、字符串、MVA 或 `PACKEDFACTORS()` 参数。
* UDF 可以返回整数、浮点数、字符串或 MVA 值（MULTI、MULTI64、FLOAT_VECTOR）。
* UDF 可以在查询设置阶段检查参数数量、类型和名称，并引发错误。

我们目前还不支持聚合函数。换句话说，您的 UDF 将仅针对单个文档调用，并期望为该文档返回某个值。编写一个可以计算整个文档组（具有相同 GROUP BY 键）的聚合值（如 AVG()）的函数目前尚不可行。但是，您可以在内置的聚合函数中使用 UDF：即，尽管 MYCUSTOMAVG() 尚未支持，但 AVG(MYCUSTOMFUNC()) 应该可以正常工作！

## MVA 返回类型

UDF 还可以返回多值属性（MVA），除了标量值。支持的 MVA 返回类型包括：

* **MULTI**：32 位无符号整数数组
* **MULTI64**：64 位有符号整数数组  
* **FLOAT_VECTOR**：浮点数数组

MVA UDF 使用相同的 `CREATE FUNCTION` 语法创建，带有适当的返回类型，并且可以在 SELECT 语句中像标量 UDF 一样使用。

UDF 提供了广泛的应用场景，例如：

* 引入自定义的数学或字符串函数；
* 在 Manticore 内部访问数据库或文件；
* 创建复杂的排序函数。

## 插件

插件提供了扩展搜索功能的额外机会。它们目前可用于计算自定义排序和对文档和查询进行分词。

以下是插件类型的完整列表：

* UDF 插件（本质上是 UDF，但由于它们是插件，因此也称为“UDF 插件”）
* 排序插件
* 索引时分词过滤插件
* 查询时分词过滤插件

本节涵盖编写和管理插件的一般过程；有关创建不同类型的插件的具体信息将在各自的子部分中讨论。

那么，如何编写和使用插件？这里有一个快速的四步指南：

* 创建一个动态库（通常是 .so 或 .dll），最有可能使用 C 或 C++；
* 使用 [CREATE PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) 将插件加载到 searchd 中；
* 通过特定于插件的调用（通常通过特定的 OPTIONS）使用插件。
* 分别使用 [DROP PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md) 和 [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) 卸载或重新加载插件。

请注意，虽然 UDF 是一流的插件，但它们使用单独的 [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) 语句进行安装。这允许对返回类型进行整洁的指定，而不会牺牲向后兼容性或更改语法。

动态插件在 threads 和 thread_pool 工作者中得到支持。单个库文件中可以包含多个插件（和/或 UDF）。您可以选择将所有项目特定的插件分组到一个大型库中，或者为每个 UDF 和插件创建单独的库；这取决于您。

与 UDF 一样，您应包含 `src/sphinxudf.h` 头文件。至少，您需要 `SPH_UDF_VERSION` 常量来实现适当的版本函数。根据特定的插件类型，您可能需要或不需要将您的插件与 `src/sphinxudf.c` 链接。然而，`sphinxudf.c` 中实现的所有函数都与解包 `PACKEDFACTORS()` blob 相关，而没有任何插件类型可以访问这些数据。因此，目前仅链接头文件就足够了。（事实上，如果您复制了 UDF 版本号，对于某些插件类型甚至不需要头文件。）

正式地说，插件只是遵循特定命名模式的一组 C 函数。通常需要定义一个用于主要任务的关键函数，但您也可以定义其他函数。例如，要实现一个名为 "myrank" 的排序器，您必须定义一个返回排序值的 `myrank_finalize()` 函数。但是，您还可以定义 `myrank_init()`、`myrank_update()` 和 `myrank_deinit()` 函数。根据插件类型，已知的后缀和调用参数的特定集合会有所不同，但 _init() 和 _deinit() 是通用的，每个插件都有它们。提示：有关已知后缀及其参数类型的快速参考，请参阅 `sphinxplugin.h`，其中在文件开头定义了调用原型。

尽管公共接口是用纯 C 定义的，但我们的插件本质上遵循 *面向对象模型*。确实，每个 `_init()` 函数接收一个 `void ** userdata` 输出参数，然后将 `(*userdata)` 存储的指针值作为所有其他插件函数的第一个参数传递。因此，您可以将插件视为一个 *类*，每当需要该类的对象来处理请求时，都会实例化它：`userdata` 指针充当 `this` 指针；函数作为方法，而 `_init()` 和 `_deinit()` 函数分别作为构造函数和析构函数。

这种在 C 中的轻微面向对象复杂性源于插件在多线程环境中运行，而某些插件需要维护状态。您不能在插件中将该状态存储在全局变量中，因此我们传递一个 userdata 参数，这自然导致面向对象模型。如果您的插件简单且无状态，接口允许您省略 `_init()`、`_deinit()` 和任何其他函数。

总结一下，以下是用仅三行 C 代码编写的最简单的完整排序插件：

```c
// gcc -fPIC -shared -o myrank.so myrank.c
#include "sphinxudf.h"
int myrank_ver() { return SPH_UDF_VERSION; }
int myrank_finalize(void *u, int w) { return 123; }
```

以下是使用简单排序插件的方法：

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


