# Connecting to the server

<!-- example connect -->
By default Manticore is waiting for your connections on:

  * port 9306 for MySQL clients
  * port 9308 for HTTP/HTTPS connections
  * port 9312 for connections from other Manticore nodes and clients based on Manticore binary API

<!-- intro -->
##### Connect via MySQL:

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### Connect via JSON over HTTP

<!-- request HTTP -->
HTTP is a stateless protocol so it doesn't require any special connection phase:

```bash
curl -s "http://localhost:9308/search"
```

<!-- intro -->
##### Connect via [PHP client](https://github.com/manticoresoftware/manticoresearch-php):

<!-- request PHP -->
```php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new \Manticoresearch\Client($config);
```

<!-- intro -->
##### Connect via Docker
If you are familiar with Docker you can use Manticore's [official Docker image](https://github.com/manticoresoftware/docker) to run Manticore. Here is how you can connect to Manticore's docker via MySQL:
<!-- request docker -->
Run Manticore container and use built-in MySQL client to connect to the node.
```bash
docker run --name manticore -d manticoresearch/manticore && docker exec -it manticore mysql
```
<!-- end -->