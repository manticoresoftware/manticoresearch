# Manticore Protocol Versioning System

This document provides a comprehensive overview of how protocol versioning works in Manticore Search, covering daemon/client version compatibility, command-specific versioning, and backward/forward compatibility mechanisms.

## Overview

Manticore Search uses a multi-layered versioning system to ensure compatibility between different client and daemon versions. The versioning system operates at several levels:

1. **Connection-level handshake** (global protocol version)
2. **Command-level versioning** (per-command protocol versions)
3. **Replication protocol versioning** (for cluster operations)
4. **Master-agent extensions** (for distributed search)

## 1. Connection-Level Handshake

### Handshake Protocol Version

The initial connection handshake uses a simple global protocol version:

```cpp
// From src/sphinx.h
#define SPHINX_SEARCHD_PROTO    1
#define SPHINX_CLIENT_VERSION   1
```

**Code Implementation:**

```cpp
// From src/netreceive_api.cpp - Daemon sends handshake
tOut.SendDword ( SPHINX_SEARCHD_PROTO ); // that is handshake

// Daemon receives and validates client handshake
auto uHandshake = tIn.GetDword();
if ( uHandshake!=SPHINX_CLIENT_VERSION && uHandshake!=0x01000000UL )
{
    sphLogDebugv ( "conn %s(%d): got handshake, major v.%d", sClientIP, iCID, uHandshake );
    return; // Drop connection on version mismatch
}
```

```cpp  
// From src/searchdha.cpp - Agent sends handshake to master
m_tOutput.SendDword ( SPHINX_CLIENT_VERSION );
```

**Key characteristics:**
- Both daemon and client exchange a 4-byte DWORD containing version `0x00000001`
- This is the **only** exchange that supports both big-endian and little-endian byte order
- If versions don't match exactly (`1` or `0x01000000` for endianness), connection is dropped
- This version has remained constant at `1` throughout Manticore's history

**Handshake Flow:**
```
1. TCP connection established
2. Daemon sends: 0x00000001 (or 0x01000000 in host byte order)
3. Client sends: 0x00000001 (or 0x01000000 in host byte order)
4. Both sides validate received version
5. Connection proceeds to command phase (if successful)
```

## 2. Command-Level Versioning

### Version Format

Each command uses a 16-bit version number (`WORD`) encoded as **MAJOR.MINOR**:
- **High byte (bits 15-8)**: Major version
- **Low byte (bits 7-0)**: Minor version
- Example: Version 1.38 = `0x126` (0x01 major, 0x26 minor)

### Current Command Versions

| Command | Code | Current Version | Hex | Description |
|---------|------|----------------|-----|-------------|
| SEARCH | 0 | 1.38 | 0x126 | Main search command |
| EXCERPT | 1 | 1.4 | 0x104 | Snippet generation |
| UPDATE | 2 | 1.4 | 0x104 | Document updates |
| KEYWORDS | 3 | 1.2 | 0x102 | Keyword extraction |
| PERSIST | 4 | N/A | N/A | Connection persistence (unversioned) |
| STATUS | 5 | 1.1 | 0x101 | Status information |
| FLUSHATTRS | 7 | 1.0 | 0x100 | Flush attributes |
| SPHINXQL | 8 | 1.0 | 0x100 | SQL proxy command |
| PING | 9 | 1.0 | 0x100 | Connectivity test |
| UVAR | 11 | 1.0 | 0x100 | User variables |
| JSON | 16 | 1.2 | 0x102 | JSON/REST proxy |
| CALLPQ | 17 | 1.0 | 0x100 | Percolate queries |
| CLUSTER | 18 | 1.10 | 0x10A | Cluster operations |
| GETFIELD | - | 1.0 | 0x100 | Field operations |
| SUGGEST | 15 | 1.1 | 0x101 | Query suggestions |

**Code Implementation:**

```cpp
// From src/searchdaemon.h - Command version definitions
enum SearchdCommandV_e : WORD
{
    VER_COMMAND_SEARCH      = 0x126, // 1.38
    VER_COMMAND_EXCERPT     = 0x104,
    VER_COMMAND_UPDATE      = 0x104,
    VER_COMMAND_KEYWORDS    = 0x102,
    VER_COMMAND_STATUS      = 0x101,
    VER_COMMAND_FLUSHATTRS  = 0x100,
    VER_COMMAND_SPHINXQL    = 0x100,
    VER_COMMAND_JSON        = 0x102,
    VER_COMMAND_PING        = 0x100,
    VER_COMMAND_UVAR        = 0x100,
    VER_COMMAND_CALLPQ      = 0x100,
    VER_COMMAND_CLUSTER     = 0x10A,
    VER_COMMAND_GETFIELD    = 0x100,
    VER_COMMAND_SUGGEST     = 0x101,
    
    VER_COMMAND_WRONG = 0,
};
```

