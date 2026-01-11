# Manticore Buddy

Manticore Buddy — это сайдкар для Manticore Search, написанный на PHP, который помогает с различными задачами. Типичный рабочий процесс таков: перед тем как вернуть ошибку пользователю, Manticore Search спрашивает Buddy, может ли он обработать проблему для демона. PHP-код Buddy облегчает реализацию высокоуровневых функций, которые не требуют чрезвычайно высокой производительности.

Для более глубокого понимания Buddy ознакомьтесь с этими статьями:
- [Введение в Buddy: PHP-сайдкар для Manticore Search](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)
- [Manticore Buddy: задачи и решения](https://manticoresearch.com/blog/manticoresearch-buddy-challenges-and-solutions/)
- [Manticore Buddy: плагинная архитектура](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/)
- [Репозиторий Manticore Buddy на GitHub](https://github.com/manticoresoftware/manticoresearch-buddy)

## Установка Manticore Buddy

Если вы следуете инструкциям по установке выше или [на сайте](https://manticoresearch.com/install), вам не нужно беспокоиться об установке или запуске Manticore Buddy: он устанавливается автоматически при установке пакета `manticore-extra`, и Manticore Search автоматически запускает его при старте.

## Отключение Manticore Buddy

Чтобы отключить Manticore Buddy, используйте настройку [buddy_path](../Server_settings/Searchd.md#buddy_path).

