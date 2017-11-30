.. _index_command_reference:

Index configuration options
---------------------------

.. _agent_blackhole:

agent_blackhole
~~~~~~~~~~~~~~~

Remote blackhole agent declaration in the `distributed
index <distributed_searching>`. Multi-value, optional,
default is empty.

``agent_blackhole`` lets you fire-and-forget queries to remote agents.
That is useful for debugging (or just testing) production clusters: you
can setup a separate debugging/testing searchd instance, and forward the
requests to this instance from your production master (aggregator)
instance without interfering with production work. Master searchd will
attempt to connect and query blackhole agent normally, but it will
neither wait nor process any responses. Also, all network errors on
blackhole agents will be ignored. The value format is completely
identical to regular
:ref:`agent <agent>` directive.

Example:


.. code-block:: ini


    agent_blackhole = testbox:9312:testindex1,testindex2

.. _agent_connect_timeout:

agent_connect_timeout
~~~~~~~~~~~~~~~~~~~~~

Remote agent connection timeout, in milliseconds. Optional, default is
1000 (ie. 1 second).

When connecting to remote agents, ``searchd`` will wait at most this
much time for connect() call to complete successfully. If the timeout is
reached but connect() does not complete, and
:ref:`retries <set_retries>` are enabled,
retry will be initiated.

Example:


.. code-block:: ini


    agent_connect_timeout = 300

.. _agent_persistent:

agent_persistent
~~~~~~~~~~~~~~~~

Persistently connected remote agent declaration. Multi-value, optional,
default is empty.

``agent_persistent`` directive syntax matches that of the
:ref:`agent <agent>` directive. The
only difference is that the master will **not** open a new connection
to the agent for every query and then close it. Rather, it will keep a
connection open and attempt to reuse for the subsequent queries. The
maximal number of such persistent connections per one agent host is
limited by
:ref:`persistent_connections_limit <persistent_connections_limit>`
option of searchd section.

Note, that you **have** to set the last one in something greater than
0 if you want to use persistent agent connections. Otherwise - when
:ref:`persistent_connections_limit <persistent_connections_limit>`
is not defined, it assumes the zero num of persistent connections, and
‘agent_persistent’ acts exactly as simple ‘agent’.

Persistent master-agent connections reduce TCP port pressure, and save
on connection handshakes. As of time of this writing, they are supported
**only** in workers=threads and workers=threadpool mode. In other
modes, simple non-persistent connections (i.e., one connection per
operation) will be used, and a warning will show up in the console.

Example:


.. code-block:: ini


    agent_persistent = remotebox:9312:index2

.. _agent_query_timeout:

agent_query_timeout
~~~~~~~~~~~~~~~~~~~

Remote agent query timeout, in milliseconds. Optional, default is 3000
(ie. 3 seconds).

After connection, ``searchd`` will wait at most this much time for
remote queries to complete. This timeout is fully separate from
connection timeout; so the maximum possible delay caused by a remote
agent equals to the sum of ``agent_connection_timeout`` and
``agent_query_timeout``. Queries will **not** be retried if this
timeout is reached; a warning will be produced instead.

Example:


.. code-block:: ini


    agent_query_timeout = 10000 # our query can be long, allow up to 10 sec

.. _agent:

agent
~~~~~

Remote agent declaration in the `distributed
index <distributed_searching>`. Multi-value, optional,
default is empty.

``agent`` directive declares remote agents that are searched every time
when the enclosing distributed index is searched. The agents are,
essentially, pointers to networked indexes. The value specifies address,
and also can additionally specify multiple alternatives (agent mirrors)
for either the address only, or the address and index list:

.. code-block:: ini


    agent = address1 [ | address2 [...] ][:index-list]
    agent = address1[:index-list [ | address2[:index-list [...] ] ] ]

In both cases the address specification must be one of the following:

.. code-block:: ini


    address = hostname[:port] # eg. server2:9312
    address = /absolute/unix/socket/path # eg. /var/run/sphinx2.sock

Where ``hostname`` is the remote host name, ``port`` is the remote TCP
port number, ``index-list`` is a comma-separated list of index names,
and square braces [] designate an optional clause.

When index name is omited, it is assumed the same index as the one where
this line is defined. I.e. when defining agents for distributed index
‘mycoolindex’ you can just point the address, and it is assumed to calll
‘mycoolindex’ index on agent's endpoints.

When port number is omited, it is assumed to be default SphinxQL IANA
port (9312). However when portnumber is pointed, but invalid (say, port
70000), it will fail (skip) such agent.

In other words, you can point every single agent to one or more remote
indexes, residing on one or more networked servers. There are absolutely
no restrictions on the pointers. To point out a couple important things,
the host can be localhost, and the remote index can be a distributed
index in turn, all that is legal. That enables a bunch of very different
usage modes:

-  sharding over multiple agent servers, and creating an arbitrary
   cluster topology;

-  sharding over multiple agent servers, mirrored for HA/LB (High
   Availability and Load Balancing) purposes;

-  sharding within localhost, to utilize multiple cores (however, it is
   simpler just to use multiple local indexes and dist_threads
   directive instead);

All agents are searched in parallel. An index list is passed verbatim to
the remote agent. How exactly that list is searched within the agent
(ie. sequentially or in parallel too) depends solely on the agent
configuration (ie. dist_threads directive). Master has no remote
control over that.

The value can additionally enumerate per agent options such as:

-  :ref:`ha_strategy <ha_strategy>` -
   random, roundrobin, nodeads, noerrors (replaces index
   :ref:`ha_strategy <ha_strategy>`
   for particular agent)

-  :ref:`conn <agent_persistent>` -
   pconn, persistent (same as
   :ref:`agent_persistent <agent_persistent>`
   agent declaration)

-  :ref:`blackhole <agent_blackhole>` -
   0,1 (same as
   :ref:`agent_blackhole <agent_blackhole>`
   agent declaration)

.. code-block:: ini


    agent = address1:index-list[[ha_strategy=value] | [conn=value] | [blackhole=value]]

Example:


.. code-block:: ini


    # config on box2
    # sharding an index over 3 servers
    agent = box2:9312:chunk2
    agent = box3:9312:chunk3

    # config on box2
    # sharding an index over 3 servers
    agent = box1:9312:chunk2
    agent = box3:9312:chunk3

    # config on box3
    # sharding an index over 3 servers
    agent = box1:9312:chunk2
    agent = box2:9312:chunk3

    # per agent options
    agent = box1:9312:chunk1[ha_strategy=nodeads]
    agent = box2:9312:chunk2[conn=pconn]
    agent = test:9312:any[blackhole=1]

.. _agent_mirrors:
	
Agent mirrors
^^^^^^^^^^^^^

The syntax lets you define so-called **agent mirrors** that can be
used interchangeably when processing a search query. Master server keeps
track of mirror status (alive or dead) and response times, and does
automatic failover and load balancing based on that. For example, this
line:

.. code-block:: ini


    agent = box1:9312|box2:9312|box3:9312:chunk2

declares that box1:9312, box2:9312, and box3:9312 all have an index
called chunk2, and can be used as interchangeable mirrors. If any single
of those servers go down, the queries will be distributed between the
other two. When it gets back up, master will detect that and begin
routing queries to all three boxes again.

Another way to define the mirrors is to explicitly specify the index
list for every mirror:

.. code-block:: ini


    agent = box1:9312:box1chunk2|box2:9312:box2chunk2

This works essentially the same as the previous example, but different
index names will be used when querying different severs: box1chunk2 when
querying box1:9312, and box2chunk when querying box2:9312.

By default, all queries are routed to the best of the mirrors. The best
one is picked based on the recent statistics, as controlled by the
:ref:`ha_period_karma <ha_period_karma>`
config directive. Master stores a number of metrics (total query count,
error count, response time, etc) recently observed for every agent. It
groups those by time spans, and karma is that time span length. The best
agent mirror is then determined dynamically based on the last 2 such
time spans. Specific algorithm that will be used to pick a mirror can be
configured
:ref:`ha_strategy <ha_strategy>`
directive.

The karma period is in seconds and defaults to 60 seconds. Master stores
up to 15 karma spans with per-agent statistics for instrumentation
purposes (see :ref:`SHOW AGENT STATUS <show_agent_status>`
statement). However, only the last 2 spans out of those are ever used
for HA/LB logic.

When there are no queries, master sends a regular ping command every
:ref:`ha_ping_interval <ha_ping_interval>`
milliseconds in order to have some statistics and at least check,
whether the remote host is still alive. ha_ping_interval defaults to
1000 msec. Setting it to 0 disables pings and statistics will only be
accumulated based on actual queries.

Example:


.. code-block:: ini


    # sharding index over 4 servers total
    # in just 2 chunks but with 2 failover mirrors for each chunk
    # box1, box2 carry chunk1 as local
    # box3, box4 carry chunk2 as local

    # config on box1, box2
    agent = box3:9312|box4:9312:chunk2

    # config on box3, box4
    agent = box1:9312|box2:9312:chunk1

.. _bigram_freq_words:

bigram_freq_words
~~~~~~~~~~~~~~~~~

A list of keywords considered “frequent” when indexing bigrams.
Optional, default is empty.

Bigram indexing is a feature to accelerate phrase searches. When
indexing, it stores a document list for either all or some of the
adjacent words pairs into the index. Such a list can then be used at
searching time to significantly accelerate phrase or sub-phrase
matching.

Some of the bigram indexing modes (see :ref:`bigram_index`)
require to define a list of frequent keywords. These are **not** to
be confused with stopwords! Stopwords are completely eliminated when
both indexing and searching. Frequent keywords are only used by bigrams
to determine whether to index a current word pair or not.

``bigram_freq_words`` lets you define a list of such keywords.

Example:


.. code-block:: ini


    bigram_freq_words = the, a, you, i

.. _bigram_index:

bigram_index
~~~~~~~~~~~~

