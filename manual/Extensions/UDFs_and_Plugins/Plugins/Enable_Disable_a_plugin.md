# ENABLE & DISABLE BUDDY PLUGIN

To simplify controlling active plugins, especially when debugging, we've introduced the enable and disable Buddy plugin feature. These settings are temporary and will reset to default after restarting the daemon or a Buddy reset. To permanently disable a plugin, you must remove it.

## ENABLE BUDDY PLUGIN

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

This command reactivates a previously disabled Buddy plugin, allowing it to process your requests again.

### Example

```sql
ENABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-queries
```

## DISABLE BUDDY PLUGIN

```sql
DISABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

This command deactivates an active Buddy plugin, preventing it from processing any further requests.

### Example

```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-queries
```

After disabling, if you try the `SHOW QUERIES` command, you'll encounter an error because the plugin is disabled.

