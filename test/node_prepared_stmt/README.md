Node prepared statement test (mysql2)

Usage:
  npm install
  node index.js

Notes:
- mysql2 binds JS numbers as DOUBLE in COM_STMT_EXECUTE, so numeric params are sent as FLOAT/DOUBLE.
  This means Manticore's "negative id" rejection won't trigger unless you force string/BigInt.
- If you want integer semantics, pass BigInt values (e.g., 1n) for id fields.
