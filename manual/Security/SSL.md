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

## Secured connection behaviour

When your SSL config is valid the following things are available:

 * you can connect to multiprotocol port (when no [listener type](Server_settings/Searchd.md#listen) is specified) over HTTPS and run queries. Both request and response will be ssl encrypted.
 * you can connect to dedicated `https` port with http and run queries. Connection will be secured. (attempt to connect to this port via plain http will be rejected with 400 error code).
 * you can connect to mysql port with a mysql client using secured connection. The session will be secured. Note, that Linux `mysql` client tries to use ssl by default, so usual connect to Manticore in case it has a valid SSL config most probably will be secured. You can check it by running SQL 'status' command after you connect.

When your SSL config is not valid by any reason, which daemon detects by the fact that a secured connection can't be established (apart non-valid config there may be other reasons, like just inability to load appropriate SSL lib at all), the following things will not work or work non-secured way:

* you can't connect to multiprotocol port with https. The connection will be dropped.
* you can't connect to dedicated `https` port. The HTTPS connections will be dropped.
* connection to `mysql` port via mysql client will not propagate possibility of SSL securing. So, if the client demands it, it will fail. If not - it will use plain mysql or compressed connection.

###Caution:

* binary API connections (such as connections from old clients, or inter-daemons master-agent communication) are not secured
* SSL for replication needs to be set up separately. However since SST stage of the replication is done by the binary API connection it is not secured too.
* you still can use any external proxies (e.g. SSH tunnelling) which will secure your connections.
