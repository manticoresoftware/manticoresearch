Additional functionality
------------------------
.. _build_excerpts:

BuildExcerpts
~~~~~~~~~~~~~

**Prototype:** function BuildExcerpts ( $docs, $index, $words,
$opts=array() )

Excerpts (snippets) builder function. Connects to ``searchd``, asks it
to generate excerpts (snippets) from given documents, and returns the
results.

``$docs`` is a plain array of strings that carry the documents'
contents. ``$index`` is an index name string. Different settings (such
as charset, morphology, wordforms) from given index will be used.
``$words`` is a string that contains the keywords to highlight. They
will be processed with respect to index settings. For instance, if
English stemming is enabled in the index, “shoes” will be highlighted
even if keyword is “shoe”. Keywords can contain wildcards, that work
similarly to star-syntax available in queries. ``$opts`` is a hash which
contains additional optional highlighting parameters:

-  “before_match”:
-  A string to insert before a keyword match. A %PASSAGE_ID% macro can
   be used in this string. The first match of the macro is replaced with
   an incrementing passage number within a current snippet. Numbering
   starts at 1 by default but can be overridden with
   “start_passage_id” option. In a multi-document call, %PASSAGE_ID%
   would restart at every given document. Default is “**”.

-  “after_match”:
-  A string to insert after a keyword match. Starting with version
   1.10-beta, a %PASSAGE_ID% macro can be used in this string. Default
   is “**”.

-  “chunk_separator”:
-  A string to insert between snippet chunks (passages). Default is " …
   “.

-  “limit”:
-  Maximum snippet size, in symbols (codepoints). Integer, default is
   256.

-  “around”:
-  How much words to pick around each matching keywords block. Integer,
   default is 5.

-  “exact_phrase”:
-  Whether to highlight exact query phrase matches only instead of
   individual keywords. Boolean, default is false.

-  “use_boundaries”:
-  Whether to additionally break passages by phrase boundary characters,
   as configured in index settings with
   :ref:`phrase_boundary <phrase_boundary>`
   directive. Boolean, default is false.

-  “weight_order”:
-  Whether to sort the extracted passages in order of relevance
   (decreasing weight), or in order of appearance in the document
   (increasing position). Boolean, default is false.

-  “query_mode”:
-  Whether to handle $words as a query in :ref:`extended
   syntax <extended_query_syntax>`, or as a bag of words
   (default behavior). For instance, in query mode (“one two” \| “three
   four”) will only highlight and include those occurrences “one two” or
   “three four” when the two words from each pair are adjacent to each
   other. In default mode, any single occurrence of “one”, “two”,
   “three”, or “four” would be highlighted. Boolean, default is false.

-  “force_all_words”:
-  Ignores the snippet length limit until it includes all the keywords.
   Boolean, default is false.

-  “limit_passages”:
-  Limits the maximum number of passages that can be included into the
   snippet. Integer, default is 0 (no limit).

-  “limit_words”:
-  Limits the maximum number of words that can be included into the
   snippet. Note the limit applies to any words, and not just the
   matched keywords to highlight. For example, if we are highlighting
   “Mary” and a passage “Mary had a little lamb” is selected, then it
   contributes 5 words to this limit, not just 1. Integer, default is 0
   (no limit).

-  “start_passage_id”:
-  Specifies the starting value of %PASSAGE_ID% macro (that gets
   detected and expanded in ``before_match``, ``after_match`` strings).
   Integer, default is 1.

