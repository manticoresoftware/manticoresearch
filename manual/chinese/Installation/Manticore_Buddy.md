# Manticore Buddy

Manticore Buddy 是一个为 Manticore Search 提供的旁车，使用 PHP 编写，帮助处理各种任务。典型的工作流程是，在向用户返回错误之前，Manticore Search 会询问 Buddy 是否可以为守护进程处理这个问题。Buddy 的 PHP 代码使实现不需要极高性能的高级功能变得简单。

要深入了解 Buddy，请查看以下文章：
- [介绍 Buddy：Manticore Search 的 PHP 旁车](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)
- [Manticore Buddy：挑战和解决方案](https://manticoresearch.com/blog/manticoresearch-buddy-challenges-and-solutions/)
- [Manticore Buddy：可插拔设计](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/)
- [Manticore Buddy GitHub 仓库](https://github.com/manticoresoftware/manticoresearch-buddy)

## Manticore Buddy 安装

如果您按照上面的安装说明或[在网站上](https://manticoresearch.com/install)进行操作，您不必担心安装或启动 Manticore Buddy：它在您安装 `manticore-extra` 包时会自动安装，并且 Manticore Search 会在启动时自动启动它。

## 禁用 Manticore Buddy

要禁用 Manticore Buddy，请使用 [buddy_path](../Server_settings/Searchd.md#buddy_path) 设置。
