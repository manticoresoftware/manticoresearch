# Подключение к серверу

<!-- example connect -->
При стандартной конфигурации Manticore ожидает ваши подключения на:

  * порт 9306 для клиентов MySQL
  * порт 9308 для HTTP/HTTPS соединений
  * порт 9312 для HTTP/HTTPS и соединений от других узлов и клиентов Manticore, основанных на бинарном API Manticore

<!-- intro -->
##### Подключение через MySQL:

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### Подключение через JSON по HTTP

<!-- request HTTP -->
HTTP является статeless протоколом, поэтому не требует специальной фазы подключения:

```bash
curl -s "http://localhost:9308/search"
```

<!-- intro -->
##### Подключение через [PHP клиент](https://github.com/manticoresoftware/manticoresearch-php):

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
##### C#
<!-- request C# -->
```clike
using ManticoreSearch.Client;
using ManticoreSearch.Api;
using ManticoreSearch.Model;

string basePath = "http://127.0.0.1:9308";
IndexApi indexApi = new IndexApi(basePath);
SearchApi searchApi = new UtilsApi(basePath);
UtilsApi utilsApi = new UtilsApi(basePath);
```

<!-- intro -->
##### Подключение через Docker
Если вы знакомы с Docker, вы можете использовать [официальный Docker образ](https://github.com/manticoresoftware/docker) Manticore для запуска Manticore. Вот как вы можете подключиться к Docker Manticore через MySQL:
<!-- request docker -->
Запустите контейнер Manticore и используйте встроенный MySQL клиент для подключения к узлу.
```bash
docker run --name manticore -d manticoresearch/manticore && docker exec -it manticore mysql
```
<!-- end -->
<!-- proofread -->
