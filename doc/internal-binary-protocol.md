# Internal binary protocol

> Last updated: Apr 9th 2019 by Alexey
> 
> This document describes Manticore's internal binary protocol.
> While made public for transparency and reference, parts of this protocol are not considered stable or supported.
> Use at your own risk. Contributions and improvements are welcome, but compatibility is not guaranteed.


Internal protocol is the one which is used by old-fashioned Sphinx clients, and it is implemented with different kinds of language APIs as php-api, python-api, c-api and so on.

From the beginning internal proto was never documented; instead developers provided ready-to-use libraries which implemented the proto and gave access to Manticore's functionality. That `API reference` mainly expressed in PHP terms is still available in documentation. However supporting ready implementations finally was considered as not so easy, so that implementing of new commands and flavours of the proto was frozen, and customers were recommended to use 'SphinxQL' proto instead, leaving internal proto solely for communication between 'master' and 'agents' hosts in distributed indexes. That is the exact text you may find in the header of any native API libraries provided by Manticore Software:

```text
// WARNING!!!
//
// As of 2015, we strongly recommend to use either SphinxQL or REST APIs
// rather than the native SphinxAPI.
//
// While both the native SphinxAPI protocol and the existing APIs will
// continue to exist, and perhaps should not even break (too much), exposing
// all the new features via multiple different native API implementations
// is too much of a support complication for us.
//
// That said, you're welcome to overtake the maintenance of any given
// official API, and remove this warning ;)
```

The purpose of this document is related to this message; it is written exactly to describe native API protocol in human language, in order to involve community people to write concrete implementations.

Times change, and for now the native API implemented in PHP does not look as fast as, say, speaking with daemon using mysql-client or PDO library using SphinxQL proto. And that is quite clear, since working with native API implies low-level working with raw bytes, chars, byte order and so on. That is really simple by nature, but is quite complex for an interpreted language like PHP. PDO client, in turn, performs all these low-level operations using internal routines, which are written in compiled languages like C or C++ and so are quite fast because of it. Even the fact that MySQL is actually a textual protocol which NEEDS to be interpreted back to native representation still does not make it slower than native API where each individual string or float is manually packed into blob using interpreted PHP code.

However, compiled implementations still show a 20-30% gain of native API over SphinxQL. That may not be so useful, since many developers now prefer lightweight interpreted languages, but among them we now also have compiled Go lang, which is also easy, but compiled and may achieve all the benefits of native API. Regarding this language, some examples will be provided using its syntax.

Apart from 'client' API, Manticore also uses special extended dialect of the proto, which comes into play in distributed indexes; that is 'agent' flavour, the proto which uses head daemon (one which has distributed indexes in its config) to speak with remote daemons (agents). That is related only to 'search' command, and that extension provides some special fields and attributes which help head daemon to perform grouping and calculate aggregate values like 'avg'. Details will be described in documentation on 'search' command below.

Internal protocol is described from lowest primitives, and then to more complex packets and entities.

## Primitives
Protocol uses bytes (octets), which are organized into numbers and other entities. Later terms `byte` and `octet` are considered the same, and mean just 8-bit integer numbers.

### Byte order
Until explicitly noted, all numbers are sent in network byte order, i.e. big-endian.
For example, 32-bit DWORD `0xDEADBEEF` will be sent as sequence of octets:

```text
0    1    2    3
0xDE 0xAD 0xBE 0xEF
```

That is done by well-known functions like `htonl` (in C/C++), or by `binary.BigEndian` package (Go).

### Integers
 * byte (BYTE) = 1 octet
 * short (WORD) = 2 octets
 * unsigned integer (DWORD) = 4 octets
 * signed integer (int) = 4 octets
 * unsigned long (uint64) = 8 octets
 * signed long (int64) = 8 octets
 
### Floats
* float = 4 octets

Internally floats are coded according to `IEEE-754` standard. To send over network, floats aliased to DWORDs, and then sent as DWORDs.
Casting may be done by union like:

```C
/// float vs dword conversion
inline DWORD sphF2DW ( float f )	{ union { float f; DWORD d; } u; u.f = f; return u.d; }
```

or, in Go lang:
```Go
d:=math.Float32bits(f)
```
  
### Arrays
All blobs with unknown size are prepended with 32-bit signed integer containing the number of objects.
For example, array of 3 bytes `{1, 2, 40}` will be sent as sequence of 7 octets:

```text
0  1  2  3  4  5  6   
00 00 00 03 01 02 28

```

where first 4 octets are occupied by integer, in big-endian order, and then follow objects (bytes) themselves.

In following description terms as `int array`, `uint64 array`, `byte array` are used to define arrays of values of mentioned type. So, when you see `uint64 array`, it assumes that following octets, first, contains 32-bits signed int with quantity of uint64 numbers following by octets representing numbers themselves, as:
```text
00 00 00 02
00 00 00 00 00 00 00 22
00 00 00 00 00 00 00 33
```
which represents `uint64 ar[2] = {0x22ULL, 0x33ULL}`

Also they may be called as `array of ints`, `array of bytes`, which is obviously same.

Also you can see clause as `array of objects`, or `objects array`, where you can guess only quantity of objects, but not their format. In such case format is described separately.

### Strings
Strings are internally expressed as arrays of octets; and so they are sent the same way as any other arrays.
By default for now all strings are represented in UTF-8 encoding. Note that length of the string in codepoints (chars) is not necessarily same as length in octets (bytes). This may be true for ASCII characters, but is not so for national texts, like Russian, Chinese, etc.
When sending over network, length of the strings is measured exactly in octets, NOT in codepoints!

Say, string 'hello' will be sent as 9 octets:

```text
0  1  2  3  4  5  6  7  8
00 00 00 05 68 65 6C 6C 6F
.  .  .  .  h  e  l  l  o

```

String 'привет' will be sent as 16 octets:

```text
0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
00 00 00 0C D0 BF D1 80 D0 B8 D0 B2 D0 B5 D1 82
.  .  .  .  п     р     и     в     е     т

```

## Communication
Clients work with Manticore daemon over socket connection, which may be either unix-socket, either internet TCP socket. In last case daemon internally works with TCPv4 addresses, however if system provides kind of mapping between ipv6 and ipv4 address, it will be also accepted.

Connection is session-based; inside session communication is dialogue-based; i.e. client send a command, receives answer, then m.b. send another, receive another answer and so on.

During a session manticore has 2 kinds of different packages. They are

 1. Handshake
 2. Command
 
### Handshake 
 Handshake is initial message which has to be sent and responded to right after establishing stream connection. Each side sends a `DWORD` with its version, and opposite side receives and checks it. Handshake is the only command where values may be sent in any byte order, i.e. as network, as little-endian, and opposite side must be aware of it and check both cases.

Couple of valid examples:

 1. Usual workflow
	```text
	server says> 00 00 00 01
	client says> 00 00 00 01
	server receives < 00 00 00 01
	client receives < 00 00 00 01	
	```

 2. Interleaved workflow
	```text
	server says> 00 00 00 01
	client receives < 00 00 00 01
	client says> 00 00 00 01
	server receives < 00 00 00 01
	```
 
 3. Heterogenic arch workflow
 	```text
	server says> 00 00 00 01
	client says> 00 00 00 01
	server receives < 00 00 00 01
	client receives < 01 00 00 00	
 	```
 (in last example server send `0x00000001`, but client received `0x01000000`, and that is also valid).
 
 Valid versions of server's and client's versions in handshake for now is only `0x00000001`.
 
 After handshake connection may be regarded as `established`. However note, that if daemon can't accept connection because of maxed out connection pool, it will send `retry` error command right after handshake, and then drop connection. So, after handshake client must expect more data from daemon, than just 4 bytes of handshake response, and react accordingly if receives `retry` command. That is critically important, since after daemon drops the connection because of 'maxed out' state, it will not receive any more data, and so, if client try to send it's command despite of possibly error, it will not luck and even may freeze for a while until streaming protocol recognizes dropped endpoint connection.
  
### Retry response
 This response is sent by daemon right after handshake in case it can't serve the connection right now. The error is not critical for usual workflow, however it is critical for current connection.
 
 Response takes about 50 octets and has the structure:
 ```text
 WORD 0x0002 // status code SEARCHD_RETRY
 WORD 0x0000 // version of the status code (0)
 DWORD nBytes // total length of following payload
 string payload_error_message // the message with reason  
 ```
 
 Typical error messages hardcoded in daemon's sources are
  1. "maxed out, dismissing client"
  2. "server maxed out, retry in a second"
  3. "failed to create worker thread"

 First and second are the same, just coming from different workers (first came from thread pool, second from threads), they have same meaning. Third one is kind of worse, since it originates not from config limitation for 'max_children', but from system thread creation, it is kind of unknown by effect (maybe retry will help, maybe not).
 
 For example, full session with maxed out message may look like:
 
 ```text
 client > 00 00 00 01 // handshake
 daemon > 00 00 00 01 // answer to handshake
 daemon > 00 02 00 00 // 'retry' status code and version
 daemon > 00 00 00 20 // length of payload (that is string length and string chars which follows)
 daemon > 00 00 00 1C // string lentgh (28 chars, big-endian)
 daemon > "maxed out, dismissing client"
 < daemon drops connection >

``` 

If retry response is not arrived and connection is not dropped, session regarded as `healthy`, and starting from that moment client and daemon may send messages to each other. Usually client sends message with necessary command, and then daemon answer with message with response (status code and body).

NOTE: handshake answer is ALWAYS sent; retry response is MAYBE sent.

### Message
All messages coming from client to daemon and from daemon to client are organized in typical structure with code, version and payload. That is:
 
```text
 WORD code      // command (from client) or status (from daemon)
 WORD version   // version (0 for error and retry statuses)
 DWORD nBytes   // size of following payload (in octets)
 ... (blob of nBytes octets follow)

```

`code` is specific to command or response. Client puts there desired command code; daemon, in turn, answers with status code.
`version` is critically important to commands which client sends to daemon. This WORD is treated as fixed-point float, as `MAJOR.MINOR`, where major part is high byte of WORD, and minor is low byte. I.e., command version 1.5 is encoded as WORD `0x0105`. Daemon has internal table with command it supports, and on receiving command message it will always look into this table and check the version of incoming command. Major version num MUST match between daemon and client. Whole version of incoming command MUST be less or equal to one daemon supports.

When answering back, response also contains `version` field. For error codes it is usually '0'. For real success codes it usually contains version of the command from daemon's internal support table. But usually this number is not necessary to interpret and it may be safely dropped.

### Optimization
Simplest session consists from handshake exchanging, then one message from client followed by the answer message from daemon, and then connection closed by both sides. So, it assumes at least 4 packages of dialogue itself (pair for handshake, pair for query/answer), and that is so despite the package exchanging during TCP session establishing. That may look ok if you have just one query, but if you have thousands it sounds not so optimal. Let's describe a little things which may help to solve it.

 1. Since usual workflow assumes exchanging with small packets, it is highly desirable to switch off TCP Naggle algorithm to avoid unnecessary delays. Consider to use `TCP_NODELAY` on your client connection.

 2. Among the others daemon supports special command '[PERSIST](#searchd_command_persist-command-payload)', which explicitly says, that you're going to send many commands next. In this case after first command query connection will NOT be dropped, but client can save opened socket and reuse it for following queries. That leads to the flow, that all network communication contains only 'query' and 'response' packets, without need to also have TCP establishing and handshake exchange each time. The cons of that approach is that daemon has permanently opened connection, and since number of available TCP ports is limited, you may not serve as many clients simultaneously, as you want. Also, number of simultaneous sessions is limited in daemon's config as 'max_children' value.
 
 3. Client may merge handshake and command message into one blob, and send it in one TCP packet, and even in initial TCP SYN packet using TCP Fast Open technique. That is also supported, and in practice gives the same effect as using persistent connections, apart the fact that handshake eats 4 bytes from that package.
 
## Protocol details 
Here are details about all commands, status codes and specific payloads.

### Status codes
Daemon has 4 different status codes:

 1. SEARCHD_OK		= 0,	///< general success, command-specific reply follows
 2. SEARCHD_ERROR	= 1,	///< general failure, error message follows
 3. SEARCHD_RETRY	= 2,	///< temporary failure, error message follows, client should retry later
 4. SEARCHD_WARNING	= 3		///< general success, warning message and command-specific reply follow

