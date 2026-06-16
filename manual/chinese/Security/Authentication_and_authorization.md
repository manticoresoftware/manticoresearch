# 身份验证和授权

Manticore 可以要求用户先通过身份验证，然后才能使用 MySQL 协议下的 SQL、HTTP/HTTPS 端点以及与复制相关的操作。身份验证用于识别用户。授权用于检查该用户是否被允许在目标上执行某个操作。

除非配置了 [auth](../Server_settings/Searchd.md#auth) 设置，否则身份验证会被禁用。启用后，没有有效凭据的请求会被拒绝。

身份验证要求 Manticore 构建时支持 SSL。通过网络发送密码或 bearer token 时，请使用 [SSL](../Security/SSL.md) 或 HTTPS。

## 启用身份验证

`auth` 设置应放在 `searchd` 部分中。

在 [RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 下，使用 `auth = 1` 启用身份验证。Manticore 会将身份验证数据存储在 [data_dir](../Server_settings/Searchd.md#data_dir) 下的 `auth.json` 中。使用 `auth = 0` 或省略该设置即可禁用身份验证。

```ini
searchd {
    data_dir = /var/lib/manticore
    auth = 1
}
```

显式禁用身份验证：

```ini
searchd {
    data_dir = /var/lib/manticore
    auth = 0
}
```

在 [plain 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 下，将 `auth` 设置为身份验证文件路径：

```ini
searchd {
    auth = /path/to/auth.json
}
```

启用身份验证后，如果身份验证文件不存在，Manticore 会创建它。在首次 bootstrap 之前，允许存储为空或缺失，包括零字节文件、仅包含空白字符的文件、空 JSON 对象，或者空的 users 和 permissions 数组。bootstrap 之后会写入完整的身份验证 JSON。如果身份验证文件已经存在，Manticore 会在启动时校验它；如果文件无效、无法读取，或者对组或其他用户可读，则拒绝启动。

## 创建第一个管理员

启用 `auth` 后，先启动 `searchd`。然后使用相同的配置文件创建第一个管理员：

```bash
searchd --config /etc/manticoresearch/manticore.conf --auth
```

对于非交互式设置，请通过 stdin 提供管理员名称、密码和密码确认：

```bash
printf 'admin\nStrongPass#2026\nStrongPass#2026\n' | searchd --config /etc/manticoresearch/manticore.conf --auth-non-interactive
```

bootstrap 命令要求有正在运行的守护进程以及已配置的 [pid_file](../Server_settings/Searchd.md#pid_file)。它会验证身份验证存储是否缺失或为空，接受空 JSON 对象或空的 users 和 permissions 数组，拒绝已存在且非空的身份验证存储，创建第一个管理员，将所有操作权限授予该用户，并请求正在运行的守护进程重新加载身份验证数据。

Bootstrap 会为管理员创建密码，但不会返回 bearer token。要创建 bearer token，请以该管理员身份连接并运行 `TOKEN`，或使用 HTTP `POST /token` 端点。

## 使用身份验证连接

对于 MySQL 协议下的 SQL，请使用 Manticore 用户名和密码连接：

```bash
MYSQL_PWD=StrongPass#2026 mysql -h0 -P9306 -uadmin
```

MySQL 协议只支持 `mysql_native_password` 身份验证。

对于 HTTP/HTTPS，可以使用 HTTP Basic 身份验证：

```bash
curl -u admin:StrongPass#2026 http://127.0.0.1:9308/sql -d "query=SELECT 1"
```

或者使用 bearer token：

```bash
curl -H "Authorization: Bearer 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" http://127.0.0.1:9308/sql -d "query=SELECT 1"
```

要为已通过身份验证的 HTTP 用户创建或轮换 bearer token：

```bash
curl -u admin:StrongPass#2026 -X POST http://127.0.0.1:9308/token -d "{}"
```

该端点只会返回一次原始 token。请妥善保存。

HTTP 头名称以及 `Basic` 和 `Bearer` 方案名称都不区分大小写。用户名区分大小写：`app_user` 和 `App_User` 是不同的用户。

## 用户和 token

在创建第一个管理员之后，使用 SQL 命令管理用户。

```sql
CREATE USER 'app_read' IDENTIFIED BY 'ReadPass#2026';
DROP USER 'app_read';
SET PASSWORD 'NewReadPass#2026' FOR 'app_read';
SET PASSWORD 'NewOwnPass#2026';
```

`CREATE USER` 会为新用户返回一个原始 bearer token：

```sql
CREATE USER 'api_user' IDENTIFIED BY 'ApiPass#2026';
```

之后要创建或轮换 bearer token：

```sql
TOKEN 'api_user';
TOKEN;
```

`TOKEN 'api_user'` 会为指定用户创建或轮换 token，除非目标是当前用户，否则需要 `admin` 权限。省略用户的 `TOKEN` 会为当前用户创建或轮换 token。

`SET PASSWORD` 只会更改 SQL/MySQL 和 HTTP Basic 身份验证。它不会轮换或撤销现有的 bearer token。要让 bearer token 失效，请运行 `TOKEN` 或 `TOKEN '<user>'` 进行轮换。

`SHOW TOKEN` 显示的是已存储的 token 哈希，而不是原始 bearer token：

```sql
SHOW TOKEN;
SHOW TOKEN FOR 'api_user';
```

## 权限

权限由操作、目标以及允许或拒绝规则组成。

支持的操作有：

* `read` - 从表中读取数据并执行只读表操作。
* `write` - 写入或删除表数据。
* `schema` - 创建、修改、删除、导入、重新加载或以其他方式管理表结构和服务器级元数据。
* `replication` - 执行复制集群操作。
* `admin` - 管理用户、token、权限和身份验证数据。

只检查表结构或表列表的命令，例如 `DESCRIBE`、`SHOW TABLES`、`SHOW CREATE TABLE`、`SHOW TABLE STATUS` 和 `SHOW TABLE SETTINGS`，需要 `read`，而不是 `schema`。

没有权限就表示默认拒绝。`admin` 操作的目标必须是 `*`，而且它并不隐含 `read`、`write`、`schema` 或 `replication`；需要时请显式授予这些操作。

目标可以是普通名称或通配符。常见示例有 `products`、`logs_*`、`posts` 和 `*`。

```sql
GRANT read ON 'products' TO 'app_read';
GRANT read ON 'logs_*' TO 'analytics';
GRANT write ON 'products' TO 'ingest';
GRANT schema ON * TO 'maintainer';
GRANT admin ON * TO 'security_admin';

REVOKE read ON 'products' FROM 'app_read';
```

### 规则解析

当多条权限规则都可能匹配同一个请求时，Manticore 会按操作、目标以及允许或拒绝值来解析。

规则只会针对请求的操作进行匹配。例如，对 `admin` 的授权并不能满足 `read`、`write`、`schema` 或 `replication`。

对于目标，精确名称和通配符都是匹配规则。如果没有任何规则匹配请求中的用户、操作和目标，则访问被拒绝。

`WITH ALLOW 0` 会创建一个显式拒绝规则。任何匹配到的拒绝规则都会覆盖匹配到的允许规则，包括更具体的允许规则。

```sql
GRANT read ON * TO 'analyst';
GRANT read ON 'private_logs' TO 'analyst' WITH ALLOW 0;
```

在这个示例中，`analyst` 可以读取其他表，但不能读取 `private_logs`。

匹配到的通配符拒绝也会阻止更具体的允许：

```sql
GRANT read ON 'logs_*' TO 'auditor' WITH ALLOW 0;
GRANT read ON 'logs_public' TO 'auditor';
```

在这个示例中，`auditor` 不能读取 `logs_public` 或任何其他匹配 `logs_*` 的表，因为通配符拒绝匹配了该请求。

`REVOKE` 会移除一条规则。它不会创建拒绝规则：

```sql
REVOKE read ON 'private_logs' FROM 'analyst';
```

`GRANT` 也可以存储一个预算 JSON 值：

```sql
GRANT read ON 'reports' TO 'analyst' WITH BUDGET '{"queries_per_day":10000}';
```

预算值会随权限一起存储和显示，但配额和速率限制的执行留给后续扩展。

## 查看身份验证数据

使用 SQL 命令查看用户和权限：

```sql
SHOW USERS;
SHOW PERMISSIONS;
SHOW PERMISSIONS FOR 'api_user';
SHOW USAGE;
SHOW USAGE FOR 'api_user';
```

`SHOW USAGE` 目前返回的是占位用法计数器。它是为将来的用量和预算报告准备的。

使用上面的 SQL 命令管理身份验证数据。底层身份验证存储是内部实现，不是面向用户的管理或查看接口。

如果身份验证文件在守护进程外被修改，请使用以下命令重新加载：

```sql
RELOAD AUTH;
```

`RELOAD AUTH` 需要 `admin` 权限。SQL 身份验证管理命令会立即生效；`RELOAD AUTH` 主要用于运维维护和 plain 模式下的文件重载。

## 密码策略

密码校验由以下设置控制：

* [auth_password_policy](../Server_settings/Searchd.md#auth_password_policy)，值为 `LOW` 或 `MEDIUM`。
* [auth_password_min_length](../Server_settings/Searchd.md#auth_password_min_length)，默认值为 `8`。

`LOW` 要求密码非空，并满足配置的最小长度。`MEDIUM` 还要求至少包含一个小写字母、一个大写字母、一个数字和一个非字母数字字符。

同一策略适用于 bootstrap、`CREATE USER` 和 `SET PASSWORD`。

## 身份验证日志

启用身份验证后，身份验证事件会写入单独的日志。如果守护进程日志是 `/var/log/manticore/searchd.log`，那么身份验证日志就是 `/var/log/manticore/searchd.log.auth`。

[auth_log_level](../Server_settings/Searchd.md#auth_log_level) 设置控制日志详细程度：

* `disabled` - 不记录身份验证事件。
* `error` - 记录权限拒绝和严重故障。
* `warning` - 记录错误和失败的身份验证尝试。
* `info` - 记录警告和成功的身份验证管理变更。
* `all` - 记录 `info` 事件以及成功的面向用户的身份验证事件。
* `trace` - 记录 `all` 事件，再加上成功的内部传输身份验证，例如 Manticore Buddy 和守护进程到守护进程的 API 身份验证。

任何级别都不会记录成功的授权允许检查。权限拒绝会被记录，但允许检查可能会发生在每个查询上，即使在 `trace` 模式下也会让身份验证日志变得很嘈杂。

默认值是 `info`。

在 `info`、`all` 和 `trace` 级别下，成功的 `JOIN CLUSTER` 如果替换了本地身份验证数据，会将之前本地 auth 数据的 JSON 备份写入 `searchd.log.auth`。这个备份包含用户名、盐值、密码哈希和 bearer 哈希。它不包含明文密码或 bearer token，但仍然属于凭据材料。请将 `searchd.log.auth` 视为敏感运维数据：限制访问、保护保留和日志转发，并在共享日志前进行脱敏。

## 分布式远程代理

启用身份验证后，对远程 [agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) 的 [distributed table](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 请求会以当前会话用户的身份进行身份验证。远程守护进程必须具有相同的用户以及匹配的已存储身份验证数据，并且该用户必须对远程目标表拥有所需权限。

不能只在每个节点上独立创建同名同密码的用户，因为已存储的身份验证数据可能不同。对于使用远程代理的分布式表，请在参与的守护进程之间保持身份验证数据同步，并在守护进程外部发生变更时使用 `RELOAD AUTH` 重新加载。

如果远程守护进程没有该用户、该用户的身份验证数据不同，或者缺少所需权限，分布式查询会被远程代理拒绝。

## 复制集群

启用身份验证后，集群操作使用 `replication` 操作。请将它授予应该运行并拥有复制操作的用户：

```sql
CREATE USER 'repl_user' IDENTIFIED BY 'ReplPass#2026';
GRANT replication ON 'posts' TO 'repl_user';
```

`CREATE CLUSTER` 和 `JOIN CLUSTER` 可以指定生效的复制用户：

```sql
CREATE CLUSTER posts 'repl_user' AS user;
JOIN CLUSTER posts AT '10.12.1.35:9312' 'repl_user' AS user;
```

如果未指定用户，则该语句使用当前会话用户。成功执行 `CREATE CLUSTER` 后，生效用户会被存储为集群用户。成功执行 `JOIN CLUSTER` 后，存储的集群用户会从捐赠集群元数据中取得。

要更改已存储的集群用户：

```sql
ALTER CLUSTER posts UPDATE user 'repl_user';
```

之后的集群操作，例如 `ALTER CLUSTER ... ADD`、`ALTER CLUSTER ... DROP`、`ALTER CLUSTER ... UPDATE nodes` 和 `DELETE CLUSTER`，使用的是已存储的集群用户，而不是当前会话用户。

> 注意：启用身份验证后，成功的 `JOIN CLUSTER` 会将加入节点上的所有本地身份验证数据替换为捐赠集群的身份验证数据。如果身份验证日志级别是 `info` 或更高，Manticore 会在替换前将之前本地 auth 数据的 JSON 备份写入 `searchd.log.auth`。这个备份包含盐值和凭据哈希，因此请保持 auth 日志私密，并在共享前脱敏。加入前请确保复制用户和 auth 数据在各节点之间一致。如果 `JOIN CLUSTER` 无法获取捐赠方用户，请检查捐赠方节点上的复制用户和匹配的 auth 数据，并查看捐赠方节点上的 `searchd.log.auth` 以排查 API 身份验证失败。

当启用身份验证时，守护进程到守护进程的流量会使用身份验证材料。这由 Manticore 在内部处理，不属于客户端协议。
