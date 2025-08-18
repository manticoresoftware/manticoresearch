### **Specification: Secure Binary Protocol for Daemon Communication**

## Overview

This document describes the specification for a secure **binary TCP API protocol** used by multiple internal daemons. It defines a secure message exchange without relying on TLS, leveraging AES-256-GCM encryption for all inter-daemon communication.

## Goals

*   Ensure confidentiality, integrity, and authenticity for all **binary API packets**.
*   Avoid TLS due to performance reasons, while maintaining equivalent security.
*   Support secure and efficient inter-daemon communication.
*   Allow for future extension to handshake-based session key exchange.

## Architecture

### Components

*   **API Server / Master Daemon**: A central daemon handling requests from other daemons.
*   **Daemon Agents**: Internal nodes (e.g., worker daemons) communicating securely with the Master Daemon and with each other.

### Communication Flows

*   **API Server ↔ Daemon Agent**: Communication occurs over a persistent TCP connection using a **binary protocol with message-level AES-256-GCM encryption**.

## Encryption Scheme (Mini-TLS)

### Algorithm

*   **AES-256-GCM** (Authenticated Encryption with Associated Data)
*   256-bit symmetric key (32 bytes)
*   12-byte nonce per message
*   16-byte authentication tag (GCM tag)

### **Binary Packet Format**

Instead of JSON, the protocol uses a packed binary format for efficiency. The encrypted portion of a packet follows this structure:

`[Nonce (12 bytes)][Tag (16 bytes)][Ciphertext (variable length)]`

This encrypted blob is typically preceded by an unencrypted header containing the packet length and command code.

### Message Lifecycle

1.  Generate a structured 12-byte nonce using the protocol's stateful generator.
2.  Encrypt the plaintext payload using AES-256-GCM with the shared key and nonce.
3.  Transmit the complete **binary packet (including the unencrypted header, nonce, tag, and ciphertext) over a TCP connection**.
4.  The receiver validates the unencrypted header, then uses the nonce and tag to decrypt the ciphertext. If tag verification fails, the packet is rejected.

## Crypto Field Explanations

### `key`

*   Symmetric secret key shared by both parties.
*   Must be securely generated and stored.
*   Key must never be transmitted across the network.

### `nonce`

*   Initialization vector to ensure encryption uniqueness for a given key. It is not secret and is sent with the ciphertext.
*   **Must never repeat** for the same key.
*   The protocol uses a **structured 12-byte nonce** composed of a sender ID, a boot session ID, and a counter. This robust structure guarantees uniqueness across the entire system, even after daemon restarts, and is the foundation for the protocol's replay protection.

### `tag`

*   Authentication code produced by AES-GCM.
*   Ensures message authenticity and integrity.
*   Prevents undetected tampering. If any byte of the ciphertext or associated data (AAD) changes, decryption will fail.

### Why `nonce` and `tag` are essential

*   `nonce` guarantees cryptographic safety of the encryption (GCM fails catastrophically if reused).
*   `tag` provides built-in message authentication — ensuring the sender is who they claim to be, and the message hasn't been modified.

### Bidirectional Authentication

*   When a master sends a request, the agent validates the message using the shared `key`, `nonce`, and `tag`.
*   When an agent replies, it encrypts its response with the same shared key and sends its own `nonce` and `tag`.
*   The master validates the response in the exact same way.
*   This mutual use of `EncryptGCM()` / `DecryptGCM()` ensures integrity and trust in both directions.

## Key Management

### Key Derivation

*   Each client (user/daemon) is assigned a **unique symmetric key**.
*   Keys may be derived from SHA-256(password+salt) or PBKDF2 if from user secrets.
*   Stored in memory or secure storage as `map<user_id, key_bytes>`.

### Nonce Creation, Management, and Validation Protocol

This section describes the design and implementation of the 96-bit (12-byte) nonce used for AES-GCM encryption in the daemon-to-daemon binary protocol. The system is designed to be secure, robust against restarts, and efficient, preventing nonce reuse and protecting against replay attacks.

#### 1. Nonce Structure

The 12-byte nonce is a structured field with three distinct components, each serving a specific security purpose. The values are packed in network byte order (big-endian).

The 96 bits are divided as follows:

