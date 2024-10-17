# Enabling and disabling Buddy plugins

To simplify the control of Buddy plugins, especially when developing a new one or modifying an existing one, the enable and disable Buddy plugin commands are provided. These commands act temporarily during runtime and will reset to their defaults after restarting the daemon or performing a Buddy reset. To permanently disable a plugin, it must be removed.

You need the fully qualified package name of the plugin to enable or disable it. To find it, you can run the `SHOW BUDDY PLUGINS` query and look for the full qualified name in the `package` field. For example, the `SHOW` plugin has the fully qualified name `manticoresoftware/buddy-plugin-show`.

<!-- example enable_buddy_plugin -->
## ENABLE BUDDY PLUGIN

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> NOTE: `ENABLE BUDDY PLUGIN` requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

This command reactivates a previously disabled Buddy plugin, allowing it to process your requests again.

<!-- intro -->
### Example

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

This command deactivates an active Buddy plugin, preventing it from processing any further requests.

<!-- intro -->
### Example

<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

After disabling, if you try the `SHOW QUERIES` command, you'll encounter an error because the plugin is disabled.
<!-- end -->
