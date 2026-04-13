# 从 XML 流获取数据

`xmlpipe2` 源类型允许用自定义的 XML 格式将自定义全文和属性数据传递给 Manticore，模式（即字段和属性集）可以在 XML 流本身或源设置中指定。

## XML 流声明
要声明 XML 流，`xmlpipe_command` 指令是必须的，它包含生成要索引的 XML 流的 shell 命令。该命令可以是一个文件，也可以是一个实时生成 XML 内容的程序。

## XML 文件格式

当索引 xmlpipe2 源时，索引器运行指定的命令，打开到其 stdout 的管道，并期望获得格式良好的 XML 流。

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

允许任意字段和属性。它们在流中每个文档内的顺序也可以是任意的；顺序会被忽略。字段长度有限制；超过 2 MB 的字段将被截断到 2 MB（此限制可在源中更改）。

模式，即完整的字段和属性列表，必须在任何文档解析之前声明。可以通过配置文件中的 `xmlpipe_field` 和 `xmlpipe_attr_XXX` 设置声明，也可以直接在流中使用 `<sphinx:schema>` 元素声明。`<sphinx:schema>` 是可选的。它只能作为 `<sphinx:docset>` 中的第一个子元素出现。如果流中没有模式定义，则使用配置文件中的设置。否则，流中的设置优先。注意文档 ID 应作为 `<sphinx:document>` 标签的属性 `id` 指定（例如 `<sphinx:document id="1235">`），并且应是唯一的正的非零 64 位整数。

未知标签（既未声明为字段也未声明为属性）将被忽略并发出警告。上面示例中的 `<misc>` 将被忽略。所有嵌套的标签及其属性（如上例中 `<subject>` 中的 `<strong>`）将被静默忽略。

对输入流编码的支持取决于系统上是否安装了 `iconv`。xmlpipe2 使用 `libexpat` 解析器解析，它原生支持 US-ASCII、ISO-8859-1、UTF-8 和几种 UTF-16 变体。Manticore 的 `configure` 脚本还会检测 `libiconv` 的存在，并利用其处理其他编码。`libexpat` 还强制要求 Manticore 侧使用 UTF-8 字符集，因为它返回的解析数据总是 UTF-8。

xmlpipe2 识别的 XML 元素（标签）及其适用的属性有：

* `sphinx:docset` - 必须的顶层元素，表示并包含 xmlpipe2 文档集。
* `sphinx:schema` - 可选元素，必须是 sphinx:docset 的第一个子元素或根本不出现。声明文档模式，包含字段和属性声明。如果存在，它将覆盖配置文件中的每个源设置。
* `sphinx:field` - 可选元素，sphinx:schema 的子元素。声明全文字段。已知属性：
    * "name"，指定将被视为全文字段的 XML 元素名。
    * "attr"，指定是否还将此字段索引为字符串。可能的值为 "string"。
* `sphinx:attr` - 可选元素，sphinx:schema 的子元素。声明属性。已知属性：
    * "name"，指定应被视为文档属性的元素名。
    * "type"，指定属性类型。可能的值有 "int"、"bigint"、"timestamp"、"bool"、"float"、"multi" 和 "json"。
    * "bits"，指定 "int" 类型属性的位数。有效值为 1 到 32。
    * "default"，指定如果文档中不存在该属性元素时应使用的默认值。
* `sphinx:document` - 必须元素，必须是 sphinx:docset 的子元素。包含声明的字段和属性值的任意其他元素，用于索引。声明方式要么使用 sphinx:field 和 sphinx:attr 元素，要么在配置文件中声明。唯一已知的属性是 "id"，必须包含唯一的整数文档 ID。
* `sphinx:killlist` - 可选元素，sphinx:docset 的子元素。包含若干 "id" 元素，其内容是属于待从表中删除的文档 ID。杀死列表用于多表搜索中抑制搜索结果中其他表中的文档。

## 源配置的数据定义

如果 XML 没有定义模式，表元素的数据类型必须在源配置中定义。

* `xmlpipe_field` - 声明一个 `text` 字段。
* `xmlpipe_field_string` - 声明一个文本字段/字符串属性。该列既被索引为文本字段，也被存储为字符串属性。
* `xmlpipe_attr_uint` - 声明一个整数属性。
* `xmlpipe_attr_timestamp` - 声明一个时间戳属性。
* `xmlpipe_attr_bool` - 声明一个布尔属性。
* `xmlpipe_attr_float` - 声明一个浮点属性。
* `xmlpipe_attr_bigint` - 声明一个大整数属性。
* `xmlpipe_attr_multi` - 声明一个多值整数属性。
* `xmlpipe_attr_multi_64` - 声明一个 64 位整数的多值属性。
* `xmlpipe_attr_string` - 声明一个字符串属性。
* `xmlpipe_attr_json` - 声明一个 JSON 属性。

### 特定 XML 源设置

如果设置了 `xmlpipe_fixup_utf8`，将启用 Manticore 端的 UTF-8 验证和过滤，以防止 XML 解析器因非 UTF-8 文档而出错。默认情况下，此选项禁用。

在某些情况下，可能很难甚至不可能保证传入的 XMLpipe2 文档主体是完全有效且符合规范的 UTF-8 编码。例如，带有国家单字节编码的文档可能会混入数据流中。libexpat XML 解析器比较脆弱，意味着在这种情况下它会停止处理。UTF8 修正功能可以让你避免这种情况。当启用修正时，Manticore 会在将传入流传递给 XML 解析器之前对其进行预处理，并将无效的 UTF-8 序列替换为空格。

```ini
xmlpipe_fixup_utf8 = 1
```

未在配置中定义模式的 XML 源示例：

```ini
source xml_test_1
{
    type = xmlpipe2
    xmlpipe_command = cat /tmp/products_today.xml
}
```

在配置中定义了模式的 XML 源示例：

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

