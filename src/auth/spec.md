# Spec v1.7.0

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
8. Initial Administrator Setup (`--auth` mode)
9. Configuration
   - Password Policy Model
10. SQL Commands for Authentication and Authorization (RT Mode)
11. DUMP AUTH SQL Command
12. Authentication and Authorization in Replication Clusters
13. Manticore Buddy Authentication and Authorization
14. Logging
15. Clients


### Overview

This document provides the specifications for implementing authentication and authorization functionality in Manticore Search. The system includes support for managing users and their credentials, as well as finely-grained permissions defined at the level of individual actions and targets. It ensures compatibility with end-user clients via the MySQL and HTTP protocols, while providing a robust, encrypted binary protocol for secure inter-daemon communication.

### Requirements

#### Protocol Support

1. **MySQL Protocol**
   - Must support `mysql_native_password` authentication mechanism only.
   - Compatible with newer versions of MySQL and MariaDB clients. For some clients (e.g., MySQL client 9), you may need to enable `mysql_native_password` explicitly.

2. **HTTP Protocol**
   - Authentication via two methods:
     - Basic Auth: Username and password encoded in the request header (e.g., `Authorization: Basic <base64(user:password)>`).
     - Bearer Token: Header like `Authorization: Bearer <token>`.

3. **Binary Protocol (for Inter-Daemon Communication)**
   - Utilizes a secure, encrypted transport layer based on **AES-256-GCM**.
   - Provides built-in confidentiality, integrity, authenticity, and replay protection without relying on an external SSL/TLS layer.
   - This protocol is designed exclusively for communication between Manticore daemon instances (e.g., in a cluster) and is not intended for general-purpose client libraries.
   - For full details, see the "Authentication via Binary Protocol" section.

#### Usability

- A command-line bootstrap mode (`searchd --auth`) for secure initial administrator setup.
- Easy ongoing user management through SQL commands.
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
     - The client uses the token received previously using the `TOKEN` SQL command or the `POST /token` HTTP endpoint:
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
     - The token (`a1b2c3...`) is the user's token, obtained earlier using the `TOKEN` SQL command or the `POST /token` HTTP endpoint.

   - **Validation**:
     - The server looks for a user by matching the provided token with `bearer_sha256`. If a match is found, the request is considered authorized.

### Authentication via Binary Protocol


Communication via the Manticore binary protocol utilizes a robust, encrypted, and authenticated transport layer designed to ensure confidentiality, integrity, and protection against replay attacks, even when SSL/TLS is not used.

The protocol implements a custom cryptographic scheme known as Mini-TLS, built upon **AES-256-GCM** (Authenticated Encryption with Associated Data).

For detailed technical specifications, refer to the [Specification: Secure Binary Protocol for Daemon Communication](spec\_api.md).

#### Key Features of the Secure Binary Protocol

1.  **Confidentiality and Integrity:** All message payloads are encrypted using **AES-256-GCM** with a per-message 12-byte nonce and a 16-byte authentication tag (GCM tag). This tag guarantees the message has not been tampered with and ensures the identity of the sender (integrity and authenticity).
2.  **Mutual Authentication:** Both the client (Daemon Agent) and the server (API Server/Master Daemon) use the same pre-shared or derived symmetric key for encryption and decryption, establishing trust in both directions of communication.
3.  **Replay Protection:** The protocol mandates a robust, stateful **12-byte structured nonce** composed of a sender ID, a monotonic Boot ID (unique per daemon lifetime), and an atomically incrementing message counter. This prevents adversaries from replaying old, captured messages.
4.  **Key Derivation:** The symmetric AES-256 key used for the session is derived from the user's securely stored password hash and salt using **PKCS5\_PBKDF2\_HMAC-SHA256**. This key is unique per user and is never transmitted over the network.

#### Binary Packet Format (Encrypted)

The binary packets sent over the wire include an unencrypted header (containing command and length) followed by the cryptographic blob:

`[Nonce (12 bytes)][Tag (16 bytes)][Ciphertext (variable length)]`

#### Authentication Failure and Error Handling

If decryption fails (due to wrong key, tampering detected by the GCM tag, or a detected nonce replay attack), the message is rejected. The server replies with a status code indicating an authentication or integrity failure (e.g., `STATUS_AUTH_ERROR` or similar internal status codes).