*   **Bits 0-31: Direction and Sender ID (`m_uDirAgentId`)**
    *   **1 bit (`DIR_MASK`): Direction Bit.** Identifies the sender's role. `1` for master-to-agent messages, `0` for agent-to-master messages. This ensures that request and reply nonces occupy separate logical spaces.
    *   **31 bits (`AGENT_ID_MASK`): Sender `server_id`**. Identifies the daemon that *created* the message. The receiver uses this ID to look up the sender's security state (their `boot_id` and last seen `counter`).

*   **Bits 32-63: Boot ID (`m_uBootId`)**
    *   A 32-bit value that is unique per daemon process lifetime. It acts as a session identifier to distinguish between messages sent before and after a daemon restart.
    *   **24 bits (Time Part):** Derived from the number of seconds elapsed since a fixed epoch (May 1, 2019). This value is **monotonically increasing** across restarts, forming a cryptographic "ratchet."
    *   **8 bits (Random Part):** A random value added to ensure uniqueness for daemons that might restart within the same second.
    *   This combined structure guarantees that a newly started daemon will generate a `boot_id` that is almost certainly greater than its previous one, which is critical for the validation logic.

*   **Bits 64-95: Counter (`m_uCounter`)**
    *   A 32-bit atomically incrementing counter. This counter is global to the sending daemon and is incremented for every outgoing encrypted message, regardless of the recipient.
    *   The combination of a unique `boot_id` and an ever-increasing `counter` ensures that every nonce generated by a daemon during its lifetime is unique.

#### 2. Nonce Generation (`NonceGenerator_c`)

The `NonceGenerator_c` is a thread-safe singleton responsible for creating nonces for all outgoing encrypted messages.

*   **Initialization:**
    *   Upon daemon startup, the singleton's constructor is called once.
    *   It calculates a single, fixed `m_uBootId` for the process's entire lifetime using the time-based ratchet and random component described above.
    *   The global `m_uCounter` is initialized to `1`.

*   **Generation (`Generate` method):**
    *   When an outgoing message needs to be encrypted, `Generate` is called.
    *   It atomically increments the single global `m_uCounter` to get a new, unique counter value for this message.
    *   It constructs a `Nonce_t` struct, filling it with:
        1.  The daemon's own `server_id` (as the sender ID).
        2.  The direction of the message (is this daemon acting as a master?).
        3.  The pre-calculated, fixed `m_uBootId`.
        4.  The newly acquired unique `m_uCounter` value.
    *   Finally, it packs this 12-byte structure into the output buffer in network byte order.

*   **Counter Overflow:** The code includes a `FIXME` note regarding counter overflow. If the 32-bit counter is exhausted (after ~4.3 billion messages), a new `boot_id` should ideally be generated to reset the security context. The current implementation does not handle this, but it is a very rare edge case for a single boot session.

#### 3. Nonce Validation (`NonceValidator_c`)

The `NonceValidator_c` is a thread-safe singleton responsible for validating all incoming nonces to protect against replay attacks and other session-related vulnerabilities.

*   **State Management:**
    *   The validator maintains an in-memory hash map (`m_hStates`) that tracks the security state of every peer (identified by their `server_id`).
    *   For each peer, it stores an `AgentState_t` struct containing:
        *   `m_uLastBootId`: The most recent `boot_id` seen from that peer.
        *   `m_uLastCounter`: The highest `counter` value seen from that peer *within the context of `m_uLastBootId`*.

*   **Validation Logic (`Validate` method):**
    When a message is received and decrypted, its nonce is passed to `Validate`, which performs a sequence of checks:

    1.  **Unpack and Direction Check:** The 12-byte nonce is unpacked. The direction bit is checked to ensure it matches the expected flow (e.g., a master expects replies from agents, where `IsMaster()` is false).

    2.  **Peer State Lookup:** The sender's `server_id` is extracted from the nonce and used to look up its state. If no state exists, this is the first message from this peer; its state is recorded, and the nonce is accepted as valid.

    3.  **Boot ID Ratchet Check (Session Integrity):**
        *   **`if new_boot_id > last_seen_boot_id`**: This indicates the peer has restarted cleanly. The validator accepts this as a new, valid session. It updates its state for the peer with the new `boot_id` and resets the `last_seen_counter` to the counter from this message.
        *   **`if new_boot_id < last_seen_boot_id`**: This is a critical failure. It indicates a delayed message from a previous, stale session or a session replay attack. The message is **rejected**. This check is effective because `boot_id`s are time-based and monotonic.

    4.  **Counter Replay Check (Message Integrity):**
        *   If `new_boot_id == last_seen_boot_id`, the message is from the current, known session.
        *   The validator checks **`if new_counter <= last_seen_counter`**. If this is true, it means this exact message (or an earlier one) has already been seen. It is a replay attack or a severe network reordering, and the message is **rejected**.

    5.  **State Update:** If all checks pass, the nonce is deemed valid. The validator updates the `m_uLastCounter` for the peer to the new, higher value.

