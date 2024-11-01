# 从 XML 流中获取数据

`xmlpipe2` 源类型允许将自定义的全文和属性数据以自定义 XML 格式传递给 Manticore，模式（即字段和属性的集合）可以在 XML 流中指定，也可以在源设置中指定。

## 声明 XML 流

声明 XML 流时，`xmlpipe_command` 指令是必需的，包含了生成要索引的 XML 流的 shell 命令。该命令可以读取文件，也可以是一个动态生成 XML 内容的程序。

## XML 文件格式

在索引 `xmlpipe2` 源时，索引器运行指定的命令，打开一个管道连接到其标准输出，并期望接收格式正确的 XML 流。

以下是 XML 流数据的示例：

```xml
<?xml version="1.0" encoding="utf-8"?>
<sphinx:docset>

<sphinx:schema>
<sphinx:field name="subject"/>
<sphinx:field name="content"/>
<sphinx:attr name="published" type="timestamp"/>
<sphinx:attr name="author_id" type="int" bits="16" default="1"/>
</sphinx:schema>

<sphinx:document id="1234">
<content>this is the main content <![CDATA[and this <cdata> entry
must be handled properly by xml parser lib]]></content>
<published>1012325463</published>
<subject>note how field/attr tags can be
in <b> class="red">randomized</b> order</subject>
<misc>some undeclared element</misc>
</sphinx:document>

<sphinx:document id="1235">
<subject>another subject</subject>
<content>here comes another document, and i am given to understand,
that in-document field order must not matter, sir</content>
<published>1012325467</published>
</sphinx:document>

<!-- ... even more sphinx:document entries here ... -->

<sphinx:killlist>
<id>1234</id>
<id>4567</id>
</sphinx:killlist>

</sphinx:docset>
```

任意字段和属性都是允许的。它们可以在每个文档内的任意顺序中出现，顺序会被忽略。字段长度有限制，超过 2 MB 的字段将会被截断至 2 MB（此限制可以在源中更改）。

在解析任何文档之前，必须声明模式，即完整的字段和属性列表。可以通过在配置文件中使用 `xmlpipe_field` 和 `xmlpipe_attr_XXX` 设置，或直接在流中使用 `<sphinx:schema>` 元素来完成声明。`<sphinx:schema>` 是可选的，只允许作为 `<sphinx:docset>` 的第一个子元素出现。如果流中没有定义模式，将使用配置文件中的设置，否则流中的设置优先。请注意，文档 ID 应该作为 `<sphinx:document>` 标签的 `id` 属性来指定（例如 `<sphinx:document id="1235">`），并且应为唯一的带符号正整数，64 位且非零。

未知的标签（未声明为字段或属性的标签）将被忽略，并伴有警告。例如，上述例子中的 `<misc>` 将被忽略。所有嵌入的标签及其属性（如例子中的 `<subject>` 中的 `<strong>`）将被静默忽略。

对传入流编码的支持取决于系统是否安装了 `iconv`。`xmlpipe2` 使用 `libexpat` 解析器进行解析，`libexpat` 原生支持 US-ASCII、ISO-8859-1、UTF-8 以及一些 UTF-16 变体。Manticore 的 `configure` 脚本还会检查是否存在 `libiconv`，并利用它来处理其他编码格式。`libexpat` 还强制要求 Manticore 端使用 UTF-8 字符集，因为它返回的解析数据始终为 UTF-8。

xmlpipe2 支持的 XML 元素（及其适用时的属性）如下：

* `sphinx:docset` -  必需的顶级元素，表示并包含 xmlpipe2 文档集。
* `sphinx:schema` -  可选元素，必须作为 `sphinx:docset` 的第一个子元素出现，否则不得出现。声明文档模式并包含字段和属性声明。如果存在，它将覆盖配置文件中的每个源设置。
* `sphinx:field` - 可选元素，`sphinx:schema` 的子元素。声明一个全文字段。已知属性：
    *   `"name"`，指定将作为全文字段处理的 XML 元素名称。
    *   `"attr"`，指定是否将此字段作为字符串索引。可能的值为 "string"。
* `sphinx:attr` - 可选元素，`sphinx:schema` 的子元素。声明一个属性。已知属性：
    *   `"name"`，指定在后续文档中作为属性处理的元素名称。
    *   `"type"`，指定属性类型。可能的值为 "int"、"bigint"、"timestamp"、"bool"、"float"、"multi" 和 "json"。
    *   `"bits"`，指定 "int" 属性类型的位大小。有效值为 1 到 32。
    *   `"default"`，指定当文档中缺少该属性时使用的默认值。
* `sphinx:document` - 必需元素，必须为 `sphinx:docset` 的子元素。包含将被索引的字段和属性值，已在 `sphinx:field` 和 `sphinx:attr` 元素中或配置文件中声明。唯一已知的属性是 "id"，必须包含唯一的整数文档 ID。
* `sphinx:killlist` - 可选元素，`sphinx:docset` 的子元素。包含若干 "id" 元素，其内容为需要放入表的 kill-list 中的文档 ID。kill-list 在多表搜索中用于抑制在其他表中找到的文档。

## 在源配置中定义数据

如果 XML 没有定义模式，表元素的数据类型必须在源配置中定义。

- `xmlpipe_field` - 声明一个 `text` 字段。
- `xmlpipe_field_string` - 声明一个文本字段/字符串属性。该列既作为文本字段索引，也作为字符串属性存储。
- `xmlpipe_attr_uint` - 声明一个整数属性。
- `xmlpipe_attr_timestamp` - 声明一个时间戳属性。
- `xmlpipe_attr_bool` - 声明一个布尔属性。
- `xmlpipe_attr_float` - 声明一个浮点属性。
- `xmlpipe_attr_bigint` - 声明一个大整数属性。
- `xmlpipe_attr_multi` - 声明一个带有整数的多值属性。
- `xmlpipe_attr_multi_64` - 声明一个带有 64 位整数的多值属性。
- `xmlpipe_attr_string` - 声明一个字符串属性。
- `xmlpipe_attr_json` - 声明一个 JSON 属性。

### 特定的 XML 源设置

如果设置了 `xmlpipe_fixup_utf8`，将启用 Manticore 端的 UTF-8 验证和过滤，以防止 XML 解析器因非 UTF-8 文档崩溃。默认情况下，此选项是禁用的。

在某些情况下，可能很难甚至无法保证传入的 XMLpipe2 文档主体是完全有效且符合 UTF-8 编码的。例如，使用单字节编码的文档可能会混入流中。`libexpat` XML 解析器相对脆弱，遇到这种情况时将停止处理。启用 UTF-8 修复功能可以避免这种情况。当启用修复时，Manticore 将在将传入流传递给 XML 解析器之前进行预处理，并将无效的 UTF-8 序列替换为空格。

```ini
xmlpipe_fixup_utf8 = 1
```

没有在配置中定义模式的 XML 源示例：

```ini
source xml_test_1
{
    type = xmlpipe2
    xmlpipe_command = cat /tmp/products_today.xml
}
```

在配置中定义模式的 XML 源示例：

```ini
source xml_test_2
{
    type = xmlpipe2
    xmlpipe_command = cat /tmp/products_today.xml
    xmlpipe_field = subject
    xmlpipe_field = content
    xmlpipe_attr_timestamp = published
    xmlpipe_attr_uint = author_id:16
}
```
<!-- proofread -->