# 从外部存储添加数据

您可以通过多种方法将数据从外部存储导入到 Manticore：

* 使用 [Indexer 工具](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) 从各种数据库抓取数据到普通表。
* 通过 [Logstash](../Integration/Logstash.md)、[Filebeat](../Integration/Filebeat.md) 和 [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) 集成，将数据从这些工具写入 Manticore 实时表。
* 使用 [Kafka 集成](../Integration/Kafka.md) 将 Kafka 主题中的数据同步到实时表。

