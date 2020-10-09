# Read this first

## About this manual

The manual is arranged as a reflection of the most likely way you would use Manticore:
* starting from some basic information about it and how to install and connect
* through some essential things like adding documents and running searches
* to some performance optimization tips and tricks and extending Manticore with help of plugins and custom functions


##### Do not skip ✔️
Key sections of the manual are marked with sign ✔️ in the menu for your convenience since their corresponding functionality is most used. If you are new to Manticore **we highly recommend to not skip them**.

##### Quick start guide
If you are looking for a quick understanding of how Manticore works in general [⚡ Quick start guide](Quick_start_guide.md) section should be good to read.

##### Using examples
Each query example has a little icon 📋 in the top-right corner:

![Copy example](copy_example.png)

You can use it to copy examples to clipboard. **If the query is an HTTP request it will be copied as a CURL command**. You can configure the host/port if you press ⚙️.

##### Search in this manual

We love search and we've made our best to make searching in this manual as convenient as possible. Of course it's backed by Manticore Search. Besides using the search bar which requires opening the manual first there is a very easy way to find something by just opening **mnt.cr/your-search-keyword** :

![mnt.cr quick manual search](mnt.cr.gif)

## Best practices
There are few things you need to understand about Manticore Search that can help you follow the best practices of using it.

#### Real-time index vs plain index
* **[Real-time index](Creating_an_index/Local_indexes/Real-time_index.md)** allows adding, updating and deleting documents with immediate availability of the changes.
* **[Plain index](Creating_an_index/Local_indexes/Plain_index.md)** is a mostly immutable data structure and a basic element used by real-time indexes. Plain index stores a set of documents, their common dictionary and indexation settings. One real-time index can consist of multiple plain indexes (chunks), but **besides that Manticore provides direct access to building plain indexes** using tool [indexer](Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool). It makes sense when your data is mostly immutable, therefore you don't need a real-time index for that.

#### Real-time mode vs plain mode
Manticore Search works in two modes:
* **Real-time mode** (RT mode). This is a default one and:
  * allows managing your data schema online using SQL commands `CREATE`/`ALTER`/`DROP TABLE` and their equivalents in non-SQL clients
  * in the configuration file you need to define only server-related settings
* **Plain mode** allows to define your data schemas in a configuration file. It makes sense in two cases:
  * when you only deal with plain indexes
  * or when your data schema is very stable and you don't need replication (as it's available only in the RT mode)

You cannot combine the 2 modes and need to decide which one you want to follow. If you are unsure **our recommendation is to follow the RT mode** as if even you need a plain index you can build it with a separate plain index config and import to your main Manticore instance.

The Real-time index can be used in both RT and plain modes. In RT mode a RT index is defined with a `CREATE TABLE` command, while in plain mode it is defined in the configuration file. Plain (offline) indexes are supported only in plain mode. Plain indexes cannot be created in RT mode, but existing plain indexes made in plain mode can converted as Real-time indexes and imported in RT mode.

#### SQL vs JSON
Manticore provides multiple ways and interfaces to manage your schemas and data, but the two main are:
* **SQL**. This is a native Manticore's language which enables all Manticore's functionality. **The best practice is to use SQL to**:
  * manage your schemas and do other DBA routines as it's the easiest way to do that
  * design your queries as SQL is much closer to natural language than the JSON DSL which is important when you design something new. You can use Manticore SQL via any MySQL client or [/sql](Connecting_to_the_server/MySQL_protocol.md).
* **JSON**. Most functionality is also available via JSON domain specific language. This is especially useful when you integrate Manticore with your application as with JSON you can do it more programmatically than with SQL. The best practice is to **first explore how to do something via SQL and then use JSON to integrate it into your application.**
