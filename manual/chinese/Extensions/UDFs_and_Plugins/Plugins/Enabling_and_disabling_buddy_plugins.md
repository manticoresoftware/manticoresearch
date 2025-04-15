# 启用和禁用 Buddy 插件

为了简化 Buddy 插件的控制，特别是在开发一个新的插件或修改现有插件时，提供了启用和禁用 Buddy 插件的命令。这些命令在运行时是临时的，重启守护进程或执行 Buddy 重置后将重置为默认值。要永久禁用插件，必须将其删除。

您需要插件的完全合格包名称才能启用或禁用它。要查找它，可以运行 `SHOW BUDDY PLUGINS` 查询，并在 `package` 字段中寻找完全合格名称。例如，`SHOW` 插件的完全合格名称为 `manticoresoftware/buddy-plugin-show`。

<!-- example enable_buddy_plugin -->
## ENABLE BUDDY PLUGIN

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> NOTE: `ENABLE BUDDY PLUGIN` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

此命令重新激活先前禁用的 Buddy 插件，允许它再次处理您的请求。

<!-- intro -->
### 示例

<!-- request SQL -->
```sql
ENABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```
<!-- end -->

<!-- example disable_buddy_plugin -->
## DISABLE BUDDY PLUGIN

```sql
DISABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

此命令禁用一个活动的 Buddy 插件，防止其处理任何进一步的请求。

<!-- intro -->
### 示例

<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

禁用后，如果您尝试 `SHOW QUERIES` 命令，您将遇到错误，因为插件已被禁用。
<!-- end -->
