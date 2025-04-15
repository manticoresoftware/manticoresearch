# समर्थित भाषाएँ

मैन्टिकोर कई भाषाओं का एक विस्तृत रेंज का समर्थन करता है, जिसमें अधिकांश भाषाओं के लिए बुनियादी समर्थन `charset_table = non_cont` के माध्यम से सक्षम है (जो कि डिफ़ॉल्ट मान है)। `non_cjk` विकल्प, जो `non_cont` का एक उपनाम है, का भी उपयोग किया जा सकता है: `charset_table = non_cjk`.

कई भाषाओं के लिए, मैन्टिकोर एक [स्टॉपवर्ड्स](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) फ़ाइल प्रदान करता है जिसका उपयोग खोज प्रासंगिकता में सुधार करने के लिए किया जा सकता है।

इसके अलावा, कुछ भाषाओं के लिए उन्नत रूपविज्ञान उपलब्ध है जो शब्दकोश-आधारित लेमाटाइजेशन या स्टेमिंग एल्गोरिदम का उपयोग करके खोज प्रासंगिकता को महत्वपूर्ण रूप से सुधार सकती है।

नीचे दी गई तालिका सभी समर्थित भाषाओं की सूची देती है और यह इंगित करती है कि कैसे सक्षम किया जाए:
* बुनियादी समर्थन (स्तंभ "समर्थित")
* स्टॉपवर्ड्स (स्तंभ "स्टॉपवर्ड्स फ़ाइल का नाम")
* उन्नत रूपविज्ञान (स्तंभ "उन्नत रूपविज्ञान")

