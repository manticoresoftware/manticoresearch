# Connecting to the server

<!-- example connect -->
With default configuration, Manticore is waiting for your connections on:

  * port 9306 for MySQL clients
  * port 9308 for HTTP/HTTPS connections
  * port 9312 for HTTP/HTTPS, and connections from other Manticore nodes and clients based on Manticore binary API

<!-- intro -->
##### Connect via MySQL:

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### Connect via JSON over HTTP

<!-- request HTTP -->
HTTP is a stateless protocol, so it doesn't require any special connection phase:

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
##### Python
<!-- request Python -->
```python
import manticoresearch
config = manticoresearch.Configuration(
    host = "http://127.0.0.1:9308"
)
client =  manticoresearch.ApiClient(config)
indexApi = manticoresearch.IndexApi(client)
searchApi = manticoresearch.searchApi(client)
utilsApi = manticoresearch.UtilsApi(client)
```
<!-- intro -->
##### Javascript
<!-- request Javascript -->
```javascript
var Manticoresearch = require('manticoresearch');
    var client= new Manticoresearch.ApiClient()
    client.basePath="http://127.0.0.1:9308";
    indexApi = new Manticoresearch.IndexApi(client);
    searchApi = new Manticoresearch.SearchApi(client);
    utilsApi = new Manticoresearch.UtilsApi(client);
```

<!-- intro -->
##### Java
<!-- request Java -->
```java
import com.manticoresearch.client.ApiClient;
import com.manticoresearch.client.ApiException;
import com.manticoresearch.client.Configuration;
import com.manticoresearch.client.model.*;
import com.manticoresearch.client.api.IndexApi;
import com.manticoresearch.client.api.UtilsApi;
import com.manticoresearch.client.api.SearchApi;

ApiClient client = Configuration.getDefaultApiClient();
client.setBasePath("http://127.0.0.1:9308");

    IndexApi indexApi = new IndexApi(client);
    SearchApi searchApi = new UtilsApi(client);
    UtilsApi utilsApi = new UtilsApi(client);
```
<!-- intro -->
##### Connect via Docker
If you are familiar with Docker, you can use Manticore's [official Docker image](https://github.com/manticoresoftware/docker) to run Manticore. Here is how you can connect to Manticore's docker via MySQL:
<!-- request docker -->
Run Manticore container and use built-in MySQL client to connect to the node.
```bash
docker run -e EXTRA=1 --name manticore -d manticoresearch/manticore && docker exec -it manticore mysql
```
<!-- end -->
<!-- proofread -->