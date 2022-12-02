# Read-only mode

Read-only mode for connection disables any table or global modifications. So, queries like `create`, `drop`, all kinds of `alter`, `attach`, `optimize`, data modifications queries, such as `insert`, `replace`, `delete`, `update` and others will all be rejected. Changing daemon-wide settings via `SET GLOBAL` is also not possible in this mode.

You still can perform all search operations, generate snippets and run `CALL PQ` queries. Also, you can modify local (connection-wide) settings.

You can check, whether your current connection is read-only or not by executing `show variables like 'session_read_only'` statement. Value `1` means read-only, `0` - usual (not read-only).

## Activation

Normally you define a separate [listen](../Server_settings/Searchd.md#listen) directive in read-only mode by adding suffix `_readonly` to it, but you can also, do it interactively for the current connection by executing `SET ro=1` statement via sql.

## Deactivation

If you're connected to a [vip](../Server_settings/Searchd.md#listen) socket, you can execute `SET ro=0` (even if the socket you are connected to was defined read-only in config, not interactively), and the connection will switch to usual (not read-only) mode with all modifications allowed.

For usual (non-VIP) connections escaping read-only mode is only possible by reconnecting, if it was set read-only interactively, or updating the configuration file and restarting the daemon.
