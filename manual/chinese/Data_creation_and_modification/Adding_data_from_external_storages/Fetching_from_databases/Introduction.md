# 介绍

Manticore Search 允许使用专用驱动程序或 ODBC 从数据库中获取数据。当前的驱动程序包括：

* `mysql` - 用于 MySQL/MariaDB/Percona MySQL 数据库
* `pgsql` - 用于 PostgreSQL 数据库
* `mssql` - 用于 Microsoft SQL 数据库
* `odbc` - 用于任何接受使用 ODBC 连接的数据库

要从数据库中获取数据，源必须配置为上述类型之一。源需要有关如何连接到数据库的信息和用于获取数据的查询。还可以设置额外的前查询和后查询 - 无论是配置会话设置还是执行前/后获取任务。源还必须包含被提取的列的数据类型定义。
<!-- proofread -->
