# 简介

Manticore Search 支持通过专用驱动或ODBC从数据库中获取数据。目前支持的驱动包括：

- `mysql` - 用于MySQL/MariaDB/Percona MySQL数据库
- `pgsql` - 用于PostgreSQL数据库
- `mssql` - 用于Microsoft SQL数据库
- `odbc` - 用于任何接受ODBC连接的数据库

为了从数据库中获取数据，必须将源的类型配置为上述之一。源需要提供有关如何连接到数据库的信息以及用于获取数据的查询。此外，还可以设置额外的预查询和后查询 - 用于配置会话设置或执行获取数据前/后的任务。源还必须包含所获取列的数据类型定义。

<!-- proofread -->
