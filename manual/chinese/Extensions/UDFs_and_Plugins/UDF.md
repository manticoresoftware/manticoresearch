# UDF

UDF（用户定义函数）存储在外部动态库中（UNIX 上为 .so 文件，Windows 系统上为 .dll 文件）。库文件必须放在 [plugin_dir](../../Server_settings/Common.md#plugin_dir) 指令指定的受信任文件夹中以确保安全：保护一个文件夹比允许任何人安装任意代码到 `searchd` 更容易。可以使用 [CREATE FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Creating_a_function.md) 和 [DROP FUNCTION](../../Extensions/UDFs_and_Plugins/UDF/Deleting_a_function.md) SQL 语句动态加载和卸载 UDF。此外，可以使用 [RELOAD PLUGINS](../../Extensions/UDFs_and_Plugins/Plugins/Reloading_plugins.md) 语句无缝重新加载 UDF（和其他插件）。Manticore 跟踪当前加载的函数；每次创建或删除 UDF 时，`searchd` 会在 [sphinxql_state](../../Server_settings/Searchd.md#sphinxql_state) 文件中以普通 SQL 脚本更新其状态。

UDF 是本地的。要在集群上使用它们，必须在所有节点上放置相同的库，并在每个节点上运行 CREATE 语句。此过程在未来版本中可能会变化。

一旦成功加载 UDF，就可以像使用任何内置函数一样在 SELECT 或其他语句中使用它：

```sql
SELECT id, MYCUSTOMFUNC (groupid, authorname), ... FROM myindex
```

多个 UDF（和其他插件）可以驻留在一个库中。该库只会加载一次，并且在其中的所有 UDF 和插件被删除后会自动卸载。

理论上，可以用任何语言编写 UDF，只要其编译器能导入标准 C 头文件并生成具有正确导出函数的标准动态库。然而，用 C++ 或纯 C 编写是最简单的方式。我们提供了一个用纯 C 编写的 UDF 库示例，该示例实现了多个函数（展示了各种技术），以及我们的源代码，位于 [src/udfexample.c](https://github.com/manticoresoftware/manticore/blob/master/src/udfexample.c)。此示例包含 [src/sphinxudf.h](https://github.com/manticoresoftware/manticore/blob/master/src/sphinxudf.h) 头文件，该文件包含几个与 UDF 相关的结构和类型的定义。对于大多数 UDF 和插件，只需像示例中那样使用 `#include "sphinxudf.h"` 即可。但如果您正在编写排行函数并需要访问 UDF 内部的排行信号（因素）数据，您还需要编译并链接 `src/sphinxudf.c`（可在我们的源代码中找到），因为允许您从 UDF 内部访问信号数据的函数*实现*位于该文件中。

`sphinxudf.h` 头文件和 `sphinxudf.c` 是独立的，因此您可以单独复制这些文件；它们不依赖于 Manticore 源代码的其他部分。

在您的 UDF 中，您 **必须** 实现并导出仅以下几个函数。首先，出于 UDF 接口版本控制，您 **必须** 定义一个名为 `int LIBRARYNAME_ver()` 的函数，其中 LIBRARYNAME 是您的库文件的名称，并且您必须从中返回 `SPH_UDF_VERSION`（在 `sphinxudf.h` 中定义的值）。下面是一个示例。

```c
#include <sphinxudf.h>

// 我们的库将被称为 udfexample.so，因此，它必须定义
// 一个名为 udfexample_ver() 的版本函数
int udfexample_ver()
{
    return SPH_UDF_VERSION;
}
```

这个预防措施保护您免于意外加载与较新或较旧的 `searchd` 不匹配的 UDF 接口版本的库。其次，您 **必须** 实现实际的函数。

```c
sphinx_int64_t testfunc ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
    return 123;
}
```

SQL 中的 UDF 函数名称不区分大小写。但是，相应的 C 函数名称不然；它们需要全部为 *小写*，否则 UDF 将无法加载。更重要的是，确保以下几点至关重要：

1. 调用约定为 C（即 __cdecl），
2. 参数列表完全符合插件系统期望，
3. 返回类型与您在 `CREATE FUNCTION` 中指定的返回类型一致。

不幸的是，加载函数时我们没有（简单的）办法检查这些错误，它们可能会使服务器崩溃和/或导致意外结果。最后但同样重要的是，您实现的所有 C 函数都需要是线程安全的。

第一个参数，指向 `SPH_UDF_INIT` 结构的指针，本质上是指向我们函数状态的指针。它是可选的。在上面的示例中，该函数是无状态的，因为它每次被调用时都仅返回 123。因此，我们不必定义初始化函数，我们可以简单地忽略该参数。
此参数还用于另一个目的。由于单个查询可以在多个线程上执行（请参见 [pseudo-sharding](../Server_settings/Searchd.md#pseudo_sharding)），守护进程试图通过检查此参数来确定 UDF 是有状态还是无状态。如果参数已初始化，将禁用并行执行。因此，如果您的 UDF 是有状态的但您不使用此参数，它将从多个线程中被调用，您的代码需要对此有所了解。

第二个参数，指向 `SPH_UDF_ARGS` 的指针，是最重要的参数。所有实际调用参数都是通过此结构传递给您的 UDF；它包含调用参数计数、名称、类型等。因此，无论您的函数是像 `SELECT id, testfunc(1)` 还是像 `SELECT id, testfunc('abc', 1000*id+gid, WEIGHT())` 被调用，它都将在所有这些情况下接收相同的 `SPH_UDF_ARGS` 结构。然而，传递到 `args` 结构中的数据将是不同的。在第一个示例中，`args->arg_count` 将被设置为 1，在第二个示例中将设置为 3，并且 `args->arg_types` 数组将包含不同的类型数据，等等。
最后，第三个参数是一个错误标志。UDF可以引发它来指示发生了某种内部错误，UDF无法继续，查询应该提前终止。你不应该将其用于参数类型检查或任何可能在正常使用期间发生的其他错误报告。此标志旨在报告突发的关键运行时错误，如内存耗尽。

如果我们想要分配临时存储以供函数使用，或提前检查参数是否为支持的类型，那么我们需要添加另外两个函数，分别用于UDF的初始化和反初始化。

```c
int testfunc_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args,
    char * error_message )
{
    // 分配并初始化一点临时存储
    init->func_data = malloc ( sizeof(int) );
    *(int*)init->func_data = 123;

    // 返回成功代码
    return 0;
}

void testfunc_deinit ( SPH_UDF_INIT * init )
{
    // 释放临时存储
    free ( init->func_data );
}
```

注意 `testfunc_init()` 也接收调用参数结构。在调用时，它不会收到任何实际值，因此 `args->arg_values` 将为NULL。但参数名称和类型是已知的并将被传递。你可以在初始化函数中检查它们，并在它们是不支持的类型时返回错误。


## SPH_UDF_ARGS 类型

UDF可以接收几乎任何有效的内部Manticore类型。请参阅 `sphinxudf.h` 中的 `sphinx_udf_argtype` 枚举以获取完整列表。大多数类型直接映射到相应的C类型。

最值得注意的类型是 `SPH_UDF_TYPE_FACTORS` 参数类型。通过使用 [PACKEDFACTOR()](../../searching-and-ranking-functions#PACKEDFACTORS()) 参数调用你的UDF来获取该类型。其数据是特定内部格式的二进制 blob，要从该 blob 中提取单个排名信号，你需要使用 `sphinx_factors_XXX()` 或 `sphinx_get_YYY_factor()` 系列函数中的任何一个。

### sphinx_factors_XXX() 函数

这个系列由3个函数组成。

* `sphinx_factors_init()` 初始化解包的 `SPH_UDF_FACTORS` 结构
* `sphinx_factors_unpack()` 将二进制 blob 解包到 `SPH_UDF_FACTORS` 结构
* `sphinx_factors_deinit()` 清理并释放 `SPH_UDF_FACTORS`。

首先，你需要调用 `init()` 和 `unpack()`，然后可以使用 `SPH_UDF_FACTORS` 字段，最后需要使用 `deinit()` 进行清理。

这种方法简单，但可能导致每个处理文档都进行大量内存分配，这可能会很慢。

### sphinx_get_YYY_factor() 函数

另一个接口，由一系列 `sphinx_get_YYY_factor()` 函数组成，使用起来更冗长，但直接访问 blob 数据并保证不分配内存。对于顶级性能的排名UDF，你会想使用这种方法。

## UDF 的返回类型

关于返回类型，UDF目前可以返回单个INTEGER、BIGINT、FLOAT或STRING值。C函数返回类型应分别为 `sphinx_int64_t`、`sphinx_int64_t`、`double` 或 `char*`。在最后一种情况下，你必须使用 `args->fn_malloc` 函数为返回的字符串值分配空间。在UDF内部，你可以使用任何你想要的方式，所以上面的 `testfunc_init()` 示例即使直接使用 malloc() 也是正确的代码：你自己管理那个指针，使用匹配的 free() 调用释放，一切都很好。但是，返回的字符串值由Manticore管理，我们有自己的分配器，所以对于返回值，你也需要使用它。

根据UDF在查询中的使用方式，主函数调用（我们示例中的 `testfunc()`）可能会以非常不同的卷和顺序被调用。具体来说，

* 在WHERE、ORDER BY或GROUP BY子句中引用的UDF必须并将为每个匹配的文档进行评估。它们将按自然匹配顺序被调用。
* 没有子选择的情况下，可以在最后阶段评估的UDF将以这种方式评估，但在应用 `LIMIT` 子句之前。它们将按结果集顺序被调用。
* 对于子选择，此类UDF也将在应用内部 `LIMIT` 子句后进行评估。

其他函数的调用顺序是固定的。具体而言，

* `testfunc_init()` 在初始化查询时调用一次。它可以返回非零代码以指示失败；在这种情况下，查询将终止，并返回 `error_message` 缓冲区中的错误消息。
* `testfunc()` 对于每个符合条件的行（如上所述）都会被调用，每当Manticore需要计算UDF值时。它还可以通过向 `error_flag` 写入非零字节值来指示（内部）失败错误。在这种情况下，保证不会为后续行调用它，并将替换默认返回值0。Manticore可能会或可能不会选择提前终止此类查询；目前不保证任何行为。
* `testfunc_deinit()` 在查询处理（在给定表分片中）结束时调用一次。

<!-- proofread -->

