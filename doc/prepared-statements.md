Prepared Statements Over MySQL Protocol (Daemon)

Overview
This documents how prepared statements are handled in the Manticore daemon over the MySQL protocol, what was changed, and how the implementation is tested.

How It Works
- COM_STMT_PREPARE:
  - The daemon counts parameter markers in the SQL template and stores a per-session prepared statement record.
  - It builds column metadata for SELECT statements by parsing a "dummy" query with substituted placeholders to infer output column types.
  - It replies with a standard PREPARE OK packet followed by parameter definitions and (for SELECT) result column definitions.
  - When CLIENT_DEPRECATE_EOF is set, it omits EOF terminators after the param/column definitions unless forced to send raw EOF (libmysql/mysql CLI).

- COM_STMT_EXECUTE:
  - The daemon parses the binary parameter payload, converts each value into a SQL literal, and then renders the final SQL query by replacing placeholders.
  - Parameters are substituted in a way that respects quoting/escaping rules and keeps raw list parameters for MVA/float_vector in a strict "(...)" format.
  - The expanded SQL is executed via the existing query pipeline.
  - Result sets are returned with binary row encoding and per-column types when possible.

- Vector/MVA binding:
  - Implicit binding: For INSERT/REPLACE/UPDATE, the daemon detects which parameters map to MVA/float_vector attributes and treats them as raw list parameters.
  - Explicit binding: TO_VECTOR(?) and TO_MULTI(?) are recognized in prepared statements to force raw list binding regardless of schema inference.
  - Only "(1,2)" style lists are accepted for vector/MVA parameters; "1,2" and "[1,2]" are rejected.

What Changed (by file/function)
All changes are in `src/netreceive_ql.cpp` unless noted.

1) MySQL protocol capability / EOF handling
- HandshakeV10_c: does not advertise CLIENT_DEPRECATE_EOF to keep libmysql/mysql CLI compatibility.
- session::SetDeprecatedEOF is set from client capabilities; EOF packets are sent as OK-as-EOF unless forced otherwise.
- libmysql/mysql CLI compatibility: when client attributes indicate `_client_name=libmysql` or `program_name=mysql`, the daemon forces raw EOF packets after resultset metadata (including COM_STMT_PREPARE) even if the client advertises DEPRECATE_EOF.
- Socket receive: SyncSockRead now tolerates EAGAIN/EWOULDBLOCK after select to avoid spurious disconnects.

2) Prepared statement parsing and rendering
- CountPreparedParams: counts parameter markers in SQL template, skipping quoted literals and comments.
- RenderPreparedQuery: renders the final SQL by replacing parameter markers with serialized literals (with optional "raw" parameters).
- ParseStmtParamValues:
  - Parses binary-encoded parameter values from COM_STMT_EXECUTE.
  - Handles numeric, string, date/time, JSON, blob types, NULL, and long-data accumulation (COM_STMT_SEND_LONG_DATA).

3) Column metadata inference for prepared SELECT
- BuildPreparedColumnDefs:
  - Uses a parsed statement to derive column names and types for PREPARE response.
  - Uses schema + expression introspection to infer MySQL column type codes (eg. numeric, float, string).

4) Raw list handling for MVA/float_vector
- NormalizeVectorFunctions:
  - Recognizes TO_VECTOR(?)/TO_MULTI(?) and normalizes them to "?" placeholders while tracking which parameters should be treated as raw list params.
- ApplyImplicitInsertTypes / ExtractUpdateParamColumns:
  - Map parameters to schema attributes in INSERT/REPLACE/UPDATE and mark MVA/float_vector parameters as raw lists.
- RenderPreparedQuery now accepts a per-parameter "raw" bitmap and inserts raw list tokens verbatim (no quoting).

5) Binary result set output
- BinarySqlRowBuffer_c:
  - Emits binary rows for prepared statements.
  - Uses typed column metadata when available (prepared SELECT) and allows non-string binary encoding for numeric types.

Specifics and Behavior Notes
- Parameter binding:
  - Numeric values are serialized as numeric SQL literals.
  - Strings are SQL-escaped and quoted.
  - JSON and blob are transmitted as strings and stored as string literals.
  - DATETIME/TIMESTAMP/DATE/TIME are rendered as SQL literals via formatted strings.
  - NULL is rendered as SQL NULL.
- Vector/MVA:
  - Accepted input: "(1,2,3)" only.
  - Rejected input: "1,2", "[1,2]", or any list with invalid tokens (eg. "(1, bad)").
- Prepared metadata termination:
  - For libmysql/mysql CLI clients, EOF packets are sent as raw EOF (0xFE) after metadata, even if the client advertises DEPRECATE_EOF.
  - For other clients, EOF is omitted when CLIENT_DEPRECATE_EOF is set; otherwise EOF packets are sent.

Testing
All tests were run against a local daemon instance using the MySQL protocol.

1) PHP (mysqli)
- Script: `test/mysql_prepared_stmt.php`
- Covers:
  - Prepared INSERT/SELECT
  - Edge values (0, empty string, negative id rejection, big integer)
  - Long data (send_long_data)
  - All supported types (int, bool, timestamp, float, string, json, multi, multi64, float_vector)
  - Update with TO_MULTI/TO_VECTOR
  - Bad list formats rejected
  - Transaction rollback behavior
  - Load and concurrency

2) Rust (sqlx)
- Script: `test/sqlx_prepared_stmt` (`cargo run --quiet`)
- Covers same cases as PHP plus:
  - SQLX client behavior with COM_STMT_PREPARE and DEPRECATE_EOF

3) Go (database/sql + go-sql-driver/mysql)
- Script: `test/go_prepared_stmt` (`go run .`)
- Covers same cases as PHP.

4) Node.js (mysql2)
- Script: `test/node_prepared_stmt` (`npm install`, `node index.js`)
- Covers same cases as PHP.
- Note: mysql2 binds JS numbers as DOUBLE in COM_STMT_EXECUTE, so negative-id rejection won’t trigger unless values are passed as BigInt or strings.

5) mysql client (libmysql)
- Command: `mysql --protocol=tcp -h127.0.0.1 -P9306 -e "show tables"`
- Verifies EOF/metadata handling with the stock CLI.
