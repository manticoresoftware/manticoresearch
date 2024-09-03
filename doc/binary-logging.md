# Manticore binary logging

	WARNING. This document is just an internal note. It might or might not be
	in sync with the actual code. Use it as an overview; refer to the source
	for precise details.

## General

Manticore serializes some operations in binary log. If a crash happened, manticore replays stored logs and restores data in appropriate tables. Binary log is set of files placed by the path provided in daemon as 'binlog_path' param.
If not defined explicitly, it is build config param `LOCALDATADIR`; in test mode that is current working dir.

* `binlog.meta`, header file
* `binlog.NNNN`, dictionary (aka wordlist)
* `binlog.lock`, 0-sized lock file

### meta

	dword magic, = chars[] 'SPLI'
	dword version
    byte digits
    zint files
        zint filenames[]


* `digits` define N of digits for naming `binlog.NNN` files. For 1 they will be `binlog.0`...`binlog.9`. For 5 they will be `binlog.00001`..`binlog.99999` and so on.
* N of files placed as BE zipped int. Most probably that is single one byte, as N of binlog files quite seldom came over 127.
* filenames is the sequence of BE zipped ints, corresponding to served `binlog.NNNN` files.

### binlog files

	dword magic, = chars[] 'SPBL'
	dword version
    [ transactions ]
    [ index cache transaction ]

* `version` is the same as version in meta file.
* `transactions` came sequentally one by another.
* `index cache transaction` is placed at the end of current binlog file in case of clean close (that it obvious - we just have no chance to write the cache in case of crash). Clean close happens if size limit for binlog file reached, or during daemon's shutdown. Size limit in the case is not 'hard' - that is, if current transaction started before limit and trespass it - it will be written without interruption. Then `index cache transaction` will be written, and daemon next will close the file and create next binlog file.

#### transactions

    dword magic, = chars[] 'TXN!'
    byte TxnID
    [ payload ]
    dword checksum, = crc32 of TxnID + payload

There are 3 kinds of payloads, differs by payload.

    // TxnID = 1 (aka 'ADD_INDEX')
    // start of payload
    zint IndexID
    zstring indexName // zstring is zint length, followed by char[length] characters
    // end of payload

This transaction introduces short index ID to be used later in the same file. It starts from 0, and increased as new indexes added to the file. That ID then used in another transactions to refer to the index. That is, ADD_INDEX by definition will be very first txn in any useful binlog file. So, first table will have ID=0, next one ID=1 and so on. ID is local to each single binlog file.

    // TxnID = 3 ( aka 'ADD_CACHE')
    // start of payload
    zind NofIndexes
    IndexRec[NofIndexes]
	IndexRec is:
        zstring indexName
        zint    minTID
        zint    maxTID
        zint    lastFlushedTID
    // end of payload

This transaction placed on the end of binlog file. Its presence is optional, and it is used solely for final checking of the binlog. For the empty binlogs it might be the single transaction of the file, in this case N of indexes will be zero, and no index records will present. Such binlog should be considered than as empty.

    // TxnID = 2 (aka 'ADD_TXN')
    // start of payload
    zint IndexID
    zint TransactionID
    dword uSize
    byte[uSize] index-specific payload
    // end of transaction

This is main transaction record contains actual binlog data. IndexID is one introduced by 'ADD_INDEX' txn before, it just replaces actual index name. 'TransactionID' is monotonically increased order num of index changes. 'uSize' allows to skip the transaction without knowing its internal structure, just as a blob. For example, if table was dropped - during replay you can just read IndexID, ensure it is absent - and then just skip uSize bytes (after uSize itself). Then you can check crc32 of skipped data, and ensure binlog is not corrupted. In case table exists - you can compare TransactionID with one stored in the table, and if table's one is greater than stored in txn - same way skip the transaction.
In the case when table is present, and TransactionID in txn is greater than tID of the table, daemon invokes replay action, which parses and uses index-specific payload.

Index-specific payload, for convenience, has the following structure:

    byte PayloadID
    ... table specific different blobs.

