# UDF

UDF 存储在外部动态库中（UNIX 系统为 .so 文件，Windows 系统为 .dll 文件）。出于安全考虑，库文件必须放置在由 [plugin_dir](../../Server_settings/Common.md#plugin_dir) 指令指定的受信任文件夹中：这样比允许任何人向 `searchd` 安装任意代码更容易保障安全。您可以使用 [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) 和 [DROP FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) SQL 语句动态加载和卸载 UDF 到 searchd。此外，您还可以使用 [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) 语句无缝重新加载 UDF（以及其他插件）。Manticore 会跟踪当前加载的函数；每当您创建或删除 UDF 时，`searchd` 会在 [sphinxql_state](../../Server_settings/Searchd.md#sphinxql_state) 文件中以纯 SQL 脚本的形式更新其状态。

UDF 是本地的。要在集群中使用它们，您必须将相同的库放到所有节点上，并且在每个节点上执行 CREATE 语句。此流程未来版本中可能会有所更改。

一旦您成功加载了 UDF，就可以像使用任何内置函数一样在 SELECT 或其他语句中使用它：

```sql
SELECT id, MYCUSTOMFUNC (groupid, authorname), ... FROM myindex
```

单个库中可以包含多个 UDF（以及其他插件）。该库只会被加载一次，而且一旦库内的所有 UDF 和插件都被删除，库会被自动卸载。

理论上，只要编译器能够导入标准 C 头文件并生成带有正确导出函数的标准动态库，您可以使用任何语言编写 UDF。不过，使用 C++ 或纯 C 是难度最低的路径。我们在源代码中提供了一个用纯 C 编写的示例 UDF 库，实现了多个函数（演示了各种技术），可见 [src/udfexample.c](https://github.com/manticoresoftware/manticore/blob/master/src/udfexample.c)。该示例包含了 [src/sphinxudf.h](https://github.com/manticoresoftware/manticore/blob/master/src/sphinxudf.h) 头文件，里面定义了多个与 UDF 相关的结构体和类型。对于大多数 UDF 和插件，像示例中一样直接使用 `#include "sphinxudf.h"` 通常就足够了。但是，如果您正在编写排名函数并且需要从 UDF 内部访问排名信号（因素）数据，您还需要编译并链接 `src/sphinxudf.c`（源代码中提供），因为访问信号数据的函数实现就在该文件中。

`sphinxudf.h` 头文件和 `sphinxudf.c` 是独立的，您可以单独复制这两个文件；它们不依赖于 Manticore 源代码的其他部分。

在您的 UDF 中，您**必须**只实现和导出几个函数。首先，为了 UDF 接口版本控制，您**必须**定义一个函数 `int LIBRARYNAME_ver()`，其中 LIBRARYNAME 是您的库文件名，并且必须从该函数返回 `SPH_UDF_VERSION`（该值定义在 `sphinxudf.h` 中）。示例如下。

```c
#include <sphinxudf.h>

// our library will be called udfexample.so, thus, so it must define
// a version function named udfexample_ver()
int udfexample_ver()
{
    return SPH_UDF_VERSION;
}
```

此措施可防止您误将与 `searchd` 版本接口不匹配的库加载进新版或旧版 `searchd`。其次，您**必须**实现实际的函数本身。

```c
sphinx_int64_t testfunc ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
    return 123;
}
```

SQL 中的 UDF 函数名不区分大小写。但相应的 C 函数名区分大小写；它们必须全部*小写*，否则 UDF 无法加载。更重要的是，确保：

1. 调用约定为 C（即 __cdecl），
2. 参数列表完全符合插件系统的预期，
3. 返回类型与您在 `CREATE FUNCTION` 中指定的类型相匹配。

遗憾的是，加载函数时我们无法（轻易）检查这些错误，这可能会导致服务器崩溃或产生意外结果。最后，您实现的所有 C 函数都必须是线程安全的。

第一个参数，是指向 `SPH_UDF_INIT` 结构的指针，本质上是我们的函数状态指针。该参数是可选的。在上例中，该函数是无状态的，因为每次调用都简单返回 123。因此，我们无需定义初始化函数，可以直接忽略该参数。
该参数还有另一个目的。因为单个查询可能会在多个线程上执行（参见[伪分片](../../Server_settings/Searchd.md#pseudo_sharding)），守护进程通过检查此参数来判断 UDF 是有状态还是无状态。如果参数被初始化，平行执行将被禁用。因此，如果您的 UDF 是有状态的但没有使用该参数，它将在多个线程中被调用，您的代码需要对此有意识。

第二个参数，是指向 `SPH_UDF_ARGS` 的指针，是最重要的参数。所有实际调用参数都通过该结构传递；它包含调用参数数量、名称、类型等信息。所以无论您的函数是以 `SELECT id, testfunc(1)` 形式调用，还是以 `SELECT id, testfunc('abc', 1000*id+gid, WEIGHT())` 或其他形式调用，这些情况下传入的 `SPH_UDF_ARGS` 结构是相同的。但传入 `args` 结构中的数据会不同。例如，第一个示例中 `args->arg_count` 为 1，第二个示例中为 3，`args->arg_types` 数组中也会包含不同的类型信息，依此类推。

最后，第三个参数是错误标志。UDF 可以设置该标志以指示发生了某种内部错误，UDF 无法继续，查询应提前终止。您**不应**用于参数类型检查或其他正常使用过程中可能发生的错误报告。该标志设计用于报告突发的关键运行时错误，例如内存不足。

如果我们需要为函数分配临时存储，或者预先检查参数是否为支持的类型，就需要额外添加两个函数，分别用于 UDF 的初始化和去初始化。

```c
int testfunc_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args,
    char * error_message )
{
    // allocate and initialize a little bit of temporary storage
    init->func_data = malloc ( sizeof(int) );
    *(int*)init->func_data = 123;

    // return a success code
    return 0;
}

void testfunc_deinit ( SPH_UDF_INIT * init )
{
    // free up our temporary storage
    free ( init->func_data );
}
```

注意 `testfunc_init()` 也会接收调用参数结构。在它被调用时，不会接收到任何实际的值，因此 `args->arg_values` 将是 NULL。但参数名称和类型是已知的，并会被传递。你可以在初始化函数中检查它们，如果它们是未支持的类型，可以返回错误。


## SPH_UDF_ARGS 类型

UDF 可以接收几乎任何有效的内部 Manticore 类型的参数。完整列表请参考 `sphinxudf.h` 中的 `sphinx_udf_argtype` 枚举。大多数类型都可以直接映射到相应的 C 类型。

最显著的类型是 `SPH_UDF_TYPE_FACTORS` 参数类型。你可以通过调用带有 [PACKEDFACTOR()](../../Functions/Searching_and_ranking_functions#PACKEDFACTORS()) 参数的 UDF 来获得该类型。它的数据是某种内部格式的二进制块，若要从该二进制块中提取各个排名信号，需要使用两组函数之一：`sphinx_factors_XXX()` 或 `sphinx_get_YYY_factor()` 系列函数。

### sphinx_factors_XXX() 函数

该系列包含3个函数。

* `sphinx_factors_init()` 初始化解包后的 `SPH_UDF_FACTORS` 结构体
* `sphinx_factors_unpack()` 将二进制块解包到 `SPH_UDF_FACTORS` 结构体中
* `sphinx_factors_deinit()` 清理并释放 `SPH_UDF_FACTORS` 结构体。

首先，你需要调用 `init()` 和 `unpack()`，然后可以使用 `SPH_UDF_FACTORS` 的字段，最后需要使用 `deinit()` 进行清理。

这种方法简单，但可能会导致每处理一个文档就产生一堆内存分配，这可能很慢。

### sphinx_get_YYY_factor() 函数

另一种接口由多个 `sphinx_get_YYY_factor()` 函数组成，使用起来稍显冗长，但它直接访问二进制数据且保证不进行内存分配。为了获得顶级排名 UDF 性能，推荐使用这种方法。

## UDF 的返回类型

至于返回类型，目前 UDF 可以返回单个 INTEGER、BIGINT、FLOAT 或 STRING 值。C 函数返回类型应分别为 `sphinx_int64_t`、`sphinx_int64_t`、`double` 或 `char*`。在最后一种情况下，你**必须**使用 `args->fn_malloc` 函数为返回的字符串值分配空间。在 UDF 内部，你可以自由使用任何方式，因此上面 `testfunc_init()` 的示例代码即使直接使用 malloc() 也是正确的：你管理那个指针，匹配的 free() 调用会释放它，一切正常。然而，返回的字符串值由 Manticore 管理，我们有自己的分配器，所以针对返回值，必须使用它。

根据 UDF 在查询中的使用方式，主函数调用（在我们的示例中为 `testfunc()`）的调用量和顺序可能有很大不同。具体来说：

* 在 WHERE、ORDER BY 或 GROUP BY 子句中引用的 UDF 必须且会对每个匹配的文档评估。它们将按照自然匹配顺序调用。
* 在无子查询时，可以在最终结果集的最后阶段评估的 UDF 将以该方式评估，但发生在应用 `LIMIT` 子句之前。它们将按照结果集顺序调用。
* 有子查询时，此类 UDF 也将在应用内层的 `LIMIT` 子句后进行评估。

其它函数的调用顺序是固定的。具体为：

* `testfunc_init()` 在初始化查询时只调用一次。它可以返回非零代码表示失败；此时查询将终止，并返回 `error_message` 缓冲区中的错误信息。
* `testfunc()` 在每个符合条件的行（见上文）被调用，每当 Manticore 需要计算 UDF 值时。它也可以通过向 `error_flag` 写入非零字节值表示（内部）失败错误。在此情况下，保证后续行不会再调用该函数，且会用默认返回值0代替。Manticore 可能会或可能不会提前终止此类查询；目前此行为未保证。
* `testfunc_deinit()` 在查询处理结束时（针对特定表分片）只调用一次。

<!-- proofread -->

