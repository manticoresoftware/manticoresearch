# Spec v1.4.0

## Changes
- Simplified the binary protocol authentication. We might implement the method described in v1.3.0 later, but for now, this should be enough.  

## Table of Contents

1. Overview
2. Requirements
   - Protocol Support
   - Usability
   - Security
   - Plain Mode Support
3. Auth File/Table Formats
   - Plain Mode (File-Based)
   - RT Mode (Table-Based)
4. Authentication via MySQL Protocol
5. Authentication via HTTP Protocol
6. Authentication via Binary Protocol
7. Permissions Model
   - Granular Permissions
   - Rule Resolution Strategy
   - Rule Evaluation Algorithm
8. Management Tool (`/usr/bin/manticore`)
9. Configuration
10. SQL Commands for Authentication and Authorization (RT Mode)
11. DUMP AUTH SQL Command
12. Authentication and Authorization in Replication Clusters
13. Manticore Buddy Authentication and Authorization
14. Logging
15. Clients


### Overview

This document provides the specifications for implementing authentication and authorization functionality in Manticore Search. The system includes support for managing users and their credentials, as well as finely-grained permissions defined at the level of individual actions and targets. It ensures compatibility with MySQL and HTTP clients while offering basic authentication in the binary protocol.

### Requirements

#### Protocol Support

1. **MySQL Protocol**
   - Must support `mysql_native_password` authentication mechanism only.
   - Compatible with newer versions of MySQL and MariaDB clients. For some clients (e.g., MySQL client 9), you may need to enable `mysql_native_password` explicitly.

2. **HTTP Protocol**
   - Authentication via two methods:
     - Basic Auth: Username and password encoded in the request header (e.g., `Authorization: Basic <base64(user:password)>`).
     - Bearer Token: Header like `Authorization: Bearer <token>`.

3. **Binary Protocol**
    - Supports basic bearer-based authentication.
    - Tokens are sent in plain text, meaning they could be intercepted if the network is not secure.
    - Unlike MySQL and HTTP protocols, SSL encryption is not available for the binary protocol.
    - If security is a concern, using MySQL or HTTP protocols with SSL is recommended. However, if the network is controlled and secure, the binary protocol may still be a viable option.

#### Usability

- Easy user management through a CLI tool (`/usr/bin/manticore`).
- File-based authentication storage for plain mode (no replication). When replication is used, authentication/authorization data must be stored in dedicated system tables.
- Permissions-based access control defined by records indicating which user can perform which action on which target, optionally with usage budgets.

#### Security

- Passwords must be securely hashed using methods required for supported protocols:
  - `password_sha1` for `mysql_native_password`
  - `password_sha256` for Basic HTTP authentication
  - `bearer_sha256` for HTTP Bearer Token
- The integrity of the authentication data (file or tables) must be validated at every operation. If manually edited and corrupted, Manticore should fail to process it to prevent security or operational issues.
- File permissions must restrict access to authorized users only.

#### Plain Mode Support

- Full functionality without requiring a `data_dir`. The configuration must explicitly specify the `auth` setting.

---

### Auth File/Table Formats

Authentication data is split into two parts:

1. **Users**: Credentials (username, password hash, and a unique salt).
2. **Permissions**: One record per permission, indicating which user can perform which action, on which target, and with what optional budget.

#### Plain Mode (File-Based)

Structure:
```json
{
  "users": [
    {
      "username": "admin",
      "salt": "<unique_salt>",
      "hashes": {
        "password_sha1_no_salt": "<sha1(password)>",
        "password_sha256": "<sha256(salt + password)>",
        "bearer_sha256": "<sha256(salt + sha256(token))>"
      }
    },
    {
      "username": "readonly",
      "salt": "<unique_salt>",
      "hashes": {
        "password_sha1_no_salt": "<sha1(password)>",
        "password_sha256": "<sha256(salt + password)>",
        "bearer_sha256": "<sha256(salt + sha256(token))>"
      }
    },
    {
      "username": "custom_user",
      "salt": "<unique_salt>",
      "hashes": {
        "password_sha1_no_salt": "<sha1(password)>",
        "password_sha256": "<sha256(salt + password)>",
        "bearer_sha256": "<sha256(salt + sha256(token))>"
      }
    }
  ],
  "permissions": [
    {
      "username": "admin",
      "action": "read",
      "target": "*",
      "allow": true,
      "budget": { "queries_per_minute": 1000 }
    },
    {
      "username": "readonly",
      "action": "read",
      "target": "*",
      "allow": true,
      "budget": { "queries_per_day": 10000 }
    },
    {
      "username": "custom_user",
      "action": "read",
      "target": "table/mytable",
      "allow": true,
      "budget": { "queries_per_minute": 500 }
    }
  ]
}
```

#### RT Mode (Table-Based)

In RT Mode, authentication data should be stored in system tables with salts and hashes for enhanced security.

**Users Table** ("system.auth_users"):
- Stores username, salt, and password hashes.

Example schema:
```sql
CREATE TABLE system.auth_users (
  username string,
  salt string,
  hashes json
);
```

**Permissions Table** ("system.auth_permissions"):
- Each record defines one permission rule.

Example schema:
```sql
CREATE TABLE system.auth_permissions (
  username string,
  action string,
  target string,
  allow bool,
  budget json
);
```

