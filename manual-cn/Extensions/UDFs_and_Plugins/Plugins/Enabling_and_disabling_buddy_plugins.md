# 启用和禁用 Buddy 插件

为了简化 Buddy 插件的控制，特别是在开发新插件或修改现有插件时，提供了启用和禁用 Buddy 插件的命令。这些命令仅在运行时临时生效，重启守护进程或执行 Buddy 重置后将恢复默认设置。如果需要永久禁用插件，必须将其删除。

要启用或禁用插件，您需要插件的完整包名称。可以通过运行 `SHOW BUDDY PLUGINS` 查询，在 `package` 字段中找到插件的完整名称。例如，`SHOW` 插件的完整包名称为 `manticoresoftware/buddy-plugin-show`。

<!-- example enable_buddy_plugin -->
## ENABLE BUDDY PLUGIN

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> 注意: `ENABLE BUDDY PLUGIN` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果命令无法运行，请确认 Buddy 已安装。

此命令可以重新激活先前已禁用的 Buddy 插件，使其能够再次处理请求。

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

此命令可以停用一个处于活动状态的 Buddy 插件，防止其继续处理请求。

<!-- intro -->
### 示例

<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

禁用后，如果您尝试执行 `SHOW QUERIES` 命令，将会遇到错误提示，因为该插件已被禁用。
<!-- end -->
