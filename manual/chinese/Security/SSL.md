# SSL

在许多情况下，您可能想要加密客户端与服务器之间的流量。为此，您可以指定服务器应使用[HTTPS协议](../Server_settings/Searchd.md#listen)而不是HTTP。

<!-- example CA 1 -->

要启用HTTPS，至少应在配置的[searchd](../Server_settings/Searchd.md)部分设置以下两个指令，并且应至少设置一个[listener](../Server_settings/Searchd.md#listen)为`https`

* [ssl_cert](../Server_settings/Searchd.md#ssl_cert)证书文件
* [ssl_key](../Server_settings/Searchd.md#ssl_key)密钥文件

此外，您可以在以下位置指定证书颁发机构的证书（即根证书）：

* [ssl_ca](../Server_settings/Searchd.md#ssl_ca)证书颁发机构的证书文件


<!-- intro -->
##### 带CA：

<!-- request with CA -->
带CA的示例：

```ini
ssl_ca = ca-cert.pem
ssl_cert = server-cert.pem
ssl_key = server-key.pem
```

<!-- request without CA -->
无CA的示例：

```ini
ssl_cert = server-cert.pem
ssl_key = server-key.pem
```
<!-- end -->

## 生成SSL文件

这些步骤将帮助您使用'openssl'工具生成SSL证书。

服务器可以使用证书颁发机构来验证证书的签名，但它也可以仅使用私钥和证书（不带CA证书）工作。

#### 生成CA密钥

```bash
openssl genrsa 2048 > ca-key.pem
```

#### 从CA密钥生成CA证书

要根据私钥生成自签名的CA（根）证书（确保至少填写“通用名称”），使用以下命令：

```bash
openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca-cert.pem
```

#### 服务器证书

服务器使用服务器证书与客户端进行安全通信。要生成证书请求和服务器私钥（确保填写至少“通用名称”，并且它与根证书的通用名称不同），执行以下命令：

```bash
openssl req -newkey rsa:2048 -days 365 -nodes -keyout server-key.pem -out server-req.pem
openssl rsa -in server-key.pem -out server-key.pem
openssl x509 -req -in server-req.pem -days 365 -CA ca-cert.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
```

完成后，您可以运行以下命令来验证密钥和证书文件是否正确生成：

```bash
openssl verify -CAfile ca-cert.pem server-cert.pem
```

## 安全连接行为

当您的SSL配置有效时，以下功能可用：

 * 您可以通过HTTPS连接到多协议端口（当未指定任何[listener类型](../Server_settings/Searchd.md#listen)时）并运行查询。请求和响应都将进行SSL加密。
 * 您可以通过HTTP连接到专用https端口并运行查询。该连接将被保护（尝试通过普通HTTP连接到该端口将被400错误代码拒绝）。
 * 您可以使用MySQL客户端通过安全连接连接到MySQL端口。会话将是安全的。请注意，Linux `mysql`客户端默认尝试使用SSL，因此，使用有效SSL配置的Manticore的典型连接很可能是安全的。您可以在连接后运行SQL 'status'命令来检查。

如果您的SSL配置出于任何原因无效（守护进程通过无法建立安全连接来检测这一点），除了配置无效之外，可能还有其他原因，例如根本无法加载适当的SSL库。在这种情况下，以下内容将无法使用或将以不安全的方式使用：

* 您无法通过HTTPS连接到多协议端口。连接将被断开。
* 您无法连接到专用`https`端口。HTTPS连接将被断开。
* 通过MySQL客户端连接到`mysql`端口将不支持SSL加密。如果客户端要求SSL，则连接将失败。如果不要求SSL，将使用普通MySQL或压缩连接。

### 注意：

* 二进制API连接（例如来自旧客户端的连接或守护进程之间的主-代理通信）是不安全的。
* 复制的SSL需要单独设置。然而，由于复制的SST阶段是通过二进制API连接完成的，因此它也不安全。
* 您仍然可以使用任何外部代理（例如SSH隧道）来保护您的连接。
<!-- proofread -->
