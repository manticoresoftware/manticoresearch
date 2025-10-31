### Конфигурация для Filebeat 7.17, 8.0, 8.1
> **Важно**: В версиях Filebeat 7.17.0, 8.0.0 и 8.1.0 известна проблема с glibc 2.35+ (используется в Ubuntu 22.04 и новее). Эти версии могут аварийно завершаться с ошибкой "Fatal glibc error: rseq registration failed". Чтобы исправить это, добавьте конфигурацию `seccomp`, как показано ниже.

```yaml
  allow_older_versions: true  # Требуется для 8.1

# Исправление для совместимости с glibc 2.35+ (Ubuntu 22.04+)
seccomp:
  default_action: allow
  syscalls:
    - action: allow
      names:
        - rseq
**Ссылки**: [Issue #30576](https://github.com/elastic/beats/issues/30576), [PR #30620](https://github.com/elastic/beats/pull/30620)