Example "system.auth_users" content:
```
| username    | salt          | hashes                                                                                                                                            |
|-------------|---------------|---------------------------------------------------------------------------------------------------------------------------------------------------|
| admin       | <unique_salt> | {"password_sha1_no_salt": "<sha1(password)>", "password_sha256": "<sha256(salt + password)>", "bearer_sha256": "<sha256(salt + sha256(token))>"}  |
| readonly    | <unique_salt> | {"password_sha1_no_salt": "<sha1(password)>", "password_sha256": "<sha256(salt + password)>", "bearer_sha256": "<sha256(salt + sha256(token))>"}  |
| custom_user | <unique_salt> | {"password_sha1_no_salt": "<sha1(password)>", "password_sha256": "<sha256(salt + password)>", "bearer_sha256": "<sha256(salt + sha256(token))>"}  |
```

Example "system.auth_permissions" content:
```
| username    | action | target             | allow | budget                                  |
|-------------|--------|--------------------|-------|-----------------------------------------|
| admin       | read   | *                  | true  | {"queries_per_minute":1000}             |
| admin       | write  | *                  | true  | null                                    |
| admin       | schema | *                  | true  | null                                    |
| readonly    | read   | *                  | true  | {"queries_per_day":10000}               |
| readonly    | write  | *                  | false | null                                    |
| readonly    | schema | *                  | false | null                                    |
| custom_user | read   | table/mytable      | true  | {"queries_per_minute":500}              |
| custom_user | write  | table/mytable      | true  | null                                    |
| custom_user | write  | table/anothertable | false | null                                    |
```

---

### Authentication via MySQL Protocol

Follows the requirements of the mysql/mariadb clients, e.g. https://dev.mysql.com/doc/refman/8.4/en/native-pluggable-authentication.html

- Must use the `password_sha1_no_salt` hash in the auth file/table.
- Must support `mysql_native_password` authentication mechanism only.
- Compatible with newer versions of MySQL and MariaDB clients. For some clients (e.g., MySQL client 9), you may need to enable `mysql_native_password` explicitly.

Here's the diagram of how it works:
```mermaid
sequenceDiagram
    participant Client
    participant Server

    Server->>Client: Generate random nonce (public_seed) and send to Client
    Client->>Client: Compute hash_stage1 = SHA1(password)
    Client->>Client: Compute hash_stage2 = SHA1(hash_stage1)
    Client->>Client: Compute scramble = XOR(hash_stage1, SHA1(public_seed + hash_stage2))
    Client->>Server: Send scramble

    Server->>Server: Retrieve stored hash_stage2 (SHA1(SHA1(password)))
    Server->>Server: Compute SHA1(public_seed + stored hash_stage2)
    Server->>Server: Compute hash_stage1 = XOR(scramble, SHA1(public_seed + stored hash_stage2))
    Server->>Server: Compute candidate_hash2 = SHA1(hash_stage1)
    Server->>Server: Compare candidate_hash2 with stored hash_stage2

    alt Authentication Successful
        Server->>Client: Authentication successful
    else Authentication Failed
        Server->>Client: Authentication failed
    end
```

### Authentication via HTTP Protocol

Authentication supports the following mechanisms:

1. **Basic Auth**:
   The client provides a username and password in the `Authorization` header, encoded in Base64.
   The server validates the password by comparing its SHA256 (with salt) hash against `password_sha256` in the `hashes` JSON object.

   Example:

   - **Request**:
     ```
     GET /search HTTP/1.1
     Host: example.com
     Authorization: Basic YWRtaW46cGFzc3dvcmQ=
     ```

     In this example:
     - `Authorization: Basic YWRtaW46cGFzc3dvcmQ=`
       - Decoded as `admin:password`
       - `admin` is the username, and `password` is the plaintext password.
     - The server calculates `sha256(salt + password)` and compares it to the `password_sha256` value stored in the `hashes` column for the `admin` user.

   - **Validation**:
     - Server retrieves the `hashes` JSON for the `admin` user:
       ```json
       {
         "password_sha256": "<sha256(salt + password)>",
         "bearer_sha256": "<sha256(salt + sha256(token))>"
       }
       ```
     - The server calculates `sha256(salt + password)` and ensures it matches the stored `password_sha256`. If they match, the user is authenticated.

2. **Bearer Token**:
   The client provides a token in the `Authorization` header using the Bearer scheme.
   The server validates the token by hashing it and comparing it to the stored `bearer_sha256`.

   - **Token Generation**:
     - The client uses the token received previously on the server using the `manticore` tool or the `TOKEN` SQL statement:
     - The token is sent to the server as-is in the `Authorization` header.

   - **Storage**:
     - The server stores the hash of the token, not the token itself:
       ```
       bearer_sha256 = sha256(salt + token)
       ```
     - This ensures that even if the database is compromised, the actual tokens cannot be directly extracted.
   - **Request**:
     ```
     GET /search HTTP/1.1
     Host: example.com
     Authorization: Bearer a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2
     ```

     In this example:
     - `Authorization: Bearer a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2`
     - The token (`a1b2c3...`) is the user's token, obtained earlier using the `manticore` tool or the `TOKEN` SQL command.

   - **Validation**:
     - The server looks for a user by matching the provided token with `bearer_sha256`. If a match is found, the request is considered authorized.

### Authentication via Binary Protocol

Authentication via the binary protocol uses a basic bearer-based authentication method.

