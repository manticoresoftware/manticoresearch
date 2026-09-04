# 理解集群状态和节点状态

## 快速参考

集群状态会显示两个关键值，帮助你了解集群健康状况：

- `cluster_%_status` - 集群是否可以接受写入
- `cluster_%_node_state` - 这个特定节点正在做什么

## 状态值

<!-- example cluster status values -->

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_node_state';
```

<!-- response SQL-->
```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | primary |
| cluster_posts_node_state   | synced |
+----------------------------+-------+
```

<!-- end -->

### cluster_%_status

- **`primary`** - 正常运行，可读可写
- **`non-primary`** - 已失去法定人数，**写入已阻止**
- **`disconnected`** - 节点已隔离，正在尝试重新连接

### cluster_%_node_state

- **`synced`** - 正常运行
- **`joining`** - 节点正在加入集群（临时状态）
- **`donor`** - 正在帮助另一台节点加入（临时状态）
- **`closed`** - 节点已停止
- **`destroyed`** - 节点已崩溃，需要重启

## 常见场景

| 发生了什么 | cluster_%_status | cluster_%_node_state | 该怎么做 |
|---------------|------------------|---------------------|------------|
| 正常运行 | `primary` | `synced` | 无需操作 |
| 大多数节点宕机 | `non-primary` | `synced` | 执行 bootstrap 或等待 |
| 节点启动中 | `disconnected` | `joining` → `synced` | 等待 |
| 节点崩溃 | 不定 | `destroyed` | 重启节点 |
| 网络分区（多数派） | `primary` | `synced` | 正常继续 |
| 网络分区（少数派） | `non-primary` | `synced` | 等待或 bootstrap |

## 从法定人数丢失中恢复

<!-- example quorum recovery -->
当大多数节点都宕机且状态显示为 `non-primary` 时：

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
-- Check which node has highest sequence number
SHOW STATUS LIKE 'cluster_%_last_committed';
-- Bootstrap the cluster (run on best node only)
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1;
```

<!-- response SQL-->
```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_last_committed | 1547 |
+----------------------------+-------+
```

<!-- request JSON -->
```json
POST /cli -d "
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```

<!-- end -->

<!-- proofread -->