Bigram indexing mode. Optional, default is none.

Bigram indexing is a feature to accelerate phrase searches. When
indexing, it stores a document list for either all or some of the
adjacent words pairs into the index. Such a list can then be used at
searching time to significantly accelerate phrase or sub-phrase
matching.

``bigram_index`` controls the selection of specific word pairs. The
known modes are:

-  ``all``, index every single word pair. (NB: probably totally not
   worth it even on a moderately sized index, but added anyway for the
   sake of completeness.)

-  ``first_freq``, only index word pairs where the *first* word is in a
   list of frequent words (see :ref:`bigram_freq_words`).
   For example, with ``bigram_freq_words = the, in, i, a``, indexing
   “alone in the dark” text will result in “in the” and “the dark” pairs
   being stored as bigrams, because they begin with a frequent keyword
   (either “in” or “the” respectively), but “alone in” would **not**
   be indexed, because “in” is a *second* word in that pair.

-  ``both_freq``, only index word pairs where both words are frequent.
   Continuing with the same example, in this mode indexing “alone in the
   dark” would only store “in the” (the very worst of them all from
   searching perspective) as a bigram, but none of the other word pairs.

For most usecases, ``both_freq`` would be the best mode, but your
mileage may vary.

Example:


.. code-block:: ini


    bigram_freq_words = both_freq

.. _blend_chars:

blend_chars
~~~~~~~~~~~

Blended characters list. Optional, default is empty.

Blended characters are indexed both as separators and valid characters.
For instance, assume that & is configured as blended and AT&T occurs in
an indexed document. Three different keywords will get indexed, namely
“at&t”, treating blended characters as valid, plus “at” and “t”,
treating them as separators.

Positions for tokens obtained by replacing blended characters with
whitespace are assigned as usual, so regular keywords will be indexed
just as if there was no ``blend_chars`` specified at all. An additional
token that mixes blended and non-blended characters will be put at the
starting position. For instance, if the field contents are “AT&T
company” occurs in the very beginning of the text field, “at” will be
given position 1, “t” position 2, “company” position 3, and “AT&T” will
also be given position 1 (“blending” with the opening regular keyword).
Thus, querying for either AT&T or just AT will match that document, and
querying for “AT T” as a phrase also match it. Last but not least,
phrase query for “AT&T company” will *also* match it, despite the
position

Blended characters can overlap with special characters used in query
syntax (think of T-Mobile or @twitter). Where possible, query parser
will automatically handle blended character as blended. For instance,
“hello @twitter" within quotes (a phrase operator) would handle
@-sign as blended, because @-syntax for field operator is not allowed
within phrases. Otherwise, the character would be handled as an
operator. So you might want to escape the keywords.

Blended characters can be remapped, so that multiple different blended
characters could be normalized into just one base form. This is useful
when indexing multiple alternative Unicode codepoints with equivalent
glyphs.

Example:


.. code-block:: ini


    blend_chars = +, &, U+23
    blend_chars = +, &->+

.. _blend_mode:

blend_mode
~~~~~~~~~~

Blended tokens indexing mode. Optional, default is ``trim_none``.

By default, tokens that mix blended and non-blended characters get
indexed in there entirety. For instance, when both at-sign and an
exclamation are in ``blend_chars``, “@dude!” will get result in two
tokens indexed: “@dude!” (with all the blended characters) and “dude”
(without any). Therefore “@dude" query will *not* match it.

``blend_mode`` directive adds flexibility to this indexing behavior. It
takes a comma-separated list of options.

.. code-block:: ini


    blend_mode = option [, option [, ...]]
    option = trim_none | trim_head | trim_tail | trim_both | skip_pure

Options specify token indexing variants. If multiple options are
specified, multiple variants of the same token will be indexed. Regular
keywords (resulting from that token by replacing blended with
whitespace) are always be indexed.

-  trim_none
-  Index the entire token.

-  trim_head
-  Trim heading blended characters, and index the resulting token.

-  trim_tail
-  Trim trailing blended characters, and index the resulting token.

-  trim_both
-  Trim both heading and trailing blended characters, and index the
   resulting token.

-  skip_pure
-  Do not index the token if it's purely blended, that is, consists of
   blended characters only.

Returning to the “@dude!” example above, setting
``blend_mode = trim_head, trim_tail`` will result in two tokens being
indexed, “@dude" and”dude!“. In this particular example,
``trim_both`` would have no effect, because trimming both blended
characters results in”dude" which is already indexed as a regular
keyword. Indexing “@U.S.A.” with ``trim_both`` (and assuming that dot is
blended two) would result in “U.S.A” being indexed. Last but not least,
``skip_pure`` enables you to fully ignore sequences of blended
characters only. For example, “one @@@ two” would be indexed exactly as
“one two”, and match that as a phrase. That is not the case by default
because a fully blended token gets indexed and offsets the second
keyword position.

Default behavior is to index the entire token, equivalent to
``blend_mode = trim_none``.

Example:


.. code-block:: ini


    blend_mode = trim_tail, skip_pure

.. _charset_table:

charset_table
~~~~~~~~~~~~~

Accepted characters table, with case folding rules. Optional, default
value are latin and cyrillic characters.

charset_table is the main workhorse of Manticore tokenizing process, ie.
the process of extracting keywords from document text or query text. It
controls what characters are accepted as valid and what are not, and how
the accepted characters should be transformed (eg. should the case be
removed or not).

You can think of charset_table as of a big table that has a mapping for
each and every of 100K+ characters in Unicode. By default, every
character maps to 0, which means that it does not occur within keywords
and should be treated as a separator. Once mentioned in the table,
character is mapped to some other character (most frequently, either to
itself or to a lowercase letter), and is treated as a valid keyword
part.

The expected value format is a commas-separated list of mappings. Two
simplest mappings simply declare a character as valid, and map a single
character to another single character, respectively. But specifying the
whole table in such form would result in bloated and barely manageable
specifications. So there are several syntax shortcuts that let you map
ranges of characters at once. The complete list is as follows:

-  A->a
-  Single char mapping, declares source char ‘A’ as allowed to occur
   within keywords and maps it to destination char ‘a’ (but does *not*
   declare ‘a’ as allowed).

-  A..Z->a..z
-  Range mapping, declares all chars in source range as allowed and maps
   them to the destination range. Does *not* declare destination range
   as allowed. Also checks ranges' lengths (the lengths must be equal).

-  a
-  Stray char mapping, declares a character as allowed and maps it to
   itself. Equivalent to a->a single char mapping.

-  a..z
-  Stray range mapping, declares all characters in range as allowed and
   maps them to themselves. Equivalent to a..z->a..z range mapping.

-  A..Z/2
-  Checkerboard range map. Maps every pair of chars to the second char.
   More formally, declares odd characters in range as allowed and maps
   them to the even ones; also declares even characters as allowed and
   maps them to themselves. For instance, A..Z/2 is equivalent to A->B,
   B->B, C->D, D->D, …, Y->Z, Z->Z. This mapping shortcut is helpful for
   a number of Unicode blocks where uppercase and lowercase letters go
   in such interleaved order instead of contiguous chunks.

Control characters with codes from 0 to 31 are always treated as
separators. Characters with codes 32 to 127, ie. 7-bit ASCII characters,
can be used in the mappings as is. To avoid configuration file encoding
issues, 8-bit ASCII characters and Unicode characters must be specified
in U+xxx form, where ‘xxx’ is hexadecimal codepoint number. This form
can also be used for 7-bit ASCII characters to encode special ones: eg.
use U+20 to encode space, U+2E to encode dot, U+2C to encode comma.

Aliases “english” and “russian” are allowed at control character
mapping.

Example:


.. code-block:: ini


    # default are English and Russian letters
    charset_table = 0..9, A..Z->a..z, _, a..z, \
        U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

    # english charset defined with alias
    charset_table = 0..9, english, _

.. _dict:

dict
~~~~

The keywords dictionary type. Known values are ‘crc’ and ‘keywords’.
‘crc’ is DEPRECATED. Use ‘keywords’ instead. Optional, default is
‘keywords’.

Keywords dictionary mode (dict=keywords), (greatly) reduces indexing
impact and enable substring searches on huge collections. They also
eliminate the chance of CRC32 collisions. That mode supported both for
disk and RT indexes.

CRC dictionaries never store the original keyword text in the index.
Instead, keywords are replaced with their control sum value (either
CRC32 or FNV64, depending whether Manticore was built with
``--enable-id64``) both when searching and indexing, and that value is
used internally in the index.

That approach has two drawbacks. First, in CRC32 case there is a chance
of control sum collision between several pairs of different keywords,
growing quadratically with the number of unique keywords in the index.
(FNV64 case is unaffected in practice, as a chance of a single FNV64
collision in a dictionary of 1 billion entries is approximately 1:16, or
6.25 percent. And most dictionaries will be much more compact that a
billion keywords, as a typical spoken human language has in the region
of 1 to 10 million word forms.) Second, and more importantly, substring
searches are not directly possible with control sums. Manticore alleviated
that by pre-indexing all the possible substrings as separate keywords
(see :ref:`min_prefix_len`,
:ref:`min_infix_len`
directives). That actually has an added benefit of matching substrings
in the quickest way possible. But at the same time pre-indexing all
substrings grows the index size a lot (factors of 3-10x and even more
would not be unusual) and impacts the indexing time respectively,
rendering substring searches on big indexes rather impractical.

Keywords dictionary fixes both these drawbacks. It stores the keywords
in the index and performs search-time wildcard expansion. For example, a
search for a 'test\*‘prefix could internally expand to
'test\|tests\|testing’ query based on the dictionary contents. That
expansion is fully transparent to the application, except that the
separate per-keyword statistics for all the actually matched keywords
would now also be reported.

For substring (infix) search extended wildcards may be used. Special
symbols like ‘?’ and ‘%’ are supported along with substring (infix)
search (e.g. “t?st\ *“,”run%“,”*\ abc\*“). Note, however, these
wildcards work only with dict=keywords, and not elsewhere.