PayloadID for now has 3 different kinds: update attributes (0), rt-commit (1) and pq add/delete (2). That is purely convenience, and binlog will work Ok without this byte. But it helps to collect replay statistic (say, 'you have 100 commits, 50 updates and 32 pq changes'). And also it can serve as last-chance error-detection, say, if by some unknown reason your rt-table became replayed with pq add/delete transactions which it doesn't know how to deal with.

## Logical structure of binlog

Binary logging is organized into several different logical layers.

Topmost binlog level - whole binary log system. It is configured on startup and knows the binlog directory - place where all binlog files live. Also, it manages periodical binlog files flushing - as a planned system periodical event. Also, whole binlog system is responsible for provide filenames for concrete binlogs - the way without collisions. It stores the global binlog meta also. And it performs replaying the binlogs on system startup. When you commit a txn - binlog system determines a single binlog your record will come to - and pass the data to it. If there is no appropriate binlog - it creates one and place into hash (by index name). When we work with global binlog - index names from commits just cleaned out, so that we push all commits into one and same binlog despite the table name. When we use per-index binlogs, hash works and provide different binlog for different tables.

Next level is one single binlog. It manages one or more binlog files. It has internal table of indexes it serves. When commit came - it searches corresponding indexID, or issues txn 'ADD_INDEX' to create new indexID. Then it creates ADD_TXN record and provides all the headers. Then it invokes index-specific writer function which fills the blob. Finally, it writes the size of the record and it's checksum. When size of binlog file reaches limit (if defined) - single binlog will create next file, asking the name from the topmost binlog system. To the end of the file reached limit it writes 'INDEX_CACHE' transaction and closes the file.

If message 'notify-index-flush' came, single binlog checks each file for records belonging to flushed table. If everything is flushed, file then considered as totally free and unlinked. In case of common binlog file can still contain not-flushed data belonging to another indexes. In this case file is kept. Single binlog traces min and max transaction ID for all the tables backed by binlog files, despite they're flushed or not. If user dropped the table, and then creates new one with the same name, single binlog will provide starting transaction ID for newborn table, not conflicting with already backed by binlog files. In case of crash, such non-zero tID will help to skip non-actual transaction for newborn table, and replay only ones actual for it.

Bottom level is just file writer. Single binlog has one active file writer, even if it has more than 1 binlog file. That is because only one file is active a time.

Summarizing, binlog contains one or more single binlogs, each of them includes one or more binlog files. Each binlog file can include records for several tables. All binlog files enumerated in metafile.

Difference between 'common' and 'per-index' binlog is purely logical: both have the same structure, but for per-table binlogging each binlog file has exactly one 'ADD_INDEX' transaction, and so, includes transactions only for one single table, and for common binlog in can have more than one such transactions, and so, serve more than one table.

### Replaying binlog

Replaying runs once on daemon startup. To this moment binary logging is configured, and also we have a table of indexes we serve. First, binlog system loads the meta. So, it knows all binlog files, and it replays them sequentially, starting from the first one. That might be files from 'common' binlog, or a file(s) per table - it doesn't matter. At this moment we have only ordered list of files and table with indexes.

First, daemon create one temporary single binlog. Then it replays transaction from first file, one-by-one. First one expected to be INDEX_ADD - mentioned table then added to the list of tables hold by the file. For ADD_TXN transactions we read indexID and tID. If index is served (included in the hash given for replaying) - we check tID, whether it appropriate or not. For example, index may be flushed, so it's actual tID is greater, than read from binlog. In this case we skip txn. Also, index might be just not served (because it was dropped). Same way, skip. If everything is ok, we invoke replay function from the table. It will read the txn (structure opaque for binlog), then binlog reader will check crc32, and replay action finally will apply or skip the transaction. When a file replayed, we have collected records about all the tables from the file. Finally, we take a single binlog for each table, and push replayed data into it. In case of common, it will provide one and same single binlog for all the tables. In case of per-table binlogging - it will create set of single binlogs.

## Order of operations

Binary logging interface includes the couple of entry points: commit and notify-table-flush.
First one used to make a record in binlog. Second informs binlog that data for given table is stored safe, and so, binlog can release the records for the table.

### Commit

... to be continued