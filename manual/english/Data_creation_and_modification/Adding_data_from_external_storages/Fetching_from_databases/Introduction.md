# Introduction

Manticore Search allows fetching data from databases using specialized drivers or ODBC. Current drivers include:

* `mysql` - for MySQL/MariaDB/Percona MySQL databases
* `pgsql` - for PostgreSQL database
* `mssql` - for Microsoft SQL database
* `odbc` - for any database that accepts connections using ODBC

To fetch data from the database, a source must be configured with type as one of the above. The source requires information about how to connect to the database and the query that will be used to fetch the data. Additional pre- and post-queries can also be set - either to configure session settings or to perform pre/post fetch tasks. The source also must contain definitions of data types for the columns that are fetched.
<!-- proofread -->
