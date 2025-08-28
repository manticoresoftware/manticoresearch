# 启用和禁用 Buddy 插件

为了简化 Buddy 插件的控制，特别是在开发新插件或修改现有插件时，提供了启用和禁用 Buddy 插件的命令。这些命令在运行时起作用，重启守护进程或执行 Buddy 重置后会恢复到默认状态。要永久禁用插件，必须将其移除。

要启用或禁用插件，您需要插件的完全限定包名。要查找它，您可以运行 `SHOW BUDDY PLUGINS` 查询，并在 `package` 字段中查找完全限定名。例如，`SHOW` 插件的完全限定名是 `manticoresoftware/buddy-plugin-show`。

<!-- example enable_buddy_plugin -->
## ENABLE BUDDY PLUGIN

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> 注意：`ENABLE BUDDY PLUGIN` 需要 [Manticore Buddy](../../../Installation/Manticore_Buddy.md)。如果无法使用，请确保 Buddy 已安装。

此命令重新激活先前已禁用的 Buddy 插件，使其能够再次处理您的请求。

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

此命令停用正在运行的 Buddy 插件，阻止其处理任何后续请求。

<!-- intro -->
### 示例

<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

禁用后，如果尝试运行 `SHOW QUERIES` 命令，将会遇到错误，因为该插件已被禁用。
<!-- end -->