Indexing with keywords dictionary should be 1.1x to 1.3x slower compared
to regular, non-substring indexing - but times faster compared to
substring indexing (either prefix or infix). Index size should only be
slightly bigger that than of the regular non-substring index, with a
1..10% percent total difference. Regular keyword searching time must be
very close or identical across all three discussed index kinds (CRC
non-substring, CRC substring, keywords). Substring searching time can
vary greatly depending on how many actual keywords match the given
substring (in other words, into how many keywords does the search term
expand). The maximum number of keywords matched is restricted by the
:ref:`expansion_limit <expansion_limit>`
directive.

Essentially, keywords and CRC dictionaries represent the two different
trade-off substring searching decisions. You can choose to either
sacrifice indexing time and index size in favor of top-speed worst-case
searches (CRC dictionary), or only slightly impact indexing time but
sacrifice worst-case searching time when the prefix expands into very
many keywords (keywords dictionary).

Example:


.. code-block:: ini


    dict = keywords

.. _docinfo:

docinfo
~~~~~~~

Document attribute values (docinfo) storage mode. Optional, default is
‘extern’. Known values are ‘none’, ‘extern’ and ‘inline’.

Docinfo storage mode defines how exactly docinfo will be physically
stored on disk and RAM. “none” means that there will be no docinfo at
all (ie. no attributes). Normally you need not to set “none” explicitly
because Manticore will automatically select “none” when there are no
attributes configured. “inline” means that the docinfo will be stored in
the ``.spd`` file, along with the document ID lists. “extern” means that
the docinfo will be stored separately (externally) from document ID
lists, in a special ``.spa`` file.

Basically, externally stored docinfo must be kept in RAM when querying.
for performance reasons. So in some cases “inline” might be the only
option. However, such cases are infrequent, and docinfo defaults to
“extern”. Refer to :ref:`attributes` for in-depth discussion and RAM
usage estimates.

Example:


.. code-block:: ini


    docinfo = inline

.. _embedded_limit:

embedded_limit
~~~~~~~~~~~~~~

Embedded exceptions, wordforms, or stopwords file size limit. Optional,
default is 16K.

Indexer can either save the file name, or embed the file contents
directly into the index. Files sized under ``embedded_limit`` get stored
into the index. For bigger files, only the file names are stored. This
also simplifies moving index files to a different machine; you may get
by just copying a single file.

With smaller files, such embedding reduces the number of the external
files on which the index depends, and helps maintenance. But at the same
time it makes no sense to embed a 100 MB wordforms dictionary into a
tiny delta index. So there needs to be a size threshold, and
``embedded_limit`` is that threshold.

Example:


.. code-block:: ini


    embedded_limit = 32K

.. _exceptions:

exceptions
~~~~~~~~~~

Tokenizing exceptions file. Optional, default is empty.

Exceptions allow to map one or more tokens (including tokens with
characters that would normally be excluded) to a single keyword. They
are similar to
:ref:`wordforms <wordforms>` in that
they also perform mapping, but have a number of important differences.

Small enough files are stored in the index header, see :ref:`embedded_limit`
for details.

Short summary of the differences is as follows:

-  exceptions are case sensitive, wordforms are not;

-  exceptions can use special characters that are **not** in
   charset_table, wordforms fully obey charset_table;

-  exceptions can underperform on huge dictionaries, wordforms handle
   millions of entries well.

The expected file format is also plain text, with one line per
exception, and the line format is as follows:

.. code-block:: ini


    map-from-tokens => map-to-token

Example file:

.. code-block:: ini


    at & t => at&t
    AT&T => AT&T
    Standarten   Fuehrer => standartenfuhrer
    Standarten Fuhrer => standartenfuhrer
    MS Windows => ms windows
    Microsoft Windows => ms windows
    C++ => cplusplus
    c++ => cplusplus
    C plus plus => cplusplus

All tokens here are case sensitive: they will **not** be processed by
:ref:`charset_table <charset_table>`
rules. Thus, with the example exceptions file above, “at&t” text will be
tokenized as two keywords “at” and “t”, because of lowercase letters. On
the other hand, “AT&T” will match exactly and produce single “AT&T”
keyword.

Note that this map-to keyword is a) always interpreted as a *single*
word, and b) is both case and space sensitive! In our sample, “ms
windows” query will *not* match the document with “MS Windows” text. The
query will be interpreted as a query for two keywords, “ms” and
“windows”. And what “MS Windows” gets mapped to is a *single* keyword
“ms windows”, with a space in the middle. On the other hand,
“standartenfuhrer” will retrieve documents with “Standarten Fuhrer” or
“Standarten Fuehrer” contents (capitalized exactly like this), or any
capitalization variant of the keyword itself, eg. “staNdarTenfUhreR”.
(It won't catch “standarten fuhrer”, however: this text does not match
any of the listed exceptions because of case sensitivity, and gets
indexed as two separate keywords.)

Whitespace in the map-from tokens list matters, but its amount does not.
Any amount of the whitespace in the map-form list will match any other
amount of whitespace in the indexed document or query. For instance, “AT
& T” map-from token will match “AT & T” text, whatever the amount of
space in both map-from part and the indexed text. Such text will
therefore be indexed as a special “AT&T” keyword, thanks to the very
first entry from the sample.

Exceptions also allow to capture special characters (that are exceptions
from general
:ref:`charset_table <charset_table>`
rules; hence the name). Assume that you generally do not want to treat
‘+’ as a valid character, but still want to be able search for some
exceptions from this rule such as ‘C++’. The sample above will do just
that, totally independent of what characters are in the table and what
are not.

Exceptions are applied to raw incoming document and query data during
indexing and searching respectively. Therefore, to pick up changes in
the file it's required to reindex and restart ``searchd``.

Example:


.. code-block:: ini


    exceptions = /usr/local/sphinx/data/exceptions.txt

.. _expand_keywords:

expand_keywords
~~~~~~~~~~~~~~~

Expand keywords with exact forms and/or stars when possible. Optional,
default is 0 (do not expand keywords).

Queries against indexes with ``expand_keywords`` feature enabled are
internally expanded as follows. If the index was built with prefix or
infix indexing enabled, every keyword gets internally replaced with a
disjunction of keyword itself and a respective prefix or infix (keyword
with stars). If the index was built with both stemming and
:ref:`index_exact_words <index_exact_words>`
enabled, exact form is also added. Here's an example that shows how
internal expansion works when all of the above (infixes, stemming, and
exact words) are combined:

.. code-block:: ini


    running -> ( running | *running* | =running )

Expanded queries take naturally longer to complete, but can possibly
improve the search quality, as the documents with exact form matches
should be ranked generally higher than documents with stemmed or infix
matches.

Note that the existing query syntax does not allow to emulate this kind
of expansion, because internal expansion works on keyword level and
expands keywords within phrase or quorum operators too (which is not
possible through the query syntax).

This directive does not affect ``indexer`` in any way, it only affects
``searchd``.

Example:


.. code-block:: ini


    expand_keywords = 1

.. _global_idf:

global_idf
~~~~~~~~~~

The path to a file with global (cluster-wide) keyword IDFs. Optional,
default is empty (use local IDFs).

On a multi-index cluster, per-keyword frequencies are quite likely to
differ across different indexes. That means that when the ranking
function uses TF-IDF based values, such as BM25 family of factors, the
results might be ranked slightly different depending on what cluster
node they reside.

The easiest way to fix that issue is to create and utilize a global
frequency dictionary, or a global IDF file for short. This directive
lets you specify the location of that file. It it suggested (but not
required) to use a .idf extension. When the IDF file is specified for a
given index *and* and OPTION global_idf is set to 1, the engine will
use the keyword frequencies and collection documents count from the
global_idf file, rather than just the local index. That way, IDFs and
the values that depend on them will stay consistent across the cluster.

IDF files can be shared across multiple indexes. Only a single copy of
an IDF file will be loaded by ``searchd``, even when many indexes refer
to that file. Should the contents of an IDF file change, the new
contents can be loaded with a SIGHUP.

You can build an .idf file using ``indextool`` utility, by dumping
dictionaries using ``--dumpdict`` switch first, then converting those to
.idf format using ``--buildidf``, then merging all .idf files across
cluser using ``--mergeidf``. Refer to :ref:`indextool_command_reference` for more
information.

Example:


.. code-block:: ini


    global_idf = /usr/local/sphinx/var/global.idf

.. _ha_strategy:

ha_strategy
~~~~~~~~~~~

Agent mirror selection strategy, for load balancing. Optional, default
is random.

The strategy used for mirror selection, or in other words, choosing a
specific :ref:`agent mirror <agent>`
in a distributed index. Essentially, this directive controls how exactly
master does the load balancing between the configured mirror agent
nodes. The following strategies are implemented:

Simple random balancing
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: ini

    ha_strategy = random

The default balancing mode. Simple linear random distribution among the
mirrors. That is, equal selection probability are assigned to every
mirror. Kind of similar to round-robin (RR), but unlike RR, does not
impose a strict selection order.

Adaptive randomized balancing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The default simple random strategy does not take mirror status, error
rate, and, most importantly, actual response latencies into account. So
to accommodate for heterogeneous clusters and/or temporary spikes in
agent node load, we have a group of balancing strategies that
dynamically adjusts the probabilities based on the actual query
latencies observed by the master.

The adaptive strategies based on **latency-weighted probabilities**
basically work as follows:

-  latency stats are accumulated, in blocks of ha_period_karma
   seconds;

-  once per karma period, latency-weighted probabilities get recomputed;

-  once per request (including ping requests), “dead or alive” flag is
   adjusted.

Currently, we begin with equal probabilities (or percentages, for
brevity), and on every step, scale them by the inverse of the latencies
observed during the last “karma” period, and then renormalize them. For
example, if during the first 60 seconds after the master startup 4
mirrors had latencies of 10, 5, 30, and 3 msec/query respectively, the
first adjustment step would go as follow:

