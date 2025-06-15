# 从 XML 流获取数据

`xmlpipe2` 源类型允许以自定义 XML 格式向 Manticore 传递自定义全文和属性数据，模式（即字段和属性的集合）可以在 XML 流本身或源配置中指定。

## 声明 XML 流
声明 XML 流时，`xmlpipe_command` 指令是必需的，包含生成要索引的 XML 流的 shell 命令。它可以是一个文件，也可以是一个动态生成 XML 内容的程序。

## XML 文件格式

在索引 xmlpipe2 源时，索引器运行指定的命令，打开其 stdout 的管道，期望接收格式良好的 XML 流。

下面是 XML 流数据的示例：

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

允许使用任意字段和属性。它们在每个文档内可以以任意顺序出现；顺序会被忽略。字段长度有限制；超过 2MB 的字段会被截断为 2MB（此限制可在源中更改）。

模式，即完整的字段和属性列表，必须在解析任何文档之前声明。可以在配置文件中使用 `xmlpipe_field` 和 `xmlpipe_attr_XXX` 设置声明，也可在流中使用 `<sphinx:schema>` 元素声明。`<sphinx:schema>` 是可选的。它只能作为 `<sphinx:docset>` 的第一个子元素出现。如果流中没有模式定义，将使用配置文件中的设置。否则，流中的设置优先。请注意，文档 ID 应作为 `<sphinx:document>` 标签的 `id` 属性指定（例如 `<sphinx:document id="1235">`），且应该是唯一的带符号正非零 64 位整数。

未声明的标签（既不是字段也不是属性）将被忽略并发出警告。在上述示例中，`<misc>` 会被忽略。所有嵌套的标签及其属性（如上例中 `<subject>` 内的 `<strong>`）将被静默忽略。

对输入流编码的支持取决于系统是否安装了 `iconv`。xmlpipe2 使用 `libexpat` 解析器解析，该解析器本地支持 US-ASCII、ISO-8859-1、UTF-8 以及几种 UTF-16 变体。Manticore 的 `configure` 脚本还会检查 `libiconv` 的存在并利用它来处理其他编码。`libexpat` 还强制要求 Manticore 端使用 UTF-8 字符集，因为它返回的解析数据始终是 UTF-8。

xmlpipe2 识别的 XML 元素（标签）及其属性（如适用）如下：

* `sphinx:docset` - 必须的顶层元素，表示并包含 xmlpipe2 文档集。
* `sphinx:schema` - 可选元素，必须是 sphinx:docset 的第一个子元素或完全不出现。声明文档模式，包含字段和属性声明。存在时，覆盖配置文件中的每个源设置。
* `sphinx:field` - 可选元素，sphinx:schema 的子元素。声明全文字段。已知属性为：
    * "name"，指定将在后续文档中作为全文字段处理的 XML 元素名称。
    * "attr"，指定是否也将该字段作为字符串索引。可能的值是 "string"。
* `sphinx:attr` - 可选元素，sphinx:schema 的子元素。声明属性。已知属性为：
    * "name"，指定将在后续文档中作为属性处理的元素名称。
    * "type"，指定属性类型。可能的值有 "int"、"bigint"、"timestamp"、"bool"、"float"、"multi" 和 "json"。
    * "bits"，指定 "int" 属性类型的位大小。有效值为 1 到 32。
    * "default"，指定如果文档中缺少该属性元素时的默认值。
* `sphinx:document` - 必须元素，必须是 sphinx:docset 的子元素。包含任意其他带有声明的字段和属性值的元素，这些声明通过 sphinx:field 和 sphinx:attr 元素或在配置文件中定义。唯一已知属性是 "id"，必须包含唯一的整数文档 ID。
* `sphinx:killlist` - 可选元素，sphinx:docset 的子元素。包含多个 "id" 元素，其内容是要放入表的 kill-list（屏蔽列表）中的文档 ID。kill-list 用于多表搜索中屏蔽在其他表中找到的文档。

## 在源配置中定义数据

如果 XML 未定义模式，表元素的数据类型必须在源配置中定义。

* `xmlpipe_field` - 声明一个 `text` 字段。
* `xmlpipe_field_string` - 声明一个文本字段/字符串属性。该列将既作为文本字段索引，也作为字符串属性存储。
* `xmlpipe_attr_uint` - 声明一个整数属性。
* `xmlpipe_attr_timestamp` - 声明一个时间戳属性。
* `xmlpipe_attr_bool` - 声明一个布尔属性。
* `xmlpipe_attr_float` - 声明一个浮点数属性。
* `xmlpipe_attr_bigint` - 声明一个大整数属性。
* `xmlpipe_attr_multi` - 声明一个多值整型属性。
* `xmlpipe_attr_multi_64` - 声明一个 64 位整型多值属性。
* `xmlpipe_attr_string` - 声明一个字符串属性。
* `xmlpipe_attr_json` - 声明一个 JSON 属性。

### 特定 XML 源设置

如果设置了 `xmlpipe_fixup_utf8`，将启用 Manticore 端的 UTF-8 验证和过滤，以防止 XML 解析器因非 UTF-8 文档而出错。此选项默认禁用。

在某些情况下，可能很难甚至不可能保证传入的 XMLpipe2 文档正文是完全有效且符合标准的 UTF-8 编码。例如，带有国家单字节编码的文档可能会偷偷进入流中。libexpat XML 解析器较为脆弱，意味着在这种情况下它将停止处理。UTF8 修复功能让你避免这种情况。当启用修复后，Manticore 会在将传入流传递给 XML 解析器之前对其进行预处理，并用空格替换无效的 UTF-8 序列。

```ini
xmlpipe_fixup_utf8 = 1
```

配置中无模式的 XML 源示例：

```ini
source xml_test_1
{
    type = xmlpipe2
    xmlpipe_command = cat /tmp/products_today.xml
}
```

配置中有模式的 XML 源示例：

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

