# Manticore Buddy

Manticore Buddy — это сайдкар для Manticore Search, написанный на PHP, который помогает с различными задачами. Типичный рабочий процесс таков: прежде чем вернуть ошибку пользователю, Manticore Search спрашивает Buddy, может ли он справиться с проблемой для демона. PHP-код Buddy облегчает реализацию высокоуровневых функций, которые не требуют чрезвычайно высокой производительности.

Для более глубокого понимания Buddy ознакомьтесь с этими статьями:
- [Introducing Buddy: the PHP sidecar for Manticore Search](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)
- [Manticore Buddy: challenges and solutions](https://manticoresearch.com/blog/manticoresearch-buddy-challenges-and-solutions/)
- [Manticore Buddy: pluggable design](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/)
- [Manticore Buddy GitHub repository](https://github.com/manticoresoftware/manticoresearch-buddy)

## Установка Manticore Buddy

Если вы следуете инструкциям по установке выше или [на сайте](https://manticoresearch.com/install), вам не нужно беспокоиться об установке или запуске Manticore Buddy: он устанавливается автоматически при установке пакета `manticore-extra`, и Manticore Search запускает его автоматически при старте.

## Отключение Manticore Buddy

Чтобы отключить Manticore Buddy, пожалуйста, используйте параметр [buddy_path](../Server_settings/Searchd.md#buddy_path).