-  initial percentages: 0.25, 0.25, 0.25, 0.2%;

-  observed latencies: 10 ms, 5 ms, 30 ms, 3 ms;

-  inverse latencies: 0.1, 0.2, 0.0333, 0.333;

-  scaled percentages: 0.025, 0.05, 0.008333, 0.0833;

-  renormalized percentages: 0.15, 0.30, 0.05, 0.50.

Meaning that the 1st mirror would have a 15% chance of being chosen
during the next karma period, the 2nd one a 30% chance, the 3rd one
(slowest at 30 ms) only a 5% chance, and the 4th and the fastest one (at
3 ms) a 50% chance. Then, after that period, the second adjustment step
would update those chances again, and so on.

The rationale here is, once the **observed latencies** stabilize, the
**latency weighted probabilities** stabilize as well. So all these
adjustment iterations are supposed to converge at a point where the
average latencies are (roughly) equal over all mirrors.

.. code-block:: ini

    ha_strategy = nodeads

Latency-weighted probabilities, but dead mirrors are excluded from the
selection. “Dead” mirror is defined as a mirror that resulted in
multiple hard errors (eg. network failure, or no answer, etc) in a row.

.. code-block:: ini

    ha_strategy = noerrors

Latency-weighted probabilities, but mirrors with worse errors/success
ratio are excluded from the selection.

Round-robin balancing
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: ini

    ha_strategy = roundrobin

Simple round-robin selection, that is, selecting the 1st mirror in the
list, then the 2nd one, then the 3rd one, etc, and then repeating the
process once the last mirror in the list is reached. Unlike with the
randomized strategies, RR imposes a strict querying order (1, 2, 3, ..,
N-1, N, 1, 2, 3, … and so on) and *guarantees* that no two subsequent
queries will be sent to the same mirror.

.. _hitless_words:

hitless_words
~~~~~~~~~~~~~

Hitless words list. Optional, allowed values are ‘all’, or a list file
name.

By default, Manticore full-text index stores not only a list of matching
documents for every given keyword, but also a list of its in-document
positions (aka hitlist). Hitlists enables phrase, proximity, strict
order and other advanced types of searching, as well as phrase proximity
ranking. However, hitlists for specific frequent keywords (that can not
be stopped for some reason despite being frequent) can get huge and thus
slow to process while querying. Also, in some cases we might only care
about boolean keyword matching, and never need position-based searching
operators (such as phrase matching) nor phrase ranking.

``hitless_words`` lets you create indexes that either do not have
positional information (hitlists) at all, or skip it for specific
keywords.

Hitless index will generally use less space than the respective regular
index (about 1.5x can be expected). Both indexing and searching should
be faster, at a cost of missing positional query and ranking support.
When searching, positional queries (eg. phrase queries) will be
automatically converted to respective non-positional (document-level) or
combined queries. For instance, if keywords “hello” and “world” are
hitless, “hello world” phrase query will be converted to (hello & world)
bag-of-words query, matching all documents that mention either of the
keywords but not necessarily the exact phrase. And if, in addition,
keywords “simon” and “says” are not hitless, “simon says hello world”
will be converted to (“simon says” & hello & world) query, matching all
documents that contain “hello” and “world” anywhere in the document, and
also “simon says” as an exact phrase.

Example:


.. code-block:: ini


    hitless_words = all

.. _html_index_attrs:

html_index_attrs
~~~~~~~~~~~~~~~~

A list of markup attributes to index when stripping HTML. Optional,
default is empty (do not index markup attributes).

Specifies HTML markup attributes whose contents should be retained and
indexed even though other HTML markup is stripped. The format is per-tag
enumeration of indexable attributes, as shown in the example below.

Example:


.. code-block:: ini


    html_index_attrs = img=alt,title; a=title;

.. _html_remove_elements:

html_remove_elements
~~~~~~~~~~~~~~~~~~~~

A list of HTML elements for which to strip contents along with the
elements themselves. Optional, default is empty string (do not strip
contents of any elements).

This feature allows to strip element contents, ie. everything that is
between the opening and the closing tags. It is useful to remove
embedded scripts, CSS, etc. Short tag form for empty elements (ie. <br
/>) is properly supported; ie. the text that follows such tag will
**not** be removed.

The value is a comma-separated list of element (tag) names whose
contents should be removed. Tag names are case insensitive.

Example:


.. code-block:: ini


    html_remove_elements = style, script

.. _html_strip:

html_strip
~~~~~~~~~~
Whether to strip HTML markup from incoming full-text data. Optional,
default is 0. Known values are 0 (disable stripping) and 1 (enable
stripping).

Both HTML tags and entities and considered markup and get processed.

HTML tags are removed, their contents (i.e., everything between <P> and
</P>) are left intact by default. You can choose to keep and index
attributes of the tags (e.g., HREF attribute in an A tag, or ALT in an
IMG one). Several well-known inline tags are completely removed, all
other tags are treated as block level and replaced with whitespace. For
example, ‘te<B>st</B>’ text will be indexed as a single keyword ‘test’,
however, ‘te<P>st</P>’ will be indexed as two keywords ‘te’ and ‘st’.
Known inline tags are as follows: A, B, I, S, U, BASEFONT, BIG, EM,
FONT, IMG, LABEL, SMALL, SPAN, STRIKE, STRONG, SUB, SUP, TT.

HTML entities get decoded and replaced with corresponding UTF-8
characters. Stripper supports both numeric forms (such as &#239;) and
text forms (such as &oacute; or &nbsp;). All entities as specified by
HTML4 standard are supported.

Stripping should work with properly formed HTML and XHTML, but, just as
most browsers, may produce unexpected results on malformed input (such
as HTML with stray <'s or unclosed >'s).

Only the tags themselves, and also HTML comments, are stripped. To strip
the contents of the tags too (eg. to strip embedded scripts), see
:ref:`html_remove_elements <html_remove_elements>`
option. There are no restrictions on tag names; ie. everything that
looks like a valid tag start, or end, or a comment will be stripped.

Example:


.. code-block:: ini


    html_strip = 1

.. _ignore_chars:

ignore_chars
~~~~~~~~~~~~

Ignored characters list. Optional, default is empty.

Useful in the cases when some characters, such as soft hyphenation mark
(U+00AD), should be not just treated as separators but rather fully
ignored. For example, if ‘-’ is simply not in the charset_table,
“abc-def” text will be indexed as “abc” and “def” keywords. On the
contrary, if ‘-’ is added to ignore_chars list, the same text will be
indexed as a single “abcdef” keyword.

The syntax is the same as for
:ref:`charset_table <charset_table>`,
but it's only allowed to declare characters, and not allowed to map
them. Also, the ignored characters must not be present in
charset_table.

Example:


.. code-block:: ini


    ignore_chars = U+AD

.. _index_exact_words:

index_exact_words
~~~~~~~~~~~~~~~~~

Whether to index the original keywords along with the stemmed/remapped
versions. Optional, default is 0 (do not index).

When enabled, ``index_exact_words`` forces ``indexer`` to put the raw
keywords in the index along with the stemmed versions. That, in turn,
enables :ref:`exact form operator <extended_query_syntax>` in the
query language to work. This impacts the index size and the indexing
time. However, searching performance is not impacted at all.

Example:


.. code-block:: ini


    index_exact_words = 1

.. _index_field_lengths:

index_field_lengths
~~~~~~~~~~~~~~~~~~~

Enables computing and storing of field lengths (both per-document and
average per-index values) into the index. Optional, default is 0 (do not
compute and store).

When ``index_field_lengths`` is set to 1, ``indexer`` will 1) create a
respective length attribute for every full-text field, sharing the same
name but with __len_ suffix; 2) compute a field length (counted in
keywords) for every document and store in to a respective attribute; 3)
compute the per-index averages. The lengths attributes will have a
special TOKENCOUNT type, but their values are in fact regular 32-bit
integers, and their values are generally accessible.

BM25A() and BM25F() functions in the expression ranker are based on
these lengths and require ``index_field_lengths`` to be enabled.
Historically, Manticore used a simplified, stripped-down variant of BM25
that, unlike the complete function, did **not** account for document
length. (We later realized that it should have been called BM15 from the
start.) Also we added support for both a complete variant of BM25, and
its extension towards multiple fields, called BM25F. They require
per-document length and per-field lengths, respectively. Hence the
additional directive.

Example:


.. code-block:: ini


    index_field_lengths = 1

.. _index_sp:

index_sp
~~~~~~~~

Whether to detect and index sentence and paragraph boundaries. Optional,
default is 0 (do not detect and index).

This directive enables sentence and paragraph boundary indexing. It's
required for the SENTENCE and PARAGRAPH operators to work. Sentence
boundary detection is based on plain text analysis, so you only need to
set ``index_sp = 1`` to enable it. Paragraph detection is however based
on HTML markup, and happens in the `HTML
stripper <html_strip>`. So to
index paragraph locations you also need to enable the stripper by
specifying ``html_strip = 1``. Both types of boundaries are detected
based on a few built-in rules enumerated just below.

Sentence boundary detection rules are as follows.

-  Question and exclamation signs (? and !) are always a sentence
   boundary.

-  Trailing dot (.) is a sentence boundary, except:

   -  When followed by a letter. That's considered a part of an
      abbreviation (as in “S.T.A.L.K.E.R” or “Goldman Sachs S.p.A.”).

   -  When followed by a comma. That's considered an abbreviation
      followed by a comma (as in “Telecom Italia S.p.A., founded in
      1994”).

   -  When followed by a space and a small letter. That's considered an
      abbreviation within a sentence (as in “News Corp. announced in
      February”).

   -  When preceded by a space and a capital letter, and followed by a
      space. That's considered a middle initial (as in “John D. Doe”).

