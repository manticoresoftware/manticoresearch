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