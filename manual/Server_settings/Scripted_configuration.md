# Scripted configuration

Manticore configuration supports shebang syntax, allowing the configuration to be written in a programming language and interpreted at loading. This enables dynamic settings, such as generating tables by querying a database table, modifying settings based on external factors, or including external files containing table and source declarations.

The configuration file is parsed by the declared interpreter, and the output is used as the actual configuration. This occurs each time the configuration is read, not only at searchd startup.

Note: This feature is not available on the Windows platform.

In the following example, PHP is used to create multiple tables with different names and to scan a specific folder for files containing extra table declarations:

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