#### Usage Recommendations

Due to the built-in security features, this protocol is suitable for inter-daemon communication over both trusted and untrusted networks. It replaces the need for an external TLS layer while maintaining equivalent security guarantees for the message payload.

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
    - SHOW PERMISSIONS
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
    - CREATE USER - implemented in daemon
    - DROP USER - implemented in daemon
    - GRANT - implemented in daemon
    - REVOKE - implemented in daemon
    - SHOW USERS - implemented in daemon
    - SHOW PERMISSIONS - implemented in daemon
    - SHOW TOKEN - implemented in daemon
    - SET PASSWORD - implemented in daemon
    - TOKEN - implemented in daemon
    - SHOW USAGE
    - RELOAD AUTH

  - HTTP endpoints that manage authentication and authorization:
    - SQL admin commands are executed through the `/sql`, `/cli`, or `/cli_json` endpoints.
    - **`POST /token`**
        - **Purpose:** Generates or updates a bearer token for the currently authenticated user.
        - **Authentication:** The request must include a valid `Authorization` header (e.g., Basic Auth).
        - **Authorization:** Any authenticated user can rotate their own token.
        - **Request Body:** An empty JSON object (e.g., `{}`).
        - **Response:** A JSON object containing the new, unhashed token: `{"token": "<new_token>"}`.

**Notes**

- Commands not explicitly listed under any permission are denied by default.
- Users with the `admin` action are restricted to managing users and permissions. For other actions, such as `read`, `write`, or `schema`, explicit grants are required.
- This list should be reviewed and updated regularly to ensure all possible SQL commands and endpoints are included.
- Direct writes to `system.auth_users` and `system.auth_permissions` are restricted to daemon-internal auth handlers. Even users with the `admin` privilege should not be able to modify these tables directly to prevent corruption.

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

### Initial Administrator Setup (`--auth` mode)

To ensure a secure-by-default configuration, Manticore Search provides a command-line bootstrap procedure for creating the initial administrator user. This process must be run after the daemon has been started for the first time.

**How it works:**

```mermaid
sequenceDiagram
    participant User
    participant Daemon as Manticore Daemon (Running)
    participant Bootstrap as searchd --auth (Client Mode)

    User->>Daemon: Starts daemon normally (e.g., `searchd -c manticore.conf`)
    Note over Daemon: Daemon is running and creates an empty `auth.json` if needed.

    User->>Bootstrap: Runs `searchd -c manticore.conf --auth`
    Bootstrap->>User: prompts for "Enter a new administrator login:"
    User-->>Bootstrap: Enters login
    Bootstrap->>User: prompts for "Enter password:" (twice)
    User-->>Bootstrap: Enters password securely

    Bootstrap->>Bootstrap: Generates salt and hashes
    Bootstrap->>Bootstrap: Writes new admin user and full permissions to `auth.json`
    Note over Bootstrap: Fails if `auth.json` is not empty.

    Bootstrap->>Daemon: Sends IPC command to trigger `RELOAD AUTH`
    Daemon->>Daemon: Receives IPC command and reloads auth data from `auth.json`
    Daemon-->>Bootstrap: Sends IPC reply with success/failure status

    Bootstrap->>User: Prints "authentication settings successfully created and reloaded"
```

**Process Flow:**

1.  **Start the Daemon:** Start the `searchd` process normally. If authentication is enabled, it will run but will not have any users defined.
2.  **Run Bootstrap Mode:** In a separate terminal, run the same `searchd` binary with the `--auth` flag (e.g., `sudo -u manticore searchd -c /etc/manticoresearch/manticore.conf --auth`).
3.  **Provide Credentials:** The tool will interactively prompt for a new administrator login and a password (which must be entered twice for confirmation).
    The provided password must satisfy the password policy rules defined in the **Configuration / Password Policy Model** section.