For example, if client wants to ping server using API's 'ping' command, it will looks like:

```text
client > 00 09 01 00    // command ping (09), version 1.0 (=0x100)
         00 00 00 04    // size of payload (4 octets)
         DE AD BE EF    // payload for ping command (just a DWORD which will be send back)
daemon > 00 00 01 00    // status (ok) and version (0x100, not used)
         00 00 00 04    // size of payload (4 octets)
         DE AD BE EF    // body of payload (command-specific, for ping is DWORD)
```

If, suppose, 'ping' command return a warning (that is not true for this command, but let it be for example):

```text
client > 00 09 01 00    // command ping (09), version 1.0
         00 00 00 04    // size of payload (4 octets)
         DE AD BE EF    // payload for ping command (just a DWORD which will be send back)
daemon > 00 03 01 00    // status (warning) and version (0x100, not used)
         00 00 00 0D    // size of payload (13 octets)
         00 00 00 09 'a warning' // warning string
         DE AD BE EF    // usual body of payload (command-specific, for ping is DWORD)
```

That is, first in payload follows one(1) string with warning message, then usual command-specific response follows.

Just a note - here in example `00 00 00 09 'a warning'` is the length, and then sequence of chars: 0x00, 0x00, 0x00, 0x09, 'a', ' ', 'w', 'a', 'r', 'n', 'i', 'n', 'g', each one is ASCII code of corresponding character, totally 4 (quantity) + 9 (chars) octets. That is exactly the num expressed by size of payload, i.e. 13, or `0x00 0x00 0x00 0x0D` above, expressed in big-endian DWORD. Further examples will follow same model without explicit explanations each time. 

For error message it will look like:

```text
client > 00 09 02 00    // command ping (09), version 2.0
         00 00 00 04    // size of payload (4 octets)
         DE AD BE EF    // payload for ping command (just a DWORD which will be send back)
daemon > 00 01 00 00    // status (error) and version (0, not used)
         00 00 00 3E    // size of payload (62 octets)
         00 00 00 3A `major command version mismatch (expected v.1.x, got v.2.0)` // error string
```

`Retry` error message response with possible text messages is already described above.

### Supported command codes and versions
Here is the list of all commands which Manticore supports. Some of them are purely internal and not exposed to network; some are used only in master-agent communication, and also some public ones.