This multi-layered validation ensures that only fresh messages from the most recent session of a peer, in a non-repeating sequence, are accepted.

## Roles and Responsibilities

### API Server

*   Validate and decrypt daemon messages.
*   Route and proxy requests between daemons.
*   Re-encrypt and forward requests where needed.

### Daemon Agent

*   Validate master-originated requests.
*   Generate responses with AES-GCM encryption.
*   Optionally authenticate other daemons via pre-shared keys.

## Authentication and Authorization

### User Requests

*   (Out of scope for this inter-daemon protocol, but may involve a login to obtain a token.)
*   A user's hashed token (`sha256(token)`) may be used as the key for AES-GCM.

### Inter-daemon Requests

*   Use a pre-shared key per daemon or a key derived from an ID/token.
*   Request identity is embedded in the nonce and can also be part of the Associated Authenticated Data (AAD).

## **Error Handling**

Error conditions are communicated via **status codes within the binary reply packets**, not HTTP codes.

*   **Decryption Errors:** If tag verification fails, the daemon must reply with a status code indicating an authentication failure (e.g., `STATUS_AUTH_ERROR`).
*   **Malformed Packet:** If a received packet has an invalid length or fails basic structural checks before decryption, the connection may be closed, or a reply with a generic error status (e.g., `STATUS_ERROR`) should be sent.
*   **Nonce Replay:** If the validator detects a replayed or stale nonce, the daemon must reply with a specific status code indicating a replay attempt (e.g., `STATUS_REPLAY_DETECTED`). The attempt should be logged for security auditing.

## Performance Considerations

### Optimizations

*   Use persistent, **keep-alive TCP connections** between daemons to avoid connection setup overhead.
*   Employ efficient, **asynchronous I/O models** (e.g., epoll, kqueue, IOCP) for network handling.
*   Use memory pools for buffers to reduce allocation overhead.

### AES-GCM Performance

*   Leverage CPU-specific instructions (AES-NI) if available.

## Optional: Handshake Protocol

If future enhancement is needed:

*   ECDH key exchange to derive session keys.
*   Derive key via `HMAC(shared_secret, label)`.
*   Use the same AES-GCM logic after the handshake.

## Security Summary

| Feature           | AES-GCM Scheme                          |
| ----------------- | --------------------------------------- |
| Confidentiality   | ✅ Yes                                   |
| Integrity         | ✅ Yes (via tag)                         |
| Mutual Auth       | ✅ (if using unique keys)                |
| Replay Protection | ✅ Yes (via mandatory nonce validation) |
| Forward Secrecy   | ❌ No (add handshake if needed)          |

## Implementation Notes

*   AES-GCM encrypt/decrypt implemented with OpenSSL.
*   Nonce & Tag must always be validated.
*   Key rotation and invalidation should be planned but is out of scope here.

### C++ Functions

#### EncryptGCM

```cpp
bool EncryptGCM(const std::vector<uint8_t>& plaintext,
                const std::vector<uint8_t>& key,
                std::vector<uint8_t>& nonce_out,
                std::vector<uint8_t>& ciphertext_out,
                std::vector<uint8_t>& tag_out);
```

* Encrypts input using AES-256-GCM
* Generates a fresh 12-byte nonce
* Outputs ciphertext and 16-byte tag

#### DecryptGCM

```cpp
bool DecryptGCM(const std::vector<uint8_t>& ciphertext,
                const std::vector<uint8_t>& key,
                const std::vector<uint8_t>& nonce,
                const std::vector<uint8_t>& tag,
                std::vector<uint8_t>& plaintext_out);
```

* Decrypts message using key and nonce
* Verifies tag
* Returns false if tag verification fails (e.g., tampering or wrong key)