# UDFs 和插件

Manticore 可以通过用户自定义函数（简称 UDF）进行扩展，方法如下：

```sql
SELECT id, attr1, myudf (attr2, attr3+attr4) ...
```

您可以动态加载和卸载 `searchd` 中的 UDF，而无需重启服务器，并且可以在搜索、排序等操作时在表达式中使用它们。UDF 功能的简要总结如下：

* UDF 可以接受整型（32 位和 64 位）、浮点型、字符串、多值属性（MVA）或 `PACKEDFACTORS()` 参数。
* UDF 可以返回整型、浮点型或字符串值。
* UDF 可以在查询设置阶段检查参数的数量、类型和名称，并且可以抛出错误。

我们还不支持聚合函数。换句话说，您的 UDF 将一次只为单个文档调用，且应返回该文档的某个值。目前还无法编写能够计算整个共享相同 GROUP BY 键的文档组上的聚合值（如 AVG()）的函数。不过，您可以在内置聚合函数中使用 UDF：即使不支持 MYCUSTOMAVG()，使用 AVG(MYCUSTOMFUNC()) 应该可以正常工作！

UDF 具有广泛的应用，包括：

* 集成自定义的数学或字符串函数；
* 从 Manticore 内部访问数据库或文件；
* 创建复杂的排序函数。

## 插件

插件提供了扩展搜索功能的更多机会。目前它们可用于计算自定义排名以及对文档和查询进行分词。

以下是插件类型的完整列表：

* UDF 插件（本质上是 UDF，但因其被插入系统中，也称为“UDF 插件”）
* 排序器插件（ranker plugins）
* 索引时的分词过滤器插件
* 查询时的分词过滤器插件

本节介绍编写和管理插件的一般流程；有关创建不同类型插件的具体内容，请参见各自章节。

那么，您如何编写和使用插件？以下是快速的四步指南：

* 创建一个动态库（.so 或 .dll），通常使用 C 或 C++；
* 使用 [CREATE PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md) 将插件加载到 searchd 中；
* 通过插件特定的调用（通常通过特定的 OPTIONS）使用插件；
* 分别使用 [DROP PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md) 和 [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) 卸载或重新加载插件。

注意，虽然 UDF 是一类重要的插件，但它们是通过单独的 [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) 语句安装的。这样可以简洁指定返回类型，同时保持向后兼容且不改变语法。

动态插件支持在线程和线程池工作线程中运行。多个插件（和/或 UDF）可以包含在单个库文件中。您可以选择将所有项目特定的插件打包成一个大库，也可以为每个 UDF 和插件创建独立库，这取决于您自己。

与 UDF 相同，您应包含 `src/sphinxudf.h` 头文件。至少，您需要 `SPH_UDF_VERSION` 常量来实现合适的版本函数。根据具体插件类型，您可能需要或不需要将插件与 `src/sphinxudf.c` 链接。不过，`sphinxudf.c` 中的所有函数都与解包 `PACKEDFACTORS()` 数据块有关，而任何插件类型都无法访问该数据。因此，目前仅链接头文件就足够了。（事实上，如果您复制了 UDF 版本号，某些插件类型甚至不需要头文件。）

从形式上讲，插件仅是一组符合特定命名模式的 C 函数。通常要求您定义一个用于主要任务的关键函数，但也可以定义额外函数。例如，要实现名为“myrank”的排序器，必须定义一个返回排名值的 `myrank_finalize()` 函数。同时，还可以定义 `myrank_init()`、`myrank_update()` 和 `myrank_deinit()` 函数。不同插件类型的已知后缀和调用参数存在差异，但 _init() 和 _deinit() 是通用的，每个插件都有。提示：有关已知后缀及其参数类型的快速参考，请参见定义了调用原型的 `sphinxplugin.h` 文件开头部分。

尽管公共接口用纯 C 定义，我们的插件基本采用*面向对象模型*。实际上，每个 `_init()` 函数都接收一个 `void ** userdata` 输出参数，存储在 `(*userdata)` 中的指针值作为首个参数传递给所有其他插件函数。因此，您可以将插件视作一个 *类*，每当需要处理请求时都会实例化该类的对象：`userdata` 指针充当 `this` 指针；函数作为方法；`_init()` 和 `_deinit()` 函数分别作为构造函数和析构函数。

这种在 C 语言中稍显复杂的面向对象设计，是因为插件运行在多线程环境中，某些插件需要维护状态。您无法在插件的全局变量中保存该状态，因此我们传递 userdata 参数，从而自然采用 OOP 模型。如果您的插件简单且无状态，接口允许省略 `_init()`，`_deinit()` 及其他函数。

总结，下面是仅用三行 C 代码实现的最简单完整的排序器插件：

```c
// gcc -fPIC -shared -o myrank.so myrank.c
#include "sphinxudf.h"
int myrank_ver() { return SPH_UDF_VERSION; }
int myrank_finalize(void *u, int w) { return 123; }
```

下面是使用该简单排序器插件的方法：

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


