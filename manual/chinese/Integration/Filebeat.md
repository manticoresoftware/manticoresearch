### Filebeat 7.17、8.0、8.1 的配置
> **重要**：Filebeat 版本 7.17.0、8.0.0 和 8.1.0 在使用 glibc 2.35+（用于 Ubuntu 22.04 及更高版本发行版）时存在已知问题。这些版本可能会因“Fatal glibc error: rseq registration failed”而崩溃。为了解决此问题，请按如下所示添加 `seccomp` 配置。

```yaml
  allow_older_versions: true  # 8.1 需要

# 解决 glibc 2.35+ 兼容性问题（Ubuntu 22.04+）
seccomp:
  default_action: allow
  syscalls:
    - action: allow
      names:
        - rseq
**参考资料**: [Issue #30576](https://github.com/elastic/beats/issues/30576), [PR #30620](https://github.com/elastic/beats/pull/30620)