-  “load_files”:
-  Whether to handle $docs as data to extract snippets from (default
   behavior), or to treat it as file names, and load data from specified
   files on the server side. Up to
   :ref:`dist_threads <dist_threads>`
   worker threads per request will be created to parallelize the work
   when this flag is enabled. Boolean, default is false. Building of the
   snippets could be parallelized between remote agents. Just set the
   :ref:`‘dist_threads’ <dist_threads>`
   param in the config to the value greater than 1, and then invoke the
   snippets generation over the distributed index, which contain only
   one(!) :ref:`local <local>` agent
   and several remotes. The
   :ref:`snippets_file_prefix <snippets_file_prefix>`
   option is also in the game and the final filename is calculated by
   concatenation of the prefix with given name. Otherwords, when
   snippets_file_prefix is ‘/var/data’ and filename is ‘text.txt’ the
   sphinx will try to generate the snippets from the file
   ‘/var/datatext.txt’, which is exactly ‘/var/data’ + ‘text.txt’.

-  “load_files_scattered”:
-  It works only with distributed snippets generation with remote
   agents. The source files for snippets could be distributed among
   different agents, and the main daemon will merge together all
   non-erroneous results. So, if one agent of the distributed index has
   ‘file1.txt’, another has ‘file2.txt’ and you call for the snippets
   with both these files, the sphinx will merge results from the agents
   together, so you will get the snippets from both ‘file1.txt’ and
   ‘file2.txt’. Boolean, default is false.

   If the “load_files” is also set, the request will return the error
   in case if any of the files is not available anywhere. Otherwise (if
   “load_files” is not set) it will just return the empty strings for
   all absent files. The master instance reset this flag when
   distributes the snippets among agents. So, for agents the absence of
   a file is not critical error, but for the master it might be so. If
   you want to be sure that all snippets are actually created, set both
   “load_files_scattered” and “load_files”. If the absence of some
   snippets caused by some agents is not critical for you - set just
   “load_files_scattered”, leaving “load_files” not set.

-  “html_strip_mode”:
-  HTML stripping mode setting. Defaults to “index”, which means that
   index settings will be used. The other values are “none” and “strip”,
   that forcibly skip or apply stripping irregardless of index settings;
   and “retain”, that retains HTML markup and protects it from
   highlighting. The “retain” mode can only be used when highlighting
   full documents and thus requires that no snippet size limits are set.
   String, allowed values are “none”, “strip”, “index”, and “retain”.

-  “allow_empty”:
-  Allows empty string to be returned as highlighting result when a
   snippet could not be generated (no keywords match, or no passages fit
   the limit). By default, the beginning of original text would be
   returned instead of an empty string. Boolean, default is false.

-  “passage_boundary”:
-  Ensures that passages do not cross a sentence, paragraph, or zone
   boundary (when used with an index that has the respective indexing
   settings enabled). String, allowed values are “sentence”,
   “paragraph”, and “zone”.

-  “emit_zones”:
-  Emits an HTML tag with an enclosing zone name before each passage.
   Boolean, default is false.

Snippets extraction algorithm currently favors better passages (with
closer phrase matches), and then passages with keywords not yet in
snippet. Generally, it will try to highlight the best match with the
query, and it will also to highlight all the query keywords, as made
possible by the limits. In case the document does not match the query,
beginning of the document trimmed down according to the limits will be
return by default. You can also return an empty snippet instead case by
setting “allow_empty” option to true.

Returns false on failure. Returns a plain array of strings with excerpts
(snippets) on success.

.. _build_keywords:

BuildKeywords
~~~~~~~~~~~~~

**Prototype:** function BuildKeywords ( $query, $index, $hits )

Extracts keywords from query using tokenizer settings for given index,
optionally with per-keyword occurrence statistics. Returns an array of
hashes with per-keyword information.

``$query`` is a query to extract keywords from. ``$index`` is a name of
the index to get tokenizing settings and keyword occurrence statistics
from. ``$hits`` is a boolean flag that indicates whether keyword
occurrence statistics are required.

Usage example:

::


    $keywords = $cl->BuildKeywords ( "this.is.my query", "test1", false );


.. _escape_string:

EscapeString
~~~~~~~~~~~~

**Prototype:** function EscapeString ( $string )

Escapes characters that are treated as special operators by the query
language parser. Returns an escaped string.