Paragraph boundaries are inserted at every block-level HTML tag. Namely,
those are (as taken from HTML 4 standard) ADDRESS, BLOCKQUOTE, CAPTION,
CENTER, DD, DIV, DL, DT, H1, H2, H3, H4, H5, LI, MENU, OL, P, PRE,
TABLE, TBODY, TD, TFOOT, TH, THEAD, TR, and UL.

Both sentences and paragraphs increment the keyword position counter by
1.

Example:


.. code-block:: ini


    index_sp = 1

.. _index_zones:

index_zones
~~~~~~~~~~~

A list of in-field HTML/XML zones to index. Optional, default is empty
(do not index zones).

Zones can be formally defined as follows. Everything between an opening
and a matching closing tag is called a span, and the aggregate of all
spans corresponding sharing the same tag name is called a zone. For
instance, everything between the occurrences of <H1> and </H1> in the
document field belongs to H1 zone.

Zone indexing, enabled by ``index_zones`` directive, is an optional
extension of the HTML stripper. So it will also require that the
:ref:`stripper <html_strip>` is enabled
(with ``html_strip = 1``). The value of the ``index_zones`` should be a
comma-separated list of those tag names and wildcards (ending with a
star) that should be indexed as zones.

Zones can nest and overlap arbitrarily. The only requirement is that
every opening tag has a matching tag. You can also have an arbitrary
number of both zones (as in unique zone names, such as H1) and spans
(all the occurrences of those H1 tags) in a document. Once indexed,
zones can then be used for matching with the ZONE operator, see :ref:`extended_query_syntax`.

Example:


.. code-block:: ini


    index_zones = h*, th, title

.. _infix_fields:

infix_fields
~~~~~~~~~~~~

The list of full-text fields to limit infix indexing to. Applies to
dict=crc only. Optional, default is empty (index all fields in infix
mode).

Similar to
:ref:`prefix_fields <prefix_fields>`,
but lets you limit infix-indexing to given fields.

Example:


.. code-block:: ini


    infix_fields = url, domain

.. _inplace_docinfo_gap:

inplace_docinfo_gap
~~~~~~~~~~~~~~~~~~~

`In-place
inversion <inplaceenable>`
fine-tuning option. Controls preallocated docinfo gap size. Optional,
default is 0.

This directive does not affect ``searchd`` in any way, it only affects
``indexer``.

Example:


.. code-block:: ini


    inplace_docinfo_gap = 1M

.. _inplace_enable:

inplace_enable
~~~~~~~~~~~~~~

Whether to enable in-place index inversion. Optional, default is 0 (use
separate temporary files).

``inplace_enable`` greatly reduces indexing disk footprint, at a cost of
slightly slower indexing (it uses around 2x less disk, but yields around
90-95% the original performance).

Indexing involves two major phases. The first phase collects, processes,
and partially sorts documents by keyword, and writes the intermediate
result to temporary files (.tmp\*). The second phase fully sorts the
documents, and creates the final index files. Thus, rebuilding a
production index on the fly involves around 3x peak disk footprint: 1st
copy for the intermediate temporary files, 2nd copy for newly
constructed copy, and 3rd copy for the old index that will be serving
production queries in the meantime. (Intermediate data is comparable in
size to the final index.) That might be too much disk footprint for big
data collections, and ``inplace_enable`` allows to reduce it. When
enabled, it reuses the temporary files, outputs the final data back to
them, and renames them on completion. However, this might require
additional temporary data chunk relocation, which is where the
performance impact comes from.

This directive does not affect ``searchd`` in any way, it only affects
``indexer``.

Example:


.. code-block:: ini


    inplace_enable = 1

.. _inplace_hit_gap:

inplace_hit_gap
~~~~~~~~~~~~~~~

`In-place
inversion <inplaceenable>`
fine-tuning option. Controls preallocated hitlist gap size. Optional,
default is 0.

This directive does not affect ``searchd`` in any way, it only affects
``indexer``.

Example:


.. code-block:: ini


    inplace_hit_gap = 1M

.. _inplace_reloc_factor:

inplace_reloc_factor
~~~~~~~~~~~~~~~~~~~~

:ref:`inplace_reloc_factor` fine-tuning option.
Controls relocation buffer size within indexing memory arena. Optional,
default is 0.1.

This directive does not affect ``searchd`` in any way, it only affects
``indexer``.

Example:


.. code-block:: ini


    inplace_reloc_factor = 0.1

.. _inplace_write_factor:

inplace_write_factor
~~~~~~~~~~~~~~~~~~~~

:ref:`inplace_write_factor` fine-tuning option.
Controls in-place write buffer size within indexing memory arena.
Optional, default is 0.1.

This directive does not affect ``searchd`` in any way, it only affects
``indexer``.

Example:


.. code-block:: ini


    inplace_write_factor = 0.1

.. _local:

local
~~~~~

Local index declaration in the `distributed
index <distributed_searching>`. Multi-value, optional,
default is empty.

This setting is used to declare local indexes that will be searched when
given distributed index is searched. Many local indexes can be declared
per each distributed index. Any local index can also be mentioned
several times in different distributed indexes.

Note that by default all local indexes will be searched
**sequentially**, utilizing only 1 CPU or core. To parallelize
processing of the local parts in the distributed index, you should use
:ref:`dist_threads`.

Before ``dist_threads``, there also was a legacy solution to configure
``searchd`` to query itself instead of using local indexes (refer to
:ref:`agent` for the
details). However, that creates redundant CPU and network load, and
``dist_threads`` is now strongly suggested instead.

Example:


.. code-block:: ini


    local = chunk1
    local = chunk2

.. _max_substring_len:

max_substring_len
~~~~~~~~~~~~~~~~~

Maximum substring (either prefix or infix) length to index. Optional,
default is 0 (do not limit indexed substrings). Applies to dict=crc
only.

By default, substring (either prefix or infix) indexing in the `dict=crc
mode <dict>` will index
**all** the possible substrings as separate keywords. That might
result in an overly large index. So the ``max_substring_len`` directive
lets you limit the impact of substring indexing by skipping too-long
substrings (which, chances are, will never get searched for anyway).

For example, a test index of 10,000 blog posts takes this much disk
space depending on the settings:

-  6.4 MB baseline (no substrings)
-  24.3 MB (3.8x) with min_prefix_len = 3
-  22.2 MB (3.5x) with min_prefix_len = 3, max_substring_len = 8
-  19.3 MB (3.0x) with min_prefix_len = 3, max_substring_len = 6
-  94.3 MB (14.7x) with min_infix_len = 3
-  84.6 MB (13.2x) with min_infix_len = 3, max_substring_len = 8
-  70.7 MB (11.0x) with min_infix_len = 3, max_substring_len = 6

So in this test limiting the max substring length saved us 10-15% on the
index size.

There is no performance impact associated with substring length when
using dict=keywords mode, so this directive is not applicable and
intentionally forbidden in that case. If required, you can still limit
the length of a substring that you search for in the application code.

Example:


.. code-block:: ini


    max_substring_len = 12

.. _min_infix_len:

min_infix_len
~~~~~~~~~~~~~

Minimum infix prefix length to index and search. Optional, default is 0
(do not index infixes), and minimum allowed non-zero value is 2.

Infix length setting enables wildcard searches with term patterns like
‘start\ *', '*\ end’, ‘*middle*’, and so on. It also lets you disable
too short wildcards if those are too expensive to search for.

Perfect word matches can be differentiated from infix matches, and
ranked higher, by utilizing all of the following options: a)
dict=keywords (on by default), b) index_exact_words=1 (off by
default), and c) expand_keywords=1 (also off by default). Note that
either with the legacy dict=crc mode (which you should ditch anyway!),
or with any of the above options disable, there is no data to
differentiate between the infixes and full words, and thus perfect word
matches can't be ranked higher.

However, query time might vary greatly, depending on how many keywords
the substring will actually expand to. Short and frequent syllables like
‘*in*’ or ‘*ti*’ just might expand to way too many keywords, all of
which would need to be matched and processed. Therefore, to generally
enable substring searches you would set min_infix_len to 2; and to
limit the impact from wildcard searches with too short wildcards, you
might set it higher.

Infixes must be at least 2 characters long, wildcards like ‘*a*’ are not
allowed for performance reasons. (While in theory it is possible to scan
the entire dictionary, identify keywords matching on just a single
character, expand ‘*a*’ to an OR operator over 100,000+ keywords, and
evaluate that expanded query, in practice this will very definitely kill
your server.)

Example:


.. code-block:: ini


    min_infix_len = 3

.. _min_prefix_len:

min_prefix_len
~~~~~~~~~~~~~~

Minimum word prefix length to index. Optional, default is 0 (do not
index prefixes).

Prefix indexing allows to implement wildcard searching by 'wordstart\*'
wildcards. When mininum prefix length is set to a positive number,
indexer will index all the possible keyword prefixes (ie. word
beginnings) in addition to the keywords themselves. Too short prefixes
(below the minimum allowed length) will not be indexed.

For instance, indexing a keyword “example” with min_prefix_len=3 will
result in indexing “exa”, “exam”, “examp”, “exampl” prefixes along with
the word itself. Searches against such index for “exam” will match
documents that contain “example” word, even if they do not contain
“exam” on itself. However, indexing prefixes will make the index grow
significantly (because of many more indexed keywords), and will degrade
both indexing and searching times.

Perfect word matches can be differentiated from prefix matches, and
ranked higher, by utilizing all of the following options: a)
dict=keywords (on by default), b) index_exact_words=1 (off by
default), and c) expand_keywords=1 (also off by default). Note that
either with the legacy dict=crc mode (which you should ditch anyway!),
or with any of the above options disable, there is no data to
differentiate between the prefixes and full words, and thus perfect word
matches can't be ranked higher.

Example:


.. code-block:: ini


    min_prefix_len = 3

.. _min_stemming_len:

min_stemming_len
~~~~~~~~~~~~~~~~