#### **Security Considerations**
- The token is sent in plain text, meaning it can be intercepted if transmitted over an untrusted network.
- The binary protocol does not support SSL encryption.
- If the environment is secure and controlled (e.g., internal network, VPN), using the binary protocol may be acceptable. Otherwise, MySQL or HTTP protocols with SSL are recommended.
- For master/agent communication between Manticore instances, it is assumed that users ensure the channel is secure, as the protocol itself does not provide encryption.

#### **Implementation Details**
- The token is included in the request headers.
- The server verifies the token against the stored `bearer_sha256` hash.
- If the token matches, access is granted.

#### **Process Overview**

1. **Client Authentication Request**:
  - The client sends the authentication request containing the bearer token.
  - The token is included in the request in plain text.

2. **Server Validation**:
  - The server retrieves the user's stored `bearer_sha256` value.
  - The server hashes the received token and compares it to the stored hash.
  - If the hashes match, authentication succeeds; otherwise, access is denied.

3. **Session Handling**:
  - Each authenticated session lasts as long as the connection remains open.
  - The client must re-authenticate if the connection is closed and reopened.

#### **Process Flow Diagram**
```mermaid
sequenceDiagram
    participant Client as Client (Manticore instance or user's app)
    participant Server as Server (Manticore instance)

    Client->>Server: Send authentication request with bearer token (unencrypted)
    Server->>Server: Retrieve stored `bearer_sha256` hash
    Server->>Server: Compute hash of received token
    alt Hash matches
        Server->>Client: Authentication successful
    else Hash does not match
        Server->>Client: Authentication failed
    end
```

#### Usage Recommendations
- If security is a concern, MySQL or HTTP protocols with SSL are recommended instead of the binary protocol.
- If operating in a controlled environment with trusted network conditions, the binary protocol may still be a viable option.
- For master/agent communication between Manticore instances, users are responsible for securing the communication channel (e.g., using a VPN or private network).

---

### Permissions Model

Permissions are defined as individual records. Each record specifies:

- `username`: The user to whom the permission applies.
- `action`: One of the defined actions (`read`, `write`, `schema`, `admin`, `replication`).
- `target`: The object the action applies to (e.g., a table name). `*` can be used as a wildcard to apply to all targets.
- `allow`: A boolean indicating if the action is permitted (`true`) or denied (`false`).
- `budget`: An optional JSON object specifying resource usage limits (e.g., `queries_per_minute`, `queries_per_day`, `cpu_time_per_hour`, etc.).

If multiple records apply to the same user/action/target, a record with `allow = false` will deny that action. If `allow = true`, the action is permitted as long as the budget is not exceeded.

#### Granular Permissions

The actions `read`, `write`, `schema`, `replication` and `admin` map to specific SQL commands, HTTP endpoints and internal commands. Any command or endpoint not listed defaults to being denied unless explicitly granted.

- **read**:
  - SQL commands that do not modify data or schema:
    - DESCRIBE
    - SHOW TABLES
    - SHOW CREATE TABLE
    - SHOW TABLE STATUS
    - SHOW TABLE SETTINGS
    - SHOW META
    - SHOW PROFILE
    - SHOW PLAN
    - SHOW WARNINGS
    - SELECT
    - EXPLAIN QUERY
    - CALL SUGGEST
    - CALL QSUGGEST
    - CALL SNIPPETS
    - CALL PQ
    - CALL KEYWORDS
    - SHOW MY PERMISSIONS
    - SHOW MY USAGE

  - HTTP endpoints that retrieve data without modification:
    - /search
    - /pq/{table}/search
    - /sql endpoint with SELECT (read-only) queries

- **write**:
  - SQL commands that modify data or session-level state:
    - INSERT
    - REPLACE
    - REPLACE ... SET
    - UPDATE
    - DELETE
    - TRUNCATE TABLE
    - KILL
    - SET (session-level or SET NAMES, SET @@var)
    - FLUSH ATTRIBUTES
    - FLUSH HOSTNAMES
    - FLUSH LOGS
    - FLUSH RAMCHUNK
    - FLUSH TABLE
    - OPTIMIZE TABLE
    - ATTACH TABLE
    - BEGIN
    - COMMIT
    - ROLLBACK

  - HTTP endpoints that modify data:
    - /bulk
    - /_bulk
    - /delete
    - /insert
    - /replace
    - /update
    - /{table}/_update/{id}
    - /sql, /cli, /cli_json endpoints with INSERT, UPDATE, DELETE, KILL, or session-level SET queries

- **schema**:
  - SQL commands that modify schema, global state, or provide insights into the system:
    - CREATE TABLE
    - ALTER TABLE
    - DROP TABLE
    - IMPORT TABLE
    - JOIN CLUSTER
    - ALTER CLUSTER
    - SET CLUSTER
    - DELETE CLUSTER
    - CREATE FUNCTION
    - DROP FUNCTION
    - CREATE PLUGIN
    - CREATE BUDDY PLUGIN
    - DROP PLUGIN
    - DELETE BUDDY PLUGIN
    - RELOAD TABLE
    - RELOAD TABLES
    - RELOAD PLUGINS
    - ENABLE BUDDY PLUGIN
    - DISABLE BUDDY PLUGIN
    - BACKUP
    - SHOW STATUS
    - SHOW QUERIES
    - SHOW THREADS
    - SHOW VARIABLES
    - SHOW PLUGINS
    - SHOW BUDDY PLUGINS

  - Global variable changes considered schema:
    - SET GLOBAL
    - SET INDEX <index_name> GLOBAL

  - HTTP endpoints that modify schema:
    - /tbl_name/_mapping
    - /sql, /cli, /cli_json endpoint with any "schema" queries