| भाषा | समर्थित | स्टॉपवर्ड्स फ़ाइल का नाम | उन्नत रूपविज्ञान | नोट्स |
| - | - | - | - | - |
| अफ्रीकांस | charset_table=non_cont | af | - | |
| अरबी | charset_table=non_cont | ar | morphology=stem_ar (अरबी स्टेमर); morphology=libstemmer_ar  | |
| आर्मेनियाई | charset_table=non_cont | hy | - | |
| असमिया | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| बास्क | charset_table=non_cont | eu | - | |
| बंगाली | charset_table=non_cont | bn | - | |
| विष्णुप्रिय | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| बूहिद | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| बल्गेरियाई | charset_table=non_cont | bg | - | |
| कैटालन | charset_table=non_cont | ca | morphology=libstemmer_ca | |
| चीनी [ICU](https://icu.unicode.org/) का उपयोग करते हुए | charset_table=chinese | zh | morphology=icu_chinese | ngrams का उपयोग करने से अधिक सटीक |
| चीनी [Jieba](https://github.com/fxsjy/jieba) का उपयोग करते हुए | charset_table=chinese | zh | morphology=jieba_chinese, पैकेज की आवश्यकता `manticore-language-packs` | ngrams का उपयोग करने से अधिक सटीक |
| ngrams का उपयोग करते हुए चीनी | ngram_chars=chinese | zh | ngram_chars=1 | तेज़ अनुक्रमण, लेकिन खोज प्रदर्शन उतना अच्छा नहीं हो सकता |
| क्रोएशियन | charset_table=non_cont | hr | - | |
| कुर्दिश | charset_table=non_cont | ckb | - | |
| चेक | charset_table=non_cont | cz | morphology=stem_cz (चेक स्टेमर) | |
| डेनिश | charset_table=non_cont | da | morphology=libstemmer_da | |
| डच | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| अंग्रेज़ी | charset_table=non_cont | en | morphology=lemmatize_en (एकल मूल रूप); morphology=lemmatize_en_all (सभी मूल रूप); morphology=stem_en (पोर्टर का अंग्रेज़ी स्टेमर); morphology=stem_enru (पोर्टर का अंग्रेज़ी और रूसी स्टेमर); morphology=libstemmer_en (libstemmer से अंग्रेज़ी)  | |
| एस्पेरांतो | charset_table=non_cont | eo | - | |
| एस्टोनियाई | charset_table=non_cont | et | - | |
| फ़िनिश | charset_table=non_cont | fi | morphology=libstemmer_fi | |
| फ़्रेंच | charset_table=non_cont | fr | morphology=libstemmer_fr | |
| गालिशियन | charset_table=non_cont | gl | - | |
| गारो | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| जर्मन | charset_table=non_cont | de | morphology=lemmatize_de (एकल मूल रूप); morphology=lemmatize_de_all (सभी मूल रूप); morphology=libstemmer_de | |
| ग्रीक | charset_table=non_cont | el | morphology=libstemmer_el | |
| हिब्रू | charset_table=non_cont | he | - | |
| हिंदी | charset_table=non_cont | hi | morphology=libstemmer_hi | |
| मोंग | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| हो | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| हंगेरियन | charset_table=non_cont | hu | morphology=libstemmer_hu | |
| इंडोनेशियाई | charset_table=non_cont | id | morphology=libstemmer_id | |
| आयरिश | charset_table=non_cont | ga | morphology=libstemmer_ga | |
| इतालवी | charset_table=non_cont | it | morphology=libstemmer_it | |
| जापानी | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | ngram-आधारित विभाजन की आवश्यकता |
| कोमी | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| कोरियाई | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | ngram-आधारित विभाजन की आवश्यकता  |
| बड़े फूलदार मियाओ | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| लैटिन | charset_table=non_cont | la | - | |
| लात्वियाई | charset_table=non_cont | lv | - | |
| लिथुआनियाई | charset_table=non_cont | lt | morphology=libstemmer_lt | |
| माबा | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| मैथिली | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| मराठी | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| मराठी | charset_table=non_cont | mr | - | |
| मेंडे | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| म्रु | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| मायने | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| नेपाली | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | morphology=libstemmer_ne | |
| न्गाम्बे | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| नॉर्वेजियन | charset_table=non_cont | no | morphology=libstemmer_no | |
| उड़िया | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| फारसी | charset_table=non_cont | fa | - | |
| पोलिश | charset_table=non_cont | pl | - | |
| पुर्तगाली | charset_table=non_cont | pt | morphology=libstemmer_pt | |
| रोमानियाई | charset_table=non_cont | ro | morphology=libstemmer_ro | |
| रूसी | charset_table=non_cont | ru | morphology=lemmatize_ru (एकल मूल रूप); morphology=lemmatize_ru_all (सभी मूल रूप); morphology=stem_ru (पोर्टर का रूसी स्टेमर); morphology=stem_enru (पोर्टर का अंग्रेज़ी और रूसी स्टेमर); morphology=libstemmer_ru (libstemmer से) | |
| संताली | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| सिंधी | charset_table को मैन्युअल रूप से निर्दिष्ट करें | - | - | |
| स्लोवाक | charset_table=non_cont | sk | - | |
| स्लोवेनियाई | charset_table=non_cont | sl | - | |
| सोमाली | charset_table=non_cont | so | - | |
| सोथो | charset_table=non_cont | st | - | |
| स्पेनिश | charset_table=non_cont | es | morphology=libstemmer_es | |
| स्वाहिली | charset_table=non_cont | sw | - | |
| स्वीडिश | charset_table=non_cont | sv | morphology=libstemmer_sv | |
| সিলেটি | specify charset_table manually | - | - | |
| तमिल | specify charset_table manually | - | morphology=libstemmer_ta | |
| थाई | charset_table=thai | th | - | |
| तुर्की | charset_table=non_cont | tr | morphology=libstemmer_tr | |
| यूक्रेनी | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491   | - | morphology=lemmatize_uk_all | Requires [installation](../../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) of UK lemmatizer |
| यॉरूबा | charset_table=non_cont | yo | - | |
| ज़ुलू | charset_table=non_cont | zu | - |  |
<!-- proofread -->