```php
// From api/sphinxapi.php - Client-side version constants
define ( "VER_COMMAND_SEARCH",      0x120 );  // Note: Client uses older version
define ( "VER_COMMAND_EXCERPT",     0x104 );
define ( "VER_COMMAND_UPDATE",      0x104 );
define ( "VER_COMMAND_KEYWORDS",    0x100 );
define ( "VER_COMMAND_STATUS",      0x101 );
define ( "VER_COMMAND_FLUSHATTRS",  0x100 );
```

### Version Compatibility Rules

The daemon implements strict version checking for each command:

```cpp
// From src/searchd.cpp - Actual implementation
bool CheckCommandVersion ( WORD uVer, WORD uDaemonVersion, ISphOutputBuffer & tOut )
{
    // Rule 1: Major versions MUST match exactly
    if ( ( uVer>>8)!=( uDaemonVersion>>8) )
    {
        SendErrorReply ( tOut, "major command version mismatch (expected v.%d.x, got v.%d.%d)",
            uDaemonVersion>>8, uVer>>8, uVer&0xff );
        return false;
    }
    
    // Rule 2: Client version MUST be <= daemon version  
    if ( uVer>uDaemonVersion )
    {
        SendErrorReply ( tOut, "client version is higher than daemon version (client is v.%d.%d, daemon is v.%d.%d)",
            uVer>>8, uVer&0xff, uDaemonVersion>>8, uDaemonVersion&0xff );
        return false;
    }
    
    return true; // Compatible
}
```

**Usage Example:**
```cpp
// From src/daemon/api_search.cpp - SEARCH command handler
void HandleCommandSearch ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
    // Check command version compatibility
    if ( !CheckCommandVersion ( uVer, VER_COMMAND_SEARCH, tOut ) )
        return;
    
    // Additional minimum version check for SEARCH
    const WORD MIN_VERSION = 0x119;  // 1.25
    if ( uVer<MIN_VERSION )
    {
        SendErrorReply ( tOut, "client version is too old; upgrade your client (client is v.%d.%d, min is v.%d.%d)", 
            uVer>>8, uVer&0xff, MIN_VERSION>>8, MIN_VERSION&0xff );
        return;
    }
    
    // Command processing continues...
}
```

**Compatibility Matrix:**
- ✅ **Compatible**: Client major = Daemon major AND Client version ≤ Daemon version
- ❌ **Incompatible**: Client major ≠ Daemon major OR Client version > Daemon version

### Special Cases

#### SEARCH Command Minimum Version
The SEARCH command has an additional minimum version requirement:

```cpp
const WORD MIN_VERSION = 0x119; // 1.25
if (uVer < MIN_VERSION) {
    // Error: client version too old
    return false;
}
```

#### Version Validation Examples

| Client | Daemon | Compatible? | Reason |
|--------|--------|-------------|---------|
| 1.25 | 1.38 | ✅ | Same major, client ≤ daemon |
| 1.38 | 1.25 | ❌ | Client > daemon |
| 1.25 | 2.0 | ❌ | Different major versions |
| 1.20 | 1.38 | ❌ | Below minimum version for SEARCH |

## 3. Master-Agent Extensions (Distributed Search)

### Master Version Protocol

For distributed searches, the SEARCH command includes a special `master_version` field that enables protocol extensions:

```cpp
// From src/searchdaemon.h
enum
{
    VER_COMMAND_SEARCH_MASTER = 25  // Current maximum master version
};
```

**Code Implementation:**

```cpp
// From src/daemon/api_search.cpp - Agent sends master version to daemon
void SearchRequestBuilder_c::BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const
{
    auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_SEARCH, VER_COMMAND_SEARCH );
    
    tOut.SendInt ( VER_COMMAND_SEARCH_MASTER );  // Send maximum supported version
    tOut.SendInt ( m_dQueries.GetLength() );
    // ... rest of request
}
```

```cpp
// From src/daemon/api_search.cpp - Daemon validates master version
void HandleCommandSearch ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
    // ... version checks ...
    
    int iMasterVer = tReq.GetInt();
    if ( iMasterVer<0 || iMasterVer>VER_COMMAND_SEARCH_MASTER )
    {
        SendErrorReply ( tOut, "master-agent version mismatch; update me first, then update master!" );
        return;
    }
    WORD uMasterVer { WORD (iMasterVer) };
    bool bAgentMode = ( uMasterVer>0 );  // Agent mode detection
    
    // ... rest of processing
}
```