- **replication**:
  - all replication-related internal commands that replication nodes use to communicate with each other

- **admin**:
  - SQL commands that manage users and permissions:
    - CREATE USER - will be implemented in Buddy
    - DROP USER - will be implemented in Buddy
    - GRANT - will be implemented in Buddy
    - REVOKE - will be implemented in Buddy
    - SHOW USERS - will be implemented in Buddy
    - SHOW PERMISSIONS - will be implemented in Buddy
    - SET PASSWORD - will be implemented in Buddy
    - TOKEN - will be implemented in Buddy
    - SHOW USAGE
    - DUMP AUTH
    - RELOAD AUTH

  - HTTP endpoints that manage authentication and authorization:
    - None explicitly defined at this time; all admin-related SQL commands must be executed through the `/sql`, `/cli`, or `/cli_json` endpoints.

**Notes**

- Commands not explicitly listed under any permission are denied by default.
- Users with the `admin` action are restricted to managing users and permissions. For other actions, such as `read`, `write`, or `schema`, explicit grants are required.
- This list should be reviewed and updated regularly to ensure all possible SQL commands and endpoints are included.
- Write access to the `system.auth*` tables is restricted in the daemon to Manticore Buddy only. Even users with the `admin` privilege should not be able to modify these tables to prevent corruption.

#### Rule Resolution Strategy

1. **Explicit Deny Takes Precedence**:
   - If a user has a rule explicitly denying access to an action/target, that rule overrides all other rules that allow access. This ensures security by default.

   **Example**:
   - Rule 1: User `admin` is allowed to `read` on `*`.
   - Rule 2: User `admin` is denied `read` on `restricted_table`.
   - Result: Access to `restricted_table` is denied for `admin`.

2. **Specificity Takes Precedence Over Wildcards**:
   - Rules targeting a specific object (e.g., `restricted_table`) take precedence over wildcard rules (e.g., `*`).

   **Example**:
   - Rule 1: User `readonly` is allowed to `read` on `*`.
   - Rule 2: User `readonly` is denied `read` on `sensitive_table`.
   - Result: Access to `sensitive_table` is denied for `readonly`, but access to all other tables is allowed.

3. **Most Restrictive Rule Wins in Ties**:
   - When two rules are equally specific, the more restrictive rule (deny) takes precedence. If both rules allow access, the one with the stricter budget (if applicable) applies.

   **Example**:
   - Rule 1: User `custom_user` is allowed `write` on `mytable` with a budget of 500 queries/minute.
   - Rule 2: User `custom_user` is allowed `write` on `mytable` with a budget of 1000 queries/minute.
   - Result: The 500 queries/minute limit applies.

4. **Default Deny**:
   - If no rules match for a given user, action, and target, access is denied by default.

#### Rule Evaluation Algorithm

When a user attempts an action, evaluate permissions in the following order:

1. **Collect All Matching Rules**:
   Identify all rules that apply to the user's action and target, including wildcard rules (e.g., `*`).

2. **Sort Rules by Specificity**:
   - Rules targeting specific objects (e.g., `table/mytable`) come before wildcards (`*`).
   - Explicit denies (`allow = false`) come before allows (`allow = true`).

3. **Apply the First Matching Rule**:
   - Process the sorted list of rules and stop at the first rule that applies. This ensures predictable and efficient resolution.

#### Documentation and Transparency

To help administrators understand the system, we should document the rule resolution strategy clearly. Examples and edge cases, such as the following should be included:

1. Conflicting rules between `allow` and `deny`.
2. Multiple budgets applying to the same user and action.
3. Behavior when no matching rules exist.

---

### Management Tool (`/usr/bin/manticore`)

The `manticore` tool must securely manage users, permissions, and authentication files. It interacts with the running Manticore Search instance to ensure changes to authentication data are dynamically applied without requiring a manual restart. The following changes outline how the tool should handle list operations and real-time (RT) mode scenarios.

**How it works**

```mermaid
graph TD
  A[Start] --> B{Is Manticore Search Instance Running?}
  B -- No --> C[Show Error: ERROR: The Manticore Search instance is not running. Please start the instance and try again.]
  B -- Yes --> D[Plain Mode or RT Mode?]

  D -- Plain Mode --> E[Modify Auth File]
  E --> F[Manticore Buddy Detects Change]
  F --> G[Calls RELOAD AUTH]
  G --> H[Manticore Search Reloads Authentication Data]

  D -- RT Mode --> I{Listing or Modify Operation?}
  I -- Listing --> J[Create Empty <data_dir>/auth.tmp]
  J --> K[Manticore Buddy Populates <data_dir>/auth.tmp with DUMP AUTH and Replaces Hashes with *]
  K --> L[Tool Reads Data for Listing Operation]

  I -- Modify --> M[Tool Reads Existing <data_dir>/auth.tmp]
  M --> N[Modify Temporary File Content]
  N --> O[Write Updated File to <data_dir>/auth.tmp]
  O --> P[Manticore Buddy Detects Update]
  P --> Q[Executes SQL Commands to Update System Tables]
  P --> R{Error Encountered?}
  R -- No --> S[Buddy Removes <data_dir>/auth.tmp]
  R -- Yes --> T[Buddy Writes Error Message in <data_dir>/auth.tmp]
  T --> U[Tool Reads Error and Fails with Buddy's Message]
  U --> U2[Tool Removes <data_dir>/auth.tmp]

  H --> V[End]
  L --> V
  S --> V
  U2 --> V

```

