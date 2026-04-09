# Подключение к серверу

<!-- example connect -->
Со стандартной конфигурацией, Manticore ожидает ваши подключения на:

  * порт 9306 для клиентов MySQL
  * порт 9308 для HTTP/HTTPS подключений
  * порт 9312 для HTTP/HTTPS, а также подключений от других узлов Manticore и клиентов, основанных на бинарном API Manticore

<!-- intro -->
##### Подключение через MySQL:

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### Подключение через JSON по HTTP

<!-- request HTTP -->
HTTP — это безсессионный протокол, поэтому не требует специальной фазы подключения:

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
##### Python-asyncio
<!-- request Python-asyncio -->
```python
import manticoresearch
config = manticoresearch.Configuration(
    host = "http://127.0.0.1:9308"
)
async with manticoresearch.ApiClient(config) as client:
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
##### Rust
<!-- request Rust -->
```rust
use std::sync::Arc;
use manticoresearch::{
    apis::{
        {configuration::Configuration,IndexApi,IndexApiClient,SearchApi,SearchApiClient,UtilsApi,UtilsApiClient}
    },
};

async fn maticore_connect {
	let configuration = Configuration {
	    base_path: "http://127.0.0.1:9308".to_owned(),
	    ..Default::default(),
	};
    let api_config = Arc::new(configuration);
    let utils_api = UtilsApiClient::new(api_config.clone());
    let index_api = IndexApiClient::new(api_config.clone());
    let search_api = SearchApiClient::new(api_config.clone());
```

<!-- intro -->
##### Подключение через Docker
Если вы знакомы с Docker, вы можете использовать официальный образ Manticore [official Docker image](https://github.com/manticoresoftware/docker) для запуска Manticore. Вот как можно подключиться к Docker контейнеру Manticore через MySQL:
<!-- request docker -->
Запустите контейнер Manticore и используйте встроенный клиент MySQL для подключения к узлу.
```bash
docker run --name manticore -d manticoresearch/manticore && docker exec -it manticore mysql
```
<!-- end -->
<!-- proofread -->