4.  **File Creation:** The tool securely generates the necessary password hashes and creates the `auth.json` file (or populates it if it was empty). This process will fail if the file already contains user or permission data, preventing accidental overwrites.
5.  **Daemon Notification:** Using a secure, cross-platform Inter-Process Communication (IPC) mechanism (named pipes), the bootstrap process signals the running daemon to execute a `RELOAD AUTH` command.
6.  **Confirmation:** The bootstrap process waits for a confirmation reply from the daemon and informs the user whether the new authentication settings were successfully loaded.

This one-time procedure is the only supported method for creating the first user, ensuring the system is immediately secured.

### Ongoing User and Permission Management

Once the initial administrator is created, all subsequent user and permission management is performed via **SQL commands**. `CREATE USER`, `DROP USER`, `GRANT`, `REVOKE`, `SHOW USERS`, `SHOW PERMISSIONS`, `SHOW TOKEN`, `SET PASSWORD`, and `TOKEN` are processed natively by the daemon.

This SQL-based approach provides a standardized and powerful interface for all day-to-day administrative tasks, including:

*   Creating and deleting users (`CREATE USER`, `DROP USER`).
*   Changing passwords and renewing tokens (`SET PASSWORD`, `TOKEN`).
*   Managing permissions (`GRANT`, `REVOKE`).
*   Auditing the system (`SHOW USERS`, `SHOW PERMISSIONS`).

Please refer to the section **"SQL Commands for Authentication and Authorization"** for a complete list of available commands and their usage.

---

### SQL Commands for Authentication and Authorization (RT Mode)

1. **Create a User**
   Adds a new user with a password.
   - **Note**: The initial administrator user must be created using the `searchd --auth` bootstrap process. This `CREATE USER` command is intended for creating subsequent, non-admin users.
   - Requires `admin` permission.
   - SQL Command:
     ```sql
     CREATE USER '<username>' IDENTIFIED BY '<password>';
     ```
   - The password is securely hashed and stored by the system.
   - The provided password must satisfy the password policy rules defined in the **Configuration / Password Policy Model** section.
   - Returns one row with columns: `token`, `username`, `generated_at`.
   - If user already exists, command fails with `user '<username>' already exists`.
   - Example:
     ```sql
     CREATE USER 'readonly' IDENTIFIED BY 'readonlypassword';
     ```

2. **Create or update a token**
   Creates a new token for an existing user or updates the current token. The result is the new token.
   - **SQL Command:**
     ```sql
     TOKEN ['<username>'];
     ```
   - The token is securely hashed and stored by the system.
   - If the username is not specified, the current user's token will be updated.
   - Any authenticated user can update their own token.
   - Updating another user's token requires `admin` permission.
   - **Examples:**
     ```sql
     TOKEN 'readonly';
     TOKEN;
     ```

3. **Create or update a password**
   Updates the password for an existing user or the current user.
   - **SQL Command:**
     ```sql
     SET PASSWORD '<password>' [FOR '<username>'];
     ```
   - The password is securely hashed and stored by the system.
   - If the username is not specified, the current user's password will be updated.
   - Any authenticated user can update their own password.
   - Updating another user's password requires `admin` permission.
   - The provided password must satisfy the password policy rules defined in the **Configuration / Password Policy Model** section.
   - **Examples:**
     ```sql
     SET PASSWORD 'abcdef' FOR 'justin';
     SET PASSWORD 'abcdef';
     ```

4. **Delete a User**
   Deletes a user and their associated permissions.
   - Requires `admin` permission.
   - SQL Command:
     ```sql
     DROP USER '<username>';
     ```
   - If `<username>` does not exist, command fails with `user '<username>' not found`.
   - Example:
     ```sql
     DROP USER 'readonly';
     ```

5. **Grant a Permission**
   Adds a permission for a specific user.
   - Requires `admin` permission.
   - SQL Command:
     ```sql
     GRANT <action> ON <target> TO '<username>' [WITH BUDGET '<json_budget>'];
     ```
   - `<action>`: `READ`, `WRITE`, `SCHEMA`, or `ADMIN`. Users with the `ADMIN` action can manage users and permissions but require explicit grants for other actions.
   - `<target>`: The object to which the permission applies. Use either `*` or `'*'` for all targets, or specify a table name preceded by the `table/` prefix.
   - **Constraint:** If `<action>` is `ADMIN`, `<target>` must be `*` (or `'*'`).
   - If `<username>` does not exist, the command fails with `user '<username>' not found`.
   - If `<action>` is unknown, the command fails with `unknown action '<action>'`.
   - If user already has permission for the same `<action>` and `<target>`, the command fails (budgets do not create a distinct permission key).
   - `<json_budget>`: Optional JSON specifying resource limits.
   - Example:
     ```sql
     GRANT READ ON * TO 'readonly' WITH BUDGET '{"queries_per_day": 10000}';
     GRANT WRITE ON 'mytable' TO 'custom_user';
     GRANT ADMIN ON * TO 'security_admin';
     ```

