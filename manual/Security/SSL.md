# SSL

In many cases you might want to encrypt traffic between your client and the server. To do that you can specify that the server should use [HTTPS protocol](Server_settings/Searchd.md#listen) rather than HTTP.

<!-- example CA 1 -->

To enable HTTPS at least the following two directives should be set in [searchd](Server_settings/Searchd.md) section of the config and there should be at least one [listener](Server_settings/Searchd.md#listen) set to `https`

* [ssl_cert](Server_settings/Searchd.md#ssl_cert) certificate file
* [ssl_key](Server_settings/Searchd.md#ssl_key) key file

In addition to that you can specify certificate authority's certificate (aka root certificate) in

* [ssl_ca](Server_settings/Searchd.md#ssl_ca) certificate authority's certificate file


<!-- intro -->
##### with CA:

<!-- request with CA -->
Example with CA:

```ini
ssl_ca = ca-cert.pem
ssl_cert = server-cert.pem
ssl_key = server-key.pem
```

<!-- request without CA -->
Example without CA:

```ini
ssl_cert = server-cert.pem
ssl_key = server-key.pem
```
<!-- end -->

## Generating SSL files

These steps will help you generate the SSL certificates with 'openssl' tool.

Server can use Certificate Authority to verify the signature of certificates, but can also work with just private key and certificate (w/o the CA certificate).

#### Generate the CA key

```bash
openssl genrsa 2048 > ca-key.pem
```

#### Generate the CA certificate from the CA key

Generate self-signed CA (root) certificate from the private key (fill in at least "Common Name"):

```bash
openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca-cert.pem
```

#### Server Certificate

Server uses the server certificate to secure communication with client. Generate certificate request and server private key (fill in at least "Common Name" different from the root certificate's common name):

```bash
openssl req -newkey rsa:2048 -days 365 -nodes -keyout server-key.pem -out server-req.pem
openssl rsa -in server-key.pem -out server-key.pem
openssl x509 -req -in server-req.pem -days 365 -CA ca-cert.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
```

When done you can verify the key and certificate files were generated correctly:

```bash
openssl verify -CAfile ca-cert.pem server-cert.pem
```

## Secured behavior

When SSL config is present and valid, so that used SSL lib can recognize it and establish secured layer of connection,
following achievements are available and may be used:

 - you can connect to multiprotocol port with https and run queries. Both query and answer will be send with ssl encryption.
 - you can connect to dedicated `https` port with http and run queries. Connection will be secured. (attempt to connect to this port via plain http will be rejected with 400 error code).
 - you can connect to mysql port with mysql client using secured connection. Session will be secured. Note, that cli `mysql` program tries to use ssl by
   default, so usual connect to the daemon in case it has valid SSL config most probably will be secured. You may check it running 'status' command in cli.
   
When SSL config is NOT valid by any reason, which daemon detects by the fact that secured connection can't be established (apart non-valid config it might be other reasons, like just unability to load appropriate SSL lib at all, because of any reason - non-compatible, absent, etc.), followihg things will not work or work non-secured way:

- you can't connect to multiprotocol port with https. Connection will be dropped.
- you can't connect to dedicated `https` port. Https connections will be dropped (and another protocols on that port is just rejected proper way by design).
- connect to mysql port via mysql client will not propagate possibility of ssl securing. So, if client demands it, it will fail. If not - it will use plain mysql41 or compressed connection.

NOTE!

- binary API connections (as connections from old clients, or inter-daemons master-agent connections) are never secured.
- replication is managed by third-party provider, which has to be set up separately. However SST stage of replication in current realization is done by binary API connection, and so, never secured.
- with this 'never secured' statement, however, you still can use any external proxies (like legacy tunneling with ssh) which will provide encryption.