1. **Ensures the Manticore Search Instance is Running**
   - The tool must verify that the corresponding Manticore Search instance is running.
   - If the instance is not running, the tool must fail with the following error message:
     `"ERROR: The Manticore Search instance is not running. Please start the instance and try again."`

2. **Plain Mode**:
   - After modifying the auth file, Manticore Buddy detects the file change (e.g., within one second) and calls `RELOAD AUTH` in the Manticore instance.
   - The `RELOAD AUTH` command triggers Manticore Search to reload the updated authentication and authorization data.

3. **Real-Time (RT) Mode**:
   - **Listing Operations**:
     - The tool creates an empty file `<data_dir>/auth.tmp` in the data directory.
     - Manticore Buddy detects this file and populates it with the current authentication and authorization information (replacing all actual hashes with `*`) by using `DUMP AUTH`.
     - The tool reads the data from `<data_dir>/auth.tmp` to perform listing operations.

   - **Modify Operations**:
     - The tool reads the existing content of `<data_dir>/auth.tmp` populated by Buddy.
     - The tool modifies the content of the temporary file in the same way it would modify the auth file in plain mode.
     - The updated file is written back to `<data_dir>/auth.tmp`.
     - Manticore Buddy detects the updated `<data_dir>/auth.tmp`, prepares SQL commands to update the `system.auth_users` and `system.auth_permissions` tables, and removes the file upon successful execution.
     - If Buddy encounters an error, it writes the error message into the file (not in JSON format). The tool must detect this, read the error, and fail with the message provided by Buddy.

4. **Password Entry**:
  - When adding or updating a user, the tool must prompt for the password interactively, without echoing.
  - Alternatively, the password can be provided via STDIN (e.g., through a pipe).

5. **File Locking**:
   - All file operations (reading and writing) on `<data_dir>/auth.tmp` must be covered by a file-based lock. The lock file should be located at `<data_dir>/auth.lock` to prevent concurrent modifications and ensure safe interactions between the `manticore` tool and Buddy.
   - If the tool cannot acquire the lock, it must log an error:
     `"ERROR: Unable to acquire lock at '<data_dir>/auth.lock'. Another process might be modifying authentication data. Please try again later."`

6. **File Permissions**: The auth files must meet the following requirements:
   - Permissions set to `600`
   - File/group owner must match the user under which the current Manticore instance is running

7. **Conflict Detection**:
   - When adding or modifying permissions via the CLI, the tool must warn users if the new rule conflicts with existing ones.

**Examples**:

- Adding a user (interactively):
  ```bash
  manticore user add admin
  # The tool then prompts twice: "Enter password:" and the user types the password securely.
  ```

- Adding a user (via pipe):
  ```bash
  echo "secret_password" | manticore user add admin
  ```

- Adding permissions to a user (no password required):
  ```bash
  manticore permission add --user admin --action read --target "*" --allow true --budget '{"queries_per_minute":1000}'
  ```

- Conflict:
  ```bash
  manticore permission add --user admin --action read --target table/restricted_table --allow true
  # Output:
  # WARNING: This rule conflicts with an existing deny rule for user 'admin' on 'restricted_table'.
  ```

**Configuration File Flag**:

- The `manticore` tool must respect the `-c` or `--config` flag specifying the path to the Manticore Search configuration file.
- If not specified, the tool looks for the config file in default locations.
- The tool should report which configuration file it used and which `auth` file path it derived from that configuration.

**Examples with Configuration Flags**:

- Add a user with a specific configuration file:
```bash
manticore -c /custom/path/manticore.conf user add admin
```

- Add permissions with a specific configuration file:
```bash
manticore -c /custom/path/manticore.conf permission add --user admin --action write --target "table/mytable" --allow true
```

**Post-Operation Reporting**:

After completing an operation, the tool should:
- Log a success message confirming that the operation was successfully written to the `auth` file or `<data_dir>/auth.tmp`.
- Log whether Buddy successfully triggered the `RELOAD AUTH` or SQL update process.
- Log any warnings or errors that occurred during the process.

#### `manticore` Tool Usage Information

When the `manticore` tool is run without any flags or with `-h`/`--help`, it should display a complete usage guide with examples for managing users, permissions, and the authentication system.

**Example Output**:

