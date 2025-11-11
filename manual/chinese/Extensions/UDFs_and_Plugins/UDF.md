# UDF

UDF 存储在外部动态库中（UNIX 系统为 .so 文件，Windows 系统为 .dll 文件）。出于安全考虑，库文件必须放置在由 [plugin_dir](../../Server_settings/Common.md#plugin_dir) 指令指定的受信任文件夹中：保护单个文件夹比允许任何人向 `searchd` 安装任意代码更容易。您可以使用 [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) 和 [DROP FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) SQL 语句动态加载和卸载 UDF 到 searchd 中。此外，您还可以使用 [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) 语句无缝重新加载 UDF（及其他插件）。Manticore 会跟踪当前加载的函数；每次创建或删除 UDF 时，`searchd` 会将其状态更新到 [sphinxql_state](../../Server_settings/Searchd.md#sphinxql_state) 文件中，作为普通的 SQL 脚本。

UDF 是本地的。要在集群中使用它们，必须将相同的库放置在所有节点上，并在每个节点上运行 CREATE 语句。此过程在未来版本中可能会有所变化。

成功加载 UDF 后，您可以像使用任何内置函数一样在 SELECT 或其他语句中使用它：

```sql
SELECT id, MYCUSTOMFUNC (groupid, authorname), ... FROM myindex
```

多个 UDF（及其他插件）可以存在于单个库中。该库只会被加载一次，并且在其中所有 UDF 和插件被删除后会自动卸载。

理论上，您可以用任何语言编写 UDF，只要其编译器能导入标准 C 头文件并生成带有正确导出函数的标准动态库。然而，使用 C++ 或纯 C 是最简单的路径。我们提供了一个用纯 C 编写的示例 UDF 库，实现了多个函数（演示了各种技术），与我们的源代码一起，位于 [src/udfexample.c](https://github.com/manticoresoftware/manticore/blob/master/src/udfexample.c)。该示例包含了 [src/sphinxudf.h](https://github.com/manticoresoftware/manticore/blob/master/src/sphinxudf.h) 头文件，其中包含了多个与 UDF 相关的结构和类型定义。对于大多数 UDF 和插件，只需像示例中那样使用 `#include "sphinxudf.h"` 即可。然而，如果您正在编写排名函数并需要从 UDF 内部访问排名信号（因子）数据，还需要编译并链接 `src/sphinxudf.c`（可在我们的源代码中找到），因为允许您从 UDF 内部访问信号数据的函数实现位于该文件中。

`sphinxudf.h` 头文件和 `sphinxudf.c` 是独立的，因此您可以单独复制这些文件；它们不依赖于 Manticore 源代码的其他部分。

在您的 UDF 中，您**必须**实现并导出仅几个函数。首先，为了 UDF 接口版本控制，您**必须**定义一个函数 `int LIBRARYNAME_ver()`，其中 LIBRARYNAME 是您的库文件名，且必须从该函数返回 `SPH_UDF_VERSION`（在 `sphinxudf.h` 中定义的值）。示例如下。

```c
#include <sphinxudf.h>

// our library will be called udfexample.so, thus, so it must define
// a version function named udfexample_ver()
int udfexample_ver()
{
    return SPH_UDF_VERSION;
}
```

此预防措施可防止您意外将接口版本不匹配的库加载到较新或较旧的 `searchd` 中。其次，您**必须**实现实际的函数。

```c
sphinx_int64_t testfunc ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
    return 123;
}
```

SQL 中的 UDF 函数名不区分大小写。但相应的 C 函数名区分大小写；它们必须全部为*小写*，否则 UDF 无法加载。更重要的是，必须确保：

1. 调用约定为 C（即 __cdecl），
2. 参数列表完全符合插件系统的预期，
3. 返回类型与您在 `CREATE FUNCTION` 中指定的类型匹配。

不幸的是，我们无法（轻易地）在加载函数时检查这些错误，这些错误可能导致服务器崩溃和/或产生意外结果。最后，您实现的所有 C 函数都必须是线程安全的。

第一个参数，是指向 `SPH_UDF_INIT` 结构的指针，本质上是指向函数状态的指针。它是可选的。在上面的示例中，该函数是无状态的，因为它每次调用都简单返回 123。因此，我们不必定义初始化函数，可以忽略该参数。
该参数还有一个作用。由于单个查询可以在多个线程上执行（参见 [pseudo-sharding](../../Server_settings/Searchd.md#pseudo_sharding)），守护进程会通过检查该参数来判断 UDF 是有状态还是无状态。如果该参数被初始化，则禁用并行执行。因此，如果您的 UDF 是有状态的但不使用该参数，它将被多个线程调用，您的代码需要对此有所准备。

第二个参数，是指向 `SPH_UDF_ARGS` 的指针，是最重要的参数。所有实际调用参数都通过该结构传递给您的 UDF；它包含调用参数的数量、名称、类型等。因此，无论您的函数是像 `SELECT id, testfunc(1)` 这样调用，还是像 `SELECT id, testfunc('abc', 1000*id+gid, WEIGHT())` 或其他方式调用，它都会收到相同的 `SPH_UDF_ARGS` 结构。但传入 `args` 结构的数据会不同。在第一个示例中，`args->arg_count` 会被设置为 1，在第二个示例中会被设置为 3，`args->arg_types` 数组会包含不同的类型数据，依此类推。

最后，第三个参数是错误标志。UDF 可以设置该标志以指示发生了某种内部错误，UDF 无法继续，查询应提前终止。您**不应**将其用于参数类型检查或其他可能在正常使用中发生的错误报告。该标志设计用于报告突发的严重运行时错误，例如内存耗尽。

如果我们想为函数分配临时存储，或预先检查参数是否为支持的类型，则需要分别添加两个函数，用于 UDF 的初始化和反初始化。

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

注意 `testfunc_init()` 也接收调用参数结构。在它被调用时，不会接收到任何实际值，因此 `args->arg_values` 将为 NULL。但参数名称和类型是已知的，并且会被传递。你可以在初始化函数中检查它们，如果它们是未支持的类型，则返回错误。


## SPH_UDF_ARGS 类型

UDF 可以接收几乎任何有效的内部 Manticore 类型的参数。完整列表请参阅 `sphinxudf.h` 中的 `sphinx_udf_argtype` 枚举。大多数类型都直接映射到相应的 C 类型。

最显著的类型是 `SPH_UDF_TYPE_FACTORS` 参数类型。你可以通过调用带有 [PACKEDFACTOR()](../../Functions/Searching_and_ranking_functions#PACKEDFACTORS()) 参数的 UDF 来获得该类型。它的数据是某种内部格式的二进制块，要从该块中提取单个排名信号，需要使用 `sphinx_factors_XXX()` 或 `sphinx_get_YYY_factor()` 函数族中的任意一个。

### sphinx_factors_XXX() 函数

该函数族包含 3 个函数。

* `sphinx_factors_init()` 初始化解包后的 `SPH_UDF_FACTORS` 结构
* `sphinx_factors_unpack()` 将二进制块解包到 `SPH_UDF_FACTORS` 结构中
* `sphinx_factors_deinit()` 清理并释放 `SPH_UDF_FACTORS`。

首先，你需要调用 `init()` 和 `unpack()`，然后可以使用 `SPH_UDF_FACTORS` 字段，最后需要用 `deinit()` 进行清理。

这种方法简单，但可能导致每个处理的文档产生大量内存分配，可能会较慢。

### sphinx_get_YYY_factor() 函数

另一种接口由一堆 `sphinx_get_YYY_factor()` 函数组成，使用起来稍显冗长，但直接访问二进制块数据，保证不进行分配。为了获得顶级的排名 UDF 性能，你会想使用这种方法。

## UDF 的返回类型

至于返回类型，UDF 目前可以返回单个 INTEGER、BIGINT、FLOAT 或 STRING 值。C 函数返回类型应分别为 `sphinx_int64_t`、`sphinx_int64_t`、`double` 或 `char*`。在最后一种情况下，你**必须**使用 `args->fn_malloc` 函数为返回的字符串值分配空间。在 UDF 内部，你可以使用任何你想要的方式，因此上面 `testfunc_init()` 示例中直接使用 malloc() 是正确的代码：你自己管理该指针，它通过匹配的 free() 调用被释放，一切正常。然而，返回的字符串值由 Manticore 管理，我们有自己的分配器，因此对于返回值，特别需要使用它。

根据你的 UDF 在查询中的使用方式，主函数调用（示例中的 `testfunc()`）可能会以相当不同的数量和顺序被调用。具体来说，

* 在 WHERE、ORDER BY 或 GROUP BY 子句中引用的 UDF 必须且将会对每个匹配的文档进行评估。它们将按照自然匹配顺序被调用。
* 如果没有子查询，可以在最终结果集的最后阶段评估的 UDF 将以这种方式评估，但在应用 `LIMIT` 子句之前。它们将按照结果集顺序被调用。
* 有子查询时，这些 UDF 也将在应用内部 `LIMIT` 子句后进行评估。

其他函数的调用顺序是固定的。即，

* `testfunc_init()` 在初始化查询时调用一次。它可以返回非零代码以指示失败；在这种情况下，查询将被终止，并返回 `error_message` 缓冲区中的错误信息。
* `testfunc()` 在每个符合条件的行（见上文）上调用，每当 Manticore 需要计算 UDF 值时。它也可以通过向 `error_flag` 写入非零字节值来指示（内部）失败错误。在这种情况下，保证不会对后续行调用它，并且会替换为默认返回值 0。Manticore 可能会也可能不会选择提前终止此类查询；目前两种行为都不保证。
* `testfunc_deinit()` 在查询处理（在给定表分片中）结束时调用一次。

<!-- proofread -->

