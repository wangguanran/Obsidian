# 热榜素材目录

这个目录用于保存每日热榜 / 趋势快照，以及基于快照生成的当日分析，作为 AI 日记的外部世界素材层。

## 目录结构

- `YYYY/MM/DD/`：某一天的热榜快照目录
- `YYYY/MM/DD/manifest.md`：当天抓取结果总表（成功 / 失败 / 备注）
- `YYYY/MM/DD/<source-id>.md`：某个榜单的人类可读快照
- `YYYY/MM/DD/analysis.md`：基于当天快照生成的分析摘要

示例：

- `00.素材/热榜/2026/03/28/manifest.md`
- `00.素材/热榜/2026/03/28/weibo-hot.md`
- `00.素材/热榜/2026/03/28/zhihu-hot.md`
- `00.素材/热榜/2026/03/28/github-trending.md`
- `00.素材/热榜/2026/03/28/ai-news-hot.md`
- `00.素材/热榜/2026/03/28/ai-tools-hot.md`
- `00.素材/热榜/2026/03/28/analysis.md`

## 当前有效来源

当前只保留以下 5 个有效来源，统一抓取前 **20 条**：

- `weibo-hot`：微博热搜
- `zhihu-hot`：知乎热榜
- `github-trending`：GitHub Trending
- `ai-news-hot`：AI 新闻热门（Google News AI RSS）
- `ai-tools-hot`：AI 工具热门（Hugging Face Trending API）

其余旧来源已在配置层禁用，不再参与抓取、归档与分析。

## 使用规则

- 每天 21:40 尝试抓取 `热榜源配置.json` 中所有 `enabled: true` 的来源。
- 抓取成功后会立即生成当天 `analysis.md`。
- 每天 21:50 运行 watchdog：先检查当天抓榜是否完整；如果不完整，会在热榜相关本地文件范围内执行一次受限修复 / 重试。
- 如果 watchdog 触发补抓且补抓成功，也会重新生成 `analysis.md`。
- 写日记时，只允许使用 `diarySource: true` 的来源作为正文素材。
- 如果某个平台当天抓取失败，必须明确记为 `unavailable` / `failed`，不能编造内容补齐。