All commands are organized in described [messages](#message), which implies that all necessary prefixes with command code, version, and also size of payload are always attached as necessary. So, below are described only payloads, assuming header is indeed present.

#### Public API commands
These commands are exposed and implemented by existing sphinx API packages.

```text
SEARCHD_COMMAND_SEARCH		= 0 // version 1.33 (0x121)
SEARCHD_COMMAND_EXCERPT		= 1 // version 1.4 (0x104)
SEARCHD_COMMAND_UPDATE		= 2 // version 1.3 (0x103)
SEARCHD_COMMAND_KEYWORDS	= 3 // version 1.1 (0x101)
SEARCHD_COMMAND_PERSIST		= 4 // special, not versioned
SEARCHD_COMMAND_STATUS		= 5 // version 1.1 (0x101)
SEARCHD_COMMAND_FLUSHATTRS	= 7 // version 1.0 (0x100)
```

#### Private API commands
These commands are mostly internal and used in master-agent communication (when you invoke corresponding commands to distributed indexes). However they're not 'secret', but just dismissed in support, so you're welcome to implement and share your implementation.

```text
SEARCHD_COMMAND_SPHINXQL	= 8  // version 1.0 (0x100)
SEARCHD_COMMAND_PING		= 9  // version 1.0 (0x100)
SEARCHD_COMMAND_UVAR		= 11 // version 1.0 (0x100)
SEARCHD_COMMAND_JSON		= 16 // version 1.0 (0x100)
SEARCHD_COMMAND_CALLPQ 		= 17 // version 1.0 (0x100)
SEARCHD_COMMAND_CLUSTERPQ	= 18 // version 1.0 (0x100)
```

#### Not exposed commands
Since these commands for current moment are not implemented for networking, they have no assigned version. However if they will be implemented later, they most probably will take version 1.0 (0x100).

```text
SEARCHD_COMMAND_DELETE		= 10
SEARCHD_COMMAND_INSERT		= 12
SEARCHD_COMMAND_REPLACE		= 13
SEARCHD_COMMAND_COMMIT		= 14
SEARCHD_COMMAND_SUGGEST		= 15
```

##### SEARCHD_COMMAND_SEARCH command payload for version 1.33
This is main command of the daemon; it performs the goal for which the whole software was created, namely - full-text search. And this is most complex by structure.

Structure of payload will be described from outer layer down to mostly internals.

Most generally search command consists from these parts:

```text
DWORD master_version
array of queries
```

###### master_version
First field, `master_version` selects dialect of search command. Namely manticore distinguishes at least between 'public API' clients and 'distributed index' clients.

API clients usually perform queries which return results for end-user, as explicitly defined schema, expected fields, tokens and weights and so on. JSON attributes (if any) returned to client as formatted texts. 

Remote distributed indexes, in turn, may expect additional service fields which help to group and calculate final result from resultsets arrived from different remote agents. Also JSONs (if any) returned with internal [BSON](#binary-json) form, which is not human readable, but assumes faster processing.

Legal values for `master_version` are:

 `0`. Remote client is usual public API client.
 
 All values `>0` means that remote client is remote distributed index (i.e. another instance of searchd, which has current host mentioned in `agent` section of the config). 
 
 Generic difference between 'client' and 'agent' dialects, as already mentioned, is extended schema which daemon provides to 'agent' queries, in opposite to exact schema requested by 'client'. The simplest example to illustrate ist, is a query, like:
 
 `SELECT a, b from idx order by c`
 
 Here we exactly want only columns `a` and `b`. And this is exactly what daemon will return to us in 'client' mode.
 However imagine that you're one who actually **calculates** the query, and you also see `order by c` at the end. Having strict columset of only `a` and `b` you just can't calculate `order by c`. So, in the case remote client states that it is not just a client, but _agent_, and daemon looking to this fact will provide also column `c` in order to make whole query be possible to calculate.
 
 Another example:
 
 `SELECT avg(a) from idx group by b`
 
 From client's end we expect only a column of numbers, which is result of `avg(a)` calculation. But imagine what you need on distributed indexes with, say, 2 agents. Apart the number for `avg(a)` from each of them, it also needs value of `b` in order to perform grouping by, and also number of matches in every group - in order to correctly calculate result between two already calculated averages. So, in this case daemon will provide also columns of `@groupby`, `@distrinct`, and `@count`.
 
 These are quite clear difference, and since it may be useful even for end-point clients, Manticore has simple mechanism to tell remote host that it expects response in _agent_ mode. To achieve this you need to start column's select list from double asterisk `*,*`. That is, `SELECT a FROM idx ORDER BY b` tells about client side and will return only the column `a`. However `SELECT *,*,a FROM idx ORDER BY b` will explicitly sign that we are remote distributed agent, and daemon will return columns `a` and `b` to us.
 
 This special mark `*,*` may be used _instead_ of providing non-zero `master_version` value in case you want just such simple thing as additional columns.
 
 Described below concrete values of `master_version` also influences to command message format, providing sometimes another things, than just extra-columns. Concrete numbers, until specially pointed, need to consider as edge values, or, literally, as clause 'starting from version N'.
 
 `1`. Query contains collation. That is one of `LIBC_CI` (default), `LIBC_CS`, `UTF8_GENERAL_CI` and `BINARY`. Details described below.
 
 `2` Query contains outer ORDER BY clause and m.b. also limit. That is used in calculating subselects. Daemon will sort resultset to make outer agent more happy when grouping results.
 
 `5` Filters in query have flag of equal bounds. That makes possible to pass not only 'less' or 'greater', but also 'less or equal' and 'greater or equal' filters.
 
 `6` Query contains limit for 'N best group by' clause. That clause entered in SphinxQL, but wasn't known to API proto, so it is extension.
 
 `13` Filters in query defines MVAFUNC, which may be 'ANY' (default), but also 'NONE' and 'ALL'
 
 `14` Query contains UDF plugin name and plugin's options. That make agent possible to use UDF ranker.
 
 `15` Query contains items and reference items for FACETs, which make FACET queries working in distributed case. Also filters in query passes properties as 'equal min', 'equal max' (in opposite to only one 'equal edge in v.5), and also 'open left' and 'open right' to define unlimited edges.
 
 `16` Query contains 'expand_keyword' flag.
 
 Below are descriptions of concrete fields of search queries with variants and versions.
 
###### array of queries
Search queries may be batched together. I.e. you may send not only one, but _several_ searches in one packet, and then expect the same number of answers in response. That is not only helps to avoid extra network traffic, but also assumes different internal optimizations. Most prominent is common sub-trees optimization. That is when you have one select-list which produces a num of matches, and then different set of group-by/order-by clauses, which produces different numbers for faceted search from this set of matches. Such set of queries may be obviously optimized by calculating matches only once, and then filtering/ordering them differently. And that is exactly achieved by packing multiple similar queries into one batch command. As usual, at the beginning of array it is quantity of the objects inside, in case of queries it just tells how many queries follow in the batch. That is mandadoty (but for only one query it is, obviously, 1).

###### Search query
Search query is a complex structure containing different fields. They follow one-by-one, octet-aligned, and they are:

```text
 1. DWORD qflags
 2. int offset
 3. int limit
 4. int mode
 5. int ranker
 6. [string ranker_expression]
 7. int sort
 8. string sort_by // as '@weight desc'
 9. string query_or_rawquery
10. int array of weights // m.b. empty
11. string list_of_indexes
12. int range64 // always 1
13. uint64 min_docid // as 0
14. uint64 max_docid // as DOCID_MAX
15. filters array // m.b. empty
16. int group_func
17. string group_by
18. int max_matches
19. string group_sort // as '@groupby desc`
20. int cutoff
21. int retry_count
22. int retry_delay
23. string group_distinct
24. int has_geoanchor
25. [ string geolatattr ]
26. [ string geolongattr ]
27. [ float fgeolatidute ]
28. [ float fgeolongitude ]
29. array of index weights // m.b. empty
30. DWORD query_timeout
31. array of field weights // m.b. empty
32. string comment
33. array of overrides // m.b. empty
34. string select_list
35. [ int max_predicted_msecs ]
36. string "" // deprecated, now simple a gap
37. int 0
38. int 0
39. int has_outer
40. [ master-agent extensions ]
41. string query_token_filter_lib ("")
42. string query_token_filter_name ("")
43. string query_token_filter_opts ("")
44. array of filter-tree
45. [ more master-agent extensions ]

```

(all detailed descriptions placed below. So, if something from that structure is not immediately clear, just scroll down and investigate details)

###### 1. DWORD qflags
That is bit field wich describes following query flags and properties
```text
QFLAG_REVERSE_SCAN	= 1 // 'reverse_scan', direction of full-scan
QFLAG_SORT_KBUFFER	= 2 // 'sort_method'; pq when 0, kbuffer when set
QFLAG_MAX_PREDICTED_TIME = 4 // `max_predicted_time` option. If set, value 35 (int max_predicted_msecs) in search query is present
QFLAG_SIMPLIFY		= 8 // 'boolean_simplify' flag
QFLAG_PLAIN_IDF		= 16 // idf is `plain`, otherwise `normalized`
QFLAG_GLOBAL_IDF	= 32 // 'global_idf' option. Use global idf or not
QFLAG_NORMALIZED_TF	= 64 // idf is `tfidf_normalized`, or `tfidf_unnormalized`.
QFLAG_LOCAL_DF		= 128 // 'local_df' flag
QFLAG_LOW_PRIORITY	= 256 // 'low_priority' option
QFLAG_FACET		= 512 // whether this is part of facet batch query
QFLAG_FACET_HEAD	= 1024 // if it is main facet query
QFLAG_JSON_QUERY	= 2048 // if it is JSON or API query.
```
The meaning of `reverse_scan`, `sort_method`, `max_predicted_time`, `boolean_simplify`, `idf`, `global_idf`, `local_df` and `low_priority` are described in options of `SELECT syntax` of SphinxQL reference. The only thing may be necessary to clearify is `idf`, since it is described as comma-separated list of IDF computation flags. That list is mapped into `QFLAG_PLAIN_IDF` and `QFLAG_NORMALIZED_TF` values, so that `OPTION idf='normalized,tfidf_normalized'` maps to bit `QFLAG_PLAIN_IDF` not set and `QFLAG_NORMALIZED_TF` set.

`QFLAG_MAX_PREDICTED_TIME` is not only bit in the flags, but also directly rules whether param 35 of search query is present or not. If bit is not set, param is skipped and right after param 34 (select_list) follows param 36 (emulated sub-query); otherwise param 35 present and express max predicted search time.

`QFLAG_FACET` and `QFLAG_FACET_HEAD` transfers depended queries which pefroms FACET searches (see command `FACET` in `SELECT` syntax of SphinxQL). Flag 'FACET_HEAD' marks the main query, and then one or more queries in the batch may be marked by 'FACET' flag also. Such combination 'links' head query with children and organizes one faceted query. Flags are mutually exclusive; i.e. a query might be either faced head, either faced member, either none, but not head and member simultaneously. Member queries differs from head query only by select list; the rest of the settings just copied from head to children.

`QFLAG_JSON_QUERY` indicates that query (that is ft-clause usually met in _WHERE MATCH('...')_) is json query. I.e. it is written as json with syntax described in `HTTP API reference` in section `json/search` - `Fulltext queries`. In opposite, usual query has another syntax which is described in `Searching` - `Extended query syntax` section of the docs. Since all are finally represented by plain texts, only this flag actually indicates whether the query contains search, or json syntax.
 

###### 2. int offset 
###### 3. int limit
Define pagination of resultset; `limit` shows amount of matches to return, `offset` sets offset into server-side result set.
Internally it is also implicitly invoked when hidden key 'divide_remote_ranges' is in game.

###### 4. int mode
That is matching mode, described in `Searching` - `Matching modes`. Actual numeric values are:
```text
SPH_MATCH_ALL = 0
SPH_MATCH_ANY = 1
SPH_MATCH_PHRASE = 2
SPH_MATCH_BOOLEAN = 3
SPH_MATCH_EXTENDED = 4
SPH_MATCH_FULLSCAN = 5
SPH_MATCH_EXTENDED2 = 6
```

###### 5. int ranker
That is ranking mode, described in `Searching` - `Search results ranking`. Actual numeric values are:
```text
SPH_RANK_PROXIMITY_BM25 = 0, // default
SPH_RANK_BM25 = 1
SPH_RANK_NONE = 2
SPH_RANK_WORDCOUNT = 3
SPH_RANK_PROXIMITY = 4
SPH_RANK_MATCHANY = 5
SPH_RANK_FIELDMASK = 6
SPH_RANK_SPH04 = 7
SPH_RANK_EXPR = 8
SPH_RANK_EXPORT = 9
SPH_RANK_PLUGIN = 10
```

All the rankers described in the docs, except the last two:

`SPH_RANK_EXPORT`  export ranker that emits BM25 as the weight, but computes and export all the factors. It may be useful for research purposes, eg. exporting the data for machine learning.

`SPH_RANK_PLUGIN` uses ranking by UDF plugin, it need the details as UDF plugin name and options. These fields provided only in master-agent extension v.14, so using of that kind of ranking assumes setting of >=14 `master_version` in the header, and so implies using `agent` flavour of the command.

###### 6. string ranker_expression
That is optional value, it present in query only of ranker is `SPH_RANK_EXPR`(=8) or `SPH_RANK_EXPORT`(=9). In first case the string contains ranking formula, as described in `Available build-in rankers` of `Searching` - `Search result ranking` section.

###### 7. int sort
That is sorting mode according to `Searching` - `Sording modes` of the docs. Concrete numeric values for the modes are:
```text
SPH_SORT_RELEVANCE = 0
SPH_SORT_ATTR_DESC = 1
SPH_SORT_ATTR_ASC = 2
SPH_SORT_TIME_SEGMENTS = 3
SPH_SORT_EXTENDED = 4
SPH_SORT_EXPR = 5
```

###### 8. string sort_by
That is sorting clause. It may be empty, but it is mandatory (can't be skipped). It provides param for all sorting modes, except the `SPH_SORT_RELEVANCE`. By default it may be set as `@weight desc`. 

###### 9. string query_or_rawquery
That string mainly intended to be used for logging purposes, since the query itself at this moment usually already parsed and it is impossible to restore it back. Query may be raw (which just taken 'as is' from original statement), or 'cooked' (possibly transformed during legacy matching modes fixup). General rule is that for JSON query types (ones with set `QFLAG_JSON_QUERY` flag) we provide 'cooked' query; for all the rest - raw (if non-empty), or 'cooked' (if raw is empty). In case of `agent` flavour query may be used to determine list of necessary fields which have to be returned together with explicitly asked columns.

###### 10. array of weights
That is optional array of integers which represent field weights. First is (as usual) array size, then integers of declared amount, each of them defines user specified per-field weight. If array is skipped (i.e., if first injected value is sequence of bytes `00 00 00 00`, indicates zero-length array), default weight considered as 1 for all fields.

###### 11. string list_of_indexes
That is list of lindexes to query. It contains string with comma-separated names of the indexes. It may contain one or more indexes which assumed to exist on the daemon. Order of indexes are important, at least in Manticore v.2, since it assumes using of 'main+delta' searching schema with kill-lists, and so, earlier in list indexes will be targets of kill-lists for laters. Also in this field may be used wildchars, as "*" which means, 'all indexes'. Index names consist from latin letters (a-z), numbers (0-9) and underscore(_). Everything else considered as separator. And also index names should not start with underscore.

###### 12. int range64
###### 13. uint64 min_docid
###### 14. uint64 max_docid
###### 15. filters array
That are filters. Originally in ancient times sphinx supported only filtering by DocID, and it was 32-bits integer. A bit later it also introduced 64-bits DocIDs, and introduced special flag 'id64' to distinguish between them. Protocol was build evolution way, and it first includes filters by docID, and later another type of filters were also added. So, let's consider all these values together, having in mind their historical meaning.

`range64` considered as boolean. Zero means that `min_docid` and `max_docid` are DWORDs (i.e., by 4 octets each, representing unsigned int values). Non-zero means that they're uint64, (i.e. by 8 octets each). Historically it was difference between 'id64' and non-'id64' builds exactly in size of document-ids. For now manticore totally switched to 'id64' flavour, so range64 is ALWAYS =1.

`min_docid` and `max_docid` - define global values, originally used in SetIDRange API function. For now you may set those values as '0' for `min_docid` and 'DOCID_MAX' for `max_docid`. That will say, that filtering by DocID is not at all necessary.

For now those values are just historical artefact; you may use them to define range filtering by DocumentID, however you may do the very same way by providing the filter in array of filters below. When speaking to remote agents, manticore itself now uses hardcoded values of 1 for `range64`, 0 for `min_docid` and 0xFFFFFFFFFFFFFFFF for `max_docid`. That combination simple means 'there is no filter here'.

Non-default values of `min_docid` and `max_docid` however still interpreted, and they will produce ranged filter by `@id` which will be appended to array of filters, defined below.

Consider to always inject default values here and use explicitly defined range filter, to keep all filtering stuff similar.

`array of filters` - define filters. First (as in any array) is integer expressing number of filters; it may be zero (no filters). Then placed filters themselves (if any), one-by-one.

Each filter has following structure:

```text
string attribute_name
DWORD filter_type
<filter-specific data>
DWORD exclude_flag
[master-specific data]

```

`attribute_name` is, obviously, name of attr by which filtering to be done.

`filter_type` is one of these values:

```text
SPH_FILTER_VALUES		= 0,	///< filter by integer values set
SPH_FILTER_RANGE		= 1,	///< filter by integer range
SPH_FILTER_FLOATRANGE	= 2,	///< filter by float range
SPH_FILTER_STRING		= 3,	///< filter by string value
SPH_FILTER_NULL			= 4,	///< filter by NULL
SPH_FILTER_USERVAR		= 5,	///< filter by @uservar
SPH_FILTER_STRING_LIST	= 6,	///< filter by string list
SPH_FILTER_EXPRESSION	= 7		///< filter by expression
```

According to each value `<filter-specific data>` is different and described below.

`SPH_FILTER_VALUES` provides filtering by integer values. Filter-specific data is the values, passed as `array of uint64`. Maximum size of that array is limited by manticore's configuration `max_filter_values`; by default it is 4096.

`SPH_FILTER_RANGE` filters by integer range. Filter-specific data contains pair of uint64 numbers:
```text
uint64 MinValue
uint64 MaxValue
```
Numbers represent left and right edges of the range.

`SPH_FILTER_FLOATRANGE` filters by float values. Filter-specific data contains pair of floats:
```text
float fMinValue
float fMaxValue
```
Floats represent left and right edges of the range.

`SPH_FILTER_STRING` filter by string value. Filter-specific data is one string.

`SPH_FILTER_NULL` filters by null/not null. Filter-specific data is one octet, treated as boolean flag (0=false,!0=true). True flag filters by 'is null', false by 'is not null'.

NOTE! That especial filter collides with `exclude_flag` described below. I.e. if you want to filter by 'is not NULL', you may either use IsNull(false) filter, or !IsNull(true). Both will work, however note that using `exclude_flag` for inversion adds special 'inverter' filter into the chain, so daemon will calculate two filters as Inv(IsNull(true)). Using param, in turn, just calculates IsNull(false) which has one less step and so, a bit faster.

`SPH_FILTER_USERVAR` filters by value of user-defined variable. Filter-specific data is one string, representing name of the variable.

`SPH_STRING_LIST` filters by set of strings. Filter-specific data is `array of strings`, representing the strings.

`SPH_FILTER_EXPRESSION` filters by expression which is already passed in `attribute_name`. So, there is no filter-specific data for this filter.

`exclude_flag` represents boolean value, 0 - no inversion, !0 - invert result. That flag causes 'inverter' filter to be created and added before the whole filter. It may be used always where necessary, however read the note to `SPH_FILTER_NULL` type if you want to use it with this type of filter.

For specific `master_version` filter also contains following fields
```text
DWORD HasEqualEdges // for v>=5 to v<15
DWORD HadEqualMin // v >=15
DWORD HadEqualMax // v >=15
DWORD OpenLeft // v >=15
DWORD OpenRight // v >=15
DWORD eMvaFunc // v >=13
```

`HasEqualEdges` (boolean) forces ranged filters to include bounds, so 'less' became 'less or equal', 'greater' became 'greater or equal'. Note that from v.15 this field is removed, and `HasEqualMin` and `HasEqualMax` are used **INSTEAD** of this value!

`HasEqualMin` and `HasEqualMax` are the same, but rules separately with left and right bounds and assume `master_version` at least 15. Starting from this version **ONLY** these values exist, and `HasEqualEdges` is **ABSENT**!

`OpenLeft` and `OpenRight` provides filters without left or right bound. In this case 'min' or 'max' filter's value just ignored.

`eMvaFunc` set type of MVA or string-list folding function, and that may be one of:
```text
SPH_MVAFUNC_NONE = 0,
SPH_MVAFUNC_ANY = 1
SPH_MVAFUNC_ALL = 2
```
Meaning of 'NONE' usually same as 'ANY', but it also fires a warning to use explicit ANY()/ALL() function.

After all explicitly defined filters, daemon will also add `@id` SPH_FILTER_RANGE filter in case when `min_docid` (param 13) is not 0, or `max_docid` (param 14) is not `0xFFFFFFFFFFFFFFFF` (aka `DOCID_MAX`).

###### 16. int group_func
This is grouping function. Possible values are described in `Searching` - `Grouping (clustering) search results` section of the docs. Concrete numeric values are:
```text
SPH_GROUPBY_DAY = 0
SPH_GROUPBY_WEEK = 1
SPH_GROUPBY_MONTH = 2
SPH_GROUPBY_YEAR = 3
SPH_GROUPBY_ATTR = 4
// SPH_GROUPBY_ATTRPAIR = 5
SPH_GROUPBY_MULTIPLE = 6
```

`SPH_GROUPBY_ATTRPAIR` is invalid choice and it will fire error if you use it.

`SPH_GROUPBY_MULTIPLE` means grouping on multiple attribute values.

###### 17. string group_by
This is name of the column to be used with grouping function from previous param. In case of grouping by attr (value 4) it is name of the attribute. In case of grouping by 'multiple' (value 6) it is several names sepatated by comma.

###### 18. int max_matches
That is maximal quantity of matches to retrieve. It shows, how much matches daemon will keep in RAM while searching.
You can't retrieve more than tnis number in client application.

###### 19. string group_sort
That is how to sort groups in result set.
Usually it is set to '@groupby desc` string.

###### 20. int cutoff
Cutoff to stop searching. It tells daemon to forcibly stop search query once this quantity of matches had been found and processed. It is intended for advanced pefrormance control.

###### 21. int retry_count
That param defines how much tries is acceptable. That is not useful for local indexes, however if daemon you connect to serves distributed indexes, it may help to repair if one of remotes failed.

Apart concrete 0 or positive numbers it also may be set to '-1', which means 'use default value from config'.

###### 22. int retry_delay
That represents delay, in milliseconds, between retries. It works only with remote agents and not used for local searches.
Also, apart concrete numbers this param may be set to '-1', which means 'use default value from config'.

###### 23. string group_distinct
Set attribute name for per-group distinct values count calculation. Available only for grouping queries.
For query like:
```sql
select ... COUNT(DISTINCT foo), count(*) as @count from idx group by fee order by @count desc
```

'foo' is value wich came to `group_distrinct` param, and 'fee' is one for `group_by`. Also `group_sort` from that clause will take '@count desc' value.

###### 24. int has_geoanchor
That is boolean, representing whether next 4 values (`geloatattr`, `gelongattr`, `fgeolatitude`, `geolongitude`) present or not in the query. If it is 0, then values are skipped, and immediately after this param follows params starting from 29.

###### 25. string geolatattr 
###### 26. string geolongattr
These two strings (optional, depends from `has_geoanchor` value) set names of attributes which contains latitude and longitude values.

###### 27. float fgeolatidute 
###### 28. float fgeolongitude
These two floats (optional, depends from `has_geoanchor` value) set coordinates of anchor point for geodistance search. These values must be expressed in radians.

###### 29. array of index weights
That is array of objects of:
```text
string index_name
int index_weight
```

Array may be empty, in this case all weights assumed as '1'. Otherwise for each named index assigned integer with weight.
This value originated from API call SetIndexWeights, and so you may refer this function for details.

###### 30. DWORD query_timeout
That is max query time, in milliseconds. Local search query will be stopped once that much time elapsed. 

###### 31. array of field weights
That is array of objects of:
```text
string field_name
int field_weight
```

Binds per-field weights by name. Match ranker can be affected by per-field weights, so you may manage ranking by specifying different non-default weights to different full-text fields.

Array may be empty.

###### 32. string comment
That is just comment which will be written together with query into the log.

###### 33. array of overrides
Meaning of overrides described in function 'SetOverride()' of API.
Internally overrides represented as array of objects of:
```text
string attribute_name
DWORD attribute_type
array of overrided_attributes
```

`attribute_name` is obvious;
`attribute_type` is internal type, there are many of them, but here especially interested values as
```text
SPH_ATTR_FLOAT = 5
SPH_ATTR_BIGINT = 6
```

`overrided_attribute`, in turn, is object, depended from `attribute_type` and that is
```text
uint64 docid
DWORD/uint64/float overriding_value
```
Here `docid` is also obvious, and `overriding_value` is a number, with type:
```text
float - for attribute_type==SPH_ATTR_FLOAT
uint64 - for attribute_type==SPH_ATTR_BIGINT
DWORD - for the rest.
```
Array of overrides may be empty. Moreover, since feature is declared as 'deprecated', prefer to always keep it empty.

###### 34. string select_list
That is select list, i.e. list of columns/expressions you want to retrieve by the query. Consider to read `API reference` - `General query settings` - `SetSelect` description for syntax details.

###### 35. int max_predicted_msecs
That is optional parameter, it present in query ONLY if `QFLAG_MAX_PREDICTED_TIME` bit is set in `qflags` param at the beginning of the query.

The meaning is described in `Configuration reference` - `searchd program configuration options` - `predicted_time_costs` section.

###### 36. string outer_orderby
###### 37. int outer_offset
###### 38. int outer_limit
###### 39. int has_outer
These values defines params for outer query, or subselect. Read `SphinxQL reference` - `SELECT syntax` - `Subselects` section for details about them. Namely, `outer_orderby` represents sorting clause, `outer_offset` and `outer_limit` represents LIMIT values, and `has_outer` is the boolean representing whether the query has subselect at all. Yes, it looks not optimal and it would be better to have `has_outer` flag before, and make following values optional depended from it, but it already made as described.

###### 40 master-agent extensions
This section represent different values depended from `master_version`. They are totally absent if you make 'client' connection, but have different blobs if you connection with `agent` flavour, depended from the version of extension. Values are inclusive, so if you have master-version=10, it will include all the fields for versions 1, 2, 6, excluding fields for v.14 and above.

`master_version>=1` adds the field:
```text
DWORD eCollation
``` 

eCollation may be one of these values:
```text
SPH_COLLATION_LIBC_CI = 0
SPH_COLLATION_LIBC_CS = 1
SPH_COLLATION_UTF8_GENERAL_CI = 2
SPH_COLLATION_BINARY = 3
```

Default value is `SPH_COLLATION_LIBC_CI`, i.e. 0.

`master_version>=2` adds the fields
```text
string outer_orderby
[ int outer_limit ]
```

`outer_orderby` is described above as param 36. It is a bit messing, since originally outer subselect was intended only for internal master-agent communication, so it was added as extension 2. Later it was considered to make it available for all API clients, so params were exposed as params 36-38 to everybody also. If both param 36 and this extended value presents, value of extension will be used.

`outer_limit` is dupe of param 38, however it present in extension ONLY if param 39 `has_outer` is true, otherwise this extension is skipped.

`master_version>=6` adds the field
```text
int groupbylimit
```
That represents value of `N` for N-best-groupby, i.e. when clauses like 'GROUP 10 by foo' used - here groupbylimit=10.

`master_version>=14` adds the fields
```text
string udf_ranker
string udf_ranker_opts
```

These params provides necessary values when `ranker=SPH_RANK_PLUGIN` is used. So, for now using this ranker assumes daemon-agent connection, and it may not be used from simple `client` flavour connection.

###### 41. string query_token_filter_lib
###### 42. string query_token_filter_name
###### 43. string query_token_filter_opts
These params provides custom token filter plugin for query. They represents name of the library, name of the filter and options.
Query-time token filter created on each search for every involved index and let you implement custom tokenizer that produces tokens from text by your own rules. These values originate from `OPTION token_filter=...` of `SELECT` clause.

###### 44. array of filter-tree items
Filter tree is new generation of filters which allows to use complex expressions. Tree is build over existing array of filters (see param 15) and refers to them by index. Each filter-tree item consists of:
```text
int iLeft
int iRight
int iFilterItem
int bOr
``` 

`iLeft` and `iRight` represents tree connection to another leaves; `iFilterItem` refers to the filter clause described in array of filters (param 15); boolean `bOr` flag shows that node represents logical opeator 'OR'.

Array may be empty.

###### 45. more master-agent extensions
`master_version>=15` adds these fields:
```text
array of query items
array of ref query items
```

These are used mostly by facets. Query items came from parsed select-list and it is list of columns from this query.
Ref query items is original select-list prior replacing by facets. These two arrays helps to perform multi-query optimization even with facets, since they hold 'common' and 'specific' schemas separate, and so, not confusing strict criteria for rejecting common optimization.

Both 'query item' and 'ref query item' has same structure as:
```text
	string alias
	string expression
	DWORD eAggrFunc
```

`alias` and `expression` may be illustrated by chunk `SELECT expression AS alias ... `.
eAggrFunc is for aggregate columns, and it may be one of:
```text
SPH_AGGR_NONE = 0
SPH_AGGR_AVG = 1
SPH_AGGR_MIN = 2
SPH_AGGR_MAX = 3
SPH_AGGR_SUM = 4
SPH_AGGR_CAT = 5
```

They are represent, that the column is may be aggregate function instead of exact value. `SPH_AGGR_NONE` means plain value (no aggretation).
`SPH_AGGR_AVG`, `SPH_AGGR_MIN` `SPH_AGGR_MAX` `SPH_AGGR_SUM` corresponds to 'AVG()', 'MIN()', 'MAX()' and 'SUM()' aggregates, and `SPH_AGGR_CAT` corresponds to 'GROUP_CONCAT()' function.

`master_version>=16` add this field:
```text
DWORD eExpandKeywords
```

This value represents how keyword expansion has to be done during the query. 
Possible values are:
```text
QUERY_OPT_DEFAULT = 0
QUERY_OPT_DISABLED = 1
QUERY_OPT_ENABLED = 2
QUERY_OPT_MORPH_NONE = 3
```

Values of 'disabled' and 'enabled' are set when option 'expand_keywords' of the query is set to 0 or 1.
Value 'morth_none' is in game when option 'morphology' is set to 'none'.
Default value means, that actual behaviour will be ruled by index-side setting from config.

This value is the very last of `SEARCHD_COMMAND_SEARCH` command payload.

##### SEARCHD_COMMAND_SEARCH response payload version 1.33
Answering to `SEARCHD_COMMAND_SEARCH` daemon sends [message](#message) with appropriate [status code](#status-codes) and following payload.

The payload contains one or several responses, according to number of queries in bach which were send in the command. Responses follows one after another, without prepending number of them, assuming that the number is the same as number of queries, and so, is already known to remote side.

Each response has following structure:

```text
DWORD result_status
```

`result_status` is one of:
```text
SEARCHD_OK = 0
SEARCHD_ERROR = 1
SEARCHD_WARNING = 3
```

Values looks similar as one `status code` in global message header, however they _ARE_ different and in this context related only to search query. This status code send as DWORD, i.e. as 4 octets, in opposite to global message's status code which occupies WORD (i.e. 2 octets). Be aware for this difference! 

in case of `SEARCHD_ERROR` code, the rest of payload is:
```text
string error_msg
```
and that's all. After string `error_msg` blob for current response is finished.

in case of `SEARCHD_WARNING` first part of response is:
```text
string warning_msg
```

immediately after `warning_msg` follows response itself, as in case of `SEARCHD_OK`.

In case of `SEARCHD_OK` no extra fields present, and then follows response to the command, which contains following parts:
 1. Schema
 2. Matches
 3. Global properties
 4. Stats (io/cpu/predicted)
 5. Agent-specific properties
 6. Word stats.
 
###### 1. Schema
Schema contains following parts:
```text
array of strings // fields
array of attributes
``` 

First array is just names of ft-fields in schema.
Second contains structures like:
```text
string attr_name
DWORD attr_type
```

`attr_name` is also just a name of a column.
`attr_type` is value representing type of a column. Possible values are:
```text
SPH_ATTR_INTEGER = 1			///< unsigned 32-bit integer
SPH_ATTR_TIMESTAMP = 2			///< this attr is a timestamp
SPH_ATTR_BOOL = 4			///< this attr is a boolean bit field
SPH_ATTR_FLOAT = 5			///< floating point number (IEEE 32-bit)
SPH_ATTR_BIGINT = 6			///< signed 64-bit integer
SPH_ATTR_STRING = 7			///< string (binary; in-memory)
SPH_ATTR_POLY2D = 9			///< vector of floats, 2D polygon (see POLY2D)
SPH_ATTR_TOKENCOUNT	= 11			///< field token count, 32-bit integer
SPH_ATTR_JSON = 12			///< JSON subset; converted, packed, and stored as string
SPH_ATTR_UINT32SET = 0x40000001UL	///< MVA, set of unsigned 32-bit integers
SPH_ATTR_INT64SET = 0x40000002UL	///< MVA, set of signed 64-bit integers
SPH_ATTR_MAPARG = 1000
SPH_ATTR_FACTORS = 1001			///< packed search factors (binary, in-memory, pooled)
SPH_ATTR_JSON_FIELD	= 1002			///< points to particular field in JSON column subset
SPH_ATTR_FACTORS_JSON = 1003	
```

Note that for `master_version<4` all columns of `SPH_ATTR_JSON_FIELD` will be rendered to strings and returned as `SPH_ATTR_STRING`.
For `master_version<3` also `SPH_ATTR_JSON` will be rendered and returned as `SPH_ATTR_STRING`. Otherwise these values will contains [BSON](#binary-json) blobs, that is 'binary-json'. This format is also described in this document.

So, you'll never see 'SPH_ATTR_JSON' or 'SPH_ATTR_JSON_FIELD' in responses received in `client` mode.

###### 2. Matches
Matches follow in this format:
```text
int num_of_matches
int use_64bits // ==1
matches of quantity `num_of_matches`
```

That looks like array, with the exception that between num of elements and elements themselves stands `use_64bits` param, that is integer and always 1 (means, we use 64-bits document ids).

Each match has this format:
```text
uint64 document_id
int weight
attributes
```

`attributes` is blob with several attributes; their quantity is NOT send with every match as for usual arrays, but instead taken from already received and parsed schema.

Each attribute has own format according to its type defined in schema.

`SPH_ATTR_UINT32SET` is sent as `array of DWORDs`

`SPH_ATTR_UINT64SET` is sent as `array of uint64s`

`SPH_ATTR_FLOAT` is sent as single `float`

`SPH_ATTR_BIGINT` is sent as single `uint64`

`SPH_ATTR_JSON` is sent as `array of octets` representing value in [BSON](#binary-json) format.

`SPH_ATTR_STRING` is sent as `string`, may be with json mark at the end; see the note below.

`SPH_ATTR_FACTORS`, `SPH_ATTR_FACTORS_JSON` are sent as `array of octets` representing value as [packed factor's](#packet_factors) blob, which is described in separate section.

`SPH_ATTR_JSON_FIELD` is sent as:
```text
byte Bson_type
array of octets // BSON of value
```
The rest of the attributes may be represented and send as single `DWORD` value.

NOTE: for `SPH_ATTR_STRING` daemon gives textual strings (not null-terminated). However sometimes it is literally json, and may be processed accordingly. Say, if you expose the string somewhere as value of json attribute, you may either escape it according to the rules of json escaping, or may put it directly, if it is json. Such dual string may be returned, for example, if you display the list of percolate index stored queries, and they may be either ql query, stored from sphinxql 'insert' routine, either json query, stored from REST API. To mark such values, instead of the `string`, daemon actually sends `array of bytes`, where last 2 bytes is sequence of `0x00 0x00` for true json, or `0x00 0x01` for true string. So, processing received `string` you must first check pre-last byte, and if it is zero, consider the actual string's length is less to 2 bytes then number of bytes came from the daemon. Then check the last byte, and if it is 0, this is json; if it is 1, it is simple plain text string. If pre-last byte is non-zero, consider whole attribute as plain text string.

###### 3. Global properties
They are:
```text
int total // total num of matches tranceived
int total_found // total found num of matches
int query_time // query time in milliseconds
```

###### 4. io/cpu stats
This section is sent only in `agent` mode (when query was issued with non-zero `master_version` value) and skipped in `client` dialect.
It contains following data:
```text
byte uStatMask
[iostats] // present if uStatMask & 1 is non-zero
[uint64 iCpuTime] // present if uStatMask & 2 is non-zero
[uint64 predicted_time] // present if uStatMask & 4 is non-zero
```

`uStatMask` indicates which exactly sections follows. Zero values mean 'nothing', and in such case next section of response follows.
Otherwise lest significant bit indicates presense of `iostats`, second points to `cpustats`, and third displays `predicted_time`.

`iostats`, if `uStatMast&1` is non-zero, represented as:
```text
uint64 iReadTime // in microseconds
DWORD iReadOps
uint64 iReadBytes
uint64 iWriteTime // in microseconds
DWORD iWriteOps
uint64 iWRiteBytes
``` 

Meaning of the fields obvious from their names; only things need to be noticed that `iReadTime` and `iWriteTime` fields are expressed in microseconds.

`iCpuTime` present if `uStatMast&2` is non-zero, it is expressed in microseconds.

`predicted_time` present if `uStatMast&4` is non-zero, it is expressed in microseconds.

###### 5. Agent-specific properties
This section is sent only in `agent` mode for specific versions, and skipped in `client` dialect.

For `master_version>=7` these properties are sent:
```text
DWORD iFetchedDocs
DWORD iFetchedHits
```

They show raw number of docs and hits processed during matching and may be helpful in regarding index size.

For `master_version>=8` this additional property is sent:
```text
DWORD iSkips
```

That is number of `Skip()` internal calls during matching.

###### 6. Word stats
That is final part of `SEARCHD_COMMAND_SEARCH` result payload, and it contains:
```text
array of wordstats
```

For clients `wordstat` is:
```text
string word
DWORD docs
DWORD hits
```

For agents, indicated by non-zero master_version, `wordstat` contains also one extra byte:
```text
string word // as for client
DWORD docs  // as for client
DWORD hits  // as for client
byte 0 // empty statistic for expanded terms
```
  
If original command contains >1 queries in the batch, after whole response will be placed next responses. After number of expected responses is exceeded, payload is done.
 
##### SEARCHD_COMMAND_EXCERPT command payload version 1.4
This is the second big command of the daemon; it performs snippets generation, and it is also quite complex.

Structure of payload will be described from outer layer down to mostly internals.

Exceprts (or, 'snippets') payload contains following fields:
```text
int field_mode // not used, skipped
int iFlags
string sIndex
string sWords
string sBeforeMatch
string sAfterMatch
string sChunkSeparator
int iLimit
int iAround
int iLimitPassages
int iLimitWords
int iPassageId
string sStripMode
string sPassageSPZ
array of strings // queries
```

First param `field_mode` is deprecated and ignored, however you have to send an integer there.

`iFlags` is a bit mask. Individual bits have following values:
```text
EXCERPT_FLAG_REMOVESPACES		= 1 // 1-st bit
EXCERPT_FLAG_EXACTPHRASE		= 2 // 2-nd bit
EXCERPT_FLAG_SINGLEPASSAGE		= 4 // 2-rd bit
EXCERPT_FLAG_USEBOUNDARIES		= 8 // 4-th bit
EXCERPT_FLAG_WEIGHTORDER		= 16 // 5-th bit
EXCERPT_FLAG_QUERY				= 32 // 6-th bit
EXCERPT_FLAG_FORCE_ALL_WORDS	= 64 // 7-th bit
EXCERPT_FLAG_LOAD_FILES			= 128 // 8-th bit
EXCERPT_FLAG_ALLOW_EMPTY		= 256 // 9-th bit
EXCERPT_FLAG_EMIT_ZONES			= 512 // 10-th bit
EXCERPT_FLAG_FILES_SCATTERED	= 1024 // 11-th bit
EXCERPT_FLAG_FORCEPASSAGES		= 2048 // 12-th bit
```

`sStripMode` string may has one of 'none', 'index', 'strip' and 'retain' values.

`sPassageSPZ` string may be one of 'sentence', 'paragraph' and 'zone', and also may be empty string.

All these values are well described in `API reference` - `Additinal functionality` - `BuildExcerpts` section, use it for explanations of meanings.

`queries` are either texts for snippets generation, either path to files with the texts; concrete meaning is ruled by LOAD_FILES flag in `iFlags` bitmask.

##### SEARCHD_COMMAND_EXCERPT response payload version 1.4
Response of snippets command is quite simple and consists of several `strings`. Number of values corresponds to number of queries passed to the command.

Each string represents snippet for one query.

##### SEARCHD_COMMAND_UPDATE command payload version 1.3
This command updates given attribute values in given documents. Updates are possible for numbers (integers, floats) and MVA attributes. Updating strings and jsons is not supported, at least in current version of protocol. However you can update numbers, saved in json attributes, using locator as name of attribute, as `json.attr[4].intvalue`

Update command payload contains following parts:
```text
string indexes
DWORD num_of_attrs
DWORD flags
attr_descriptors[num_of_attrs] // schema
array of update // new values to assign
```

`indexes` contains names of indexes to update. It may be one or several indexes, including distributed. In latest case updates will be applied to all local and remote agents (with very same API command being sent by head master to daemons). Rules for naming and enumerating indexes here are the same s for `SEARCHD_COMMAND_SEARCH` command, except that for updates you can't use wildchar.

`num_of_attrs` contains, namely, number of attributes you want to update. And it is also quantity of `attr_descriptors` described below.

`flags` is a bit field. For now only one bit (LSB) used, it indicates whether to ignore non-existent attributes, or warn about them. If may be useful when you simultaneously update list of indexes using 'fat' package containing new values for all possible attributes, but some of the indexes just don't contain them in their schema.

`attr_descriptors` is actually array, but it's size is not passed with every blob, but determined by param `num_of_attrs` instead.
Each `attr_descriptor` in the array contains:

```text
string attr_name
DWORD mva_flag
```

where `attr_name` is, namely, name of attribute to update, and `mva_flag` regarded as boolean (0=false, !0=true) and determines that this attribute is actually MVA attribute.

Next follows `array of update`. That is usual array, prepended by it's size.
Each `update` record in array contains:

```text
uint64 document_id
update_blobs[num_of_attrs]
```

`document_id` is obvious.

`update_blobs` is actually array of `update_blob`, but it's size is not passed with every blob, but determined by param `num_of_attrs` instead. Structure of `update_blob` depends from whether corresponding attribute has set `mva_flag` or not.

For attributes with `mva_flag` is true (i.e. !=0), `update_blob` is `array of DWORD`.

For attributes with `mva_flag` is false (i.e. ==0), `update_blob` is single `DWORD` value.

NOTE, that `SEARCHD_COMMAND_UPDATE` can update attributes only by specifying direct document-ids; updates by condition as 'where docid<1000' is NOT possible by this command on daemon side. Also note, that update payload can hold only 32-bit values. For example, when updating 64-bits MVAs in this case only low DWORD will be written, and high DWORD will be explicitly set to 0.

If you want to perform more complex updates, consider to do it with private command [`SEARCHD_COMMAND_SPHINXQL`](#searchd_command_sphinxql-command-payload-version-10), wrapping necessary clause into that command.

##### SEARCHD_COMMAND_UPDATE response payload version 1.3
Response of update command is single `int` containing number of updated documents.

##### SEARCHD_COMMAND_KEYWORDS command payload version 1.1
Keywords are used to extract tokens from query using tokenizer settings for given index.

Command's payload contains following parts:
```text
string query
string index
int bNeedStats
int bFoldLemmas
int bFoldBlended
int bFoldWildcards
int ExpansionLimit
```

`query` is query to tokenize

`index` is the index from which tokenizer settins will be used

`bNeedStats` is boolean (0=false, !0=true), means that for each keyword will be also returned pair of ints - number of docs and hists.

`bFoldLemmas` will return only one normalized token from lemmatizer, even if it provides many lemmas (which is usual for non-trivial forms). For example, for token like 'синей' with russian AOT morphology with folding only 'синий' will be returned; however without folding it will be lemmas 'синий', 'синеть', and 'синь'.

`bFoldBlended` will return normalized tokens blended. I.e. for token like 're@bus' with blend char '@' normalized with folded blended will be only 're@bus', but without it will be 're@bus', 're', and 'bus'.

`bFoldWildcards` will return normalized token with wildcards. Without this flag wildcards will be unfold and return m.b. many tokens. For example, 'qu*' will produce 'qu*' for folded wildcards, but bunch of 'quorum', 'quorumm', 'quorumm', etc. for unfolded.

`ExpansionLimit` will work together with wildcards and helps to limit expansion when too short wildcard token otherwise would return you millions of expansions. 

##### SEARCHD_COMMAND_KEYWORDS response payload version 1.1
Response of keywords command is `array of keyword`

Each `keyword` contains:
```text
string tokenized
string normalized
int querypos
[int Docs] // optional, sent only if `bNeedStats` was set
[int Hits] // optional, sent only if `bNeedStats` was set
```

`tokenized` is just a token cutted from your query.

`normalized` is result of normalization, ruled by provided flags

`querypos` is position of original token in the query. For folded blends sequence of positions might contain gaps on the place of blends, as 're@bus fe@bus' will return qpos 1 for 're@bus' and qpos 3 for 'fe@bus'.

Optional `Docs` and `Hits` contains number of documents and positions, accordingly.

##### SEARCHD_COMMAND_PERSIST command payload
Persist is a special command which tells daemon to NOT close the connection after next commands, but keep it alive and expect new commands on the same connection. This command has to be send right after handshake in order to set 'persistend mode' for whole following session. Persistent command has no version, so you have to pass any `WORD` (say, 0) as it's value.

NOTE that persistent request is not necessary will be executed by daemon. If limit for 'max_children' is set in daemons' config, and if current connection would hit the limit, connection will not became persistent and will be closed right after the command. In this case, if non-persistent mode of work is ok for you, you have to connect again and perform your query.

Also, persistent connection may be closed by timeout. In this case you, again, have to establish new connection before continuing your work.

So, when working in persistent mode you have to always check whether connection still alive before using it, and re-establish it, if not.

Persist payload contains single `int` which interpreted as boolean (0=false, !0=true) and means either to enter (true) persistent mode, either to exit (false) it.

##### SEARCHD_COMMAND_PERSIST response payload
Persist command is the only one which has no payload, and also it doesn't send any response message at all. So, you have not to wait _any_ response to this command at all.

##### SEARCHD_COMMAND_STATUS command payload version 1.1
Status returns you status of the daemon. As payload you have to send single `DWORD global_status` which will be interpreted as boolean (0=false, !0=true), which indicates whether you want to get global daemon status (true), or meta of last query (false). 

Apart direct call by API, this command is also invoked when you invoke console 'searchd' with '--status' param. In that case daemon will parse the config, then locate the instance of running daemon from it (by reading `pid_file` param), and send this very command to the found instance with `global_status` set to 1 (true).

##### SEARCHD_COMMAND_STATUS response payload version 1.1
Response status command contains table of strings in following structure:
```text
int num_of_rows
int num_of_columns // =2
string[num_of_rows*num_of_columns] values
```

That is, two integers containing dimensions of the table, and than many strings `values` placed one-by-one in quantity of table's capacity (that is number of rows * number of columns). Every odd string contains name of a variable; every even contains the value.

`num_of_rows` depends from the command, may be different. Consider to receive and process about 30-100 rows.

`num_of_columns` is always 2. I.e. you'll receive key-value pairs, one value per each key (column)

Refer to `SphinxQL reference` - `SHOW STATUS syntax` for data available in global status.

Refer to `SphinxQL reference` - `SHOW META syntax` for data available in last meta status.

In addition to data returned by 'SHOW META' sphinxql command, API command will also return, in addition, values for `predicted_time` and `dist_predicted_time`.

In opposite to sphinxql statements 'show status' and 'show meta', API command has no way to pass additional 'like' filtering string, so payload of response will always return all available lines of status or meta.

##### SEARCHD_COMMAND_FLUSHATTRS command payload version 1.0
Flush all in-memory attribute updates in all the active disk indexes to disk.

Command has zero payload, so you have nothing to send apart the message with that command, and 0 as size of it's payload.

##### SEARCHD_COMMAND_FLUSHATTRS response payload version 1.0
Response of `SEARCHD_COMMAND_FLUSHATTRS` contains a single `int tag` value, which identifies the result on-disk state (basically, a number of actual disk attribute saves performed since the daemon's startup).

#### Private API commands
These commands are mostly internal and used in master-agent communication (when you invoke corresponding commands to distributed indexes). However they're not 'secret', but just dismissed in support, so you're welcome to implement and share your implementation.

##### SEARCHD_COMMAND_SPHINXQL command payload version 1.0
Sphinxql is special 'proxy' command which allows you to invoke and execute SphinxQL queries over binary API protocol. In contrast to real SphinxQL, each such command is stateless and intended only for appropriate SphinxQL statements which do not require state.

For example, you can perform a command like complex conditional update or delete; you may execute 'select' - but you can't execute properly 'show meta' after such select. However during the call temporary session persists, so you may execute 'select ...;show meta' multistatement and take expected result.

Payload of the command is single `string sqlquery`, containing any query or batch of queries, ';'-separated, you want to execute, written in sphinxql syntax. 

##### SEARCHD_COMMAND_SPHINXQL response payload version 1.0
Sphinxql response contains [raw dump](#sphinxql-dumps) of mysql proto response. If it would be real sphinxql session, mysql client would receive this bytes from network and then interpret this blob and show you the result(s) accordingly. But with API command you just have this row blob with proto dump and have to parse/interpret it yourself.
Small reference for this dump is placed at the end of this document. 

##### SEARCHD_COMMAND_PING command payload version 1.0
Ping command is simple 'ping'. It has a single `int cookie` as payload. You can send any integer here.

This command may be used to evaluate daemon's connection responsibility and measure, if necessary, 'pure delay' of protocol interface. Since command does nothing, you can measure pure time of networking and protocol roundtrip. That command is by default issued by head's daemon to all the agents to imagine their's responsibility. That is done in period of `ha_ping_interval`, which is 1sec by default.

##### SEARCHD_COMMAND_PING response payload version 1.0
Ping command returns single `int cookie` value, the one which was passed in the command.

##### SEARCHD_COMMAND_UVAR command payload version 1.0
`SEARCHD_COMMAND_UVAR` command is used by head daemon to distribute user-defined variables, which usually used in different kind of filtering with 'IN' clause.

Look for `SphinxQL reference` - `SET syntax` to learn about user variables. Here we speaks about line which set so-called 'global user variable' and has general syntax as
```sql
SET [INDEX index_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
```

When you invoke it without pointing an index, only local variable will be set. However when you point index by name, head daemon will determine if the index is distributed and has remote agents, and if so, will send the values to agents over network using this very command.

Command contains following data:
```text
string var_name
int num_values
array of octets // values as VLB-LE-packed delta-encoded blob
```

`var_name` is the name of user variable we want to set

`num_values` is quantity of values in the list we want to assign to `var_name` uservar

Then follows blob (array of octets) with packed values.

This blob produced from original array of `uint64`, which represents actual variable list of values. Before sending the array, first, has to be  sorted. Then it values have to be transformed using [delta-encoding](#delta-encoding). And these deltas finally need to be packed into blob one-by-one with [VLB LE compression](#vlb-little-endian-compression).

Example:

 * Let original variables are `uint64` numbers: 100, 40, 2. Being written 'as is' they will occupy 24 octets.
 
 * After sorting we have sequence of `uint64`: 2, 40, 1000.
 
 * After delta-encoding, we have sequence of `uint64`: 2, 38, 960
 
 * After VLB little-endian compression, we have sequence of `octet`s: 2, 38, 192, 7
 
So finally from 3 `uint64` (which are 24 octets) we have blob of only 4 `octet`s which will be send over network.

##### SEARCHD_COMMAND_UVAR response payload version 1.0
Response to UVAR has only single `int` in payload, which interpreted as boolean indicates whether command was success(1) or not (0).
(actually if response status is `SEARCHD_OK`, this value is always 1, i.e. 'success')

##### SEARCHD_COMMAND_JSON command payload version 1.0
JSON is special 'proxy' command which allows you to invoke and execute REST queries over binary API protocol instead of HTTP.
Payload contains two strings:
```text
string endpoint
string request
``` 

`endpoint` contains endpoint's address, as one who usually written in URL after host root when you issue such query via HTTP proto.

`request` is the request in JSON format.

All the section about `HTTP API reference` will help you to imagine what kind of queries you can pass via this wrapper.

##### SEARCHD_COMMAND_JSON response payload version 1.0
Response of JSON consists from two fields:
```text
string endpoint
array of octets // result
```

`endpoint` is actually the same that came in command;

`result` contains dump of the answer. By nature that is same 'raw dump' as in case of sphinxql. But since HTTP proto is textual, you may expect to recognize in this blob simple textual JSON object with result of your query.

##### SEARCHD_COMMAND_CALLPQ command payload version 1.0
Call PQ is internal function of Manticore which was implemented and exposed over network to make it working with distributed indexes.

Read first `Searching` - `Percolate query` section, and also `SphinxQL reference` - `CALL PQ syntax` to learn about percolate queries.

CALLPQ command payload contains following data:
```text
DWORD flags
string sIdAlias
string sIndex
int iShift
array of arrays of bytes // documents
```

`flags` is a bitset, containing following masked booleans (each bit set to 1 means 'true', 0 means 'false'):
```text
&1 - need_docs
&2 - need_query
&4 - json_docs
&8 - verbose
&16 - skip_bad_json
```

`need_docs` - whether we need to return list of matched docs (namely, they document-ids)

`need_query` - whether we need to dump original stored matched queries from the index

`json_docs` - is set when `documents` are jsons passed as [BSON blobs](#binary-json), otherwise they are plain strings.

`verbose` - whether daemon need or not return also additional data for queries, for now is times spent by each matched query.

`skip_bad_json` - indicates what is to do if provided JSON document is invalid. Daemon should either immediately stop whole call and report error, or it should skip the doc and process the rest or the query. Actually this flag over API proto works only with totally empty JSON documents, since initial parsing is already done on client side, and it already processed all critical errors.

`sIdAlias` - has sense only for JSON documents. It says, which of attributes in such document contains documentID value. This value is useful only if you set boolean `need_docs`; otherwise these values will be never shown.

`sIndex` - name of the index we work with. That is one local PQ index, or one distributed index which built from local or remote PQ indexes.

`iShift` - is value which will be added to all document-ids. That works only if documents contains no ids, and so, auto-numerated. Otherwise provided Document-id will be returned always as-is, despite this value.

`documents` - is either array of strings (if `json_docs` is false), or array of [BSONs](#binary-json) (if `json_docs` is true). But despite that value it is always safe to consider them as 'array of arrays of bytes', since both strings and BSON blobs are arrays of octets by nature.

Plain-text documents contains nothing, but plain text. Json documents, passed as BSON blobs, could contain several fields and attributes, and so, saved filters of PQ may be applied to these attributes.

##### SEARCHD_COMMAND_CALLPQ response payload version 1.0
This payload is the answer to CALLPQ command. It contains from:
```text
DWORD queryflags
array of querydesc
uint64 tmTotal
uint64 tmSetup
int QueriesMatched
int QueriesFailed
int DocsMatched
int TotalQueries
int OnlyTerms
int EarlyOutQueries
array of int QueryDT
string sWarnings
```

`queryflags` is a bit mask, describing content of `querydesc` descriptors in following array.

It provides following masked booleans (1 means 'true', 0 means 'false'):
```text
&1 - has_docs
&2 - dump_queries
&4 - has_docids
```

`querydesc` is a structure, describing matched stored query from PQ index. It contains:
```text
uint64 queryID
[docs] // present only if bit has_docs of flags is set
[query] // present ONLY if bit dump_queries of flags is set
```

`queryID` is id of stored query which matched.

`docs` is optional, and will present ONLY if `has_docs` bit of `flags` is set (i.e. if flags&1 is not zero)

If also `has_docids` is set (i.e. if flags&4 is not zero), `docs` is `array of uint64s`, containing list of documents which matched by stored query with `queryID` of the match.

If `has_docids` is NOT set, `docs` is `array of ints`, containing list of order numbers of matched documents, increased to value of `iShift`. Remote client may disbribute big chunk of docs among several agents, and using `iShift` param then collect returned data into one solid resultset.

`query` is optional, and will present ONLY if `dump_queries` bit of `flags` is set (i.e. if flags&2 is not zero).
It consists, in turn, from:
```text
DWORD uDescFlags
[string sQuery] // optional
[string sTags] // optional
[string sFilters] // optional
``` 

`uDescFlags` is a bit mask, describing presence of following strings and format of the query.

It provides following masked booleans (1 means 'true', 0 means 'false'):
```text
&1 - query_present
&2 - tags_present
&4 - filters_present
&8 - query_is_ql
```

`sQuery` present ONLY if uDescFlags&1 is not zero, and it contains text of the query.

`sTags` present ONLY if uDescFlags&2 is not zero, and it contains text of the tags.

`sFilters` present ONLY if uDescFlags&4 is not zero, and it contains text of the filters.

`query_is_ql` indicates, that query is in ql-format; otherwise it is json. That flag may be used by client to format given text of the query to client application. Say, for ql queries published in json resultset it may be escaped and placed in special 'query' string, but pure json queries may be returned 'as is'.

In simplest case, when `queryflags` is zero (i.e. no bit is set), `querydesc` is just a single `uint64` value of queryID, and so, whole set of querydesc is just `array of unit64`.

Then follow meta values, returned together with resultset.

`tmTotal` represents total time spent to `call pq` command, in microseconds

`tmSetup` is time spent to initial setup of matching - parsing docs, settins options, etc.; in microseconds

`QueriesMatched` - how many stored queries match the document(s)

`QueriesFailed` - number of failed queries

`DocsMatched` - how many times the documents match the queries stored in the index

`TotalQueries` - total number of stored (in the index) queries

`OnlyTerms` - how many queries in the index have terms. The rest of queries have extended query syntax

`EarlyOutQueries` - how many queries quickly matched and rejected with filters or other conditions

`QueryDT` - array of execution time, in microseconds, per each query. This array is filled only if you provided `verbose` flag in the command, otherwise it is empty array (but still present, i.e. will be single sequence of `00 00 00 00` bytes indicating an empty array)

`sWarnings` - warnings. That is a bit of out-of-rules, since warnings usually have to be send with status code before payload, and this is a little bug in CALL PQ payload implementation.

##### SEARCHD_COMMAND_CLUSTERPQ command payload version 1.0
ClusterQP is purely internal command which helps daemon to manage synchronous cluster work as initial synchronisation and so on.
That is now under active development, so this description is most probably already out-of-date. So, don't implement and use this command! Also consider, that actually it may have sense only for master-agent communication, and it may not be useful at all for external clients.

Cluster PQ has its own set of commands, encapsulated into binary API protocol.
Common payload of the command is quite simple, and contains:
```text
DWORD cluster_command
cluster_command_payload
```

Command is one of these values:
```text
CLUSTER_DELETE = 0
CLUSTER_FILE_RESERVE = 1
CLUSTER_FILE_SIZE = 2
CLUSTER_FILE_SEND = 3
CLUSTER_INDEX_ADD_LOCAL = 4
CLUSTER_SYNCED = 5
```
For each of `cluster_command` concrete payload is different.

`CLUSTER_DELETE` - has single `string` in payload, which is name of the cluster. Answer payload contains single `byte` =1.

`CLUSTER_FILE_RESERVE` has payload of:
```text
string sCluster
string sIndex
string sFilename
uint64 fileSize
string sFileHash
```

Reply to `CLUSTER_FILE_RESERVE` has payload:
```text
uint64 filesize
string sFileHash
string sIndexPath
```

`CLUSTER_FILE_SIZE` is not implemented.

`CLUSTER_FILE_SEND` has payload of:
```text
string sCluster
string sFilename
uint64 iFileOffset
array of bytes // data
```

Reply to `CLUSTER_FILE_SEND` has payload:
```text
uint64 IndexFileSize
```

`CLUSTER_INDEX_ADD_LOCAL` has payload of:
```text
string sCluster
string sIndex
string sIndexPath
byte IndexType // == 3, means PERCOLATE
uint64 IndexFileSize
string sFileHash
```

Reply to `CLUSTER_INDEX_ADD_LOCAL` has payload:
```text
uint64 IndexFileSize
string sFilePath
```

`CLUSTER_SYNCED` has payload of:
```text
string sCluster
string m_sGTID
array of strings // names of indexes
```

Reply to `CLUSTER_SYNCED` has payload of single `byte` value (1).

## Appendix. Another formats and algorithms
Here is small reference description on formats and approaches, mentioned in the doc.

### Delta encoding
Delta encoding is used when the sequence of the values to store is monotonically non-decreasing. Each value is replaced with its difference (delta) from the previous value.

Example:
```text
	source-sequence = 3, 5, 7, 11, 13, 17, ...
	delta-encoded = 3, 2, 2, 4, 2, 4, ...
```
The resulting deltas are smaller, and compress more efficiently. Very first value is written 'as is'.

### VLB compression

VLB compression encodes a fixed-length (32-bit or 64-bit) integer value to a variable-length byte string, depending on the value. 7 lower bits of every octet contain next 7 low bits of the compressed value; and 8-th bit signals whether the are more bytes following. Note that high bits come first! And that is essential difference from described below VLB LSB compression!

Hence, values that take 7 bits (0 to 127, inclusive) are stored using 1 byte, values that fit in 14 bits (128 to 16383) are stored using 2 bytes, etc.

Examples:
```text
	source-value = 0x37
	encoded-value = 0x37
```

```text
	source-value = 0x12345
	encoded-value = 0x84 0xC6 0x45
		// 0x84 == ( ( 0x12345>>14 ) & 0x7F ) | 0x80
		// 0xC6 == ( ( 0x12345>>7 ) & 0x7F ) | 0x80
		// 0x45 == ( ( 0x12345>>0 ) & 0x7F )
```

### VLB Little-endian compression

VLB little endian compression encodes a fixed-length (32-bit or 64-bit) integer value to a variable-length byte string, depending on the value. 7 lower bits of every octet contain next 7 high bits of the compressed value; and 8-th bit signals whether the are more bytes following. Note that low bits come first!

Hence, values that take 7 bits (0 to 127, inclusive) are stored using 1 byte, values that fit in 14 bits (128 to 16383) are stored using 2 bytes, etc. In opposite to legacy (big-endian) variant little endian as a bit simpler in implementation since it doesn't require to calculate final blob length before you start the encoding.

Examples:
```text
	source-value = 0x37
	encoded-value = 0x37
```

```text
	source-value = 0x12345
	encoded-value = 0xC5 0xC6 0x04
		// 0xC5 == ( ( 0x12345>>0 ) & 0x7F ) | 0x80
		// 0xC6 == ( ( 0x12345>>7 ) & 0x7F ) | 0x80
		// 0x04 == ( ( 0x12345>>14 ) & 0x7F )
```


### Binary json
Disclaimer: BSON is not part of binary API protocol, it is just internal structure that sometimes may come in blob from daemon, or sometimes may be provided from client side (consider using custom 'CALL PQ' implementation which sends structured documents in BSON form). So, from viewpoint of binary API protocol, BSON is just a blob, or `array of bytes`, and it is sent over network as is, without any conversion like byte order adjusting and etc. So, one side sends `array of bytes`, another receives it - and area of communication protocol is finished at this point; next you need to parse or interpret that blob some way.

So, this section is just description of such internal blob format as BSON, it is not related to anything mentioned above, as status codes, messages, byte order and so on. That is totally different.

BSON is 'binary json' - serialized machine-readable binary format which Manticore uses internally to store and process json documents. It was inspired by [standart BSON](http://bsonspec.org/) which developed by MongoDB, and was adapted to Sphinx requirements, and sometimes simplified. For example our BSON doesn't support long 128-bits floats. Also it has special packed form for fixed-size array. Also it has embedded bloom filters to boost navigation over associative arrays. So, just take in mind that BSON described here is NOT the standard BSON, but Manticore's own dialect, and they are NOT mutually compatible.

All numbers, which are stored values, are stored in native format, that is explicit little-endian (since majority of servers works on intel arch where this order is native) for length values.

Floats stored also in native `IEEE-754` standard, as they used, without any conversions. 

All variable-sised data, if possible, prepended by the size, size is assumed to fit in DWORD (i.e. unsigned 32-bits integer), but stored in special packed [BSON compression format](#bson-number-compression) and occupy 1 to 5 octets of the storage.

All numbers, which are meta-data (sizes of arrays, lengths of strings, etc.) always stored in [BSON compressed form](#bson-number-compression). The only exception is [Bloom hash](#bloom-hash) at the beginning of associatives - since it is random by nature, this is no reason to compress it. Packed numbers are referenced later as `packednum`.

As a special example - strings in BSON are stored as packet length, followed by actual string characters, That is quite similar as binary protocol string, but the length, note, is packed number. Such strings later mentioned as `packstr`.

For example, 'hello world' in `packstr` is sequence of `0x0B` 'h' 'e' 'l' 'l' 'o' ' ' 'w' 'o' 'r' 'l' 'd', and occupies totally 12 bytes. If length of the string is more than 251 chars, it will ocupy 3+ bytes, and so on.

For associative arrays (objects) BSON also stores [Bloom filter](#bloom-hash), which helps quickly reject the whole array if necessary key is NOT present there.

All nodes are either `named_node`, either simple `node`.

Each node has following format:
```text
byte node_type
[packstr node_name]
[node_value]
```

`node_name` may present ONLY if it is `named_node`, otherwise absent.

`node_value` may present and follow `node_type`, or may absent, if value is obvious from the type itself.

`node_type` is one of:
```text
JSON_EOF = 0
JSON_INT32 = 1
JSON_INT64 = 2
JSON_DOUBLE = 3
JSON_STRING = 4
JSON_STRING_VECTOR = 5
JSON_INT32_VECTOR  = 6
JSON_INT64_VECTOR  = 7
JSON_DOUBLE_VECTOR = 8
JSON_MIXED_VECTOR  = 9
JSON_OBJECT = 10
JSON_TRUE = 11
JSON_FALSE = 12
JSON_NULL = 13
JSON_ROOT = 14
```

`JSON_EOF` node is terminator. It finished json object and root element, and has nor name, neither value.

`JSON_TRUE`, `JSON_FALSE`, and `JSON_NULL` has no `node_value`, since it is obvious from the type.

`JSON_INT32` has value `int`.

`JSON_INT64` has value `int64`

`JSON_DOUBLE` has value `double`, that is long float

`JSON_STRING` has value `packedstr`

`JSON_STRING_VECTOR` represented by:
```text
packedint size
packedint length
packstr[length] values // packedstrs of length quantity.
```

`size` represents size (in bytes) of whole array; so that when navigating over BSON you can skip it as whole, using this value.

`length` represents number of stored strings, i.e. length of array.

`values` is set of `packstr` values, one-by-one.

`JSON_INT32_VECTOR` represented by:
```text
packedint length
int[length] values // 32-bit integers of length quantity.
```

`length` represents number of stored numbers, i.e. length of array.

Since size of every number is determined (that is, 32 bits, or 4 bytes), no `size` as in case with `JSON_STRING_VECTOR` is stored, since you always can take the size by multiplying `length` to size of element (=4).

`JSON_INT64_VECTOR` represented by:
```text
packedint length
int64[length] values // 64-bit integers of length quantity.
```

`length` represents number of stored numbers, i.e. length of array.

Since size of every number is determined (that is, 64 bits, or 8 bytes), no `size` as in case with `JSON_STRING_VECTOR` is stored, since you always can take the size by multiplying `length` to size of element (=8).

`JSON_DOUBLE_VECTOR` represented by:
```text
packedint length
double[length] values // 64-bit doubles (floats) of length quantity.
```

`length` represents number of stored numbers, i.e. length of array.

Since size of every number is determined (that is, 64 bits, or 8 bytes), no `size` as in case with `JSON_STRING_VECTOR` is stored, since you always can take the size by multiplying `length` to size of element (=8).

`JSON_MIXED_VECTOR` represented by:
```text
packedint size
packedint length
nodes // values of any type, except `JSON_ROOT` and `JSON_EOF`
```

`size` represents size (in bytes) of whole array; so that when navigating over BSON you can skip it as whole, using this value.

`length` represents number of stored nodes, i.e. length of array.

`JSON_OBJECT` represented by:
```text
packedint size
DWORD Bloom
packedint length
named_nodes // values of any type, except `JSON_ROOT` and `JSON_EOF`
JSON_EOF
```

`size` represents size (in bytes) of whole object; so that when navigating over BSON you can skip it as whole, using this value.

`Bloom` is the Bloom filter hash. If you need to check if a `key` is represented in the object, you can calculate [Bloom hash](#bloom-hash) of that key and intersect it with this value by logical `AND`. If result is different from your calculated hash, then this object is definitely NOT contains value, corresponding to your key. Otherwise you have to linearly navigate array elem-by-elem and check name of every node, if it equals to the `key`.

`length` represents number of stored nodes, i.e. length of array.

Root element is the very first node, and it has it's own special format:
```text
dword Bloom
named_nodes
byte node_eof // = 0
```

It differs from `JSON_OBJECT` by that it has no `node_type` (it is obviously `JSON_ROOT`, since it its very first), and also it has no `size` element. Last is nesessary only to step over the node, but root element occupies whole blob, so you don't even need to step over it, or can take size of payload, if you need it.

#### Bloom hash
Hashing of the keys done by following algorithm:

 1. Caluclate CRC32 of the string key.
 2. take `( 1UL<<( uCrc & 31 ) ) + ( 1UL<<( ( uCrc>>8 ) & 31 ) )`;
 
 Otherwords, we take low WORD of CRC32, and then take two numbers from them, as:
 
 `000BBBBB 000AAAAA`
 
 Then we take zero DWORD and set A-th and B-th bits of it to '1'. Final value of hash contains mostly 2 non-zero bits in pseudo-random positions.
 
Whole hash mask (bloom filter) is made by logically summing all member hashes by arithmetic 'OR' operation.

So, say, for set of 'key1', 'key2', and 'key3' we first take hash1, hash2, and hash3, and then calculate common hash as `hash1 | has2 | hash3`. This common 32-bits hash is final bloom filter, and it is placed at the beginning of the root node, or at the beginning of any json object (which is associative array).

Hack: if you dont' want to calculate bloom mask, you may provide value `0xFFFFFFFF` instead. That will effectively switch off all Bloom filtering. However searching of every key in such case will be pefromed as checking of all values, and can't be boosted by Bloom filter.

#### BSON number compression
BSON internally compress DWORD numbers with own VBL compression. Bits of the number are NOT shifted, instead whole significant bytes of original number are always written in little-endian order by following template:

 - numbers which are >0 and <0x000000FC occupies exactly 1 byte, that is the only significant byte of the num.
 - numbers <0x00010000 occupies 3 bytes; first is prefix `0xFC` and then two bytes, little-endian order.
 - numbers <0x01000000 occupies 4 bytes; first is prefix `0xFD` and then three significant bytes, little-endian order.
 - rest of the numbers occupies 5 bytes; first is prefix `0xFE` and then number as is, in little-endian order.
 - blob starting from `0xFF` is illegal.

#### Hints about BSON
 1. That is not enough to have just pointer to BSON node, since you can't immediately determine if it is `node`, or `named_node`. So, even if you read first byte and extract the type, you don't know either next bytes are part of value(s) itself, or it is `packstr` name of the node. So, if you need such kind of locator, you have to store there node's type, and pointer to real node's value.

 2. If you parse text json into BSON yourself and meet an array, you can always provide it as 'mixed vector'. However it would be better if you try to cast its type to one of 'fixed-size' arrays. Collect all the types of the nodes, and if they are, say, all are 'string' - pack result as BSON's string array. If they're numbers, try to determine their range: if all of them are integer and fits into 32 bits - make vector of int32, or try vector of int64, or vector of doubles.

### Packed factors
Disclaimer: Packed factors is not part of binary API protocol, it is just internal structure that sometimes may come in blob from daemon. So, from viewpoint of binary API protocol, packed factors is just a blob, or `array of bytes`, and it is send over network as is, without any conversion like byte order adjusting and etc. So, one side sends `array of bytes`, another receives it - and area of communication protocol is finished at this point; next you need to parse or interpret that blob some way.

Packed factors contains different metrics available during ranking of the matches. Also they may be passed outside as attribute of type SPH_ATTR_FACTORS, and in this case they will be provided as raw blob.

Packed factors is block of memory containing may DWORDs and [floats](#floats).
In opposite to numbers for internal binary sphinx API, they stored in raw byte order which is specific for runtime system. So, from intel platform you'll get them as little-endian, another (like MIPS) will give you big-endian blobs; be aware!

Blob has following format:
```text
DWORD size // size of whole blob in bytes

// document level factors
DWORD doc_bm25
float doc_bm25a
DWORD matched_fields
DWORD doc_word_count

// field level factors
DWORD num_of_fields
DWORD exact_hit_mask[(num_of_fields+31)/32] // bitfield
DWORD exact_order_mask[(num_of_fields+31)/32] // bitfield
struct field {
	DWORD hit_count
	// if hit_count!=0 [
		DWORD id
		DWORD lcs
		DWORD word_count
		float tf_idf
		float min_idf
		float max_idf
		float sum_idf
		int min_hit_pos
		int min_best_span_pos
		int max_window_hits
		int min_gaps
		float atc
		DWORD lccs
		float wlccs
	// ] // end of optional block
} [num_of_field]

// word level factors
DWORD max_uniq_qpos
struct qpos {
	DWORD keyword_mask
	// if keyword_mask!=0 [
		DWORD id
		int tf
		float idf
	// ] // end of optional block	
} [max_uniq_qpos]

// fields tf factors
DWORD fields
int fields_tf[fields]
```
Read `Searching` - `Search results ranking` section to learn about factors.

### Sphinxql dumps
Disclaimer: Sphinxql dumps is not part of binary API protocol, it is just internal structure that sometimes may come in blob from daemon. So, from viewpoint of binary API protocol, sphinxql dumps is just a blob, or `array of bytes`, and it is send over network as is, without any conversion like byte order adjusting and etc. So, one side sends `array of bytes`, another receives it - and area of communication protocol is finished at this point; next you need to parse or interpret that blob some way.

Sphinxql dumps is usual dump of mysql proto message, and you can find necesary information on MySQL site. However Manticore supports mysql proto only partically, on the most basic level which is enough to send queries and receive answers, without any modern and sometimes complex things like compression and cryptography.

Manticore implements both parsing of incoming messages from mysql proto and also sending responses in mysql proto. Since in binary API you can meet only responses, encapsulated as blobs of [SEARCHD_COMMAND_SPHINXQL](#searchd_command_sphinxql-response-payload-version-10), there is no need to describe format of incoming messages here, so such information is omited.

#### MYSQL packed numbers
Mysql uses own VLB compression for integer numbers. It is prefix-based, so that first byte of sequence determines the size of payload and m.b. even value itself.

An interer can consume 1, 3, 4 or 9 bytes depending from it's numeric value. A fixed-length integer stores its value in a series of bytes in little-endian order (least significant byte first).

 * Values < 251 stored as 1-byte integer
 * Values from 251 and up to 65535 (0xFFFF) stored as `0xFC` + 2-byte integer
 * Values from 65536 and up to 0xFFFFFF (24 bits) stored as `0xFD` + 3-byte integer
 * Values from 0x1000000 up to 0xFFFFFFFF (32 bits) stored as `0xFE` + 4-byte integer + `0x00 0x00 0x00 0x00` sequence.

NOTE: prefix `0xFE` actually sends 8-byte integer, but manticore uses only 4 byte, padding the rest 4 bytes with zeros.
 
In next sections such packet numbers will be refferd as having `mysqlint` type.

#### MYSQL strings
In following sections strings marked with following ways:

`string[EOF]` - a string, which is the last component of a packet, its length can be calculated from the overall packet length minus the current position.

`string[lenenc]` - a string, prefixed with `mysqlint` describing the length of the string.

#### MYSQL packet
Each packet starts with packet ID and size of following payload. These values packed into one `DWORD`, as:

`0xPPS3S2S1` Highest byte `0xPP` is packet ID, it is incremented for each next packed. When it reach `0xFF` it starts again from `0x00`. Rest 24 bits of value (`0xS3S2S1`)is size of following payload, in bytes. So, one mysql packet could contain maxumum `0x00FFFFFF` bytes + 4 bytes for header itself.

This DWORD placed in little-endian order, so that in terms of raw bytes you'll see sequence `0xS1 0xS2 0xS3 0xPP` in the beginning of mysql response.

Using the `length` part of header's DWORD you can rapidly step to the next packet.

Blob may contain one or many packets, placed one-by-one.

Manticore uses following types of messages:

##### OK packet
```text
DWORD packet_header // described above
byte header // =0x00
mysqlint rows_affected
mysqlint last_insert_id // =0
WORD status
WORD num_of_warnings
string[EOF] message // ok message
```

OK packet is successive response to an operation which returns no rowset, like 'insert into', 'set variable', etc.

`header` for OK packet is always 0 (that is flag of 'OK' packet)

`rows_affected` - number of rows affected by operation (like '10 rows' inserted, or '1 row' deleted, etc.)

`last_insert_id` - Manticore always returns 0. (real mysql returns last ID used in ops with auto-incrementing values)

`status` - is a bitfield.

Manticore may use 2 bits to indicate:
```text
FLAG_STATUS_AUTOCOMMIT 2 // auto-commit is enabled
FLAG_MORE_RESULTS = 8 // if packet is just a part of big resultset
```
The rest of bits are zero.

`num_of_warnings` - obvious. That is just a number; messages themselves are not placed in this packet.

`message` - status message. 

Example: OK packet with 0 affected rows, enabled AUTOCOMMIT and 0 warnings.
```text
07 00 00 02 00 00 00 02 00 00 00
```

##### EOF packet
```text
DWORD packet_header // described above
byte neader // =0xFE
WORD num_of_warnings
WORD status
```

EOF packet looks similar as OK packet. It terminates any continuous sequence of packets as list of header's columns, or whole resultset.

Note that header byte `0xFE` is same as prefix for 32-bits numbers, so that meeting this byte just after the header may occur both at the beginning of `EOF` packet, and also as a part of the number of columns of `RESULTHEADER` where right after the packet header follows packed int with the num. You must check whether the packet length is less than 9 to make sure that it is a EOF packet.

Example: EOF packet with 0 warnings, AUTOCOMMIT enabled:
```text
05 00 00 05 FE 00 00 02 00
```

##### ERROR packet
It contains no rowset, but error message. Manticore implements these format:
```text
DWORD packet_header // described above
byte header // 0xFF
WORD error_code
string[EOF] message // error message
```

`header` for Error packet is always `0xFF`

`error_code` contains one of:
```text
UNKNOWN_COM_ERROR = 1047
SERVER_SHUTDOWN = 1053
PARSE_ERROR = 1064
FIELD_SPECIFIED_TWICE = 1110
NO_SUCH_TABLE = 1146
TOO_MANY_USER_CONNECTIONS = 1203
```

`message` - error message. It begins with fixed 6-chars prefix, depends from `error_code`:

 * "#08S01" - for `SERVER_SHUTDOWN` and `UNKNOWN_COM_ERROR` codes
 * "#42S02" - for `NO_SUCH_TABLE` code
 * "#42000" - for the rest of the codes (`PARSE_ERROR`, `FIELD_SPECIFIED_TWICE` and `TOO_MANY_USER_CONNECTIONS`)
 
##### FIELD packet
That is packet, describing one field of a table as name, type, and size
It has following structure:
```text
DWORD packet_header // described above
string[lenenc] def // ="def"
string[lenenc] db // =""
string[nenenc] table // =""
string[lenenc] org_table // =""
string[lenenc] name
string[lenenc] org_name // same as name
byte filter // =12
WORD charset_nr // =0x0021, means utf8
DWORD column_length 
byte type
WORD flags
byte decimals // =0
WORD filter // =0 
```

All fixed values are defined in description and just mysql-specific.
Manticore set following fields from the packet:

`name` - name of the column
`org_name` - is, again, name of the column. Manticore set both `name` and `org_name` to one and same string.

`type` - type of the column. That is one of:
```text
MYSQL_COL_DECIMAL = 0
MYSQL_COL_LONG = 3
MYSQL_COL_FLOAT = 4
MYSQL_COL_LONGLONG = 8
MYSQL_COL_STRING = 254
```

`column_length` - length of the column. It depends from `type` and is:

 * 11 for `MYSQL_COL_LONG`
 * 20 for `MYSQL_COL_DECIMAL`, `MYSQL_COL_FLOAT` and `MYSQL_COL_LONGLONG`
 * 255 for `MYSQL_COL_STRING`
 
 `flags` - is logical part of the `type`. Manticore uses only 1 bit which describe whether numeric is signed or not. So that whole value may be either 0 (column is signed), or 32 (column is unsigned).

##### RESULTSET
Resultset contains table data from daemon. It is not one packet, but sequence of packets, describing first schema of resultset, and then data itself.

First packet in sequence determines number of columns in resultset. It is tiny and contains just header and `mysqlint` number of columns:
```text
DWORD packet_header // described above
mysqlint column_count
```
Above it is already described possible collision between first `0xFE` byte as flag of `EOF` packet in opposite of the same as part of `mysqlint` encoding schema of `column_count`.

Then follow [FIELD](#field-packet) packets, according to number of columns passed in first packet.

Then follows [EOF](#eof-packet)

That 3+ packets determines table's header. After this header may follow data itself.

Each data packet contains values of a whole row of a table and consists from usual `packed_header`, followed by the data itself. Data is:

 * NULL values is sent as `0xFB`
 * all other values are converted into a string and send as `string[lenenc]`.

After all data packets follows [EOF](#eof-packet) packet.
