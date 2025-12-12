# 启用和禁用 Buddy 插件

为了简化对 Buddy 插件的控制，尤其是在开发新插件或修改现有插件时，提供了启用和禁用 Buddy 插件的命令。这些命令在运行时临时生效，并将在守护进程重启或执行 Buddy 重置后恢复默认。要永久禁用插件，必须将其移除。

您需要插件的完全限定包名才能启用或禁用它。要查找包名，可以运行 `SHOW BUDDY PLUGINS` 查询，并在 `package` 字段中查找完全限定名。例如，`SHOW` 插件的完全限定名是 `manticoresoftware/buddy-plugin-show`。

<!-- example enable_buddy_plugin -->
## ENABLE BUDDY PLUGIN

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> 注意：`ENABLE BUDDY PLUGIN` 需要 [Manticore Buddy](../../../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

此命令重新激活先前禁用的 Buddy 插件，使其再次处理您的请求。

<!-- intro -->
### SQL 示例

<!-- request SQL -->
```sql
ENABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

<!-- intro -->
### JSON 示例

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ENABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show"
```

<!-- end -->

<!-- example disable_buddy_plugin -->
## DISABLE BUDDY PLUGIN

```sql
DISABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

此命令停用一个活跃的 Buddy 插件，防止其处理任何后续请求。

<!-- intro -->
### 示例

<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

<!-- intro -->
### JSON 示例

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show"
```

禁用后，如果您尝试执行 `SHOW QUERIES` 命令，会遇到错误，因为插件已被禁用。
<!-- end -->

