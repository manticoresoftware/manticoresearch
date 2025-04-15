# Manticore Buddy
Manticore Buddy — это сайдкар для Manticore Search, написанный на PHP, который помогает с различными задачами. Типичный рабочий процесс заключается в том, что перед тем, как вернуть ошибку пользователю, Manticore Search спрашивает Buddy, может ли он решить проблему для демона. PHP-код Buddy упрощает реализацию высокоуровневых функций, которые не требуют исключительно высокой производительности.
Для более глубокого понимания Buddy ознакомьтесь с этими статьями:
- [Представляем Buddy: PHP сайдкар для Manticore Search](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)
- [Manticore Buddy: проблемы и решения](https://manticoresearch.com/blog/manticoresearch-buddy-challenges-and-solutions/)
- [Manticore Buddy: модульный дизайн](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/)
- [Репозиторий Manticore Buddy на GitHub](https://github.com/manticoresoftware/manticoresearch-buddy)
## Установка Manticore Buddy
Если вы следуете инструкциям по установке выше или [на сайте](https://manticoresearch.com/install), вам не нужно беспокоиться об установке или запуске Manticore Buddy: он устанавливается автоматически, когда вы устанавливаете пакет `manticore-extra`, и Manticore Search запускает его автоматически при запуске.
## Отключение Manticore Buddy
Чтобы отключить Manticore Buddy, используйте настройку [buddy_path](../Server_settings/Searchd.md#buddy_path).






