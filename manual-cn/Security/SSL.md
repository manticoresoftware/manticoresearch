# SSL

在许多情况下，您可能希望加密客户端与服务器之间的流量。为此，您可以指定服务器应使用 [HTTPS 协议](../Server_settings/Searchd.md#listen) 而不是 HTTP。

<!-- example CA 1 -->

要启用 HTTPS，至少需要在配置的 [searchd](../Server_settings/Searchd.md) 部分中设置以下两个指令，并且应至少有一个 [listener](../Server_settings/Searchd.md#listen) 设置为 `https`：

- [ssl_cert](../Server_settings/Searchd.md#ssl_cert) 证书文件
- [ssl_key](../Server_settings/Searchd.md#ssl_key) 密钥文件

除此之外，您还可以指定证书颁发机构的证书（又称根证书），使用以下指令：

- [ssl_ca](../Server_settings/Searchd.md#ssl_ca) 证书颁发机构的证书文件


<!-- intro -->
##### 使用CA:

<!-- request with CA -->
使用CA示例:

```ini
ssl_ca = ca-cert.pem
ssl_cert = server-cert.pem
ssl_key = server-key.pem
```

<!-- request without CA -->
不使用CA示例:

```ini
ssl_cert = server-cert.pem
ssl_key = server-key.pem
```
<!-- end -->

## 生成 SSL 文件

以下步骤将帮助您使用 'openssl' 工具生成 SSL 证书。

服务器可以使用证书颁发机构（CA）来验证证书的签名，但它也可以仅使用私钥和证书（无需 CA 证书）工作。

#### 生成 CA 密钥

```bash
openssl genrsa 2048 > ca-key.pem
```

#### 通过 CA 密钥生成 CA 证书

要从私钥生成自签名的 CA（根）证书（确保至少填写“Common Name”），请使用以下命令：

```bash
openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca-cert.pem
```

#### 服务器证书

服务器使用服务器证书来与客户端建立安全通信。要生成证书请求和服务器私钥（确保填写“Common Name”，且与根证书的 "Common Name" 不同），请执行以下命令：

```bash
openssl req -newkey rsa:2048 -days 365 -nodes -keyout server-key.pem -out server-req.pem
openssl rsa -in server-key.pem -out server-key.pem
openssl x509 -req -in server-req.pem -days 365 -CA ca-cert.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
```

完成后，您可以运行以下命令来验证密钥和证书文件是否生成正确：

```bash
openssl verify -CAfile ca-cert.pem server-cert.pem
```

## 安全连接行为

当您的 SSL 配置有效时，将启用以下功能：

- 您可以通过 HTTPS 连接到多协议端口（当未指定 [listener type](../Server_settings/Searchd.md#listen) 时）并运行查询，请求和响应都将被 SSL 加密。
- 您可以通过 HTTP 连接到专用的 https 端口并运行查询，连接将是安全的（尝试通过普通 HTTP 连接此端口将返回 400 错误）。
- 您可以使用 MySQL 客户端通过 MySQL 端口使用安全连接进行连接，连接会被加密。请注意，Linux 的 `mysql` 客户端默认尝试使用 SSL，因此在 Manticore 上进行典型连接时，SSL 配置有效的情况下连接很可能是安全的。您可以通过连接后运行 SQL 'status' 命令进行检查。

如果由于任何原因 SSL 配置无效（例如守护进程检测到无法建立安全连接），可能是由于无效配置或无法加载适当的 SSL 库。在这种情况下，以下内容将无法正常工作或以非安全方式工作：

- 您无法通过 HTTPS 连接到多协议端口，连接将被中断。
- 您无法连接到专用的 `https` 端口，HTTPS 连接将被中断。
- 无法通过 MySQL 客户端在 MySQL 端口上使用 SSL 安全连接。如果客户端需要 SSL，连接将失败；如果不需要 SSL，则将使用普通 MySQL 或压缩连接。

### 注意事项：

- 二进制 API 连接（如旧客户端或守护进程之间的主代理通信）未加密。
- 复制的 SSL 需要单独设置。此外，由于复制的 SST 阶段是通过二进制 API 连接完成的，该连接也没有加密。
- 您仍然可以使用任何外部代理（例如 SSH 隧道）来保护您的连接。

<!-- proofread -->