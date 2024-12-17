# Manticore Buddy

**Manticore Buddy** 是用 PHP 编写的 Manticore Search 辅助程序，可帮助完成各种任务。典型的流程是，在向用户返回错误之前，Manticore Search 会询问 Buddy 是否可以处理守护进程的该问题。Buddy 的 PHP 代码使得实现不需要极高性能的高级功能变得容易。

要更深入地了解 Buddy，请查看以下文章：
- [介绍 Buddy：Manticore Search 的 PHP 辅助程序](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)
- [Manticore Buddy：挑战和解决方案](https://manticoresearch.com/blog/manticoresearch-buddy-challenges-and-solutions/)
- [Manticore Buddy：可插拔设计](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/)
- [Manticore Buddy GitHub 代码仓库](https://github.com/manticoresoftware/manticoresearch-buddy)

## Manticore Buddy 安装

如果您按照上述说明或 [网站](https://manticoresearch.com/install) 上的说明进行安装，则无需担心安装或启动 Manticore Buddy：当您安装 `manticore-extra` 包时，它会自动安装，并且 Manticore Search 在启动时会自动启动它。

## 禁用 Manticore Buddy

要禁用 Manticore Buddy，请使用 buddy_path 设置。
