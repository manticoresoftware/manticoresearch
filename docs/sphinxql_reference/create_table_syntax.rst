.. _create_table_syntax:

CREATE TABLE syntax
--------------------

.. code-block:: none


    CREATE TABLE [IF NOT EXISTS] name [( create_definition,...)] [table_options]
    
    CREATE TABLE [IF NOT EXISTS] name LIKE old_name

    create_definition:
       col_name data_type [data_type_options]

    table_options:
       table_option [ table_option] ...

    table_option:
       option_name = 'option_value'


CREATE TABLE creates a new index. Available for :ref:`rt_mode` only.

Columns data types:

+--------------+---------------------+-------------------+----------------------+
| data_type    |  Config equivalent  | Options           |  Alias               |
+==============+=====================+===================+======================+
| text         | rt_field            |  indexed, stored  |                      |
+--------------+---------------------+-------------------+----------------------+
| integer      | rt_attr_uint        |                   |  int uint            |
+--------------+---------------------+-------------------+----------------------+
| float        | rt_attr_float       |                   |                      |
+--------------+---------------------+-------------------+----------------------+
| multi        | rt_attr_multi       |                   |                      |
+--------------+---------------------+-------------------+----------------------+
| multi64      | rt_attr_multi64     |                   |                      |
+--------------+---------------------+-------------------+----------------------+
| bool         | rt_attr_bool        |                   |                      |
+--------------+---------------------+-------------------+----------------------+
| json         | rt_attr_json        |                   |                      |
+--------------+---------------------+-------------------+----------------------+
| string       | rt_attr_string      |                   |                      |
+--------------+---------------------+-------------------+----------------------+
| timestamp    | rt_attr_timestamp   |                   |                      |
+--------------+---------------------+-------------------+----------------------+
| bit(n)       | rt_attr_uint        |                   |                      |
+--------------+---------------------+-------------------+----------------------+

The "text" type supports additional options. By default both "indexed" and "stored" are enabled.

Table options are the same as defined at :ref:`index_command_reference` and are supported by RT, PQ and distributed index types: 

  - :ref:`access_plain_attrs` 
  - :ref:`access_blob_attrs`  
  - :ref:`access_doclists`
  - :ref:`access_hitlists`
  - :ref:`agent_connect_timeout`
  - :ref:`agent_query_timeout`
  - :ref:`agent_retry_count`
  - :ref:`agent`
  - :ref:`attr_update_reserve`
  - :ref:`bigram_freq_words`
  - :ref:`bigram_index`
  - :ref:`blend_chars`
  - :ref:`blend_mode`
  - :ref:`charset_table`
  - :ref:`docstore_block_size`
  - :ref:`docstore_compression`
  - :ref:`docstore_compression_level`
  - :ref:`exceptions`
  - :ref:`expand_keywords`
  - :ref:`global_idf`
  - :ref:`ha_strategy`
  - :ref:`hitless_words`
  - :ref:`html_index_attrs`
  - :ref:`html_remove_elements`
  - :ref:`html_strip`
  - :ref:`ignore_chars`
  - :ref:`index_exact_words`
  - :ref:`index_field_lengths`
  - :ref:`index_sp`
  - :ref:`index_token_filter`
  - :ref:`index_zones`
  - :ref:`local`
  - :ref:`min_infix_len`
  - :ref:`min_prefix_len`
  - :ref:`min_stemming_len`
  - :ref:`min_word_len`
  - :ref:`mirror_retry_count`
  - :ref:`morphology`
  - :ref:`morphology_skip_fields`
  - :ref:`ngram_chars`
  - :ref:`ngram_len`
  - :ref:`overshort_step`
  - :ref:`phrase_boundary`
  - :ref:`phrase_boundary_step`
  - :ref:`read_buffer_docs`
  - :ref:`read_buffer_hits`
  - :ref:`regexp_filter`
  - :ref:`rt_mem_limit`
  - :ref:`stopwords`
  - :ref:`stopword_step`
  - :ref:`stopwords_unstemmed`
  - :ref:`type` (plain is not supported)
  - :ref:`wordforms`
  
  
The options must be separated by whitespace.

Default `type` is rt.

Optionally `IF EXISTS` can be used to not return error in case the index already exists.


.. code-block:: mysql

    mysql> CREATE TABLE test1 ( title text, tag integer );
    Query OK, 0 rows affected (0.00 sec)

    mysql> CREATE TABLE test2 (t1 text indexed stored, t2 text indexed, t3 text indexed, gid int) rt_mem_limit='128k' min_infix_len='3';
    Query OK, 0 rows affected (0.00 sec)

    mysql> CREATE TABLE distr type='distributed' local='test1' local='test2';
    Query OK, 0 rows affected (0.00 sec)



Another way of creating a table is by cloning the settings of an existing table:

.. code-block:: mysql

    mysql> CREATE TABLE test2 LIKE test1;
    Query OK, 0 rows affected (0.00 sec)