Minimum word length at which to enable stemming. Optional, default is 1
(stem everything).

Stemmers are not perfect, and might sometimes produce undesired results.
For instance, running “gps” keyword through Porter stemmer for English
results in “gp”, which is not really the intent. ``min_stemming_len``
feature lets you suppress stemming based on the source word length, ie.
to avoid stemming too short words. Keywords that are shorter than the
given threshold will not be stemmed. Note that keywords that are exactly
as long as specified **will** be stemmed. So in order to avoid
stemming 3-character keywords, you should specify 4 for the value. For
more finely grained control, refer to
:ref:`wordforms <wordforms>` feature.

Example:


.. code-block:: ini


    min_stemming_len = 4

.. _min_word_len:

min_word_len
~~~~~~~~~~~~

Minimum indexed word length. Optional, default is 1 (index everything).

Only those words that are not shorter than this minimum will be indexed.
For instance, if min_word_len is 4, then ‘the’ won't be indexed, but
‘they’ will be.

Example:


.. code-block:: ini


    min_word_len = 4

.. _mlock:

mlock
~~~~~

Memory locking for cached data. Optional, default is 0 (do not call
mlock()).

For search performance, ``searchd`` preloads a copy of ``.spa`` and
``.spi`` files in RAM, and keeps that copy in RAM at all times. But if
there are no searches on the index for some time, there are no accesses
to that cached copy, and OS might decide to swap it out to disk. First
queries to such “cooled down” index will cause swap-in and their latency
will suffer.

Setting mlock option to 1 makes Manticore lock physical RAM used for that
cached data using mlock(2) system call, and that prevents swapping (see
man 2 mlock for details). mlock(2) is a privileged call, so it will
require ``searchd`` to be either run from root account, or be granted
enough privileges otherwise. If mlock() fails, a warning is emitted, but
index continues working.

Example:


.. code-block:: ini


    mlock = 1

.. _morphology:

morphology
~~~~~~~~~~

A list of morphology preprocessors (stemmers or lemmatizers) to apply.
Optional, default is empty (do not apply any preprocessor).

Morphology preprocessors can be applied to the words being indexed to
replace different forms of the same word with the base, normalized form.
For instance, English stemmer will normalize both “dogs” and “dog” to
“dog”, making search results for both searches the same.

There are 3 different morphology preprocessors that Manticore implements:
lemmatizers, stemmers, and phonetic algorithms.

-  Lemmatizer reduces a keyword form to a so-called lemma, a proper
   normal form, or in other words, a valid natural language root word.
   For example, “running” could be reduced to “run”, the infinitive verb
   form, and “octopi” would be reduced to “octopus”, the singular noun
   form. Note that sometimes a word form can have multiple corresponding
   root words. For instance, by looking at “dove” it is not possible to
   tell whether this is a past tense of “dive” the verb as in “He dove
   into a pool.”, or “dove” the noun as in “White dove flew over the
   cuckoo's nest.” In this case lemmatizer can generate all the possible
   root forms.

-  Stemmer reduces a keyword form to a so-called stem by removing and/or
   replacing certain well-known suffixes. The resulting stem is however
   *not* guaranteed to be a valid word on itself. For instance, with
   a Porter English stemmers “running” would still reduce to “run”,
   which is fine, but “business” would reduce to “busi”, which is not a
   word, and “octopi” would not reduce at all. Stemmers are essentially
   (much) simpler but still pretty good replacements of full-blown
   lemmatizers.

-  Phonetic algorithms replace the words with specially crafted phonetic
   codes that are equal even when the words original are different, but
   phonetically close.

The morphology processors that come with our own built-in Manticore
implementations are:

-  English, Russian, and German lemmatizers;

-  English, Russian, Arabic, and Czech stemmers;

-  SoundEx and MetaPhone phonetic algorithms.

You can also link with **libstemmer** library for even more stemmers
(see details below). With libstemmer, Manticore also supports morphological
processing for more than 15 other languages. Binary packages should come
prebuilt with libstemmer support, too.

Lemmatizers require a dictionary that needs to be additionally
downloaded from the Manticore website. That dictionary needs to be
installed in a directory specified by
:ref:`lemmatizer_base <lemmatizer_base>`
directive. Also, there is a
:ref:`lemmatizer_cache <lemmatizer_cache>`
directive that lets you speed up lemmatizing (and therefore indexing) by
spending more RAM for, basically, an uncompressed cache of a dictionary.

Chinese segmentation using Rosette Linguistics Platform is also
available. It is a much more precise but slower way (compared to
n-grams) to segment Chinese documents.
:ref:`charset_table`
must contain all Chinese characters except Chinese punctuation marks
because incoming documents are first processed by sphinx tokenizer and
then the result is processed by RLP. Manticore performs per-token language
detection on the incoming documents. If token language is identified as
Chinese, it will only be processed the RLP, even if multiple morphology
processors are specified. Otherwise, it will be processed by all the
morphology processors specified in the “morphology” option. Rosette
Linguistics Platform must be installed and configured and sphinx must be
built with a –with-rlp switch. See also
:ref:`rlp_root`,
:ref:`rlp_environment`
and :ref:`rlp_context`
options. A batched version of RLP segmentation is also available
(``rlp_chinese_batched``). It provides the same functionality as the
basic ``rlp_chinese`` segmentation, but enables batching documents
before processing them by the RLP. Processing several documents at once
can result in a substantial indexing speedup if the documents are small
(for example, less than 1k). See also
:ref:`rlp_max_batch_size`
and
:ref:`rlp_max_batch_docs`
options.

Additional stemmers provided by
`Snowball <http://snowball.tartarus.org/>`__ project
`libstemmer <http://snowball.tartarus.org/dist/libstemmer_c.tgz>`__
library can be enabled at compile time using ``--with-libstemmer``
``configure`` option. Built-in English and Russian stemmers should be
faster than their libstemmer counterparts, but can produce slightly
different results, because they are based on an older version.

Soundex implementation matches that of MySQL. Metaphone implementation
is based on Double Metaphone algorithm and indexes the primary code.

Built-in values that are available for use in ``morphology`` option are
as follows:

-  none - do not perform any morphology processing;

-  lemmatize_ru - apply Russian lemmatizer and pick a single root form;

-  lemmatize_en - apply English lemmatizer and pick a single root form;

-  lemmatize_de - apply German lemmatizer and pick a single root form;

-  lemmatize_ru_all - apply Russian lemmatizer and index all possible
   root forms;

-  lemmatize_en_all - apply English lemmatizer and index all possible
   root forms;

-  lemmatize_de_all - apply German lemmatizer and index all possible
   root forms;

-  stem_en - apply Porter's English stemmer;

-  stem_ru - apply Porter's Russian stemmer;

-  stem_enru - apply Porter's English and Russian stemmers;

-  stem_cz - apply Czech stemmer;

-  stem_ar - apply Arabic stemmer;

-  soundex - replace keywords with their SOUNDEX code;

-  metaphone - replace keywords with their METAPHONE code.

-  rlp_chinese - apply Chinese text segmentation using Rosette
   Linguistics Platform

-  rlp_chinese_batched - apply Chinese text segmentation using Rosette
   Linguistics Platform with document batching

Additional values provided by libstemmer are in ‘libstemmer_XXX’
format, where XXX is libstemmer algorithm codename (refer to
``libstemmer_c/libstemmer/modules.txt`` for a complete list).

Several stemmers can be specified (comma-separated). They will be
applied to incoming words in the order they are listed, and the
processing will stop once one of the stemmers actually modifies the
word. Also when
:ref:`wordforms <wordforms>` feature
is enabled the word will be looked up in word forms dictionary first,
and if there is a matching entry in the dictionary, stemmers will not be
applied at all. Or in other words,
:ref:`wordforms <wordforms>` can be
used to implement stemming exceptions.

Example:


.. code-block:: ini


    morphology = stem_en, libstemmer_sv

.. _ngram_chars:

ngram_chars
~~~~~~~~~~~

N-gram characters list. Optional, default is empty.

To be used in conjunction with in
:ref:`ngram_len <ngram_len>`, this
list defines characters, sequences of which are subject to N-gram
extraction. Words comprised of other characters will not be affected by
N-gram indexing feature. The value format is identical to
:ref:`charset_table <charset_table>`.

Example:


.. code-block:: ini


    ngram_chars = U+3000..U+2FA1F

.. _ngram_len:

ngram_len
~~~~~~~~~

N-gram lengths for N-gram indexing. Optional, default is 0 (disable
n-gram indexing). Known values are 0 and 1 (other lengths to be
implemented).

N-grams provide basic CJK (Chinese, Japanese, Korean) support for
unsegmented texts. The issue with CJK searching is that there could be
no clear separators between the words. Ideally, the texts would be
filtered through a special program called segmenter that would insert
separators in proper locations. However, segmenters are slow and error
prone, and it's common to index contiguous groups of N characters, or
n-grams, instead.

When this feature is enabled, streams of CJK characters are indexed as
N-grams. For example, if incoming text is “ABCDEF” (where A to F
represent some CJK characters) and length is 1, in will be indexed as if
it was “A B C D E F”. (With length equal to 2, it would produce “AB BC
CD DE EF”; but only 1 is supported at the moment.) Only those characters
that are listed in
:ref:`ngram_chars <ngram_chars>` table
will be split this way; other ones will not be affected.

Note that if search query is segmented, ie. there are separators between
individual words, then wrapping the words in quotes and using extended
mode will result in proper matches being found even if the text was
**not** segmented. For instance, assume that the original query is BC
DEF. After wrapping in quotes on the application side, it should look
like “BC” “DEF” (*with* quotes). This query will be passed to Manticore and
internally split into 1-grams too, resulting in “B C” “D E F” query,
still with quotes that are the phrase matching operator. And it will
match the text even though there were no separators in the text.

