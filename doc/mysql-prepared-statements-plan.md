# MySQL prepared statements support plan

## Current state
- `src/netreceive_ql.cpp` only handles COM_QUERY, COM_INIT_DB, COM_PING, COM_SET_OPTION, and COM_FIELD_LIST. COM_STMT_* are unhandled and returned as unknown command.
- `RowBuffer_i` and `SqlRowBuffer_c` emit text protocol rows only; prepared statements require binary rows.
- The SQL parser does not accept `?` placeholders, so prepared statements need placeholder scanning/substitution or grammar changes.
- There is no per-session storage for statement ids or parameter metadata in `ClientSession_c`.

## Plan
1. Statement storage and lifecycle
   - Extend `ClientSession_c` in `src/client_session.h` and `src/searchd.cpp` with a prepared statement registry keyed by statement id.
   - Store template SQL, parameter count, last bound types, and per-parameter long data buffers. Add helpers to allocate ids, look up, reset, and free on session close.

2. Protocol handling for COM_STMT_*
   - Add command cases in `LoopClientMySQL` in `src/netreceive_ql.cpp` for COM_STMT_PREPARE, COM_STMT_EXECUTE, COM_STMT_SEND_LONG_DATA, COM_STMT_CLOSE, COM_STMT_RESET, and COM_STMT_FETCH.
   - Implement packet parsing for COM_STMT_EXECUTE: statement id, flags, iteration count, null bitmap, optional types, and values. Persist type info when new-params-bound is set.
   - For unsupported features like cursors (COM_STMT_FETCH), return a clear error.

3. Placeholder scanning and SQL rendering
   - Implement a lightweight scanner to count and replace `?` placeholders, skipping quoted strings, backticks, and comments. Place it in `src/netreceive_ql.cpp` or a new helper file.
   - Build SQL literals from binary parameter values: NULL, numeric, float, string/blob (quoted and escaped), and date/time types. Reuse `EscapedStringBuilder` from `src/std/escaped_builder.h` to avoid injection issues.
   - For long data, accumulate via COM_STMT_SEND_LONG_DATA and use that buffer as the parameter value when executing.

4. COM_STMT_PREPARE response metadata
   - Generate a Prepare OK packet (statement id, param count, column count, warnings).
   - For parameter definitions, return `MYSQL_COL_STRING` by default or infer type from SQL context if a parser-based approach is added.
   - For result column metadata, add a schema-only query path that parses the SQL and constructs a result schema without executing the full query. Use it to send column definitions for SELECT; return 0 columns for non-result statements.

5. Binary result set output
   - Add a binary row buffer (`BinarySqlRowBuffer_c`) that implements `RowBuffer_i` but emits binary protocol rows (0x00 + null bitmap + typed values).
   - Extend `CreateSqlRowBuffer` in `src/netreceive_ql.cpp` or add a second factory to choose text vs binary for COM_STMT_EXECUTE.
   - Audit `SendMysqlMatch` in `src/searchd.cpp` to use `PutNULL` for actual NULLs so binary rows can set null bits correctly. Keep text behavior stable (possibly via a row-buffer flag).

6. Execution flow
   - COM_STMT_EXECUTE: expand the template SQL with rendered parameters, then call `session::Execute` with the expanded string using the binary row buffer for result sets.
   - COM_STMT_RESET: clear bound types/values and long data for the statement, return OK.
   - COM_STMT_CLOSE: delete the statement from the registry with no response.

7. Testing
   - Add integration tests under `test/` that use a MySQL client library to prepare/execute statements with:
     - Numeric, string, NULL, and blob parameters.
     - `MATCH(?)`, `WHERE attr IN (?, ?)` (multi-params), and INSERT/REPLACE.
     - Long data via COM_STMT_SEND_LONG_DATA.
   - Validate column metadata returned on prepare and binary row decoding on execute.