6. **Revoke a Permission**
   Removes a specific permission for a user.
   - Requires `admin` permission.
   - SQL Command:
     ```sql
     REVOKE <action> ON <target> FROM '<username>';
     ```
   - **Constraint:** If `<action>` is `ADMIN`, `<target>` must be `*` (or `'*'`).
   - If `<username>` does not exist, the command fails with `user '<username>' not found`.
   - If `<action>` is unknown, the command fails with `unknown action '<action>'`.
   - If user does not have the specified permission, the command fails with `user '<username>' does not have '<action>' permission on '<target>'`.
   - Example:
     ```sql
     REVOKE READ ON '*' FROM 'readonly';
     REVOKE ADMIN ON * FROM 'security_admin';
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

8. **Show Permissions**
   Lists permissions.
   - SQL Command:
     ```sql
     SHOW PERMISSIONS;
     ```
   - **Behavior**:
     - **Admin**: Lists permissions for all users.
     - **Non-admin**: Lists only current user's permissions.
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

9. **Show Permissions for a Specific User**
   Lists permissions for the specified user.
   - SQL Command:
     ```sql
     SHOW PERMISSIONS FOR '<username>';
     ```
   - **Behavior**:
     - **Admin**: Lists only the target user's permissions.
     - **Non-admin**: Returns `Permission denied`.
     - **Unknown target user**: Returns `user '<username>' not found`.
   - Example:
     ```sql
     SHOW PERMISSIONS FOR 'custom_user';
     ```
   - `SHOW MY PERMISSIONS` is not supported.

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

12. **Show Bearer Token Hash**
Displays the securely hashed bearer token (`bearer_sha256`) for a specified user or the current user. This is useful for administrative verification.
   - **SQL Command:**
     ```sql
     SHOW TOKEN [FOR '<username>' | '<username>'];
     ```
   - **Behavior:**
     - `SHOW TOKEN`: Shows the token hash for the current user.
     - `SHOW TOKEN FOR 'some_user'`: Shows the token hash for the specified user.
   - **Permissions:**
     - Any user can run `SHOW TOKEN` to see their own token hash.
     - Requires `admin` permission to run `SHOW TOKEN FOR '<username>'` to see another user's token hash.
   - **Example Output:**
     ```
     +----------+------------------------------------------------------------------+
     | Username | Token                                                            |
     +----------+------------------------------------------------------------------+
     | admin    | 27f955c72fa08387001c6cb5f83985d7baf002e632e60cdbd0b1985136a366c1 |
     +----------+------------------------------------------------------------------+

**Notes on `ADMIN` Action**

- Users with the `ADMIN` action can only manage users and permissions.
- `ADMIN` permissions are global-only and must target `*`.
- `GRANT ADMIN` and `REVOKE ADMIN` with non-`*` targets are rejected.
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

#### Password Policy Model

Password quality requirements are controlled by the following configuration options:

- `auth_password_policy`
  - Defines which password checks are enforced.
  - Allowed values:
    - `LOW`: applies minimum length and non-empty checks.
    - `MEDIUM`: applies `LOW` plus complexity checks (at least one lowercase, one uppercase, one digit, and one non-alphanumeric character).
  - Default: `LOW`.

- `auth_password_min_length`
  - Defines the minimum allowed password length.
  - Default: `8`.

The same password policy must be applied consistently by all password-setting entry points:

- `searchd --auth` bootstrap flow (initial admin password)
- `CREATE USER ... IDENTIFIED BY ...`
- `SET PASSWORD ...`

If validation fails, the command must return a validation error that explains which policy rule failed.