Even if the search query is not segmented, Manticore should still produce
good results, thanks to phrase based ranking: it will pull closer phrase
matches (which in case of N-gram CJK words can mean closer
multi-character word matches) to the top.

Example:


.. code-block:: ini


    ngram_len = 1

.. _ondisk_attrs:

ondisk_attrs
~~~~~~~~~~~~

Allows for fine-grain control over how attributes are loaded into memory
when using indexes with external storage. It is possible to keep
attributes on disk. Although, the daemon does map them to memory and the
OS loads small chunks of data on demand. This allows use of docinfo =
extern instead of docinfo = inline, but still leaves plenty of free
memory for cases when you have large collections of pooled attributes
(string/JSON/MVA) or when you're using many indexes per daemon that
don't consume memory. It is not possible to update attributes left on
disk when this option is enabled and the constraint of 4Gb of entries
per pool is still in effect.

Note that this option also affects RT indexes. When it is enabled, all
attribute updates will be disabled, and also all disk chunks of RT
indexes will behave described above. However inserting and deleting of
docs from RT indexes is still possible with enabled ondisk_attrs.

Possible values:


-  0 - disabled and default value, all attributes are loaded in memory
   (the normal behaviour of docinfo = extern)
-  1 - all attributes stay on disk. Daemon loads no files (spa, spm,
   sps). This is the most memory conserving mode, however it is also the
   slowest as the whole doc-id-list and block index doesn't load.
-  pool - only pooled attributes stay on disk. Pooled attributes are
   string, MVA, and JSON attributes (sps, spm files). Scalar attributes
   stored in docinfo (spa file) load as usual.

This option does not affect indexing in any way, it only requires daemon
restart.

Example:


.. code-block:: ini


    ondisk_attrs = pool #keep pooled attributes on disk

.. _overshort_step:

overshort_step
~~~~~~~~~~~~~~

Position increment on overshort (less that
:ref:`min_word_len <min_word_len>`)
keywords. Optional, allowed values are 0 and 1, default is 1.

This directive does not affect ``searchd`` in any way, it only affects
``indexer``.

Example:


.. code-block:: ini


    overshort_step = 1

.. _path:

path
~~~~

Index files path and file name (without extension). Mandatory.

Path specifies both directory and file name, but without extension.
``indexer`` will append different extensions to this path when
generating final names for both permanent and temporary index files.
Permanent data files have several different extensions starting with
‘.sp’; temporary files' extensions start with ‘.tmp’. It's safe to
remove ``.tmp*`` files is if indexer fails to remove them automatically.

For reference, different index files store the following data:

-  ``.spa`` stores document attributes (used in `extern
   docinfo <docinfo>` storage
   mode only);

-  ``.spd`` stores matching document ID lists for each word ID;

-  ``.sph`` stores index header information;

-  ``.spi`` stores word lists (word IDs and pointers to ``.spd`` file);

-  ``.spk`` stores kill-lists;

-  ``.spm`` stores MVA data;

-  ``.spp`` stores hit (aka posting, aka word occurrence) lists for each
   word ID;

-  ``.sps`` stores string attribute data.

-  ``.spe`` stores skip-lists to speed up doc-list filtering

Example:


.. code-block:: ini


    path = /var/data/test1

.. _phrase_boundary:

phrase_boundary
~~~~~~~~~~~~~~~

Phrase boundary characters list. Optional, default is empty.

This list controls what characters will be treated as phrase boundaries,
in order to adjust word positions and enable phrase-level search
emulation through proximity search. The syntax is similar to
:ref:`charset_table <charset_table>`.
Mappings are not allowed and the boundary characters must not overlap
with anything else.

On phrase boundary, additional word position increment (specified by
:ref:`phrase_boundary_step <phrase_boundary_step>`)
will be added to current word position. This enables phrase-level
searching through proximity queries: words in different phrases will be
guaranteed to be more than phrase_boundary_step distance away from
each other; so proximity search within that distance will be equivalent
to phrase-level search.

Phrase boundary condition will be raised if and only if such character
is followed by a separator; this is to avoid abbreviations such as
S.T.A.L.K.E.R or URLs being treated as several phrases.

Example:


.. code-block:: ini


    phrase_boundary = ., ?, !, U+2026 # horizontal ellipsis

.. _phrase_boundary_step:

phrase_boundary_step
~~~~~~~~~~~~~~~~~~~~

Phrase boundary word position increment. Optional, default is 0.

On phrase boundary, current word position will be additionally
incremented by this number. See
:ref:`phrase_boundary <phrase_boundary>`
for details.

Example:


.. code-block:: ini


    phrase_boundary_step = 100

.. _prefix_fields:

prefix_fields
~~~~~~~~~~~~~

The list of full-text fields to limit prefix indexing to. Applies to
dict=crc only. Optional, default is empty (index all fields in prefix
mode).

Because prefix indexing impacts both indexing and searching performance,
it might be desired to limit it to specific full-text fields only: for
instance, to provide prefix searching through URLs, but not through page
contents. prefix_fields specifies what fields will be prefix-indexed;
all other fields will be indexed in normal mode. The value format is a
comma-separated list of field names.

Example:


.. code-block:: ini


    prefix_fields = url, domain

.. _preopen:

preopen
~~~~~~~

Whether to pre-open all index files, or open them per each query.
Optional, default is 0 (do not preopen).

This option tells ``searchd`` that it should pre-open all index files on
startup (or rotation) and keep them open while it runs. Currently, the
default mode is **not** to pre-open the files (this may change in the
future). Preopened indexes take a few (currently 2) file descriptors per
index. However, they save on per-query ``open()`` calls; and also they
are invulnerable to subtle race conditions that may happen during index
rotation under high load. On the other hand, when serving many indexes
(100s to 1000s), it still might be desired to open the on per-query
basis in order to save file descriptors.

This directive does not affect ``indexer`` in any way, it only affects
``searchd``.

Example:


.. code-block:: ini


    preopen = 1

.. _regexp_filter:

regexp_filter
~~~~~~~~~~~~~

Regular expressions (regexps) to filter the fields and queries with.
Optional, multi-value, default is an empty list of regexps.

In certain applications (like product search) there can be many
different ways to call a model, or a product, or a property, and so on.
For instance, ‘iphone 3gs’ and ‘iphone 3 gs’ (or even ‘iphone3 gs’) are
very likely to mean the same product. Or, for a more tricky example,
‘13-inch’, ‘13 inch’, '13“‘, and '13in’ in a laptop screen size
descriptions do mean the same.

Regexps provide you with a mechanism to specify a number of rules
specific to your application to handle such cases. In the first ‘iphone
3gs’ example, you could possibly get away with a wordforms files
tailored to handle a handful of iPhone models. However even in a
comparatively simple second ‘13-inch’ example there is just way too many
individual forms and you are better off specifying rules that would
normalize both ‘13-inch’ and ‘13in’ to something identical.

Regular expressions listed in ``regexp_filter`` are applied in the order
they are listed. That happens at the earliest stage possible, before any
other processing, even before tokenization. That is, regexps are applied
to the raw source fields when indexing, and to the raw search query text
when searching.

We use the  `RE2 engine <https://github.com/google/re2>`__ to implement
regexps. So when building from the source, the library must be installed
in the system and Manticore must be configured built with a ``--with-re2``
switch. Binary packages should come with RE2 builtin.

Example:


.. code-block:: ini


    # index '13-inch' as '13inch'
    regexp_filter = \**(\d+)\" => \1inch

    # index 'blue' or 'red' as 'color'
    regexp_filter = (blue|red) => color

.. _rlp_context:

rlp_context
~~~~~~~~~~~

RLP context configuration file. Mandatory if RLP is used.

Example:


.. code-block:: ini


    rlp_context = /home/myuser/RLP/rlp-context.xml

.. _rt_attr_bigint:

rt_attr_bigint
~~~~~~~~~~~~~~

BIGINT attribute declaration. Multi-value (an arbitrary number of
attributes is allowed), optional. Declares a signed 64-bit attribute.

Example:


.. code-block:: ini


    rt_attr_bigint = guid

.. _rt_attr_bool:

rt_attr_bool
~~~~~~~~~~~~

Boolean attribute declaration. Multi-value (there might be multiple
attributes declared), optional. Declares a 1-bit unsigned integer
attribute.

Example:


.. code-block:: ini


    rt_attr_bool = available

.. _rt_attr_float:

rt_attr_float
~~~~~~~~~~~~~

Floating point attribute declaration. Multi-value (an arbitrary number
of attributes is allowed), optional. Declares a single precision, 32-bit
IEEE 754 format float attribute.

Example:


.. code-block:: ini


    rt_attr_float = gpa

.. _rt_attr_json:

rt_attr_json
~~~~~~~~~~~~

JSON attribute declaration. Multi-value (ie. there may be more than one
such attribute declared), optional.

Refer to :ref:`sql_attr_json`
for more details on the JSON attributes.

Example:


.. code-block:: ini


    rt_attr_json = properties

.. _rt_attr_multi_64:

rt_attr_multi_64
~~~~~~~~~~~~~~~~

:ref:`Multi-valued attribute <mva_multi-valued_attributes>` (MVA)
declaration. Declares the BIGINT (signed 64-bit) MVA attribute.
Multi-value (ie. there may be more than one such attribute declared),
optional. Applies to RT indexes only.

Example:


.. code-block:: ini


    rt_attr_multi_64 = my_wide_tags

.. _rt_attr_multi:

rt_attr_multi
~~~~~~~~~~~~~

:ref:`Multi-valued attribute <mva_multi-valued_attributes>` (MVA)
declaration. Declares the UNSIGNED INTEGER (unsigned 32-bit) MVA
attribute. Multi-value (ie. there may be more than one such attribute
declared), optional. Applies to RT indexes only.

Example:


.. code-block:: ini


    rt_attr_multi = my_tags

.. _rt_attr_string:

rt_attr_string
~~~~~~~~~~~~~~

