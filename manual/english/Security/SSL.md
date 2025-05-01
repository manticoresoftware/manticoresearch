# SSL

In many cases, you might want to encrypt traffic between your client and the server. To do that, you can specify that the server should use the [HTTPS protocol](../Server_settings/Searchd.md#listen) rather than HTTP.

<!-- example CA 1 -->

To enable HTTPS, at least the following two directives should be set in the [searchd](../Server_settings/Searchd.md) section of the config, and there should be at least one [listener](../Server_settings/Searchd.md#listen) set to `https`

* [ssl_cert](../Server_settings/Searchd.md#ssl_cert) certificate file
* [ssl_key](../Server_settings/Searchd.md#ssl_key) key file

In addition to that, you can specify the certificate authority's certificate (aka root certificate) in:

* [ssl_ca](../Server_settings/Searchd.md#ssl_ca) certificate authority's certificate file


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

These steps will help you generate the SSL certificates using the 'openssl' tool.

The server can use a Certificate Authority to verify the signature of certificates, but it can also work with just a private key and certificate (without the CA certificate).

#### Generate the CA key

```bash
openssl genrsa 2048 > ca-key.pem
```

#### Generate the CA certificate from the CA key

To generate a self-signed CA (root) certificate from the private key (make sure to fill in at least the "Common Name"), use the following command:

```bash
openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca-cert.pem
```

#### Server Certificate

The server uses the server certificate to secure communication with the client. To generate the certificate request and server private key (ensure that you fill in at least the "Common Name" and that it is different from the root certificate's common name), execute the following commands:

```bash
openssl req -newkey rsa:2048 -days 365 -nodes -keyout server-key.pem -out server-req.pem
openssl rsa -in server-key.pem -out server-key.pem
openssl x509 -req -in server-req.pem -days 365 -CA ca-cert.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
```

Once completed, you can verify that the key and certificate files were generated correctly by running:

```bash
openssl verify -CAfile ca-cert.pem server-cert.pem
```

## Secured connection behaviour

When your SSL configuration is valid, the following features are available:

 * You can connect to the multi-protocol port (when no [listener type](../Server_settings/Searchd.md#listen) is specified) over HTTPS and run queries. Both the request and response will be SSL encrypted.
 * You can connect to the dedicated https port with HTTP and run queries. The connection will be secured (attempting to connect to this port via plain HTTP will be rejected with a 400 error code).
 * You can connect to the MySQL port with a MySQL client using a secured connection. The session will be secured. Note that the Linux `mysql` client tries to use SSL by default, so a typical connection to Manticore with a valid SSL configuration will most likely be secured. You can check this by running the SQL 'status' command after connecting.

If your SSL configuration is not valid for any reason (which the daemon detects by the fact that a secured connection cannot be established), apart from an invalid configuration there may be other reasons, such as the inability to load the appropriate SSL library at all. In this case, the following things will not work or will work in a non-secured manner:

* You cannot connect to the multi-protocol port with HTTPS. The connection will be dropped.
* You cannot connect to the dedicated `https` port. The HTTPS connections will be dropped
* Connection to the `mysql` port via a MySQL client will not support SSL securing. If the client requires SSL, the connection will fail. If SSL is not required, it will use plain MySQL or compressed connections.

###Caution:

* Binary API connections (such as connections from old clients or inter-daemons master-agent communication) are not secured.
* SSL for replication needs to be set up separately. However, since the SST stage of the replication is done through the binary API connection, it is not secured either.
* You can still use any external proxies (e.g., SSH tunneling) to secure your connections.
<!-- proofread -->