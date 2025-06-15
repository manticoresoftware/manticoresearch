# 脚本化配置

Manticore 配置支持 shebang 语法，允许配置以编程语言编写并在加载时解释。这使得动态设置成为可能，例如通过查询数据库表生成表，根据外部因素修改设置，或包含包含表和源声明的外部文件。

配置文件由声明的解释器解析，输出作为实际配置使用。每次读取配置时都会发生此操作，而不仅仅是在 searchd 启动时。

注意：此功能在 Windows 平台不可用。

在以下示例中，使用 PHP 创建多个不同名称的表，并扫描特定文件夹中的文件以获取额外的表声明：

```php
#!/usr/bin/php
...
<?php for ($i=1; $i<=6; $i++) { ?>
table test_<?=$i?> {
  type = rt
  path = /var/lib/manticore/data/test_<?=$i?>
  rt_field = subject
  ...
 }
 <?php } ?>
 ...

 <?php
 $confd_folder='/etc/manticore.conf.d/';
 $files = scandir($confd_folder);
 foreach($files as $file)
 {
         if(($file == '.') || ($file =='..'))
         {} else {
                 $fp = new SplFileInfo($confd_folder.$file);
                 if('conf' == $fp->getExtension()){
                         include ($confd_folder.$file);
                 }
         }
 }
 ?>
```
<!-- proofread -->

