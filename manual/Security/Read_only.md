# Read-only mode

Read-only mode for a connection disables any table or global modifications. Therefore, queries like `create`, `drop`, various types of `alter`, `attach`, `optimize`, and data modification queries such as `insert`, `replace`, `delete`, `update`, and others will all be rejected. Changing daemon-wide settings using `SET GLOBAL` is also not possible in this mode.

However, you can still perform all search operations, generate snippets, and run `CALL PQ` queries. Additionally, you can modify local (connection-wide) settings.

To check if your current connection is read-only or not, execute the `show variables like 'session_read_only'` statement. A value of `1` indicates read-only, while `0` means not read-only (usual).

## Activation

Typically, you define a separate [listen](../Server_settings/Searchd.md#listen) directive in read-only mode by adding the suffix `_readonly` to it. However, you can also do this interactively for the current connection by executing the `SET ro=1` statement via SQL.

## Deactivation

If you're connected to a [VIP](../Server_settings/Searchd.md#listen) socket, you can execute `SET ro=0` (even if the socket you are connected to was defined as read-only in the config and not interactively). This will switch the connection to the usual (not read-only) mode with all modifications allowed.

For standard (non-VIP) connections, escaping read-only mode is only possible by reconnecting if it was set interactively, or by updating the configuration file and restarting the daemon.

<!-- proofread -->