### Master Version Capabilities

Each master version enables specific protocol features:

| Version | Feature |
|---------|---------|
| 0 | Regular client mode (no extensions) |
| 1+ | Agent mode with extended schema |
| 1 | Collation support |
| 2 | Outer ORDER BY and LIMIT |
| 5 | Equal bounds filters |
| 6 | GROUP BY limits |
| 13 | MVA functions (ANY/ALL/NONE) |
| 14 | UDF ranker support |
| 15 | FACET support, enhanced filters |
| 16 | Keyword expansion options |
| up to 25 | Additional distributed features |

### Client vs Agent Mode

- **Client Mode** (`master_version = 0`): Returns exactly requested columns
- **Agent Mode** (`master_version > 0`): Returns additional service columns needed for distributed aggregation

**Version-Dependent Features:**

```cpp
// From src/daemon/api_search.cpp - Version-dependent attribute handling
static ESphAttr FixupAttrForNetwork ( const CSphColumnInfo & tCol, const CSphSchema & tSchema, 
                                      int iVer, WORD uMasterVer, bool bAgentMode )
{
    bool bSendJson = ( bAgentMode && uMasterVer>=3 );       // JSON support from v3
    bool bSendJsonField = ( bAgentMode && uMasterVer>=4 );  // JSON fields from v4
    
    switch ( tCol.m_eAttrType )
    {
        // Attribute type handling based on version...
    }
}
```

```cpp
// From src/daemon/api_search.cpp - Conditional feature inclusion
void SendResult ( int iVer, ISphOutputBuffer & tOut, const AggrResult_t& tRes, 
                 bool bAgentMode, const CSphQuery & tQuery, WORD uMasterVer )
{
    // Various features enabled based on master version
    if ( bAgentMode && uMasterVer>=7 )
    {
        // Send additional statistics (introduced in v7)
        tOut.SendDword ( tRes.m_tStats.m_iFetchedDocs );
        tOut.SendDword ( tRes.m_tStats.m_iFetchedHits );
    }
    
    if ( bAgentMode && uMasterVer>=19 )
    {
        // Even more advanced features (v19+)
        // ...
    }
}
```

## 4. Replication Protocol Versioning

### Replication Version

The replication subsystem has its own versioning:

```cpp
// From src/replication/serialize.cpp
static constexpr WORD VER_COMMAND_REPLICATE = 0x109; // 1.9
```

**Code Implementation:**

```cpp
// From src/replication/serialize.cpp - Version validation
bool LoadCmdHeader( MemoryReader_c& tReader, ReplicationCommand_t* pCmd )
{
    TlsMsg::ResetErr();
    auto eCommand = (ReplCmd_e) tReader.GetVal<WORD> ();
    if ( eCommand<ReplCmd_e::PQUERY_ADD || eCommand>ReplCmd_e::TOTAL )
        return TlsMsg::Err ( "bad replication command %d", (int) eCommand );

    pCmd->m_uVersion = tReader.GetVal<WORD> ();
    if ( pCmd->m_uVersion>VER_COMMAND_REPLICATE )
        return TlsMsg::Err ( "replication command %d, version mismatch %d, got %d", 
                           (int) eCommand, VER_COMMAND_REPLICATE, (int)pCmd->m_uVersion );

    pCmd->m_eCommand = eCommand;
    pCmd->m_sIndex = tReader.GetString ();
    return true;
}
```

```cpp
// From src/replication/serialize.cpp - Version setting  
void SaveCmdHeader ( const ReplicationCommand_t & tCmd, MemoryWriter_c & tWriter )
{
    tWriter.PutWord ((WORD) tCmd.m_eCommand );
    tWriter.PutWord ( VER_COMMAND_REPLICATE );  // Always use current version
    tWriter.PutString ( tCmd.m_sIndex );
}
```

### Version History

```cpp
// ver 0x105 - Fixed CSphWordHit serialization  
// ver 0x106 - Add total indexed bytes to accum
// ver 0x107 - Add blobs vector to replicate update statement
// ver 0x108 - GTID sent as blob (was string)
// ver 0x109 - Index support for ALTER ADD/DROP table
```

### Replication Compatibility

Replication uses a simpler compatibility rule:
- Client replication version MUST be ≤ daemon replication version
- No major/minor version distinction

## 5. Backward and Forward Compatibility

### Backward Compatibility Strategy

1. **Command Support**: Daemon supports older client command versions within the same major version
2. **Graceful Degradation**: Newer daemon features are disabled when serving older clients
3. **Protocol Extensions**: New features added as optional extensions that older clients can ignore

### Forward Compatibility Limitations

