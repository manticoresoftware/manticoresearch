# 从 XML 流中获取数据

`xmlpipe2` 源类型允许以自定义 XML 格式将自定义全文和属性数据传递给 Manticore，模式（即字段和属性的集合）可以在 XML 流本身或源设置中指定。

## XML 流声明
要声明 XML 流，`xmlpipe_command` 指令是强制性的，并包含生成要索引的 XML 流的 shell 命令。这可以是一个文件，也可以是一个动态生成 XML 内容的程序。

## XML 文件格式

在索引 xmlpipe2 源时，索引器运行指定的命令，打开到其 stdout 的管道，并期望一个格式良好的 XML 流。 

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
<content>这是主要内容 <![CDATA[而这个 <cdata> 条目
必须被 XML 解析库正确处理]]></content>
<published>1012325463</published>
<subject>注意字段/属性标签可以
以 <b> class="red">随机</b> 顺序出现</subject>
<misc>一些未声明的元素</misc>
</sphinx:document>

<sphinx:document id="1235">
<subject>另一个主题</subject>
<content>这里是另一个文档，我理解到，
文档内字段的顺序不重要，先生</content>
<published>1012325467</published>
</sphinx:document>

<!-- ... 这里还有更多 sphinx:document 条目 ... -->

<sphinx:killlist>
<id>1234</id>
<id>4567</id>
</sphinx:killlist>

</sphinx:docset>
```

允许使用任意字段和属性。它们也可以在每个文档中以任意顺序出现；顺序被忽略。字段的最大长度有限制；长度超过 2 MB 的字段将截断为 2 MB（此限制可以在源中更改）。

模式，即完整的字段和属性列表，必须在解析任何文档之前声明。这可以通过使用 `xmlpipe_field` 和 `xmlpipe_attr_XXX` 设置在配置文件中完成，也可以通过使用 `<sphinx:schema>` 元素直接在流中完成。 `<sphinx:schema>` 是可选的。它只能作为 `<sphinx:docset>` 中的第一个子元素出现。如果没有流中的模式定义，则使用配置文件中的设置。否则，流设置优先。注意，文档 ID 应作为 `<sphinx:document>` 标签的 `id` 属性指定（例如，`<sphinx:document id="1235">`），并且应该是唯一的正的非零 64 位整数。

未知标签（既未声明为字段也未声明为属性的标签）将被忽略并发出警告。在上面的示例中，`<misc>` 将被忽略。所有嵌套标签及其属性（例如上例中的 `<subject>` 中的 `<strong>`）将被静默忽略。

对输入流编码的支持取决于系统上是否安装了 `iconv`。xmlpipe2 使用 `libexpat` 解析器进行解析，它本地支持 US-ASCII、ISO-8859-1、UTF-8 和一些 UTF-16 变体。Manticore 的 `configure` 脚本还将检查 `libiconv` 的存在并利用它来处理其他编码。`libexpat` 还强制 Manticore 端使用 UTF-8 字符集，因为返回的解析数据始终是 UTF-8 格式。

xmlpipe2 识别的 XML 元素（标签）及其适用的属性如下：

* `sphinx:docset` - 强制性顶级元素，表示并包含xmlpipe2文档集。
* `sphinx:schema` - 可选元素，必须作为 sphinx:docset 的第一个子元素出现，或者根本不出现。声明文档模式并包含字段和属性声明。如果存在，它将覆盖来自配置文件的每源设置。
* `sphinx:field` - 可选元素，sphinx:schema 的子元素。声明一个全文字段。已知属性有：
    *   "name"，指定将在后续文档中作为全文字段对待的 XML 元素名称。
    *   "attr"，指定是否将此字段作为字符串索引。可能的值是 "string"。
* `sphinx:attr` - 可选元素，sphinx:schema 的子元素。声明一个属性。已知属性有：
    *   "name"，指定在后续文档中应视为属性的元素名称。
    *   "type"，指定属性类型。可能的值为 "int"、"bigint"、"timestamp"、"bool"、"float"、"multi" 和 "json"。
    *   "bits"，指定 "int" 属性类型的位大小。有效值为 1 到 32。
    *   "default"，指定该属性的默认值，如果文档中不存在该属性的元素，则使用该值。
* `sphinx:document` - 强制性元素，必须是 sphinx:docset 的子元素。包含任意其他元素及其字段和属性值进行索引，字段和属性根据 sphinx:field 和 sphinx:attr 元素或配置文件中的声明。唯一已知的属性是 "id"，必须包含唯一的整数文档 ID。
* `sphinx:killlist` - 可选元素，sphinx:docset 的子元素。包含多个 "id" 元素，其内容是要放入表的 kill-list 中的文档 ID。kill-list 在多表搜索中用于抑制在搜索的其他表中找到的文档。

## 源配置中的数据定义

如果 XML 没有定义模式，则必须在源配置中定义表元素的数据类型。

* `xmlpipe_field` - 声明一个 `text` 字段。
* `xmlpipe_field_string` - 声明一个文本字段/字符串属性。列将同时作为文本字段进行索引，也将作为字符串属性存储。
* `xmlpipe_attr_uint` - 声明一个整数属性
* `xmlpipe_attr_timestamp` - 声明一个时间戳属性
* `xmlpipe_attr_bool` - 声明一个布尔属性
* `xmlpipe_attr_float` - 声明一个浮点属性
* `xmlpipe_attr_bigint` - 声明一个大整数属性
* `xmlpipe_attr_multi` - 声明一个多值属性，值为整数
* `xmlpipe_attr_multi_64` - 声明一个多值属性，值为64位整数
* `xmlpipe_attr_string` - 声明一个字符串属性
* `xmlpipe_attr_json` - 声明一个JSON属性

### 特定XML源设置

如果设置了 `xmlpipe_fixup_utf8`，它将启用Manticore端的UTF-8验证和过滤，以防止XML解析器在非UTF-8文档中崩溃。默认情况下，此选项是禁用的。

在某些情况下，可能很难甚至不可能保证传入的XMLpipe2文档体是完全有效且符合UTF-8编码的。例如，带有国家单字节编码的文档可能会混入流中。libexpat XML解析器是脆弱的，这意味着在这种情况下它会停止处理。UTF8修正功能可以让您避免这种情况。当启用修正时，Manticore将在将传入流传递给XML解析器之前对其进行预处理，并用空格替换无效的UTF-8序列。

```ini
xmlpipe_fixup_utf8 = 1
```

没有模式的XML源配置示例：

```ini
source xml_test_1
{
    type = xmlpipe2
    xmlpipe_command = cat /tmp/products_today.xml
}
```

带有模式的XML源配置示例：

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
