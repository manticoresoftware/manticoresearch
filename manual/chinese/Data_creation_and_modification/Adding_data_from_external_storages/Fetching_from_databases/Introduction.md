# 介绍

Manticore Search 允许使用专用驱动程序或 ODBC 从数据库中获取数据。当前的驱动程序包括：

* `mysql` - 用于 MySQL/MariaDB/Percona MySQL 数据库
* `pgsql` - 用于 PostgreSQL 数据库
* `mssql` - 用于 Microsoft SQL 数据库
* `odbc` - 用于任何接受 ODBC 连接的数据库

要从数据库获取数据，必须配置一个类型为上述之一的源。该源需要包含有关如何连接到数据库的信息以及将用来获取数据的查询。还可以设置额外的预查询和后查询——用于配置会话设置或执行获取前/获取后任务。源还必须包含所获取列数据类型的定义。
<!-- proofread -->

