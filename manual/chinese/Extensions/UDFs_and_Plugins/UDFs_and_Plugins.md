# UDFs和插件

Manticore可以通过用户定义函数（简称UDF）进行扩展，如下所示：

```sql
SELECT id, attr1, myudf (attr2, attr3+attr4) ...
```

您可以在不重启服务器的情况下动态加载和卸载UDF到`searchd`中，并在搜索、排名等操作时使用它们。UDF的一些特点如下：

* UDF可以接受整数（32位和64位）、浮点数、字符串、MVA或`PACKEDFACTORS()`参数。
* UDF可以返回整数、浮点数或字符串值。
* UDF可以在查询设置阶段检查参数数量、类型和名称，并在必要时抛出错误。

我们尚未支持聚合函数。换句话说，您的UDF将仅针对单个文档调用，并且期望为该文档返回某些值。编写一个可以计算整个具有相同GROUP BY键的文档组的聚合值（如AVG()）的函数目前是不可能的。但是，您可以在内置聚合函数中使用UDF：也就是说，即使MYCUSTOMAVG()尚未支持，AVG(MYCUSTOMFUNC())也应该可以正常工作！

UDF提供了广泛的应用场景，例如：

* 集成自定义数学或字符串函数；
* 从Manticore访问数据库或文件；
* 创建复杂的排名函数。

## 插件

插件提供了扩展搜索功能的额外机会。它们当前可用于计算自定义排名和分词文档和查询。

以下是插件类型的完整列表：

* UDF插件（本质上是UDF，但由于它们是插件，因此也被称为“UDF插件”）
* 排名插件
* 索引时分词过滤器插件
* 查询时分词过滤器插件

本节涵盖了编写和管理插件的一般过程；有关创建不同类型的插件的具体细节将在各自的子节中讨论。

那么，如何编写和使用插件？这里有一个快速的四步指南：

* 创建一个动态库（可能是.so或.dll），最有可能使用C或C++；
* 使用[CREATE PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Creating_a_plugin.md)将插件加载到searchd中；
* 使用插件特定的调用（通常通过特定的OPTIONS）使用插件；
* 使用[DROP PLUGIN](../../Extensions/UDFs_and_Plugins/Plugins/Deleting_a_plugin.md)卸载或使用[RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md)重新加载插件。

请注意，虽然UDF是一等插件，但它们使用单独的[CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md)语句安装。这允许简洁地指定返回类型，而不牺牲向后兼容性或更改语法。

动态插件在多线程和thread_pool工作线程中受支持。一个库文件中可以包含多个插件（和/或UDF）。您可以选择将所有项目特定的插件放在一个大型库中，或者为每个UDF和插件创建单独的库；这取决于您。

与UDF一样，您应该包含`src/sphinxudf.h`头文件。至少，您需要`SPH_UDF_VERSION`常量来实现适当版本的函数。根据特定的插件类型，您可能或可能不需要将插件链接到`src/sphinxudf.c`。但是，`sphinxudf.c`中实现的所有函数都与`PACKEDFACTORS()`的解包有关，而没有任何插件类型可以访问这些数据。因此，目前仅链接头文件就足够了。（事实上，如果您复制了UDF版本号，您甚至不需要头文件来实现某些插件类型。）

形式上，插件是一组遵循特定命名模式的C函数。通常，您需要定义一个关键函数来执行主要任务，但也可以定义其他函数。例如，要实现一个名为“myrank”的排名器，您必须定义一个`myrank_finalize()`函数，该函数返回排名值。但是，您也可以定义`myrank_init()`、`myrank_update()`和`myrank_deinit()`函数。特定的已知后缀集和调用参数根据插件类型而异，但_init()和_deinit()是通用的，每个插件都有它们。提示：要快速参考已知的后缀及其参数类型，请参阅`sphinxplugin.h`，其中在文件开头定义了调用原型。

尽管公共接口是纯C定义的，但我们的插件实际上遵循一种*面向对象模型*。确实，每个_init()函数接收一个`void ** userdata`输出参数，然后将存储在`(*userdata)`中的指针值作为所有其他插件函数的第一个参数传递。因此，您可以将插件视为每次需要处理请求的对象的*类*实例化：userdata指针充当this指针；函数作为方法，而_init()和_deinit()函数分别作为构造函数和析构函数。

由于插件运行在多线程环境中，且某些插件需要维护状态，因此这种面向对象的C语言微小复杂性是因为您不能在插件中使用全局变量来存储状态，所以我们传递userdata参数，自然导致面向对象模型。如果您的插件简单且无状态，接口允许您省略_init()、_deinit()和其他任何函数。

总之，这里是一个最简单的完整排名器插件，仅用三行C代码实现：

```c
// gcc -fPIC -shared -o myrank.so myrank.c
#include "sphinxudf.h"
int myrank_ver() { return SPH_UDF_VERSION; }
int myrank_finalize(void *u, int w) { return 123; }
```

这是如何使用简单的排名器插件：

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