```text
Manticore Search Management Tool

Usage:
  manticore [options] <command>

Commands:
  user add <username>             Add a new user. Prompts for a password interactively or accepts it via a pipe.
  user token <username>           Generates or updates a user's token for HTTP authentication.
  user delete <username>          Delete an existing user.
  user password <username>        Update an existing user's password.
  user list                       List all users.

  permission add --user <username> --action <action> --target <target> --allow <true|false> [--budget <budget>]
                                  Add permissions for a user. Specify action (read, write, schema),
                                  target (e.g., "table/<table name>" or "*"), allow (true/false), and an optional budget.
  permission delete --user <username> --id <permission id>
                                  Remove specific permission for a user.
  permission list                 List all permissions.

Options:
  -c, --config <path>             Specify the path to the Manticore Search configuration file.
                                  If not provided, the tool uses default locations (e.g., /etc/manticoresearch/manticore.conf).
  -h, --help                      Show this usage information.

Examples:

  # Add a new user interactively
  manticore user add admin
  # The tool prompts for a password:
  # "Enter password:"

  # Add a new user with a password provided via a pipe
  echo "mypassword" | manticore user add admin

  # Delete a user
  manticore user delete admin

  # List all users
  manticore user list

  # Creates or updates the token of user "admin"
  manticore user token admin

  # Add permissions for a user
  manticore permission add --user admin --action read --target "*" --allow true --budget '{"queries_per_minute":1000}'

  # Remove permissions for a user for a specific table
  manticore permission delete --user admin --action read --target "table/mytable"

  # List all users and their permissions
  manticore permission list
```

---

### SQL Commands for Authentication and Authorization (RT Mode)

1. **Create a User**
   Adds a new user with a password.
   - **Note**: Users with the `ADMIN` action cannot be created via SQL commands; they must be created using the `manticore` tool.
   - SQL Command:
     ```sql
     CREATE USER '<username>' IDENTIFIED BY '<password>';
     ```
   - The password is securely hashed and stored by the system.
   - Example:
     ```sql
     CREATE USER 'readonly' IDENTIFIED BY 'readonlypassword';
     ```

2. **Create or update a token**
   Creates a new token for an existing user or updates the current token. The result is the new token.
   - **SQL Command:**
     ```sql
     TOKEN[ '<username>'];
     ```
   - The token is securely hashed and stored by the system.
   - If the username is not specified, the current user's token will be updated.
   - **Examples:**
     ```sql
     TOKEN 'readonly';
     TOKEN;
     ```

3. **Create or update a password**
   Updates the password for an existing user or the current user.
   - **SQL Command:**
     ```sql
     SET PASSWORD '<password>'[ FOR '<username>'];
     ```
   - The password is securely hashed and stored by the system.
   - If the username is not specified, the current user's password will be updated.
   - **Examples:**
     ```sql
     SET PASSWORD 'abcdef' FOR 'justin';
     SET PASSWORD 'abcdef';
     ```

4. **Delete a User**
   Deletes a user and their associated permissions.
   - SQL Command:
     ```sql
     DROP USER '<username>';
     ```
   - Example:
     ```sql
     DROP USER 'readonly';
     ```

5. **Grant a Permission**
   Adds a permission for a specific user.
   - SQL Command:
     ```sql
     GRANT <action> ON <target> TO '<username>' [WITH BUDGET <json_budget>];
     ```
   - `<action>`: `READ`, `WRITE`, or `SCHEMA`. Users with the `ADMIN` action can manage users and permissions but require explicit grants for other actions.
   - `<target>`: The object to which the permission applies. Use either `*` or `'*'` for all targets, or specify a table name preceded by the `table/` prefix.
   - `<json_budget>`: Optional JSON specifying resource limits.
   - Example:
     ```sql
     GRANT READ ON * TO 'readonly' WITH BUDGET '{"queries_per_day": 10000}';
     GRANT WRITE ON 'mytable' TO 'custom_user';
     ```

6. **Revoke a Permission**
   Removes a specific permission for a user.
   - SQL Command:
     ```sql
     REVOKE <action> ON <target> FROM '<username>';
     ```
   - Example:
     ```sql
     REVOKE READ ON '*' FROM 'readonly';
     ```

7. **List All Users**
   Displays all users (admin access required).
   - SQL Command:
     ```sql
     SHOW USERS;
     ```
   - Example Output:
     ```
     +-------------+
     | username    |
     +-------------+
     | admin       |
     | readonly    |
     | custom_user |
     +-------------+
     ```

8. **Show Permissions for Current User**
   Lists permissions granted to the current user.
   - SQL Command:
     ```sql
     SHOW MY PERMISSIONS;
     ```
   - Example Output for `custom_user`:
     ```
     +-------------+--------+---------------+-------+--------------------------+
     | username    | action | target        | allow | budget                   |
     +-------------+--------+---------------+-------+--------------------------+
     | custom_user | read   | table/mytable | true  | {"queries_per_day": 500} |
     | custom_user | write  | table/mytable | true  | null                     |
     +-------------+--------+---------------+-------+--------------------------+
     ```

9. **Show All Permissions**
   Lists all permissions.
   - **Behavior**:
     - **Admin**: Lists permissions for all users.
     - **Non-admin**: Behaves like `SHOW MY PERMISSIONS`.
   - SQL Command:
     ```sql
     SHOW PERMISSIONS;
     ```
   - Example Output for Admin:
     ```
     +-------------+--------+---------------+-------+----------------------------+
     | username    | action | target        | allow | budget                     |
     +-------------+--------+---------------+-------+----------------------------+
     | admin       | admin  | *             | true  | null                       |
     | readonly    | read   | *             | true  | {"queries_per_day": 10000} |
     | custom_user | write  | table/mytable | true  | null                       |
     +-------------+--------+---------------+-------+----------------------------+
     ```
   - Example Output for Non-admin (`custom_user`):
     ```
     +-------------+--------+---------------+-------+--------------------------+
     | username    | action | target        | allow | budget                   |
     +-------------+--------+---------------+-------+--------------------------+
     | custom_user | read   | table/mytable | true  | {"queries_per_day": 500} |
     | custom_user | write  | table/mytable | true  | null                     |
     +-------------+--------+---------------+-------+--------------------------+
     ```