If the `system.auth_users` and `system.auth_permissions` tables (RT Mode) or the authentication file (Plain Mode) do not exist, Manticore should create them. If they already exist, Manticore should validate them during startup. If validation fails, Manticore should not start.

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
    participant SQLClient as SQL Client
    participant ManticoreSearch as Manticore Search
    participant Buddy as Manticore Buddy
    participant EndUserClient as User's Client

    Note over ManticoreSearch: On start, generates and holds BUDDY_TOKEN.
    ManticoreSearch->>Buddy: Starts Buddy, passing BUDDY_TOKEN via env var.

    alt Delegated Command Path
        EndUserClient->>ManticoreSearch: Send request with `Authorization: Bearer <user_token>`
        Note over ManticoreSearch: Authenticates user and resolves delegated identity (`user`)
        ManticoreSearch->>Buddy: Delegate command with `user`
        Buddy->>Buddy: Rewrites/prepares command for daemon
        Buddy->>ManticoreSearch: Execute rewritten command with `Authorization: Bearer <BUDDY_TOKEN>` and optional `X-Manticore-User`
        ManticoreSearch->>ManticoreSearch: Authenticates Buddy token, authorizes as delegated `user`, executes command
        ManticoreSearch-->>Buddy: Return success/error
        ManticoreSearch->>EndUserClient: Return final result
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

3. **Delegated Command Identity**:
    - For commands delegated to Buddy, the daemon must first authenticate the client request and resolve the authenticated user identity.
    - The daemon forwards this identity to Buddy in the request payload as `user`.
    - Buddy sends rewritten/internal commands to daemon using:
      - `Authorization: Bearer <BUDDY_TOKEN>` (always)
      - `X-Manticore-User: <username>` only when incoming payload `user` is non-empty
    - If incoming payload `user` is empty, Buddy must omit `X-Manticore-User`.

4. **Authorization**:
    - The daemon must authenticate Buddy-originated requests using the Buddy bearer token.
    - If `X-Manticore-User` is present, daemon must authorize the command against that delegated user identity.
    - If `X-Manticore-User` is absent, daemon must treat the request as a non-delegated Buddy request.
    - The daemon must accept delegated user identity only for Buddy-authenticated requests from the internal daemon-Buddy channel.

5. **Security**:
    - The security of this model relies on protecting the plaintext token of the highly privileged Buddy user, which is guarded by the integrity of the HTTPS channel.
    - Sensitive information, including the token and SSL materials, is not logged or stored permanently.
    - To prevent unintended exposure, Buddy must clear the sensitive environment variables (`BUDDY_TOKEN`, `BUDDY_SSL_CERT_CONTENT`, `BUDDY_SSL_KEY_CONTENT`) from its own process environment immediately after reading them at startup. As it is impossible to clear child process environment from the parent process with c++ code.
    - The token and SSL materials are kept in memory by Buddy only for its lifetime.

---

### Logging

All changes and significant events related to authentication and authorization are recorded in a dedicated, human-readable log file. This log is crucial for security auditing, troubleshooting access issues, and monitoring administrative actions.

#### **Configuration**

The authentication log's behavior is controlled by settings in the `searchd` section of your Manticore configuration file.

**1. Log File Location:**
The authentication log is automatically created alongside the main `searchd` log file, with an `.auth` suffix.
*   **Example:** If `log` is set to `/var/log/manticore/searchd.log`, the authentication log will be located at `/var/log/manticore/searchd.log.auth`.

**2. Log Level (`auth_log_level`):**
This setting controls the verbosity of the authentication log.
*   **Syntax:** `auth_log_level = <level>`
*   **Default:** `info`
*   **Available Levels:**
    *   `disabled`: No authentication events are logged.
    *   `error`: Logs only critical failures and permission denials.
    *   `warning`: Logs errors plus failed authentication attempts.
    *   `info`: Logs warnings plus all successful administrative changes and successful logins. This is the recommended level for most production environments.

#### **Log Entry Format**

Each line in the authentication log follows a consistent format, providing clear context for every event:

`[Timestamp][ThreadID][LEVEL] Log Message`

