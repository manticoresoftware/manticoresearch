# Read-only mode

Read-only mode for a connection disables any table or global modifications. Therefore, queries like `create`, `drop`, various types of `alter`, `attach`, `optimize`, and data modification queries such as `insert`, `replace`, `delete`, `update`, and others will all be rejected. Changing daemon-wide settings using `SET GLOBAL` is also not possible in this mode.

However, you can still perform all search operations, generate snippets, and run `CALL PQ` queries. Additionally, you can modify local (connection-wide) settings.

To check if your current connection is read-only or not, execute the `show variables like 'session_read_only'` statement. A value of `1` indicates read-only, while `0` means not read-only (usual).

## Activation

Read-only mode can be enabled at 3 different levels:

1. Per listener, by adding the suffix `_readonly` to a [listen](../Server_settings/Searchd.md#listen) directive
2. Per session, by executing `SET ro=1`
3. Daemon-wide, by setting [`read_only = 1`](../Server_settings/Searchd.md#read_only) in the `searchd` section

When `searchd.read_only=1` is enabled, all new client connections start in read-only mode automatically. This mode is intended for deployments where Manticore must not modify table storage, for example when serving plain tables from a replicated read-only filesystem. In that mode, modifying queries are rejected in the same way as on a `_readonly` listener.

## Deactivation

If you're connected to a [VIP](../Server_settings/Searchd.md#listen) socket, you can execute `SET ro=0` (even if the socket you are connected to was defined as read-only in the config and not interactively). This will switch the connection to the usual (not read-only) mode with all modifications allowed.

For standard (non-VIP) connections, escaping read-only mode is only possible by reconnecting if it was set interactively, or by updating the configuration file and restarting the daemon.

If read-only mode was enabled daemon-wide via `searchd.read_only=1`, `SET ro=0` is not allowed even on VIP connections. Disable `read_only` in the config and restart the daemon to leave daemon-wide read-only mode.

<!-- proofread -->

