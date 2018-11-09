# 增加中日韩分词的Manticoresearch
## 为什么要对中日韩分词
Manticore(Sphinx) 是非常棒的全文搜索引擎。它可以支持几乎所有语言。中文、日文、韩文可以通过ngram的tokenizer进行支持。
当Ngram=1时，中文（日文、韩文）被分解成一个个的单字，就像把英文分解成一个个字母那样。这会导致每个单字的索引很长，搜索效率下降，同时搜索结果习惯性比较差。
当Ngram=2或更大时，会产生很多无意义的“词”，导致索引的字典、索引文件等非常大，同时也影响搜索速度。
所以，给中日韩分词是很有必要的。
Manticoresearch 从Sphinx search 2.x 继承而来，并做了[性能优化](https://manticoresearch.com/2018/03/18/sphinx-3-vs-manticore-performance-benchmark/). 并且 Sphinxsearch 3.x 开始，目前[不再开源](http://sphinxsearch.com/downloads/). 因此，我选择了Manticoresearch 来添加中日韩分词。

我会尽力与Manticoresearch的主分支保持同步。

## 算法实现
算法基于字典，具体是cedar的实现的双数组trie。cedar是C++实现的高效双数组trie，也是分词字典的最佳之选。cedar的协议是GNU GPLv2, LGPLv2.1, and BSD;或者email联系作者所要其它协议。

通过最小匹配（而非单字）来匹配字典和字符串，把字符串分割成最短（而非单字）的词。如果遇到处理不了的歧义时，以单字做词。这样的目的是，保证搜索时能找到这些内容而不丢失。

## 安装
跟Manticoresearch一样，具体看[官方文档](https://docs.manticoresearch.com/latest/html/)。

## 使用方法
**1. 准备词表**
把所有词写到一个txt文件，一行一个词，如下所示：
```text
# words.txt
中文
中国語
중국어
```

**2. 创建字典**
成功编译代码后，就会得到创建字典的可执行程序<code>make_segdictionary</code>. 然后执行命令:
```shell
./make_segdictionary words.txt words.dict
```
这样就得到了字典文件: words.dict

**3. 配置索引**
只需在配置文件的 <code>index {...} </code> 添加一行即可：
```config
index {
    ...
    seg_dictionary = path-to-your-segmentation-words-dictionary
    ...
}
```
**提醒:** 分词对批量索引和实时索引都起作用。