*   **Timestamp:** The date and time of the event, with microsecond precision.
*   **ThreadID:** The internal ID of the worker thread that handled the event.
*   **LEVEL:** The severity of the event (`INFO`, `WARN`, `ERROR`, `CRITICAL`).
*   **Log Message:** A detailed, human-readable description of the event.

#### **Logged Events**

The log captures a comprehensive set of events, which can be grouped into the following categories. The log level at which each event is recorded is shown in parentheses.

**1. User and Permission Management (`INFO`)**
Tracks all administrative changes to the authentication system, always including which user performed the action and from where.
*   User creation and deletion.
*   Bearer token regeneration.
*   Granting or revoking permissions.

    *Example:* `[...][INFO] granted read on 'my_index' to user 'readonly_user' by 'admin' via SphinxQL`

**2. Security and Access Events**
Tracks real-time access attempts and the enforcement of permissions.
*   **Successful authentication (`INFO`):** Records when a user successfully logs in, including the method (HTTP, MySQL, etc.) and source IP.
    *Example:* `[...][INFO] user 'admin' successfully authenticated via HTTP Basic from 127.0.0.1`
*   **Failed authentication (`WARN`):** Records any failed login attempt, including the reason (e.g., unknown user, invalid password).
    *Example:* `[...][WARN] failed authentication attempt for user 'admin' via HTTP Basic from 192.168.1.50: invalid password`
*   **Permission denied (`ERROR`):** Records when an authenticated user is denied access to a resource due to insufficient permissions.
    *Example:* `[...][ERROR] user 'readonly_user' from 127.0.0.1 denied write on 'my_index'`

**3. System and Cluster Events**
Tracks high-level changes to the authentication state.
*   **Auth data reloaded (`INFO`):** Logs when the authentication configuration is successfully reloaded.
*   **Cluster join (`CRITICAL`):** A high-priority message is logged when a node joins a cluster and its local authentication data is about to be overwritten.
*   **Local auth data backup (`INFO`):** When joining a cluster, the node's original authentication data is dumped to the log as a JSON backup before being replaced.
*   **Failed Actions (`WARN`):** Any administrative action that fails (e.g., trying to create a duplicate user) is logged with an error reason.

---

### Clients

This section provides guidance for client library developers and users on how to interact with a Manticore Search instance that has authentication enabled.

#### **MySQL and HTTP Clients**

All standard Manticore Search clients that communicate over the **MySQL** or **HTTP** protocols are fully compatible with the authentication system.

*   **HTTP Clients (Official and Third-Party):**
    *   Clients must be updated or configured to support sending an `Authorization` header.
    *   **Basic Authentication:** The client should send the header `Authorization: Basic <base64(user:password)>`.
    *   **Bearer Token Authentication:** The client should send the header `Authorization: Bearer <token>`.
    *   If SSL/TLS is enabled on the daemon's HTTP port, it is strongly recommended that clients use `https` for all connections to protect credentials.

*   **MySQL Clients (Official and Third-Party):**
    *   Clients must support the `mysql_native_password` authentication mechanism.
    *   Users should provide their username and password in the connection string or parameters, just as they would when connecting to a standard MySQL database.
    *   If SSL/TLS is enabled on the daemon's MySQL port, it is strongly recommended that clients enable SSL in their connection options to protect credentials.

#### **Binary Protocol Clients and Inter-Daemon Communication**

The secure binary protocol described in this specification is designed exclusively for **inter-daemon communication** (e.g., between a master and agents in a replication cluster). It is **not intended for use by general-purpose, third-party client libraries** (e.g., from PHP, Python, Java, etc.).

*   **Why can't standard clients use it?**
    *   The binary protocol's security relies on a complex, stateful, and custom cryptographic scheme (AES-256-GCM with a structured nonce and replay protection).
    *   The symmetric encryption key is derived on the server from the user's password hash and salt; it is not directly available to an external client.
    *   Implementing this custom "mini-TLS" layer would be a significant and complex undertaking for every client library, and it falls outside their intended scope.

*   **Guidance for Users of Binary Client Libraries:**
    *   If you have authentication enabled on your Manticore Search instance, you **must use the HTTP or MySQL protocols** for your application's client connections.
    *   The binary protocol port should be firewalled off from public access and used only for communication between Manticore nodes.