10. **Show Usage Information**
   - **Admin**: Displays usage statistics for all users.
   - **Non-admin**: Shows only the current user’s usage.
   - SQL Command:
     ```sql
     SHOW USAGE;
     ```
   - Example Output for Admin:
     ```
     +-------------+-----------------+-----------------+--------------------+
     | username    | queries_per_min | queries_per_day | last_login         |
     +-------------+-----------------+-----------------+--------------------+
     | admin       | 500             | 10000           | 2024-12-15 14:32   |
     | readonly    | 200             | 5000            | 2024-12-15 12:45   |
     | custom_user | 50              | 300             | 2024-12-15 15:20   |
     +-------------+-----------------+-----------------+--------------------+
     ```
   - Example Output for Non-admin (`custom_user`):
     ```
     +-------------+-----------------+-----------------+--------------------+
     | username    | queries_per_min | queries_per_day | last_login         |
     +-------------+-----------------+-----------------+--------------------+
     | custom_user | 50              | 300             | 2024-12-15 15:20   |
     +-------------+-----------------+-----------------+--------------------+
     ```

11. **Show Accessible Tables**
   Lists tables the current user can access based on their permissions.
   - SQL Command:
     ```sql
     SHOW TABLES;
     ```
   - Example Output for `admin`:
     ```
     +-------------+
     | table_name  |
     +-------------+
     | 'mytable'   |
     | 'anothertable' |
     | 'thirdtable' |
     +-------------+
     ```
   - Example Output for `custom_user`:
     ```
     +-------------+
     | table_name  |
     +-------------+
     | 'mytable'   |
     +-------------+
     ```

**Notes on `ADMIN` Action**

- Users with the `ADMIN` action can only manage users and permissions.
- To perform other actions (`READ`, `WRITE`, `SCHEMA`), explicit grants are required.
- Both `*` and `'*'` are valid in commands for targets, ensuring flexibility.
- The `ADMIN` action provides unrestricted access to the `SHOW USERS`, `SHOW PERMISSIONS`, and `SHOW USAGE` commands.

---

### Configuration

- RT mode:
  ```ini
  auth = 1
  ```
- Plain mode:
  ```ini
  auth = 1
  ```
  (in this case `auth.json` is put to the same directory where we put a binlog by default)
  or
  ```ini
  auth = /path/to/file
  ```

If the `system.auth*` tables (RT Mode) or the authentication file (Plain Mode) do not exist, Manticore should create them. If they already exist, Manticore should validate them during startup. If validation fails, Manticore should not start.

---

### DUMP AUTH SQL command

`DUMP AUTH` is an SQL statement designed to facilitate migration from RT mode (tables) to plain mode (JSON). It extracts the contents of the `system.auth_users` and `system.auth_permissions` tables into a JSON format compatible with plain mode.

- The command outputs the data directly as a JSON object, allowing administrators to redirect the output to a file for use as the plain mode `auth` file.

---

### Authentication and Authorization in Replication Clusters

- Joining a cluster overrides local `system.auth_users` and `system.auth_permissions` with the cluster’s version.
- Dumps the original local data to `searchd` log as a backup.
- Ensures all nodes share consistent authentication and authorization data.
- Replication involves several internal commands, which, while not directly accessible to users, must still be protected by authentication and authorization. To address this, the `CREATE CLUSTER`, `JOIN CLUSTER`, and `ALTER CLUSTER` commands have been updated. These commands now leverage a special permission action called **replication**, which allows a user to perform all internal replication-related operations.

If a cluster is associated with a specific user, that user's credentials will be used for all internal replication commands executed on behalf of the cluster.

#### **CREATE CLUSTER**

The `CREATE CLUSTER` command now allows specifying a user who will own and manage the cluster. The specified user must have the **replication** action, as it is required for performing internal replication operations.

**New Syntax:**
```sql
CREATE CLUSTER <cluster_name> [AS USER '<username>']
```

**Alternative Syntax:**
```sql
CREATE CLUSTER <cluster_name> ['<username>' AS USER]
```

**Validation:**
- If the specified user does not have the **replication** action, the command will fail with an error.
- It may also fail later when performing one of the internal replication commands. In such cases, the error should be logged in the searchd log.

**Error Message:**
```
User '<username>' is not permitted to do the "replication" action with the target.
```

#### **JOIN CLUSTER**

The `JOIN CLUSTER` command allows a node to join an existing cluster and optionally specifies the user whose credentials will authenticate the operation. This user must also have the **replication** action.

**New Syntax:**
```sql
JOIN CLUSTER <cluster_name> AT '<ip:port>' [AS USER '<username>']
```

**Alternative Syntax:**
```sql
JOIN CLUSTER <cluster_name> AT '<ip:port>' ['<username>' AS USER]
```

**Validation:**
- Same as for the `CREATE CLUSTER` command.
- Later on when performing one of the internal replication commands if the specified user lacks the **replication** action, the operation will fail, and the error should be logged.

---

#### **ALTER CLUSTER**

The `ALTER CLUSTER` command introduces the ability to reassign a cluster to a different user. The new user must have the **replication** action to manage the cluster’s internal operations.

