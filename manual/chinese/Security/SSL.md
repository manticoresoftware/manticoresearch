# SSL

在许多情况下，你可能希望加密客户端和服务器之间的流量。为此，可以指定服务器应使用[HTTPS协议](../Server_settings/Searchd.md#listen)而不是HTTP。

<!-- example CA 1 -->

要启用HTTPS，至少应在配置的[searchd](../Server_settings/Searchd.md)部分设置以下两个指令，并且至少应有一个监听器设置为`https`

* [ssl_cert](../Server_settings/Searchd.md#ssl_cert) 证书文件
* [ssl_key](../Server_settings/Searchd.md#ssl_key) 密钥文件

此外，还可以在以下位置指定证书颁发机构的证书（即根证书）：

* [ssl_ca](../Server_settings/Searchd.md#ssl_ca) 证书颁发机构的证书文件


<!-- intro -->
##### 使用CA：

<!-- request with CA -->
带有CA的示例：

```ini
ssl_ca = ca-cert.pem
ssl_cert = server-cert.pem
ssl_key = server-key.pem
```

<!-- request without CA -->
没有CA的示例：

```ini
ssl_cert = server-cert.pem
ssl_key = server-key.pem
```
<!-- end -->

## 生成SSL文件

这些步骤将帮助您使用`openssl`工具生成SSL证书。

服务器可以使用证书颁发机构来验证证书签名，但也可以仅使用私钥和证书（无需CA证书）进行工作。

#### 生成CA密钥

```bash
openssl genrsa 2048 > ca-key.pem
```

#### 从CA密钥生成CA证书

要从私钥生成自签名CA（根）证书（请确保填写至少“通用名称”），请使用以下命令：

```bash
openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca-cert.pem
```

#### 服务器证书

服务器使用服务器证书来保护与客户端的通信。要生成证书请求和服务器私钥（请确保填写至少“通用名称”，并且它不同于根证书的通用名称），执行以下命令：

```bash
openssl req -newkey rsa:2048 -days 365 -nodes -keyout server-key.pem -out server-req.pem
openssl rsa -in server-key.pem -out server-key.pem
openssl x509 -req -in server-req.pem -days 365 -CA ca-cert.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
```

完成后，可以通过运行以下命令来验证密钥和证书文件是否正确生成：

```bash
openssl verify -CAfile ca-cert.pem server-cert.pem
```

## 安全连接行为

当您的SSL配置有效时，以下功能可用：

* 可以通过HTTPS连接到多协议端口（当未指定[监听类型](../Server_settings/Searchd.md#listen)时），并运行查询。请求和响应都将被SSL加密。
* 可以通过HTTP连接到专用的`https`端口并运行查询。连接将被安全地保护（尝试通过纯HTTP连接到此端口将收到400错误代码）。
* 可以使用安全连接通过MySQL客户端连接到MySQL端口。会话将被安全地保护。请注意，Linux `mysql`客户端默认尝试使用SSL，因此具有有效SSL配置的Manticore连接通常将是安全的。可以通过在连接后运行SQL 'status'命令来检查这一点。

如果由于任何原因（守护进程通过无法建立安全连接的事实检测到无效配置）SSL配置无效，则除了无效配置之外，还可能存在其他原因，例如根本无法加载适当的SSL库。在这种情况下，以下内容将不起作用或将以非安全方式工作：

* 不能通过HTTPS连接到多协议端口。连接将被断开。
* 不能连接到专用的`https`端口。HTTPS连接将被断开
* 通过MySQL客户端连接到`mysql`端口将不支持SSL加密。如果客户端需要SSL，连接将失败。如果不需要SSL，它将使用纯MySQL或压缩连接。

### 警告：

* 二进制API连接（如来自旧客户端或守护进程间主代理通信的连接）不受保护。
* 复制的SSL需要单独设置。但是，由于复制的SST阶段是通过二进制API连接完成的，因此也不受保护。
* 您仍然可以使用任何外部代理（例如SSH隧道）来保护您的连接。
<!-- proofread -->