1. **No Future Support**: Daemon cannot serve clients with higher command versions
2. **Major Version Boundaries**: Breaking changes require major version increments
3. **Strict Validation**: Version mismatches result in immediate connection termination

### Version Evolution Strategy

1. **Minor Increments**: Add new optional fields or extend existing behavior
2. **Major Increments**: Breaking changes, protocol restructuring
3. **Extension Mechanisms**: Master-agent extensions allow feature additions without breaking compatibility

## 6. Error Handling

### Version Mismatch Errors

1. **Handshake Failure**: Connection dropped silently or with retry message
2. **Command Version Mismatch**: Error response with specific version information
3. **Minimum Version**: Clear error message indicating required upgrade

### Example Error Messages

**From actual code - these are the real error messages:**

```cpp
// From src/searchd.cpp - Major version mismatch
SendErrorReply ( tOut, "major command version mismatch (expected v.%d.x, got v.%d.%d)",
    uDaemonVersion>>8, uVer>>8, uVer&0xff );

// From src/searchd.cpp - Client version too new  
SendErrorReply ( tOut, "client version is higher than daemon version (client is v.%d.%d, daemon is v.%d.%d)",
    uVer>>8, uVer&0xff, uDaemonVersion>>8, uDaemonVersion&0xff );

// From src/daemon/api_search.cpp - Client version too old
SendErrorReply ( tOut, "client version is too old; upgrade your client (client is v.%d.%d, min is v.%d.%d)", 
    uVer>>8, uVer&0xff, MIN_VERSION>>8, MIN_VERSION&0xff );

// From src/daemon/api_search.cpp - Master-agent version mismatch
SendErrorReply ( tOut, "master-agent version mismatch; update me first, then update master!" );

// From src/replication/serialize.cpp - Replication version mismatch
TlsMsg::Err ( "replication command %d, version mismatch %d, got %d", 
             (int) eCommand, VER_COMMAND_REPLICATE, (int)pCmd->m_uVersion );
```

**Example outputs:**
```
"major command version mismatch (expected v.1.x, got v.2.0)"
"client version is higher than daemon version (client is v.1.40, daemon is v.1.38)"  
"client version is too old; upgrade your client (client is v.1.20, min is v.1.25)"
"master-agent version mismatch; update me first, then update master!"
"replication command 5, version mismatch 265, got 266"
```

## 7. Implementation Guidelines

### For Client Developers

1. **Version Detection**: Always check daemon capabilities before using advanced features
2. **Graceful Fallback**: Implement fallback behavior for unsupported versions
3. **Error Handling**: Properly handle version mismatch errors
4. **Testing**: Test against multiple daemon versions

**Example Client Implementation:**

```php
// From api/sphinxapi.php - How the PHP client uses versions
public function Query ( $query, $index="*", $comment="" )
{
    // Build request with command version
    $req = pack ( "nnNNN", SEARCHD_COMMAND_SEARCH, VER_COMMAND_SEARCH, $len, 0, $nreqs ) . $req;
    
    // Send and validate response
    if ( !( $this->_Send ( $fp, $req, $len+8 ) ) ||
         !( $response = $this->_GetResponse ( $fp, VER_COMMAND_SEARCH ) ) )
    {
        // Handle version mismatch or communication errors
        return false;
    }
    
    // Process response...
}
```

### For Daemon Developers

1. **Backward Compatibility**: Maintain support for reasonable range of older client versions
2. **Version Documentation**: Document all protocol changes with version numbers
3. **Deprecation Policy**: Provide clear migration paths for breaking changes
4. **Extension Design**: Use optional extensions rather than breaking changes when possible

## 8. Best Practices

### Version Management

1. **Conservative Approach**: Don't bump versions unnecessarily
2. **Documentation**: Maintain detailed changelog of protocol changes
3. **Testing Matrix**: Test all supported version combinations
4. **Migration Planning**: Plan deprecation timeline for old versions

### Development Workflow

1. **Feature Design**: Design new features as backward-compatible extensions
2. **Version Planning**: Plan version increments as part of feature design
3. **Compatibility Testing**: Test new features with older clients
4. **Documentation**: Update protocol documentation with every change

## Conclusion

Manticore's multi-layered versioning system provides a robust framework for managing compatibility between different client and daemon versions. The system balances the need for innovation with the requirement for backward compatibility, ensuring that existing clients continue to work while new features can be added progressively.

Understanding this versioning system is crucial for:
- Client library developers
- System administrators managing mixed-version environments  
- Contributors working on protocol extensions
- Users planning upgrades and migrations

The key principle is that **major versions must match exactly**, while **minor versions support backward compatibility** within reasonable bounds. 