**New Syntax:**
```sql
ALTER CLUSTER <cluster_name> USER='<username>'
```

**Validation:**
- If the new user lacks the **replication** action, the reassignment will fail.
- Later on when performing one of the internal replication commands if the specified user lacks the **replication** action, the operation will fail, and the error should be logged.

**Error Message:**
```
User '<username>' is not permitted to do the "replication" action with the target.
```

---

### Manticore Buddy Authentication and Authorization

Manticore Buddy and the Manticore Search daemon communicate via a secure HTTPS channel. Authentication is handled using a standard bearer token model, where Buddy acts as a highly privileged user. The daemon uses a unified process to authenticate all requests it receives.

```mermaid
sequenceDiagram
    participant ManticoreTool as manticore tool
    participant ManticoreSearch as Manticore Search
    participant Buddy as Manticore Buddy
    participant Client as User's Client

    Note over ManticoreSearch: On start, generates and holds BUDDY_TOKEN.
    ManticoreSearch->>Buddy: Starts Buddy, passing BUDDY_TOKEN via env var.

    alt Administrative Path (e.g., manticore user add)
        ManticoreTool->>Buddy: Send command
        Buddy->>ManticoreSearch: Send request with `Authorization: Bearer BUDDY_TOKEN`
        ManticoreSearch->>ManticoreSearch: Authenticate BUDDY_TOKEN (grants admin rights)
        ManticoreSearch->>Buddy: Return result
        Buddy->>ManticoreTool: Return result
    end

    alt User Request Path (e.g., /search)
        Client->>ManticoreSearch: Send request with `Authorization: Bearer <user_token>`
        Note over ManticoreSearch: Forwards request to a Buddy plugin
        ManticoreSearch->>Buddy: Send request with original `Authorization: Bearer <user_token>`
        Note over Buddy: Buddy processes the request...
        Buddy->>ManticoreSearch: Forwards request back with original `Authorization: Bearer <user_token>`
        ManticoreSearch->>ManticoreSearch: Authenticate <user_token> and check permissions
        ManticoreSearch->>Client: Return final result
    end
```

1. **Token Initialization for Buddy User**:
    - When Manticore Search starts Manticore Buddy, it generates a random, unique token for Buddy user with all necessary permissions. 
    - The token is passed to Buddy using an environment variable `BUDDY_TOKEN` along with SSL materials via environment variables: `BUDDY_SSL_CERT_CONTENT`, and `BUDDY_SSL_KEY_CONTENT`.
    - All communication between the daemon and Buddy occurs exclusively over a secure HTTPS channel.

2. **Token Usage and Secure Communication**:
   - Buddy reads the token from the environment variable during startup and stores it in memory.
   - Buddy reads the SSL data the environment variable during startup and uses it for TLS setup.
   - Buddy communicates with the daemon via HTTPS for enhanced security. These variables contain the full content of the certificate and key, not file paths.
    - Buddy sends the request with an `Authorization` header containing the raw token it received at startup:
      ```text
      Authorization: Bearer <BUDDY_TOKEN>
      ```
    - The daemon authenticates this token against the `system.auth_users` table. The `manticore_buddy` user must have the required permissions for the operation to succeed.

3. **User Request**:
    - The daemon forwards the user request to Buddy, preserving the user's original `Authorization` header (e.g., `Authorization: Bearer <user_token>`).
    - After Buddy processes the data, it forwards the request back to the daemon with the user's original `Authorization` header. Buddy does not need to interpret or validate the user's token.

4. **Authorization**:
    - The daemon uses a single, unified authentication process for all requests received from Buddy. It extracts the bearer token from the `Authorization` header, hashes it, and looks for a matching user in the `system.auth_users` table.
    - Whether the token identifies the Buddy system user or a regular end-user, the daemon proceeds to the authorization step, where it verifies that the identified user has the required permissions for the requested action.

5. **Security**:
    - The security of this model relies on protecting the plaintext token of the highly privileged Buddy user, which is guarded by the integrity of the HTTPS channel.
    - Sensitive information, including the token and SSL materials, is not logged or stored permanently.
    - To prevent unintended exposure, Buddy must clear the sensitive environment variables (`BUDDY_TOKEN`, `BUDDY_SSL_CERT_CONTENT`, `BUDDY_SSL_KEY_CONTENT`) from its own process environment immediately after reading them at startup. As it is impossible to clear child process environment from the parent process with c++ code.
    - The token and SSL materials are kept in memory by Buddy only for its lifetime.

---

### Logging

All changes to authentication and authorization settings should be recorded in a dedicated authentication log. The log file path should match the path of the `searchd` log, with the addition of a `.auth` suffix. For example: `/var/log/manticore/searchd.log.auth`. The following actions must be included in this log:
1. **User Management**:
   - Adding, deleting, or updating users.
   - Changes to user permissions.
2. **Cluster Synchronization**:
   - Any synchronization of the `system.auth` table across nodes.
   - Overriding of a node's local `system.auth` table when joining a cluster, along with the backup dump of the original table.
3. **Errors and Warnings**:
   - Logging any errors during authentication or replication-related operations.
   - Warnings for missing or corrupted authentication data.
Comprehensive logging ensures traceability of changes and aids in troubleshooting and auditing.

---

### Clients

- Update binary protocol clients to handle the token-based and hashed password approach.
- Update HTTP JSON clients to handle Basic Auth and Bearer Tokens.
