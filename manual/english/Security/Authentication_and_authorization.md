# Authentication and authorization

Manticore can require users to authenticate before using SQL over the MySQL protocol, HTTP/HTTPS endpoints, and replication-related operations. Authentication identifies the user. Authorization checks whether that user is allowed to perform an action on a target.

Authentication is disabled unless the [auth](../Server_settings/Searchd.md#auth) setting is configured. When it is enabled, requests without valid credentials are rejected.

Authentication requires a Manticore build with SSL support. Use [SSL](../Security/SSL.md) or HTTPS when sending passwords or bearer tokens over a network.

## Enabling authentication

The `auth` setting belongs in the `searchd` section.

In [RT mode](../Read_this_first.md#Real-time-mode-vs-plain-mode), enable authentication with `auth = 1`. Manticore stores authentication data in `auth.json` under [data_dir](../Server_settings/Searchd.md#data_dir). Use `auth = 0` or omit the setting to disable authentication.

```ini
searchd {
    data_dir = /var/lib/manticore
    auth = 1
}
```

To disable authentication explicitly:

```ini
searchd {
    data_dir = /var/lib/manticore
    auth = 0
}
```

In [plain mode](../Read_this_first.md#Real-time-mode-vs-plain-mode), set `auth` to the authentication file path:

```ini
searchd {
    auth = /path/to/auth.json
}
```

When authentication is enabled, Manticore creates the authentication file if it is missing. If an authentication file already exists, Manticore validates it during startup and refuses to start if the file is invalid.

## Creating the first administrator

After enabling `auth`, start `searchd` first. Then create the first administrator with the same configuration file:

```bash
searchd --config /etc/manticoresearch/manticore.conf --auth
```

For non-interactive setup, provide the administrator name, password, and password confirmation on stdin:

```bash
printf 'admin\nStrongPass#2026\nStrongPass#2026\n' | searchd --config /etc/manticoresearch/manticore.conf --auth-non-interactive
```

The bootstrap command requires a running daemon and a configured [pid_file](../Server_settings/Searchd.md#pid_file). It validates that the authentication store is missing or empty, accepts an empty JSON object or empty users and permissions arrays, rejects a non-empty existing authentication store, creates the first administrator, grants all actions to that user, and asks the running daemon to reload authentication data.

Bootstrap creates a password for the administrator, but it does not return a bearer token. To create a bearer token, connect as that administrator and run `TOKEN` or use the HTTP `POST /token` endpoint.

## Connecting with authentication

For SQL over the MySQL protocol, connect with a Manticore user name and password:

```bash
MYSQL_PWD=StrongPass#2026 mysql -h0 -P9306 -uadmin
```

The MySQL protocol supports only `mysql_native_password` authentication.

For HTTP/HTTPS, use either HTTP Basic authentication:

```bash
curl -u admin:StrongPass#2026 http://127.0.0.1:9308/sql -d "query=SELECT 1"
```

or a bearer token:

```bash
curl -H "Authorization: Bearer 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" http://127.0.0.1:9308/sql -d "query=SELECT 1"
```

To create or rotate a bearer token for the authenticated HTTP user:

```bash
curl -u admin:StrongPass#2026 -X POST http://127.0.0.1:9308/token -d "{}"
```

The endpoint returns the raw token once. Store it securely.

## Users and tokens

After the first administrator exists, manage users with SQL commands.

```sql
CREATE USER 'app_read' IDENTIFIED BY 'ReadPass#2026';
DROP USER 'app_read';
SET PASSWORD 'NewReadPass#2026' FOR 'app_read';
SET PASSWORD 'NewOwnPass#2026';
```

`CREATE USER` returns a raw bearer token for the new user:

```sql
CREATE USER 'api_user' IDENTIFIED BY 'ApiPass#2026';
```

To create or rotate a bearer token later:

```sql
TOKEN 'api_user';
TOKEN;
```

`TOKEN 'api_user'` creates or rotates a token for the specified user and requires `admin` permission unless the target is the current user. `TOKEN` without a user creates or rotates the current user's token.

`SHOW TOKEN` displays the stored token hash, not the raw bearer token:

```sql
SHOW TOKEN;
SHOW TOKEN FOR 'api_user';
```

## Permissions

Permissions combine an action, a target, and an allow or deny rule.

The supported actions are:

* `read` - read data from tables and run read-only table operations.
* `write` - write or delete table data.
* `schema` - create, alter, drop, import, reload, or otherwise manage table schemas and server-level metadata.
* `replication` - run replication cluster operations.
* `admin` - manage users, tokens, permissions, and authentication data.

Commands that only inspect table schemas or table lists, such as `DESCRIBE`, `SHOW TABLES`, `SHOW CREATE TABLE`, `SHOW TABLE STATUS`, and `SHOW TABLE SETTINGS`, require `read`, not `schema`.

No permission means default deny. The `admin` action must target `*` and does not imply `read`, `write`, `schema`, or `replication`; grant those actions explicitly when needed.

Targets are plain names or wildcards. Common examples are `products`, `logs_*`, `posts`, and `*`.

```sql
GRANT read ON 'products' TO 'app_read';
GRANT read ON 'logs_*' TO 'analytics';
GRANT write ON 'products' TO 'ingest';
GRANT schema ON * TO 'maintainer';
GRANT admin ON * TO 'security_admin';

REVOKE read ON 'products' FROM 'app_read';
```

### Rule resolution

When several permission rules can match the same request, Manticore resolves them by action, target, and allow or deny value.

Rules are matched for the requested action only. A grant for `admin`, for example, does not satisfy `read`, `write`, `schema`, or `replication`.

For targets, exact names are more specific than wildcard rules. If no rule matches the requested user, action, and target, access is denied.

`WITH ALLOW 0` creates an explicit deny rule. An explicit deny rule overrides an allow rule that also matches the request.

```sql
GRANT read ON * TO 'analyst';
GRANT read ON 'private_logs' TO 'analyst' WITH ALLOW 0;
```

In this example, `analyst` can read other tables, but cannot read `private_logs`.

A more specific allow can grant access inside a broader default deny pattern:

```sql
GRANT read ON 'logs_*' TO 'auditor' WITH ALLOW 0;
GRANT read ON 'logs_public' TO 'auditor';
```

In this example, `auditor` can read `logs_public`, but cannot read other tables matching `logs_*`.

`REVOKE` removes a rule. It does not create a deny rule:

```sql
REVOKE read ON 'private_logs' FROM 'analyst';
```

`GRANT` can also store a budget JSON value:

```sql
GRANT read ON 'reports' TO 'analyst' WITH BUDGET '{"queries_per_day":10000}';
```

The budget value is stored and shown with permissions, but quota and rate-limit enforcement is reserved for a later extension.

## Inspecting authentication data

Use SQL commands to inspect users and permissions:

```sql
SHOW USERS;
SHOW PERMISSIONS;
SHOW PERMISSIONS FOR 'api_user';
SHOW USAGE;
SHOW USAGE FOR 'api_user';
```

`SHOW USAGE` currently returns placeholder usage counters. It is intended for future usage and budget reporting.

Manage authentication data with the SQL commands above. The underlying authentication storage is internal and is not a user-facing management interface.

If the authentication file was changed outside the daemon, reload it with:

```sql
RELOAD AUTH;
```

`RELOAD AUTH` requires `admin` permission. SQL authentication management commands apply their changes immediately; `RELOAD AUTH` is mostly useful for operational maintenance and plain-mode file reloads.

## Password policy

Password validation is controlled by:

* [auth_password_policy](../Server_settings/Searchd.md#auth_password_policy), either `LOW` or `MEDIUM`.
* [auth_password_min_length](../Server_settings/Searchd.md#auth_password_min_length), default `8`.

`LOW` requires a non-empty password that satisfies the configured minimum length. `MEDIUM` also requires at least one lowercase letter, one uppercase letter, one digit, and one non-alphanumeric character.

The same policy applies to bootstrap, `CREATE USER`, and `SET PASSWORD`.

## Authentication log

Authentication events are written to a separate log when authentication is enabled. If the daemon log is `/var/log/manticore/searchd.log`, the authentication log is `/var/log/manticore/searchd.log.auth`.

The [auth_log_level](../Server_settings/Searchd.md#auth_log_level) setting controls verbosity:

* `disabled` - do not log authentication events.
* `error` - log permission denials and critical failures.
* `warning` - log errors and failed authentication attempts.
* `info` - log warnings and successful authentication management changes.
* `all` - log `info` events and successful authentication events.

The default is `info`.

## Distributed remote agents

When authentication is enabled, [distributed table](../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) requests to remote [agents](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) are authenticated as the current session user. The remote daemon must have the same user with matching stored authentication data, and that user must have the required permission on the remote target table.

Creating a user independently on each node with the same password is not enough, because the stored authentication data can differ. For distributed tables that use remote agents, keep authentication data synchronized across the participating daemons and reload it with `RELOAD AUTH` when it is changed outside the daemon.

If the remote daemon does not have the user, has different authentication data for that user, or lacks the required permission, the distributed query is rejected by the remote agent.

## Replication clusters

When authentication is enabled, cluster operations use the `replication` action. Grant it to the user that should run and own replication operations:

```sql
CREATE USER 'repl_user' IDENTIFIED BY 'ReplPass#2026';
GRANT replication ON 'posts' TO 'repl_user';
```

`CREATE CLUSTER` and `JOIN CLUSTER` can specify the effective replication user:

```sql
CREATE CLUSTER posts 'repl_user' AS user;
JOIN CLUSTER posts AT '10.12.1.35:9312' 'repl_user' AS user;
```

If no user is specified, the current session user is used for that statement. On successful `CREATE CLUSTER`, the effective user is stored as the cluster user. On successful `JOIN CLUSTER`, the stored cluster user is taken from the donor cluster metadata.

To change the stored cluster user:

```sql
ALTER CLUSTER posts UPDATE user 'repl_user';
```

Later cluster operations such as `ALTER CLUSTER ... ADD`, `ALTER CLUSTER ... DROP`, `ALTER CLUSTER ... UPDATE nodes`, and `DELETE CLUSTER` use the stored cluster user, not the current session user.

> NOTE: Joining a cluster can replace the local authentication data with the donor cluster's authentication data. Make sure the replication user and auth data are consistent across nodes before joining.

Daemon-to-daemon traffic uses authentication material when authentication is enabled. This is handled by Manticore internally and is not a client protocol.
