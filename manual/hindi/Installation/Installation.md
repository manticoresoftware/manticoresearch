# स्थापना

<!-- उदाहरण स्थापना विस्तारित -->

<!-- RHEL, Centos, Alma, Amazon, Oracle का अनुरोध -->

``` bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-extra
```

यदि आप पुरानी संस्करण से अपग्रेड कर रहे हैं, तो यह अनुशंसा की जाती है कि पहले अपने पुराने पैकेज हटा दें ताकि अपडेट की गई पैकेज संरचना के कारण संकल्पनाओं से बचा जा सके:
```bash
sudo yum --setopt=tsflags=noscripts remove manticore*
```
यह आपकी जानकारी को नहीं हटाएगा। यदि आपने कॉन्फ़िगरेशन फ़ाइल में परिवर्तन किए हैं, तो इसे `/etc/manticoresearch/manticore.conf.rpmsave` में सुरक्षित रखा जाएगा।

यदि आप अलग पैकेज की तलाश कर रहे हैं, तो कृपया उन्हें [यहां](https://manticoresearch.com/install/#separate-packages) पाएं।

स्थापना के अधिक विवरण के लिए, [नीचे](../Installation/RHEL_and_Centos.md) देखें।

<!-- Debian, Ubuntu, Mint का अनुरोध -->

``` bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-extra
```

यदि आप एक पुराने संस्करण से Manticore 6 पर अपग्रेड कर रहे हैं, तो यह अनुशंसा की जाती है कि पहले अपने पुराने पैकेज हटा दें ताकि अपडेट की गई पैकेज संरचना के कारण संकल्पनाओं से बचा जा सके:
```bash
sudo apt remove manticore*
```
यह आपकी जानकारी या कॉन्फ़िगरेशन फ़ाइल को नहीं हटाएगा।

यदि आप अलग पैकेज की तलाश कर रहे हैं, तो कृपया उन्हें [यहां](https://manticoresearch.com/install/#separate-packages) पाएं।

स्थापना के अधिक विवरण के लिए, [नीचे](../Installation/Debian_and_Ubuntu.md) देखें।

<!-- MacOS का अनुरोध -->

``` bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

कृपया स्थापना के बारे में अधिक विवरण [नीचे](../Installation/MacOS.md) पाएं।

<!-- Windows का अनुरोध -->

1. [Manticore Search Installer](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.4-230314-1a3a4ea82-x64.exe) डाउनलोड करें और इसे चलाएं। स्थापना निर्देशों का पालन करें।
2. स्थापित करने के लिए निर्देशिका चुनें।
3. उन घटकों का चयन करें जिन्हें आप स्थापित करना चाहते हैं। हम सभी को स्थापित करने की सिफारिश करते हैं।
4. Manticore एक पूर्व कॉन्फ़िगर किया गया `manticore.conf` फ़ाइल के साथ आता है [RT मोड](https://manual.manticoresearch.com/Read_this_first.md#Real-time-mode-vs-plain-mode) में। कोई अतिरिक्त कॉन्फ़िगरेशन की आवश्यकता नहीं है।

स्थापना के अधिक विवरण के लिए, [नीचे](../Installation/Windows.md#Installing-Manticore-on-Windows) देखें।

<!-- Docker का अनुरोध -->

एक सैंडबॉक्स के लिए एक-पंक्ति (उत्पादन उपयोग के लिए अनुशंसित नहीं):
``` bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Manticore डॉकर स्टार्ट होने का इंतजार कर रहा है। अगली बार इसे तेजी से शुरू करने के लिए data_dir को मैप करने पर विचार करें" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

यह Manticore कंटेनर चलाएगा और इसके बूट होने की प्रतीक्षा करेगा। शुरू होने के बाद, यह एक MySQL क्लाइंट सत्र लॉन्च करता है। जब आप MySQL क्लाइंट से बाहर निकलते हैं, तो Manticore कंटेनर संचालन बंद कर देता है और हटा दिया जाता है, बिना किसी स्टोर किए डेटा के पीछे। लाइव उत्पादन सेटिंग में Manticore का उपयोग करने के तरीके के लिए, निम्नलिखित अनुभाग को देखें।

उत्पादन उपयोग के लिए:
``` bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

यह सेटअप Manticore कॉलम लाइब्रेरी और Manticore Buddy को सक्षम करेगा, और Manticore को MySQL कनेक्शनों के लिए 9306 और सभी अन्य कनेक्शनों के लिए 9308 पोर्ट पर चलाएगा, `./data/` को नामित डेटा निर्देशिका के रूप में उपयोग करते हुए।

उत्पादन उपयोग के बारे में अधिक पढ़ें [दस्तावेज़ में](https://github.com/manticoresoftware/docker#production-use)।

<!-- Kubernetes का अनुरोध -->

``` bash
helm repo add manticoresearch https://helm.manticoresearch.com
# यदि आवश्यक हो तो values.yaml अपडेट करें
helm install manticore -n manticore --create-namespace manticoresearch/manticoresearch
```

आप Helm चार्ट स्थापित करने के बारे में अधिक जानकारी [दस्तावेज़ में](https://github.com/manticoresoftware/manticoresearch-helm#installation) पा सकते हैं।

<!-- अंत -->

<!-- प्रूफरीड -->
