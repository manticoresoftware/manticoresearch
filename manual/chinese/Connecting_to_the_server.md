# 连接到服务器

<!-- example connect -->
使用默认配置，Manticore 正在等待您的连接：

  * 为 MySQL 客户端开放端口 9306
  * 为 HTTP/HTTPS 连接开放端口 9308
  * 为 HTTP/HTTPS 以及来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端开放端口 9312

<!-- intro -->
##### 通过 MySQL 连接：

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### 通过 HTTP 上的 JSON 连接

<!-- request HTTP -->
HTTP 是无状态协议，因此不需要任何特殊的连接阶段：

```bash
curl -s "http://localhost:9308/search"
```

<!-- intro -->
##### 通过 [PHP 客户端](https://github.com/manticoresoftware/manticoresearch-php) 连接：

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
##### 通过 Docker 连接
如果您熟悉 Docker，可以使用 Manticore 的[官方 Docker 镜像](https://github.com/manticoresoftware/docker)来运行 Manticore。下面是如何通过 MySQL 连接到 Manticore 的 docker：
<!-- request docker -->
运行 Manticore 容器并使用内置的 MySQL 客户端连接到节点。
```bash
docker run --name manticore -d manticoresearch/manticore && docker exec -it manticore mysql
```
<!-- end -->
<!-- proofread -->

