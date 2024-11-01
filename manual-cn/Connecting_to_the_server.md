# 连接到服务器

<!-- example connect -->

在默认配置下，Manticore 等待你通过以下端口进行连接：

- 9306 端口用于 MySQL 客户端
- 9308 端口用于 HTTP/HTTPS 连接
- 9312 端口用于 HTTP/HTTPS 连接，以及其他 Manticore 节点和基于 Manticore 二进制 API 的客户端连接

<!-- intro -->
##### 通过 MySQL 连接：

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->

##### 通过 HTTP 使用 JSON 进行连接

<!-- request HTTP -->
HTTP is a stateless protocol, so it doesn't require any special connection phase:

```bash
curl -s "http://localhost:9308/search"
```

<!-- intro -->

##### 通过[PHP 客户端](https://github.com/manticoresoftware/manticoresearch-php)进行连接：

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
##### 通过 Docker 连接
如果你熟悉 Docker，可以使用 Manticore 的[官方 Docker 镜像](https://github.com/manticoresoftware/docker) 来运行 Manticore。以下是通过 MySQL 连接到 Manticore 的 Docker 容器的方法：
<!-- request docker -->
运行 Manticore 容器并使用内置的 MySQL 客户端连接到节点。

```bash
docker run -e EXTRA=1 --name manticore -d manticoresearch/manticore && docker exec -it manticore mysql
```
<!-- end -->
<!-- proofread -->
