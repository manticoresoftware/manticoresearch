* [☝ 介绍](Introduction.md)
* [❗ 请先阅读](Read_this_first.md)
* [1️⃣ 安装](Installation/Installation.md)
    * [• Docker](Installation/Docker.md)
    * [• RedHat 和 Centos](Installation/RHEL_and_Centos.md)
    * [• Debian 和 Ubuntu](Installation/Debian_and_Ubuntu.md)
    * [• MacOS](Installation/MacOS.md)
    * [• Windows](Installation/Windows.md)
    * [• 从源码编译](Installation/Compiling_from_sources.md)
    * [• Manticore 助手](Installation/Manticore_Buddy.md)
    * [• 从 Sphinx 迁移](Installation/Migration_from_Sphinx.md)
* [🔰 快速开始指南](Quick_start_guide.md)
* [2️⃣ 启动服务器](Starting_the_server.md)
    * [• 在 Linux 中](Starting_the_server/Linux.md)
    * [• 手动方式](Starting_the_server/Manually.md)
    * [• 在 Docker 中](Starting_the_server/Docker.md)
    * [• 在 Windows 中](Starting_the_server/Windows.md)
    * [• 在 MacOS 中](Starting_the_server/MacOS.md)
* [3️⃣ 创建表](Creating_a_table.md)
    * [⪢ 数据类型](Creating_a_table/Data_types.md)
        * [• 按行和按列的属性存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)
    * [⪢ 创建本地表](Creating_a_table/Local_tables.md)
        * [✔ 实时表](Creating_a_table/Local_tables/Real-time_table.md)
        * [• 简单表](Creating_a_table/Local_tables/Plain_table.md)
        * [• 简单表和实时表设置](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md)
        * [• 触发表](Creating_a_table/Local_tables/Percolate_table.md)
        * [• 模板表](Creating_a_table/Local_tables/Template_table.md)
    * [⪢ 自然语言处理和分词]
        * [• 数据分词](Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
        * [• 支持的语言](Creating_a_table/NLP_and_tokenization/Supported_languages.md)
        * [• 连续文字脚本语言](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
        * [• 低级分词](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
        * [• 通配符搜索设置](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md)
        * [• 忽略停用词](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
        * [• 词形](Creating_a_table/NLP_and_tokenization/Wordforms.md)
        * [• 例外情况](Creating_a_table/NLP_and_tokenization/Exceptions.md)
        * [• 形态学](Creating_a_table/NLP_and_tokenization/Morphology.md)
        * [• 高级 HTML 分词](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md)
    * [⪢ 创建分布式表](Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md)
        * [• 创建本地分布式表](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)
        * [• 远程表](Creating_a_table/Creating_a_distributed_table/Remote_tables.md)
* [• 列出表](Listing_tables.md)
* [• 删除表](Deleting_a_table.md)
* [• 清空表](Emptying_a_table.md)
* [⪢ 创建集群](Creating_a_cluster/Creating_a_cluster.md)
    * [添加新节点](Creating_a_cluster/Adding_a_new_node.md)
    * [⪢ 远程节点](Creating_a_cluster/Remote_nodes.md)
        * [镜像](Creating_a_cluster/Remote_nodes/Mirroring.md)
        * [负载均衡](Creating_a_cluster/Remote_nodes/Load_balancing.md)
    * [⪢ 设置复制](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)
        * [创建复制集群](Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md)
        * [加入复制集群](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md)
        * [删除复制集群](Creating_a_cluster/Setting_up_replication/Deleting_a_replication_cluster.md)
        * [添加和移除复制集群中的表](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md)
        * [管理复制节点](Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)
        * [复制集群状态](Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md)
        * [重启集群](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)
        * [集群恢复](Creating_a_cluster/Setting_up_replication/Cluster_recovery.md)
* [4️⃣ 连接服务器](Connecting_to_the_server.md)
    * [MySQL 协议](Connecting_to_the_server/MySQL_protocol.md)
    * [HTTP](Connecting_to_the_server/HTTP.md)
    * [HTTP 上的 SQL](Connecting_to_the_server/HTTP.md#SQL-over-HTTP)
* [⪢ 数据创建和修改](Data_creation_and_modification/Data_creation_and_modification.md)
    * [⪢ 向表中添加文档]
        * [✔ 向实时表中添加文档](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
        * [向触发表中添加规则](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)
    * [⪢ 从外部存储添加数据](Data_creation_and_modification/Adding_data_from_external_storages.md)
        * [创建简单表](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)
        * [⪢ 从数据库拉取数据]
            * [介绍](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Introduction.md)
            * [数据库连接](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md)
            * [执行查询语句](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md)
            * [处理拉取的数据](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md)








































































































