``$string`` is a string to escape.

This function might seem redundant because it's trivial to implement in
any calling application. However, as the set of special characters might
change over time, it makes sense to have an API call that is guaranteed
to escape all such characters at all times.

Usage example:

::


    $escaped = $cl->EscapeString ( "escaping-sample@query/string" );


.. _flush_attributes:

FlushAttributes
~~~~~~~~~~~~~~~

**Prototype:** function FlushAttributes ()

Forces ``searchd`` to flush pending attribute updates to disk, and
blocks until completion. Returns a non-negative internal “flush tag” on
success. Returns -1 and sets an error message on error.

Attribute values updated using
:ref:`UpdateAttributes() <update_attributes>`
API call are only kept in RAM until a so-called flush (which writes the
current, possibly updated attribute values back to disk).
FlushAttributes() call lets you enforce a flush. The call will block
until ``searchd`` finishes writing the data to disk, which might take
seconds or even minutes depending on the total data size (.spa file
size). All the currently updated indexes will be flushed.

Flush tag should be treated as an ever growing magic number that does
not mean anything. It's guaranteed to be non-negative. It is guaranteed
to grow over time, though not necessarily in a sequential fashion; for
instance, two calls that return 10 and then 1000 respectively are a
valid situation. If two calls to FlushAttrs() return the same tag, it
means that there were no actual attribute updates in between them, and
therefore current flushed state remained the same (for all indexes).

Usage example:

::


    $status = $cl->FlushAttributes ();
    if ( $status<0 )
        print "ERROR: " . $cl->GetLastError();


.. _Status:

Status
~~~~~~

**Prototype:** function Status ()

Queries searchd status, and returns an array of status variable name and
value pairs.

Usage example:

::


    $status = $cl->Status ();
    foreach ( $status as $row )
        print join ( ": ", $row ) . "\n";


.. _update_attributes:

UpdateAttributes
~~~~~~~~~~~~~~~~

**Prototype:** function UpdateAttributes ( $index, $attrs, $values,
$mva=false, $ignorenonexistent=false )

Instantly updates given attribute values in given documents. Returns
number of actually updated documents (0 or more) on success, or -1 on
failure.

``$index`` is a name of the index (or indexes) to be updated. ``$attrs``
is a plain array with string attribute names, listing attributes that
are updated. ``$values`` is a hash where key is document ID, and value
is a plain array of new attribute values. Optional boolean parameter
``mva`` points that there is update of MVA attributes. In this case the
values must be a dict with int key (document ID) and list of lists of int values (new MVA attribute values). Optional boolean parameter ``$ignorenonexistent``
points that the update will silently ignore any warnings about trying to
update a column which is not exists in current index schema.

``$index`` can be either a single index name or a list, like in
``Query()``. Unlike ``Query()``, wildcard is not allowed and all the
indexes to update must be specified explicitly. The list of indexes can
include distributed index names. Updates on distributed indexes will be
pushed to all agents.

The updates only work with ``docinfo=extern`` storage strategy. They are
very fast because they're working fully in RAM, but they can also be
made persistent: updates are saved on disk on clean ``searchd`` shutdown
initiated by SIGTERM signal. With additional restrictions, updates are
also possible on MVA attributes; refer to
:ref:`mva_updates_pool <mva_updates_pool>`
directive for details.

Usage example:

::


    $cl->UpdateAttributes ( "test1", array("group_id"), array(1=>array(456)) );
    $cl->UpdateAttributes ( "products", array ( "price", "amount_in_stock" ),
        array ( 1001=>array(123,5), 1002=>array(37,11), 1003=>(25,129) ) );

The first sample statement will update document 1 in index “test1”,
setting “group_id” to 456. The second one will update documents 1001,
1002 and 1003 in index “products”. For document 1001, the new price will
be set to 123 and the new amount in stock to 5; for document 1002, the
new price will be 37 and the new amount will be 11; etc.