String attribute declaration. Multi-value (an arbitrary number of
attributes is allowed), optional.

Example:


.. code-block:: ini


    rt_attr_string = author

.. _rt_attr_timestamp:

rt_attr_timestamp
~~~~~~~~~~~~~~~~~

Timestamp attribute declaration. Multi-value (an arbitrary number of
attributes is allowed), optional.

Example:


.. code-block:: ini


    rt_attr_timestamp = date_added

.. _rt_attr_uint:

rt_attr_uint
~~~~~~~~~~~~

Unsigned integer attribute declaration. Multi-value (an arbitrary number
of attributes is allowed), optional. Declares an unsigned 32-bit
attribute.

Example:


.. code-block:: ini


    rt_attr_uint = gid

.. _rt_field:

rt_field
~~~~~~~~

Full-text field declaration. Multi-value, mandatory

Full-text fields to be indexed are declared using ``rt_field``
directive. The names must be unique. The order is preserved; and so
field values in INSERT statements without an explicit list of inserted
columns will have to be in the same order as configured.

Example:


.. code-block:: ini


    rt_field = author
    rt_field = title
    rt_field = content

.. _rt_mem_limit:

rt_mem_limit
~~~~~~~~~~~~

RAM chunk size limit. Optional, default is 128M.

RT index keeps some data in memory (so-called RAM chunk) and also
maintains a number of on-disk indexes (so-called disk chunks). This
directive lets you control the RAM chunk size. Once there's too much
data to keep in RAM, RT index will flush it to disk, activate a newly
created disk chunk, and reset the RAM chunk.

The limit is pretty strict; RT index should never allocate more memory
than it's limited to. The memory is not preallocated either, hence,
specifying 512 MB limit and only inserting 3 MB of data should result in
allocating 3 MB, not 512 MB.

Example:


.. code-block:: ini


    rt_mem_limit = 512M

.. _source:

source
~~~~~~

Adds document source to local index. Multi-value, mandatory.

Specifies document source to get documents from when the current index
is indexed. There must be at least one source. There may be multiple
sources, without any restrictions on the source types: ie. you can pull
part of the data from MySQL server, part from PostgreSQL, part from the
filesystem using xmlpipe2 wrapper.

However, there are some restrictions on the source data. First, document
IDs must be globally unique across all sources. If that condition is not
met, you might get unexpected search results. Second, source schemas
must be the same in order to be stored within the same index.

No source ID is stored automatically. Therefore, in order to be able to
tell what source the matched document came from, you will need to store
some additional information yourself. Two typical approaches include:

1. mangling document ID and encoding source ID in it:

.. code-block:: ini


       source src1
       {
           sql_query = SELECT id*10+1, ... FROM table1
           ...
       }

       source src2
       {
           sql_query = SELECT id*10+2, ... FROM table2
           ...
       }

2. storing source ID simply as an attribute:

.. code-block:: ini


       source src1
       {
           sql_query = SELECT id, 1 AS source_id FROM table1
           sql_attr_uint = source_id
           ...
       }

       source src2
       {
           sql_query = SELECT id, 2 AS source_id FROM table2
           sql_attr_uint = source_id
           ...
       }

Example:


.. code-block:: ini


    source = srcpart1
    source = srcpart2
    source = srcpart3

.. _stopwords:

stopwords
~~~~~~~~~

Stopword files list (space separated). Optional, default is empty.

Stopwords are the words that will not be indexed. Typically you'd put
most frequent words in the stopwords list because they do not add much
value to search results but consume a lot of resources to process.

You can specify several file names, separated by spaces. All the files
will be loaded. Stopwords file format is simple plain text. The encoding
must be UTF-8. File data will be tokenized with respect to
:ref:`charset_table <charset_table>`
settings, so you can use the same separators as in the indexed data.

The :ref:`stemmers <morphology>` will
normally be applied when parsing stopwords file. That might however lead
to undesired results. You can turn that off with
:ref:`stopwords_unstemmed <stopwords_unstemmed>`.

Small enough files are stored in the index header, see :ref:`embedded_limit`
for details.

While stopwords are not indexed, they still do affect the keyword
positions. For instance, assume that “the” is a stopword, that document
1 contains the line “in office”, and that document 2 contains “in the
office”. Searching for “in office” as for exact phrase will only return
the first document, as expected, even though “the” in the second one is
stopped. That behavior can be tweaked through the
:ref:`stopword_step <stopword_step>`
directive.

Stopwords files can either be created manually, or semi-automatically.
``indexer`` provides a mode that creates a frequency dictionary of the
index, sorted by the keyword frequency, see ``--buildstops`` and
``--buildfreqs`` switch in :ref:`indexer_command_reference`. Top keywords from
that dictionary can usually be used as stopwords.

Example:


.. code-block:: ini


    stopwords = /usr/local/sphinx/data/stopwords.txt
    stopwords = stopwords-ru.txt stopwords-en.txt

.. _stopword_step:

stopword_step
~~~~~~~~~~~~~

Position increment on
:ref:`stopwords <stopwords>`.
Optional, allowed values are 0 and 1, default is 1.

This directive does not affect ``searchd`` in any way, it only affects
``indexer``.

Example:


.. code-block:: ini


    stopword_step = 1

.. _stopwords_unstemmed:

stopwords_unstemmed
~~~~~~~~~~~~~~~~~~~

Whether to apply stopwords before or after stemming. Optional, default
is 0 (apply stopword filter after stemming).

By default, stopwords are stemmed themselves, and applied to tokens
*after* stemming (or any other morphology processing). In other words,
by default, a token is stopped when stem(token) == stem(stopword). That
can lead to unexpected results when a token gets (erroneously) stemmed
to a stopped root. For example, ‘Andes’ gets stemmed to ‘and’ by our
current stemmer implementation, so when ‘and’ is a stopword, ‘Andes’ is
also stopped.

stopwords_unstemmed directive fixes that issue. When it's enabled,
stopwords are applied before stemming (and therefore to the original
word forms), and the tokens are stopped when token == stopword.

Example:


.. code-block:: ini


    stopwords_unstemmed = 1

.. _type:

type
~~~~

Index type. Known values are ``plain``, ``distributed``, ``rt`` and
``template``. Optional, default is ‘plain’ (plain local index).

Manticore supports several different types of indexes. 
Plain local indexes
are stored and processed on the local machine. 
Distributed indexes
involve not only local searching but querying remote ``searchd``
instances over the network as well (see :ref:`distributed_searching`). 
Real-time indexes (or
RT indexes for short) are also stored and processed locally, but
additionally allow for on-the-fly updates of the full-text index (see
:ref:`real-time_indexes`). Note that
*attributes* can be updated on-the-fly using either plain local indexes
or RT ones. 
Template indexes are actually a pseudo-indexes because they
do not store any data. That means they do not create any files on your
hard drive. But you can use them for keywords and snippets generation,
which may be useful in some cases, and also as templates to inherit real
indexes from them.

Index type setting lets you choose the needed type. By default, plain
local index type will be assumed.

Example:


.. code-block:: ini


    type = distributed

.. _wordforms:

wordforms
~~~~~~~~~

Word forms dictionary. Optional, default is empty.

Word forms are applied after tokenizing the incoming text by
:ref:`charset_table <charset_table>`
rules. They essentially let you replace one word with another. Normally,
that would be used to bring different word forms to a single normal form
(eg. to normalize all the variants such as “walks”, “walked”, “walking”
to the normal form “walk”). It can also be used to implement stemming
exceptions, because stemming is not applied to words found in the forms
list.

Small enough files are stored in the index header, see :ref:`embedded_limit`
for details.

Dictionaries are used to normalize incoming words both during indexing
and searching. Therefore, to pick up changes in wordforms file it's
required to rotate index.

Word forms support in Manticore is designed to support big dictionaries
well. They moderately affect indexing speed: for instance, a dictionary
with 1 million entries slows down indexing about 1.5 times. Searching
speed is not affected at all. Additional RAM impact is roughly equal to
the dictionary file size, and dictionaries are shared across indexes:
ie. if the very same 50 MB wordforms file is specified for 10 different
indexes, additional ``searchd`` RAM usage will be about 50 MB.

Dictionary file should be in a simple plain text format. Each line
should contain source and destination word forms, in UTF-8 encoding,
separated by “greater” sign. Rules from the
:ref:`charset_table <charset_table>`
will be applied when the file is loaded. So basically it's as case
sensitive as your other full-text indexed data, ie. typically case
insensitive. Here's the file contents sample:

.. code-block:: ini


    walks > walk
    walked > walk
    walking > walk

There is a bundled ``spelldump`` utility that helps you create a
dictionary file in the format Manticore can read from source ``.dict`` and
``.aff`` dictionary files in ``ispell`` or ``MySpell`` format (as
bundled with OpenOffice).

You can map several source words to a single destination word. Because
the work happens on tokens, not the source text, differences in
whitespace and markup are ignored.

You can use “=>” instead of “>”. Comments (starting with “#” are also
allowed. Finally, if a line starts with a tilde (“~”) the wordform will
be applied after morphology, instead of before.

.. code-block:: ini


    core 2 duo > c2d
    e6600 > c2d
    core 2duo => c2d # Some people write '2duo' together...

You can specify multiple destination tokens:

.. code-block:: ini


    s02e02 > season 2 episode 2
    s3 e3 > season 3 episode 3

Example:


.. code-block:: ini


    wordforms = /usr/local/sphinx/data/wordforms.txt
    wordforms = /usr/local/sphinx/data/alternateforms.txt
    wordforms = /usr/local/sphinx/private/dict*.txt

You can specify several files and not only just one. Masks can be used
as a pattern, and all matching files will be processed in simple
ascending order. (If multi-byte codepages are used, and file names can
include foreign characters, the resulting order may not be exactly
alphabetic.) If a same wordform definition is found in several files,
the latter one is used, and it overrides previous definitions.
