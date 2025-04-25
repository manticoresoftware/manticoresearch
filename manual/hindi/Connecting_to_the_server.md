# सर्वर से कनेक्ट करना

<!-- example connect -->
डिफ़ॉल्ट कॉन्फ़िगरेशन के साथ, मैन्टिकोर आपके कनेक्शनों की प्रतीक्षा कर रहा है:

  * MySQL क्लाइंट के लिए पोर्ट 9306
  * HTTP/HTTPS कनेक्शनों के लिए पोर्ट 9308
  * HTTP/HTTPS के लिए पोर्ट 9312, और अन्य मैन्टिकोर नोड्स और मैन्टिकोर बाइनरी एपीआई पर आधारित क्लाइंट्स से कनेक्शन

<!-- intro -->
##### MySQL के जरिए कनेक्ट करें:

<!-- request SQL -->
```bash
mysql -h0 -P9306
```

<!-- intro -->
##### HTTP पर JSON के जरिए कनेक्ट करें

<!-- request HTTP -->
HTTP एक स्टेटलेस प्रोटोकॉल है, इसलिए इसे किसी विशेष कनेक्शन चरण की आवश्यकता नहीं होती:

```bash
curl -s "http://localhost:9308/search"
```

<!-- intro -->
##### [PHP क्लाइंट](https://github.com/manticoresoftware/manticoresearch-php) के जरिए कनेक्ट करें:

<!-- request PHP -->
```php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new ManticoresearchClient($config);
```
<!-- intro -->
##### पायथन
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
##### जावास्क्रिप्ट
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
##### जावा
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
##### रस्ट
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
##### डॉकर के जरिए कनेक्ट करें
यदि आप डॉकर के साथ परिचित हैं, तो आप मैन्टिकोर के [आधिकारिक डॉकर इमेज](https://github.com/manticoresoftware/docker) का उपयोग करके मैन्टिकोर को चला सकते हैं। यहाँ आप MySQL के माध्यम से मैन्टिकोर के डॉकर से कैसे कनेक्ट कर सकते हैं:
<!-- request docker -->
मैन्टिकोर कंटेनर चलाएँ और नोड से कनेक्ट करने के लिए बिल्ट-इन MySQL क्लाइंट का उपयोग करें।
```bash
# सर्वर से कनेक्ट करना

<!-- उदाहरण कनेक्ट करें -->
डिफ़ॉल्ट कॉन्फ़िगरेशन के साथ, माइन्टिकोर आपके कनेक्शन की प्रतीक्षा कर रहा है:

  * MySQL क्लाइंट के लिए पोर्ट 9306
  * HTTP/HTTPS कनेक्शनों के लिए पोर्ट 9308
  * HTTP/HTTPS और अन्य माइन्टिकोर नोड्स और माइन्टिकोर बाइनरी API पर आधारित क्लाइंट से कनेक्शनों के लिए पोर्ट 9312

<!-- परिचय -->
##### MySQL के माध्यम से कनेक्ट करें:

<!-- अनुरोध SQL -->
```bash
mysql -h0 -P9306
```

<!-- परिचय -->
##### JSON के माध्यम से HTTP पर कनेक्ट करें

<!-- अनुरोध HTTP -->
HTTP एक स्टेटलेस प्रोटोकॉल है, इसलिए इसे किसी विशेष कनेक्शन चरण की आवश्यकता नहीं है:

```bash
curl -s "http://localhost:9308/search"
```

<!-- परिचय -->
##### [PHP क्लाइंट](https://github.com/manticoresoftware/manticoresearch-php) के माध्यम से कनेक्ट करें:

<!-- अनुरोध PHP -->
```php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new \Manticoresearch\Client($config);
```
<!-- परिचय -->
##### पायथन
<!-- अनुरोध पायथन -->
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
<!-- परिचय -->
##### जावास्क्रिप्ट
<!-- अनुरोध जावास्क्रिप्ट -->
```javascript
var Manticoresearch = require('manticoresearch');
var client= new Manticoresearch.ApiClient()
client.basePath="http://127.0.0.1:9308";
indexApi = new Manticoresearch.IndexApi(client);
searchApi = new Manticoresearch.SearchApi(client);
utilsApi = new Manticoresearch.UtilsApi(client);
```

<!-- परिचय -->
##### जावा
<!-- अनुरोध जावा -->
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

<!-- परिचय -->
##### C#
<!-- अनुरोध C# -->
```clike
using ManticoreSearch.Client;
using ManticoreSearch.Api;
using ManticoreSearch.Model;

string basePath = "http://127.0.0.1:9308";
IndexApi indexApi = new IndexApi(basePath);
SearchApi searchApi = new UtilsApi(basePath);
UtilsApi utilsApi = new UtilsApi(basePath);
```

<!-- परिचय -->
##### डॉकर के माध्यम से कनेक्ट करें
यदि आप डॉकर से परिचित हैं, तो आप माइन्टिकोर का [आधिकारिक डॉकर चित्र](https://github.com/manticoresoftware/docker) का उपयोग करके माइन्टिकोर चलाने के लिए कनेक्ट कर सकते हैं। यहां बताया गया है कि आप MySQL के माध्यम से माइन्टिकोर के डॉकर से कैसे कनेक्ट कर सकते हैं:
<!-- अनुरोध डॉकर -->
माइन्टिकोर कंटेनर चलाएँ और नोड से कनेक्ट करने के लिए इन-बिल्ट MySQL क्लाइंट का उपयोग करें।
```bash
docker run --name manticore -d manticoresearch/manticore && docker exec -it manticore mysql
```
<!-- अंत -->
<!-- प्रूफरीड -->
