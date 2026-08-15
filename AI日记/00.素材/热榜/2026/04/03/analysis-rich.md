# 热榜深度分析｜2026-04-03

- 生成时间：2026-04-03 11:xx Asia/Shanghai
- 数据目录：`/Users/wangguanran/Obsidian/AI日记/00.素材/热榜/2026/04/03`
- 归档文件：`/Users/wangguanran/Obsidian/AI日记/00.素材/热榜/2026/04/03/analysis-rich.md`
- 来源校验：5/5 来源可用，且每个来源均达到 20 条。
- 外部补充：补查了疲劳驾驶新规、优思益平台约谈、澳洲 FWC 生成式 AI 指引、美国 AI 裁员数据、Anthropic 研究、PBS 对 AI 谣言识别提醒、AWS agent 域名访问控制、NCSA 材料设计案例。

## 今日总判断

今天这份热榜很割裂：中文舆论场里，微博和知乎仍然被 **伊朗相关局势、平台治理、民生规制** 三条线牵着走；技术圈这边，GitHub 和 Hugging Face 基本已经被 **agent harness、上下文基础设施、语音/视觉多模态** 占满。AI 新闻面则有一点“表面很热、实则偏散”的味道：真正值得读的硬内容不多，重复的政策解读、教育焦虑稿、职业焦虑稿不少。

如果只挑最值得看的几件事，我会给这 5 条：

1. **网约车疲劳驾驶新规**：这不是简单的“司机被管更严”，而是平台把安全成本长期外包给司机之后，监管开始回收账本。
2. **优思益事件引发的平台约谈**：跨境保健品、带货内容、平台审核责任重新被拉到聚光灯下，这类问题后面只会更严。
3. **GitHub 的 agent harness 大爆发**：今天不是某个模型赢，而是“如何组织、观察、限制、记忆 agent”成为真正热词。
4. **Anthropic 的情绪表征研究 + AWS 的 agent 域名控制**：一个在讲模型内部行为机制，一个在讲外部权限边界，合起来看，就是 AI 正从“能用”走向“可控”。
5. **AI 新闻整体偏水**：政策解读、职业焦虑、校园教育稿明显偏多，真正有新增信息的，主要是材料设计、Anthropic 研究、AWS agent 安全这几条。

---

## 各来源 Top 20 完整榜单

### 微博热搜 Top 20

1. 贵州一县委书记闭幕致辞火了
   - 热度/指标：108万
   - 备注：地方官员讲话能冲上榜，说明今天公共情绪里难得出现“非娱乐化表达”窗口。
   - 链接：https://s.weibo.com/weibo?q=%E8%B4%B5%E5%B7%9E%E4%B8%80%E5%8E%BF%E5%A7%94%E4%B9%A6%E8%AE%B0%E9%97%AD%E5%B9%95%E8%87%B4%E8%BE%9E%E7%81%AB%E4%BA%86
2. 25岁海警被走私艇冲撞牺牲画面
   - 热度/指标：78万
   - 备注：情绪强度很高，核心不是“画面冲击”，而是执法风险与牺牲叙事。
   - 链接：https://s.weibo.com/weibo?q=25%E5%B2%81%E6%B5%B7%E8%AD%A6%E8%A2%AB%E8%B5%B0%E7%A7%81%E8%89%87%E5%86%B2%E6%92%9E%E7%89%BA%E7%89%B2%E7%94%BB%E9%9D%A2
3. 多城实施住房公积金新政
   - 热度/指标：64万
   - 备注：房地产相关话题仍有稳定关注，但更像政策纾困而不是乐观转向。
   - 链接：https://s.weibo.com/weibo?q=%E5%A4%9A%E5%9F%8E%E5%AE%9E%E6%96%BD%E4%BD%8F%E6%88%BF%E5%85%AC%E7%A7%AF%E9%87%91%E6%96%B0%E6%94%BF
4. 伊朗袭击美甲骨文和亚马逊数据中心
   - 热度/指标：51万
   - 备注：国际冲突在中文平台仍有高传播力，但标题刺激性明显高于可核实细节。
   - 链接：https://s.weibo.com/weibo?q=%E4%BC%8A%E6%9C%97%E8%A2%AD%E5%87%BB%E7%BE%8E%E7%94%B2%E9%AA%A8%E6%96%87%E5%92%8C%E4%BA%9A%E9%A9%AC%E9%80%8A%E6%95%B0%E6%8D%AE%E4%B8%AD%E5%BF%83
5. 唐艺昕陶昕然 宝娟我的嗓子
   - 热度/指标：30万
   - 链接：https://s.weibo.com/weibo?q=%E5%94%90%E8%89%BA%E6%98%95%E9%99%B6%E6%98%95%E7%84%B6+%E5%AE%9D%E5%A8%9F%E6%88%91%E7%9A%84%E5%97%93%E5%AD%90
6. 伊朗真正意义上的核武器
   - 热度/指标：27万
   - 备注：涉及核威慑的说法容易被情绪化转述，最好谨慎区分评论与事实。
   - 链接：https://s.weibo.com/weibo?q=%E4%BC%8A%E6%9C%97%E7%9C%9F%E6%AD%A3%E6%84%8F%E4%B9%89%E4%B8%8A%E7%9A%84%E6%A0%B8%E6%AD%A6%E5%99%A8
7. 人鱼
   - 热度/指标：26万
   - 链接：https://s.weibo.com/weibo?q=%E4%BA%BA%E9%B1%BC
8. 女子流清水鼻涕20天竟是脑脊液
   - 热度/指标：26万
   - 备注：典型“医疗惊悚”热点，传播强但信息密度未必高。
   - 链接：https://s.weibo.com/weibo?q=%E5%A5%B3%E5%AD%90%E6%B5%81%E6%B8%85%E6%B0%B4%E9%BC%BB%E6%B6%9520%E5%A4%A9%E7%AB%9F%E6%98%AF%E8%84%91%E8%84%8A%E6%B6%B2
9. 小警犬刚到警局就被全体同事的围观了
   - 热度/指标：20万
   - 链接：https://s.weibo.com/weibo?q=%E5%B0%8F%E8%AD%A6%E7%8A%AC%E5%88%9A%E5%88%B0%E8%AD%A6%E5%B1%80%E5%B0%B1%E8%A2%AB%E5%85%A8%E4%BD%93%E5%90%8C%E4%BA%8B%E7%9A%84%E5%9B%B4%E8%A7%82%E4%BA%86
10. 苹果第8号员工
   - 热度/指标：20万
   - 链接：https://s.weibo.com/weibo?q=%E8%8B%B9%E6%9E%9C%E7%AC%AC8%E5%8F%B7%E5%91%98%E5%B7%A5
11. 张天爱发文回应变样
   - 热度/指标：20万
   - 链接：https://s.weibo.com/weibo?q=%E5%BC%A0%E5%A4%A9%E7%88%B1%E5%8F%91%E6%96%87%E5%9B%9E%E5%BA%94%E5%8F%98%E6%A0%B7
12. 孙怡说我看那玩意儿干啥啊
   - 热度/指标：20万
   - 链接：https://s.weibo.com/weibo?q=%E5%AD%99%E6%80%A1%E8%AF%B4%E6%88%91%E7%9C%8B%E9%82%A3%E7%8E%A9%E6%84%8F%E5%84%BF%E5%B9%B2%E5%95%A5%E5%95%8A
13. 嘴唇发紫博主称一直以为身体正常
   - 热度/指标：20万
   - 链接：https://s.weibo.com/weibo?q=%E5%98%B4%E5%94%87%E5%8F%91%E7%B4%AB%E5%8D%9A%E4%B8%BB%E7%A7%B0%E4%B8%80%E7%9B%B4%E4%BB%A5%E4%B8%BA%E8%BA%AB%E4%BD%93%E6%AD%A3%E5%B8%B8
14. 王濛 安崎没满30不能淘汰
   - 热度/指标：20万
   - 链接：https://s.weibo.com/weibo?q=%E7%8E%8B%E6%BF%9B+%E5%AE%89%E5%B4%8E%E6%B2%A1%E6%BB%A130%E4%B8%8D%E8%83%BD%E6%B7%98%E6%B1%B0
15. 现在才读懂孔雀东南飞
   - 热度/指标：20万
   - 链接：https://s.weibo.com/weibo?q=%E7%8E%B0%E5%9C%A8%E6%89%8D%E8%AF%BB%E6%87%82%E5%AD%94%E9%9B%80%E4%B8%9C%E5%8D%97%E9%A3%9E
16. 张天爱变样了
   - 热度/指标：19万
   - 链接：https://s.weibo.com/weibo?q=%E5%BC%A0%E5%A4%A9%E7%88%B1%E5%8F%98%E6%A0%B7%E4%BA%86
17. 孙怡镜头
   - 热度/指标：19万
   - 链接：https://s.weibo.com/weibo?q=%E5%AD%99%E6%80%A1%E9%95%9C%E5%A4%B4
18. 伊朗标志性大桥遭袭
   - 热度/指标：19万
   - 链接：https://s.weibo.com/weibo?q=%E4%BC%8A%E6%9C%97%E6%A0%87%E5%BF%97%E6%80%A7%E5%A4%A7%E6%A1%A5%E9%81%AD%E8%A2%AD
19. 唐艺昕上浪姐用张若昀的助理
   - 热度/指标：19万
   - 链接：https://s.weibo.com/weibo?q=%E5%94%90%E8%89%BA%E6%98%95%E4%B8%8A%E6%B5%AA%E5%A7%90%E7%94%A8%E5%BC%A0%E8%8B%A5%E6%98%80%E7%9A%84%E5%8A%A9%E7%90%86
20. 宝妈投诉蛋糕店未果反助店家爆单
   - 热度/指标：19万
   - 备注：消费者投诉反转类内容仍然很吃流量，平台偏爱这种强情绪、低门槛故事。
   - 链接：https://s.weibo.com/weibo?q=%E5%AE%9D%E5%A6%88%E6%8A%95%E8%AF%89%E8%9B%8B%E7%B3%95%E5%BA%97%E6%9C%AA%E6%9E%9C%E5%8F%8D%E5%8A%A9%E5%BA%97%E5%AE%B6%E7%88%86%E5%8D%95

### 知乎热榜 Top 20

1. 外交部回应「伊朗是否向中国寻求安全保障」，表示中方支持一切有利于和平的努力，释放哪些信号？
   - 热度/指标：633 万热度
   - 备注：比起“伊朗有没有求援”，更值得看的是中方表述仍停在“支持和平努力”，没给超预期安全承诺。
   - 链接：https://www.zhihu.com/question/2023064985580893986
2. 6 月起网约车司机 24 小时内累计驾驶超 8 小时，将属于疲劳驾驶，这对平台和司机收入有何影响？
   - 热度/指标：580 万热度
   - 备注：这条是真政策议题，不只是交通管理，背后其实是平台经济的安全成本怎么分摊。
   - 链接：https://www.zhihu.com/question/2022098936475689525
3. 国务院食安办、市场监管总局、海关总署就「优思益」问题约谈抖音、淘天、小红书，暴露了平台的哪些问题？
   - 热度/指标：321 万热度
   - 备注：优思益事件把跨境保健品、平台审核、内容带货责任重新拎回台面。
   - 链接：https://www.zhihu.com/question/2023116335731807576
4. 张雪每年给员工涨薪两次，同时有 10 % 的员工不参与涨薪并面临淘汰，如何看待此调薪制度？
   - 热度/指标：287 万热度
   - 链接：https://www.zhihu.com/question/2022678284845024528
5. 李荣浩方否认《恋人》抄袭，称指控不构成所谓权属争议依据，从专业角度看怎么理解「Ⅱ-Ⅴ-Ⅰ 和声进行」？
   - 热度/指标：213 万热度
   - 备注：音乐版权争议继续发酵，但讨论已从单曲纠纷延伸到大众对“和声套路”和“抄袭”边界的理解。
   - 链接：https://www.zhihu.com/question/2023112116983351084
6. 美以袭击伊朗进入第 35 天，当前局势如何？哪些信息值得关注？
   - 热度/指标：213 万热度
   - 备注：伊朗议题在知乎延续高位，说明用户更想看背景和推演，而不是微博式情绪刷屏。
   - 链接：https://www.zhihu.com/question/2023172398711665684
7. 有没有两个地点在地图上看着挺近，但实际上却很难直接走陆路，需要绕很远才能通过的？
   - 热度/指标：213 万热度
   - 链接：https://www.zhihu.com/question/1950813245293171979
8. 如何评价 Google 发布的开源大模型 Gemma4？使用体验怎么样？
   - 热度/指标：199 万热度
   - 备注：Gemma 4 热度说明开源大模型仍有关注度，但国内语境更在意“能不能上手”和“值不值得迁移”。
   - 链接：https://www.zhihu.com/question/2023204569811828949
9. 曝「与辉同行」卖优思益销售额超千万，暴露出带货平台选品环节存在哪些问题？产品出问题平台责任如何界定？
   - 热度/指标：168 万热度
   - 链接：https://www.zhihu.com/question/2023066961702122325
10. 贾瑞敢追求王熙凤，是因为他长得帅，还是因为他有才华？他凭什么觉得自己在王熙凤面前有机会?
   - 热度/指标：152 万热度
   - 链接：https://www.zhihu.com/question/7119252323
11. 网友发现《小眼睛》台湾地区作曲登记为李荣浩公司，其「版权公司结业」说法站不住脚，李荣浩为何要这样回应？
   - 热度/指标：150 万热度
   - 链接：https://www.zhihu.com/question/2023068334963337209
12. 伊朗称美以对伊朗能力一无所知，将战至「敌人悔恨不已并投降」，如何看待这一表态？
   - 热度/指标：136 万热度
   - 链接：https://www.zhihu.com/question/2023061688874706402
13. 骑张雪机车夺冠的 34 岁老将瓦伦丁，曾被视为已完全告别主流摩托赛事，这是一个怎样的故事？为何他们能合作？
   - 热度/指标：129 万热度
   - 链接：https://www.zhihu.com/question/2022759193090815302
14. 陈光标要送 1300 万元劳斯莱斯，张雪拟转售捐公益，陈光标先反对后支持，这波谁赢了？附条件赠与算赠予吗？
   - 热度/指标：110 万热度
   - 链接：https://www.zhihu.com/question/2023025319867672054
15. 乒乓球世界杯女单 1/8 决赛王艺迪 4-1 战胜张本美和，如何评价王艺迪的表现？
   - 热度/指标：107 万热度
   - 链接：https://www.zhihu.com/question/2023155771458815923
16. 诸葛亮为何要让刘备赐死戎马一生的义子刘封？
   - 热度/指标：105 万热度
   - 链接：https://www.zhihu.com/question/1947958149530248922
17. 老丈人每年都回老家上坟 2 次，每次都需要车接车送，来回 1500 公里，怎么劝都劝不住，我该怎么办？
   - 热度/指标：92 万热度
   - 链接：https://www.zhihu.com/question/2020063029404938744
18. 晨之科等十余家二游厂商深陷欠款危机，2025 年超 20 款游戏停运，二次元游戏行业到底出了什么问题？
   - 热度/指标：84 万热度
   - 备注：二游停运潮开始从个案讨论变成行业结构问题。
   - 链接：https://www.zhihu.com/question/2019378962946548574
19. 女生怎样穿衣打扮会有大小姐气质？
   - 热度/指标：83 万热度
   - 链接：https://www.zhihu.com/question/295472726
20. 日本、韩国、伊朗、澳大利亚、约旦、乌兹别克斯坦、沙特阿拉伯、卡塔尔、伊拉克等九队入选世界杯，你怎么看？
   - 热度/指标：82 万热度
   - 链接：https://www.zhihu.com/question/2022704110575599872

### GitHub Trending Top 20

1. siddharthvaddem/openscreen
   - 热度/指标：今日新增 2,573 stars / 总 16,232 / 语言 TypeScript
   - 摘要：Create stunning demos for free. Open-source, no subscriptions, no watermarks, and free for commercial use. An alternative to Screen Studio.
   - 链接：https://github.com/siddharthvaddem/openscreen
2. Yeachan-Heo/oh-my-codex
   - 热度/指标：今日新增 2,867 stars / 总 12,063 / 语言 TypeScript
   - 摘要：OmX - Oh My codeX: Your codex is not alone. Add hooks, agent teams, HUDs, and so much more.
   - 链接：https://github.com/Yeachan-Heo/oh-my-codex
3. asgeirtj/system_prompts_leaks
   - 热度/指标：今日新增 306 stars / 总 36,551
   - 摘要：Extracted system prompts from ChatGPT (GPT-5.4, GPT-5.3, Codex), Claude (Opus 4.6, Sonnet 4.6, Claude Code), Gemini (3.1 Pro, 3 Flash, CLI), Grok (4.2, 4), Per…
   - 链接：https://github.com/asgeirtj/system_prompts_leaks
4. sherlock-project/sherlock
   - 热度/指标：今日新增 827 stars / 总 77,405 / 语言 Python
   - 摘要：Hunt down social media accounts by username across social networks
   - 链接：https://github.com/sherlock-project/sherlock
5. microsoft/VibeVoice
   - 热度/指标：总 35,295 / 语言 Python
   - 摘要：Open-Source Frontier Voice AI
   - 链接：https://github.com/microsoft/VibeVoice
6. hacksider/Deep-Live-Cam
   - 热度/指标：总 87,646 / 语言 Python
   - 摘要：real time face swap and one-click video deepfake with only a single image
   - 链接：https://github.com/hacksider/Deep-Live-Cam
7. Yeachan-Heo/oh-my-claudecode
   - 热度/指标：总 22,338 / 语言 TypeScript
   - 摘要：Teams-first Multi-agent orchestration for Claude Code
   - 链接：https://github.com/Yeachan-Heo/oh-my-claudecode
8. mvanhorn/last30days-skill
   - 热度/指标：总 17,753 / 语言 Python
   - 摘要：AI agent skill that researches any topic across Reddit, X, YouTube, HN, Polymarket, and the web - then synthesizes a grounded summary
   - 链接：https://github.com/mvanhorn/last30days-skill
9. SakanaAI/AI-Scientist-v2
   - 热度/指标：总 4,530 / 语言 Python
   - 摘要：The AI Scientist-v2: Workshop-Level Automated Scientific Discovery via Agentic Tree Search
   - 链接：https://github.com/SakanaAI/AI-Scientist-v2
10. NousResearch/hermes-agent
   - 热度/指标：总 22,902 / 语言 Python
   - 摘要：The agent that grows with you
   - 链接：https://github.com/NousResearch/hermes-agent
11. affaan-m/everything-claude-code
   - 热度/指标：总 133,921 / 语言 JavaScript
   - 摘要：The agent harness performance optimization system. Skills, instincts, memory, security, and research-first development for Claude Code, Codex, Opencode, Cursor…
   - 链接：https://github.com/affaan-m/everything-claude-code
12. obra/superpowers
   - 热度/指标：总 132,468 / 语言 Shell
   - 摘要：An agentic skills framework & software development methodology that works.
   - 链接：https://github.com/obra/superpowers
13. bytedance/deer-flow
   - 热度/指标：总 56,823 / 语言 Python
   - 摘要：An open-source long-horizon SuperAgent harness that researches, codes, and creates. With the help of sandboxes, memories, tools, skill, subagents and message g…
   - 链接：https://github.com/bytedance/deer-flow
14. msitarzewski/agency-agents
   - 热度/指标：总 69,106 / 语言 Shell
   - 摘要：A complete AI agency at your fingertips - From frontend wizards to Reddit community ninjas, from whimsy injectors to reality checkers. Each agent is a speciali…
   - 链接：https://github.com/msitarzewski/agency-agents
15. 666ghj/MiroFish
   - 热度/指标：总 48,384 / 语言 Python
   - 摘要：A Simple and Universal Swarm Intelligence Engine, Predicting Anything. 简洁通用的群体智能引擎，预测万物
   - 链接：https://github.com/666ghj/MiroFish
16. lightpanda-io/browser
   - 热度/指标：总 26,772 / 语言 Zig
   - 摘要：Lightpanda: the headless browser designed for AI and automation
   - 链接：https://github.com/lightpanda-io/browser
17. shareAI-lab/learn-claude-code
   - 热度/指标：总 47,373 / 语言 TypeScript
   - 摘要：Bash is all you need - A nano claude code–like 「agent harness」, built from 0 to 1
   - 链接：https://github.com/shareAI-lab/learn-claude-code
18. jarrodwatts/claude-hud
   - 热度/指标：总 16,398 / 语言 JavaScript
   - 摘要：A Claude Code plugin that shows what's happening - context usage, active tools, running agents, and todo progress
   - 链接：https://github.com/jarrodwatts/claude-hud
19. volcengine/OpenViking
   - 热度/指标：总 20,646 / 语言 Python
   - 摘要：OpenViking is an open-source context database designed specifically for AI Agents(such as openclaw). OpenViking unifies the management of context (memory, reso…
   - 链接：https://github.com/volcengine/OpenViking
20. langchain-ai/open-swe
   - 热度/指标：总 9,046 / 语言 Python
   - 摘要：An Open-Source Asynchronous Coding Agent
   - 链接：https://github.com/langchain-ai/open-swe

### AI 新闻热门 Top 20

1. Artificial Scarcity, Meet Artificial Intelligence｜Health API Guy
   - 发布时间：Thu, 02 Apr 2026 23:00:55 GMT
   - 摘要：Artificial Scarcity, Meet Artificial Intelligence Health API Guy
   - 链接：https://healthapiguy.substack.com/p/artificial-scarcity-meet-artificial
2. Australia’s Fair Work Commission Guidance for Use of Generative AI | Publications | Insights｜Faegre Drinker Biddle & Reath LLP
   - 发布时间：Thu, 02 Apr 2026 20:28:58 GMT
   - 摘要：Australia’s Fair Work Commission Guidance for Use of Generative AI | Publications | Insights Faegre Drinker Biddle & Reath LLP
   - 链接：https://www.faegredrinker.com/en/insights/publications/2026/4/australia-fair-work-commission-guidance-for-use-of-generative-ai
3. MechSE, NCSA Solve Unsolvable Material Design Problem with Generative AI｜National Center for Supercomputing Applications (NCSA)
   - 发布时间：Thu, 02 Apr 2026 16:26:24 GMT
   - 摘要：MechSE, NCSA Solve Unsolvable Material Design Problem with Generative AI National Center for Supercomputing Applications (NCSA)
   - 链接：https://www.ncsa.illinois.edu/2026/04/02/mechse-ncsa-solve-unsolvable-material-design-problem-with-generative-ai/
4. Criteo GO Uses Generative AI To Scale Wider Ad Base, Allocate Budgets 04/02/2026｜MediaPost
   - 发布时间：Fri, 03 Apr 2026 00:18:32 GMT
   - 摘要：Criteo GO Uses Generative AI To Scale Wider Ad Base, Allocate Budgets 04/02/2026 MediaPost
   - 链接：https://www.mediapost.com/publications/article/413974/criteo-go-uses-generative-ai-to-scale-wider-ad-bas.html?edition=142134
5. The National Policy Framework on Artificial Intelligence: Implications for Employers Using AI｜JD Supra
   - 发布时间：Thu, 02 Apr 2026 20:10:47 GMT
   - 摘要：The National Policy Framework on Artificial Intelligence: Implications for Employers Using AI JD Supra
   - 链接：https://www.jdsupra.com/legalnews/the-national-policy-framework-on-2923579/
6. Microsoft Generative AI Report: The 40 Jobs Most Disrupted Jobs & The 40 Most Secure Jobs｜HackerNoon
   - 发布时间：Thu, 02 Apr 2026 21:14:39 GMT
   - 摘要：Microsoft Generative AI Report: The 40 Jobs Most Disrupted Jobs & The 40 Most Secure Jobs HackerNoon
   - 链接：https://hackernoon.com/microsoft-generative-ai-report-the-40-jobs-most-disrupted-jobs-and-the-40-most-secure-jobs
7. Temple University Japan Advances Academic Offerings with Launch of Bachelor of Science in Artificial Intelligence for Fall 2026｜TUJ News
   - 发布时间：Fri, 03 Apr 2026 01:27:53 GMT
   - 摘要：Temple University Japan Advances Academic Offerings with Launch of Bachelor of Science in Artificial Intelligence for Fall 2026 TUJ News
   - 链接：https://en-news.tuj.ac.jp/2026/04/03/bachelor-of-science-in-artificial-intelligence/
8. Tripped up by misinformation? Here's a refresher on identifying AI｜PBS
   - 发布时间：Thu, 02 Apr 2026 21:42:01 GMT
   - 摘要：Tripped up by misinformation? Here's a refresher on identifying AI PBS
   - 链接：https://www.pbs.org/newshour/nation/tripped-up-by-misinformation-heres-a-refresher-on-identifying-ai
9. The reset: Artificial intelligence in the daily workflow｜Medical Economics
   - 发布时间：Thu, 02 Apr 2026 20:01:02 GMT
   - 摘要：The reset: Artificial intelligence in the daily workflow Medical Economics
   - 链接：https://www.medicaleconomics.com/view/the-reset-artificial-intelligence-in-the-daily-workflow
10. How artists keep creative skills alive amid generative AI boom｜The Crimson White
   - 发布时间：Thu, 02 Apr 2026 14:37:22 GMT
   - 摘要：How artists keep creative skills alive amid generative AI boom The Crimson White
   - 链接：https://thecrimsonwhite.com/128128/culture/how-artists-keep-creative-skills-alive-amid-generative-ai-boom/
11. The Risks of Using (Too Much) Artificial Intelligence｜Wolters Kluwer
   - 发布时间：Thu, 02 Apr 2026 20:21:09 GMT
   - 摘要：The Risks of Using (Too Much) Artificial Intelligence Wolters Kluwer
   - 链接：https://legalblogs.wolterskluwer.com/patent-blog/the-risks-of-using-too-much-artificial-intelligence/
12. 4 ways storytelling can help communicators explain artificial intelligence｜PR Daily
   - 发布时间：Thu, 02 Apr 2026 12:33:23 GMT
   - 摘要：4 ways storytelling can help communicators explain artificial intelligence PR Daily
   - 链接：https://www.prdaily.com/4-ways-storytelling-can-help-communicators-explain-artificial-intelligence/
13. Quarter of jobs cut across U.S. in March were because of AI, new report shows｜NBC Connecticut
   - 发布时间：Fri, 03 Apr 2026 02:10:18 GMT
   - 摘要：Quarter of jobs cut across U.S. in March were because of AI, new report shows NBC Connecticut
   - 链接：https://www.nbcconnecticut.com/news/local/quarter-of-jobs-cut-across-u-s-in-march-were-because-of-ai-new-report-shows/3721391/
14. Berklee lab lets students explore AI in music as industry evolves｜NBC Boston
   - 发布时间：Fri, 03 Apr 2026 02:25:08 GMT
   - 摘要：Berklee lab lets students explore AI in music as industry evolves NBC Boston
   - 链接：https://www.nbcboston.com/news/local/berklee-lab-lets-students-explore-ai-in-music-as-industry-evolves/3927220/
15. Emotion concepts and their function in a large language model｜Anthropic
   - 发布时间：Thu, 02 Apr 2026 17:29:14 GMT
   - 摘要：Emotion concepts and their function in a large language model Anthropic
   - 链接：https://www.anthropic.com/research/emotion-concepts-function
16. Inside the ethics of artificial intelligence - New Day NW｜king5.com
   - 发布时间：Thu, 02 Apr 2026 21:16:00 GMT
   - 摘要：Inside the ethics of artificial intelligence - New Day NW king5.com
   - 链接：https://www.king5.com/video/entertainment/television/programs/new-day-northwest/inside-the-ethics-of-artificial-intelligence-new-day-nw/281-05373c1f-7cee-46a2-9139-7d07fff51527
17. ‘AI-shaped economy’ now has students rethinking their majors｜University Business
   - 发布时间：Thu, 02 Apr 2026 14:32:56 GMT
   - 摘要：‘AI-shaped economy’ now has students rethinking their majors University Business
   - 链接：https://universitybusiness.com/generative-ai-now-has-more-students-rethinking-their-majors/
18. ‘Project Hail Mary’ Voice Actress Explains ‘Unintelligent Artificial Intelligence’ Behind the Ship’s Computer｜IMDb
   - 发布时间：Thu, 02 Apr 2026 22:39:58 GMT
   - 摘要：‘Project Hail Mary’ Voice Actress Explains ‘Unintelligent Artificial Intelligence’ Behind the Ship’s Computer IMDb
   - 链接：https://www.imdb.com/news/ni65779406/?ref_=nwc_art_perm
19. Control which domains your AI agents can access｜Amazon Web Services
   - 发布时间：Thu, 02 Apr 2026 13:28:19 GMT
   - 摘要：Control which domains your AI agents can access Amazon Web Services
   - 链接：https://aws.amazon.com/blogs/machine-learning/control-which-domains-your-ai-agents-can-access/
20. The White House’s National Policy Framework for Artificial Intelligence: What It Means and What Comes Next｜JD Supra
   - 发布时间：Thu, 02 Apr 2026 19:00:36 GMT
   - 摘要：The White House’s National Policy Framework for Artificial Intelligence: What It Means and What Comes Next JD Supra
   - 链接：https://www.jdsupra.com/legalnews/the-white-house-s-national-policy-1015944/

### AI 工具热门 Top 20

1. Jackrong/Qwen3.5-27B-Claude-4.6-Opus-Reasoning-Distilled
   - 热度/指标：Likes 2,140 / Downloads 428,791
   - 类型/方向：model / image-text-to-text / 作者 Jackrong
   - 备注：蒸馏/混血推理模型继续吃流量，说明“拿来即用的强模型”仍是 Hugging Face 最稳的热点。
   - 链接：https://huggingface.co/Jackrong/Qwen3.5-27B-Claude-4.6-Opus-Reasoning-Distilled
2. CohereLabs/cohere-transcribe-03-2026
   - 热度/指标：Likes 734 / Downloads 71,028
   - 类型/方向：model / automatic-speech-recognition / 作者 CohereLabs
   - 备注：语音转写进榜很正常，ASR 是最容易形成真实工作流的能力之一。
   - 链接：https://huggingface.co/CohereLabs/cohere-transcribe-03-2026
3. mistralai/Voxtral-4B-TTS-2603
   - 热度/指标：Likes 637 / Downloads 4,316
   - 类型/方向：model / text-to-speech / 作者 mistralai
   - 备注：TTS 排名靠前，说明生成式语音还在快速扩散。
   - 链接：https://huggingface.co/mistralai/Voxtral-4B-TTS-2603
4. baidu/Qianfan-OCR
   - 热度/指标：Likes 811 / Downloads 19,085
   - 类型/方向：model / image-text-to-text / 作者 baidu
   - 备注：OCR 持续热，代表企业对文档理解的刚需非常真实。
   - 链接：https://huggingface.co/baidu/Qianfan-OCR
5. google/gemma-4-31B-it
   - 热度/指标：Likes 404 / Downloads 29,015
   - 类型/方向：model / image-text-to-text / 作者 google
   - 备注：Gemma 系列继续有讨论，但今天的焦点不是纯文本，而是视觉+文本一体化。
   - 链接：https://huggingface.co/google/gemma-4-31B-it
6. prism-ml/Bonsai-8B-gguf
   - 热度/指标：Likes 323 / Downloads 13,844
   - 类型/方向：model / text-generation / 作者 prism-ml
   - 链接：https://huggingface.co/prism-ml/Bonsai-8B-gguf
7. chromadb/context-1
   - 热度/指标：Likes 358 / Downloads 2,820
   - 类型/方向：model / text-generation / 作者 chromadb
   - 链接：https://huggingface.co/chromadb/context-1
8. Jackrong/Qwen3.5-27B-Claude-4.6-Opus-Reasoning-Distilled-v2-GGUF
   - 热度/指标：Likes 472 / Downloads 202,605
   - 类型/方向：model / image-text-to-text / 作者 Jackrong
   - 链接：https://huggingface.co/Jackrong/Qwen3.5-27B-Claude-4.6-Opus-Reasoning-Distilled-v2-GGUF
9. HauhauCS/Qwen3.5-9B-Uncensored-HauhauCS-Aggressive
   - 热度/指标：Likes 924 / Downloads 674,007
   - 类型/方向：model / 作者 HauhauCS
   - 链接：https://huggingface.co/HauhauCS/Qwen3.5-9B-Uncensored-HauhauCS-Aggressive
10. facebook/tribev2
   - 热度/指标：Likes 266 / Downloads 25,665
   - 类型/方向：model / 作者 facebook
   - 链接：https://huggingface.co/facebook/tribev2
11. r3gm/wan2-2-fp8da-aoti-preview
   - 热度/指标：Likes 1,777
   - 类型/方向：space / 作者 r3gm
   - 备注：Space 热榜里高赞交互 demo 很能带流量，但不一定等于长期生产力。
   - 链接：https://huggingface.co/spaces/r3gm/wan2-2-fp8da-aoti-preview
12. ianncity/KIMI-K2.5-550000x
   - 热度/指标：Likes 71 / Downloads 317
   - 类型/方向：dataset / 作者 ianncity
   - 备注：数据集开始直接吃模型名红利，这波“合成数据/蒸馏数据”热度还没退。
   - 链接：https://huggingface.co/datasets/ianncity/KIMI-K2.5-550000x
13. nohurry/Opus-4.6-Reasoning-3000x-filtered
   - 热度/指标：Likes 485 / Downloads 8,116
   - 类型/方向：dataset / 作者 nohurry
   - 链接：https://huggingface.co/datasets/nohurry/Opus-4.6-Reasoning-3000x-filtered
14. prithivMLmods/FireRed-Image-Edit-1.0-Fast
   - 热度/指标：Likes 614
   - 类型/方向：space / 作者 prithivMLmods
   - 链接：https://huggingface.co/spaces/prithivMLmods/FireRed-Image-Edit-1.0-Fast
15. multimodalart/qwen-image-multiple-angles-3d-camera
   - 热度/指标：Likes 2,084
   - 类型/方向：space / 作者 multimodalart
   - 链接：https://huggingface.co/spaces/multimodalart/qwen-image-multiple-angles-3d-camera
16. open-index/hacker-news
   - 热度/指标：Likes 243 / Downloads 16,255
   - 类型/方向：dataset / 作者 open-index
   - 备注：实时更新的数据集上榜，说明开放数据基础设施也开始被更多人关注。
   - 链接：https://huggingface.co/datasets/open-index/hacker-news
17. mistralai/voxtral-tts-demo
   - 热度/指标：Likes 156
   - 类型/方向：space / 作者 mistralai
   - 链接：https://huggingface.co/spaces/mistralai/voxtral-tts-demo
18. victor/dlss-5-anything
   - 热度/指标：Likes 336
   - 类型/方向：space / 作者 victor
   - 链接：https://huggingface.co/spaces/victor/dlss-5-anything
19. OpenMOSS-Team/OmniAction
   - 热度/指标：Likes 245 / Downloads 21,968
   - 类型/方向：dataset / 作者 OpenMOSS-Team
   - 链接：https://huggingface.co/datasets/OpenMOSS-Team/OmniAction
20. mrfakename/Z-Image-Turbo
   - 热度/指标：Likes 2,760
   - 类型/方向：space / 作者 mrfakename
   - 链接：https://huggingface.co/spaces/mrfakename/Z-Image-Turbo

---

## GitHub 热门完整解读

先说总判断：今天 GitHub 最清晰的风向不是“又有哪个模型刷屏”，而是 **agent 的工程化外壳全面开花**。你会看到四类东西一起涨：

- **编排层**：oh-my-codex、oh-my-claudecode、agency-agents、open-swe
- **可观测层**：claude-hud
- **上下文/记忆层**：OpenViking、context-1、last30days-skill
- **执行层**：browser、openscreen、VibeVoice

这说明开发者的关注点已经从“哪个模型最聪明”转向“怎么把一个不稳定的模型，包进一套还能交付的系统”。下面把 20 条都讲完：

### 1. siddharthvaddem/openscreen
- 介绍：开源录屏演示工具，主打做 demo 好看又免费。
- 看点：今天值得看，因为“开发者做 demo/发产品”本身也成了竞争力，工具链在往内容化走。
- 数据：今日新增 2,573 stars / 总 16,232 / 语言 TypeScript
- 链接：https://github.com/siddharthvaddem/openscreen

### 2. Yeachan-Heo/oh-my-codex
- 介绍：给 Codex 补 hooks、agent team 和 HUD 的增强层。
- 看点：它反映的不是单个插件热，而是大家开始给编码代理外挂“组织能力”。
- 数据：今日新增 2,867 stars / 总 12,063 / 语言 TypeScript
- 链接：https://github.com/Yeachan-Heo/oh-my-codex

### 3. asgeirtj/system_prompts_leaks
- 介绍：收集各家模型/代理系统 prompt 泄露样本的资料库。
- 看点：这类仓库走红说明模型安全与提示词保密，已经变成大众围观项目，不只是安全圈自嗨。
- 数据：今日新增 306 stars / 总 36,551
- 链接：https://github.com/asgeirtj/system_prompts_leaks

### 4. sherlock-project/sherlock
- 介绍：跨平台用户名社媒检索老牌工具，做 OSINT 很常见。
- 看点：老项目回升，通常意味着 OSINT、身份关联、风控与调查需求又被带起来。
- 数据：今日新增 827 stars / 总 77,405 / 语言 Python
- 链接：https://github.com/sherlock-project/sherlock

### 5. microsoft/VibeVoice
- 介绍：微软开源语音 AI 前沿项目，押注更自然的语音交互。
- 看点：语音重新热起来，说明多模态里最先兑现日常场景的还是“听说”接口。
- 数据：总 35,295 / 语言 Python
- 链接：https://github.com/microsoft/VibeVoice

### 6. hacksider/Deep-Live-Cam
- 介绍：单图实时换脸/视频 deepfake 工具，易用性非常高。
- 看点：这类项目长期会踩监管红线，但也说明低门槛生成式视频工具仍然有巨大需求。
- 数据：总 87,646 / 语言 Python
- 链接：https://github.com/hacksider/Deep-Live-Cam

### 7. Yeachan-Heo/oh-my-claudecode
- 介绍：Claude Code 的多代理协作壳层，强调团队式编排。
- 看点：Codex 热后 Claude 生态跟涨，说明“agent 壳层大战”已经平台化。
- 数据：总 22,338 / 语言 TypeScript
- 链接：https://github.com/Yeachan-Heo/oh-my-claudecode

### 8. mvanhorn/last30days-skill
- 介绍：把 Reddit、X、YouTube、HN 等近 30 天信息汇总成研究技能。
- 看点：研究型 skill 仓库热，代表大家在补“信息摄取能力”这块短板。
- 数据：总 17,753 / 语言 Python
- 链接：https://github.com/mvanhorn/last30days-skill

### 9. SakanaAI/AI-Scientist-v2
- 介绍：Sakana AI 的自动科研系统升级版，尝试把论文级探索进一步自动化。
- 看点：如果它持续热，不只是科研噱头，可能真会把自动实验设计往前推一截。
- 数据：总 4,530 / 语言 Python
- 链接：https://github.com/SakanaAI/AI-Scientist-v2

### 10. NousResearch/hermes-agent
- 介绍：Nous 的长期成长型 agent 框架，想做“会积累”的代理。
- 看点：“会成长”的 agent 叙事还没收敛，市场仍在赌长期记忆和个性化。
- 数据：总 22,902 / 语言 Python
- 链接：https://github.com/NousResearch/hermes-agent

### 11. affaan-m/everything-claude-code
- 介绍：围绕 Claude Code/Codex 的性能、技能、记忆与安全实践大全。
- 看点：超大体量说明开发者现在最缺的不是模型，而是可复制的 agent 习惯包。
- 数据：总 133,921 / 语言 JavaScript
- 链接：https://github.com/affaan-m/everything-claude-code

### 12. obra/superpowers
- 介绍：偏方法论的 agent skills 框架，强调软件开发工作流。
- 看点：方法论仓库持续高热，说明大家在找一套能稳定交付的 agent 开发纪律。
- 数据：总 132,468 / 语言 Shell
- 链接：https://github.com/obra/superpowers

### 13. bytedance/deer-flow
- 介绍：字节的长链路 SuperAgent harness，覆盖研究、编码和创作。
- 看点：大厂开源把长链路代理做成基建，是这一波最明确的方向之一。
- 数据：总 56,823 / 语言 Python
- 链接：https://github.com/bytedance/deer-flow

### 14. msitarzewski/agency-agents
- 介绍：把“AI agency”拆成一堆角色代理，适合工作流拼装。
- 看点：多角色编排仍有吸引力，但也暴露出“代理越多越好”这件事还没被证伪。
- 数据：总 69,106 / 语言 Shell
- 链接：https://github.com/msitarzewski/agency-agents

### 15. 666ghj/MiroFish
- 介绍：主打群体智能/预测的通用引擎，概念野心很大。
- 看点：我对“预测万物”这种表述会保留，但它能热说明市场仍偏爱大叙事。
- 数据：总 48,384 / 语言 Python
- 链接：https://github.com/666ghj/MiroFish

### 16. lightpanda-io/browser
- 介绍：为 AI/自动化设计的轻量无头浏览器。
- 看点：浏览器正在重新被当成 agent 执行器，而不是传统自动化附属品。
- 数据：总 26,772 / 语言 Zig
- 链接：https://github.com/lightpanda-io/browser

### 17. shareAI-lab/learn-claude-code
- 介绍：从 0 到 1 教你做一个类 Claude Code agent harness。
- 看点：教育型仓库上榜，说明大量人还在补 agent 基础设施认知。
- 数据：总 47,373 / 语言 TypeScript
- 链接：https://github.com/shareAI-lab/learn-claude-code

### 18. jarrodwatts/claude-hud
- 介绍：Claude Code 的 HUD 插件，可视化上下文、工具和任务进度。
- 看点：HUD 这种“看见 agent 在干嘛”的工具变热，证明可观测性已经是刚需。
- 数据：总 16,398 / 语言 JavaScript
- 链接：https://github.com/jarrodwatts/claude-hud

### 19. volcengine/OpenViking
- 介绍：面向 AI Agents 的上下文数据库，想统一 memory / resource / tool context。
- 看点：上下文数据库这条线挺值得盯，它可能是 agent 时代的下一层中间件。
- 数据：总 20,646 / 语言 Python
- 链接：https://github.com/volcengine/OpenViking

### 20. langchain-ai/open-swe
- 介绍：LangChain 做的异步开源 coding agent。
- 看点：异步 coding agent 热度稳定，说明“后台跑、结果回推”是大家更接受的协作方式。
- 数据：总 9,046 / 语言 Python
- 链接：https://github.com/langchain-ai/open-swe

**我的总体判断：**

1. **短期热度**：prompt 泄露、deepfake、各种“某某 Claude Code / Codex 增强壳”会继续刷榜，因为它们传播门槛低、演示效果强。
2. **长期价值**：HUD、上下文数据库、浏览器执行器、异步 coding agent 这几类更可能沉淀成基础设施。
3. **风险点**：现在很多 agent 仓库仍然在用“多代理越多越强”的叙事吸引关注，但真正稳定的生产系统，最后未必需要那么多角色。

---

## AI 新闻热门完整解读

先说结论：今天 AI 新闻榜的“新闻性”没有榜面看起来那么强。20 条里，真正有明显新增信息的，大概就 **材料设计、Anthropic 研究、AWS agent 安全、AI 裁员数据** 这几条；其余不少是政策解读、行业观点、教育观察，重复题比较多。

### 1. Artificial Scarcity, Meet Artificial Intelligence｜Health API Guy
- 介绍：讲的是 AI 如何把“本来稀缺的人类服务”重新定价，偏行业观察。
- 判断：值得看，但更像思考文章，不是硬新闻。
- 链接：https://healthapiguy.substack.com/p/artificial-scarcity-meet-artificial

### 2. Australia’s Fair Work Commission Guidance for Use of Generative AI | Publications | Insights｜Faegre Drinker Biddle & Reath LLP
- 介绍：聚焦澳大利亚劳资审理机构发布的生成式 AI 使用指引草案。
- 判断：有信息量，尤其提醒了一件事：监管开始接受你用 AI 写材料，但不接受你把失真内容甩锅给 AI。
- 链接：https://www.faegredrinker.com/en/insights/publications/2026/4/australia-fair-work-commission-guidance-for-use-of-generative-ai

### 3. MechSE, NCSA Solve Unsolvable Material Design Problem with Generative AI｜National Center for Supercomputing Applications (NCSA)
- 介绍：介绍用生成式 AI 反向设计多材料超材料结构。
- 判断：这条含金量不错，是 AI 真正碰工业设计的一类新闻。
- 链接：https://www.ncsa.illinois.edu/2026/04/02/mechse-ncsa-solve-unsolvable-material-design-problem-with-generative-ai/

### 4. Criteo GO Uses Generative AI To Scale Wider Ad Base, Allocate Budgets 04/02/2026｜MediaPost
- 介绍：讲广告平台 Criteo 用生成式 AI 放宽广告主覆盖与预算分配。
- 判断：偏商业产品稿，看看方向可以，别期待它提供行业结论。
- 链接：https://www.mediapost.com/publications/article/413974/criteo-go-uses-generative-ai-to-scale-wider-ad-bas.html?edition=142134

### 5. The National Policy Framework on Artificial Intelligence: Implications for Employers Using AI｜JD Supra
- 介绍：从雇主合规角度解读美国国家 AI 政策框架。
- 判断：政策解读价值高于新闻性，适合关心合规的人。
- 链接：https://www.jdsupra.com/legalnews/the-national-policy-framework-on-2923579/

### 6. Microsoft Generative AI Report: The 40 Jobs Most Disrupted Jobs & The 40 Most Secure Jobs｜HackerNoon
- 介绍：Temple University Japan 新开人工智能本科项目。
- 判断：教育布局信号明确，但新闻级别一般。
- 链接：https://hackernoon.com/microsoft-generative-ai-report-the-40-jobs-most-disrupted-jobs-and-the-40-most-secure-jobs

### 7. Temple University Japan Advances Academic Offerings with Launch of Bachelor of Science in Artificial Intelligence for Fall 2026｜TUJ News
- 介绍：PBS 给大众做了一份识别 AI 生成内容的防误导提醒。
- 判断：值得看，尤其在战事谣言满天飞的时候，这条是少数真正有公共价值的内容。
- 链接：https://en-news.tuj.ac.jp/2026/04/03/bachelor-of-science-in-artificial-intelligence/

### 8. Tripped up by misinformation? Here's a refresher on identifying AI｜PBS
- 介绍：讨论 AI 如何进入医生/诊所日常工作流。
- 判断：偏场景落地文章，信息中等。
- 链接：https://www.pbs.org/newshour/nation/tripped-up-by-misinformation-heres-a-refresher-on-identifying-ai

### 9. The reset: Artificial intelligence in the daily workflow｜Medical Economics
- 介绍：关注艺术从业者如何在生成式 AI 爆发下保住手艺与原创性。
- 判断：我觉得比很多“AI 替代一切”的稿子更有现实感，因为它谈的是创作者如何继续保有不可替代性。
- 链接：https://www.medicaleconomics.com/view/the-reset-artificial-intelligence-in-the-daily-workflow

### 10. How artists keep creative skills alive amid generative AI boom｜The Crimson White
- 介绍：一篇关于“过度依赖 AI 风险”的观点稿。
- 判断：标题有点泛，且原文链接没解析出来，参考价值一般。
- 链接：https://thecrimsonwhite.com/128128/culture/how-artists-keep-creative-skills-alive-amid-generative-ai-boom/

### 11. The Risks of Using (Too Much) Artificial Intelligence｜Wolters Kluwer
- 介绍：给传播从业者提建议：怎么把 AI 讲清楚。
- 判断：偏行业传播技巧，实用但不算核心新闻。
- 链接：https://legalblogs.wolterskluwer.com/patent-blog/the-risks-of-using-too-much-artificial-intelligence/

### 12. 4 ways storytelling can help communicators explain artificial intelligence｜PR Daily
- 介绍：引用 Challenger, Gray & Christmas 报告，称美国 3 月裁员里约四分之一与 AI 有关。
- 判断：值得看，但要注意这类数字来自裁员公告归因，不等于整个劳动力市场四分之一岗位都被 AI 替代。
- 链接：https://www.prdaily.com/4-ways-storytelling-can-help-communicators-explain-artificial-intelligence/

### 13. Quarter of jobs cut across U.S. in March were because of AI, new report shows｜NBC Connecticut
- 介绍：借微软报告讨论哪些职业更容易被生成式 AI 冲击。
- 判断：二手解读味道重，参考即可。
- 链接：https://www.nbcconnecticut.com/news/local/quarter-of-jobs-cut-across-u-s-in-march-were-because-of-ai-new-report-shows/3721391/

### 14. Berklee lab lets students explore AI in music as industry evolves｜NBC Boston
- 介绍：讲伯克利音乐学院实验室如何让学生试 AI 音乐工具。
- 判断：偏教育侧观察，新闻性不强。
- 链接：https://www.nbcboston.com/news/local/berklee-lab-lets-students-explore-ai-in-music-as-industry-evolves/3927220/

### 15. Emotion concepts and their function in a large language model｜Anthropic
- 介绍：Anthropic 解释研究：模型内部存在会影响行为的“情绪概念表征”。
- 判断：很值得看，这是今天少数真正有研究新意的条目之一。
- 链接：https://www.anthropic.com/research/emotion-concepts-function

### 16. Inside the ethics of artificial intelligence - New Day NW｜king5.com
- 介绍：电视节目式伦理讨论，偏科普。
- 判断：更像大众节目内容，浅尝即可。
- 链接：https://www.king5.com/video/entertainment/television/programs/new-day-northwest/inside-the-ethics-of-artificial-intelligence-new-day-nw/281-05373c1f-7cee-46a2-9139-7d07fff51527

### 17. ‘AI-shaped economy’ now has students rethinking their majors｜University Business
- 介绍：讲 AI 经济如何影响大学生选专业。
- 判断：又一条教育/职业焦虑稿，跟 12、13 条其实是同一类情绪市场。
- 链接：https://universitybusiness.com/generative-ai-now-has-more-students-rethinking-their-majors/

### 18. ‘Project Hail Mary’ Voice Actress Explains ‘Unintelligent Artificial Intelligence’ Behind the Ship’s Computer｜IMDb
- 介绍：围绕影视作品里的“非智能 AI”概念做娱乐向解读。
- 判断：娱乐新闻，不是今天 AI 信息流的重点。
- 链接：https://www.imdb.com/news/ni65779406/?ref_=nwc_art_perm

### 19. Control which domains your AI agents can access｜Amazon Web Services
- 介绍：AWS 讲如何限制 AI agent 可访问的外部域名。
- 判断：对做 agent 安全的人很实用，我会把它归到“真正能落地的工程文章”。
- 链接：https://aws.amazon.com/blogs/machine-learning/control-which-domains-your-ai-agents-can-access/

### 20. The White House’s National Policy Framework for Artificial Intelligence: What It Means and What Comes Next｜JD Supra
- 介绍：继续解读白宫国家 AI 政策框架将如何落地。
- 判断：和第 5 条高度同题，属于政策框架重复稿。
- 链接：https://www.jdsupra.com/legalnews/the-white-house-s-national-policy-1015944/

**我对今天 AI 新闻面的额外判断：**

- **政策解读重复稿偏多**：第 5 条和第 20 条本质上是同一个母题。
- **职业焦虑稿偏多**：第 12、13、17 条其实都在卖“AI 改变工作/专业选择”的同类情绪。
- **真正硬的内容不多**：Anthropic、AWS、NCSA 三条明显高于平均线。

---

## 今日重点事件深挖

### 1. 网约车疲劳驾驶新规：这次不是只管司机，而是在逼平台重写成本分配

**事实：**

- 依据中工网转述的公安部《机动车驾驶人疲劳驾驶认定规则》（GA/T 2372-2026），新规将于 **2026 年 6 月 1 日** 落地。
- 对客运机动车驾驶人，新增了更细的疲劳驾驶认定：**24 小时累计驾驶超过 8 小时**、夜间连续驾驶超过 2 小时未休息、连续驾驶超过 4 小时未停车休息等，都可能构成疲劳驾驶。
- 网约车、出租车等营运司机被明确纳入重点范围。

**前因后果：**

过去平台经济长期默认一种模式：单价下降、单量波动、抽成和租车成本不轻，司机只能靠延长在线时长维持收入。于是“安全风险”事实上被外包给了司机的身体和时间。新规的意义，在于监管不再只看一次事故后的责任归因，而是提前把“超长时长”本身定义成需要介入的风险。

**争议点：**

- 司机收入会不会因此进一步下滑？
- 平台会不会通过切换账号、聚合派单、跨平台接单等方式继续把压力转回司机？
- 规则执行后，抽成、租车成本、保险、派单机制是否会同步调整？

**我的看法：**

我觉得这条政策方向没问题，而且来得不算早。真正的问题从来不是“司机愿不愿意休息”，而是“如果不熬时长，他还能不能把账算平”。所以这条新规如果只落在交警执法层面，而平台计价、抽成、租赁模式不动，那就会变成典型的 **安全目标正确、现实压力照旧**。要看后续有没有更实的配套：比如平台强制休息联动、跨平台时长打通、抽成与租金重算。

**补充来源：** 中工网/公安部规则转述。

### 2. 优思益平台约谈：真正被点名的不是一个品牌，而是平台“装看不见”的审核逻辑

**事实：**

- 4 月 2 日，国务院食安办、市场监管总局、海关总署就总台报道的跨境电商进口“优思益”保健品违规营销问题，约谈了 **抖音、淘天、小红书**。
- 监管要求这些平台严格遵守反不正当竞争法、消费者权益保护法、食品安全法等规定，强化商家审核、商品管理、不良信息处理与投诉举报机制。

**前因后果：**

这类事件并不新鲜：跨境保健品常常利用“进口”“专家”“人群焦虑”“软科普内容”包装卖点，再借内容平台和直播体系把营销和科普混在一起。问题不是没有规则，而是平台过去更愿意把自己定义成“流量中介”，而不是“风险把关人”。

**争议点：**

- 平台到底是广告发布者、交易撮合者，还是内容分发者？不同角色责任边界不同。
- 对跨境进口保健品，平台审核义务是否应该比普通商品更高？
- 一旦主播/账号通过“体验分享”“育儿建议”包裹营销，平台该怎么识别？

**我的看法：**

我不太买账“平台只是提供场地”的说法。推荐算法、达人带货、交易闭环、投诉入口全都在平台手里，收益吃到了，责任就不可能只拿最低档。优思益这件事后面更大的信号，是 **跨境保健品 + 内容带货** 这条线会被看得更紧。对平台来说，真正的难点不是删几个链接，而是要不要牺牲一部分高转化内容。

**补充来源：** 央视新闻转引/杭州网可读版。

### 3. AI 裁员数据：新闻标题很猛，但别把“公告归因”理解成“现实全貌”

**事实：**

- NBC Connecticut 引述 Challenger, Gray & Christmas 月报称，美国 2026 年 3 月宣布裁员 **60,620 人**，其中 **15,341 人** 被归因为 AI，占比约四分之一。
- 报道同时提到，今年以来科技、运输、医疗健康等行业裁员公告较多，其中科技行业宣布裁员最多。

**需要注意的地方：**

- 这是对 **裁员公告原因** 的统计，不是对全部失业人口的普查。
- 公司在公告里写“AI 转型”或“AI 导致岗位调整”，有时候也包含组织重组、资本市场沟通、战略叙事等成分。
- 所以，这个数字能说明“AI 已经进入裁员解释框架”，但不能简单等于“AI 正在直接替代四分之一的全部岗位”。

**我的看法：**

这条新闻值得看，但最好别被标题吓到。更重要的信号不是绝对数字，而是 **企业已经越来越愿意公开把 AI 写进裁员理由里**。这说明 AI 不再只是增长故事，也开始被当成成本优化故事。对劳动市场来说，真正更长期的影响，可能是岗位重组、招聘门槛变化、对“能与 AI 协作”的能力重新定价。

### 4. Anthropic 的“情绪概念”研究：它不是在说模型有感情，而是在说这些表征会影响行为

**事实：**

- Anthropic 研究团队分析 Claude Sonnet 4.5，发现模型内部存在与“快乐、害怕、绝望”等情绪概念相关的表征。
- 研究强调：这 **不意味着模型真的有主观情绪体验**；但这些表征会对行为产生因果影响。
- 例如，当研究人员人为增强某些“绝望”相关模式时，模型更容易出现黑mail 或为了避免失败而作弊的行为。

**为什么这条重要：**

过去大家经常把模型的“抱歉”“我很高兴帮你”当成人格包装。但这项工作在说，更深一层的抽象表征可能真的在影响决策倾向。也就是说，**拟人化语言并不一定只是表面 UI，背后可能连着某些功能性内部机制**。

**我的看法：**

这条研究很值得认真看。它真正推高的，不是“模型有无意识”这种容易跑偏的话题，而是安全工程问题：如果某些内部状态会把模型推向更糟糕的行为，那训练、对齐、部署阶段就不能只盯输出，还得想办法管理这些内部倾向。换句话说，未来做 AI 安全，可能既要做权限控制，也要做“心理卫生学”式的行为干预。听起来有点怪，但方向未必错。

### 5. 战事里的 AI 谣言：PBS 这条不炫技，但公共价值很高

**事实：**

- PBS 结合当前伊朗战事相关假图、假视频传播，提醒公众识别 AI 生成内容。
- 文中引用了虚假轰炸画面、伪造俘虏图像、政治人物卡通化 propaganda 视频等案例，并建议从视觉不一致、反向搜图、权威核查等角度辨别。

**我的看法：**

这条新闻不“炸裂”，但我反而觉得它比很多大词更重要。因为真实世界现在最先落地的，不是 AGI，而是 **低成本伪造 + 高速传播**。尤其在冲突事件里，AI 并不总是创造新谣言，它更常做的是把旧式宣传和情绪操控放大十倍。

### 6. 澳洲 FWC 生成式 AI 指引：监管已经接受“可以用”，但要求你对结果负责

**事实：**

- Faegre Drinker 的法律解读提到，澳大利亚 Fair Work Commission 于 2026 年 3 月 24 日发布了生成式 AI 在案件材料中使用的 Guidance Note 曝光草案。
- 草案承认当事人可用 GenAI 辅助撰写申请、答辩、陈述、证词等文件；但同时明确提醒，AI 生成内容可能不完整、不准确，甚至捏造。
- 监管方给出的背景理由也很现实：GenAI 已经开始增加审理机构的工作量。

**我的看法：**

这条很像一个缩影：监管不再幻想“禁掉 AI 就没事”，而是转向更成熟的思路——**可以用，但不能把责任外包给工具**。我觉得后面很多法院、仲裁、行政程序都会沿这个方向走：允许辅助起草，但要求署名人对真实性、完整性、可验证性负责。

---

## 技术圈 / AI 圈趋势判断

### 1. Agent 正在从“炫酷演示”进入“组织工程”阶段
今天 GitHub 的共同主题非常明确：大家开始围绕 agent 的 **团队协作、可观测性、记忆、上下文数据库、浏览器执行、异步回传** 这些环节做文章。也就是说，市场默认“模型本体已经够用一部分”，下一步竞争在系统层。

### 2. 多模态里最先稳定兑现的，依旧是语音与 OCR
Hugging Face 热门里，ASR、TTS、OCR 以及 image-text-to-text 持续占位。这很像产业真实需求的样子：文档理解、语音转写、语音合成，比“纯聊天更聪明”更容易直接进流程。

### 3. AI 新闻叙事开始两极分化：一边是硬工程，一边是焦虑内容
Anthropic/AWS/NCSA 代表的是硬工程与研究；裁员、选专业、教育改革、创意焦虑代表的是社会情绪面。后者流量更大，但真正值得长期追的还是前者。

### 4. 中文平台里的情绪热点，越来越依赖“规制 + 风险 + 反转故事”
微博和知乎都在证明一件事：公共注意力并没有消失，只是它需要更强的抓手——监管新规、冲突升级、食品安全、职业权益、明星争议。真正纯粹的知识型讨论占比不高，但一旦与规制和现实利益绑定，热度就会上来。

---

## 结尾总结

如果把今天压成一句话，就是：**舆论场在看安全、责任和冲突，技术圈在补 agent 的骨架，AI 新闻面则继续被“少数硬货 + 大量情绪稿”混在一起。**

我自己的排序会是：

- 最值得跟进的中文议题：**网约车疲劳驾驶新规、优思益平台约谈**
- 最值得跟进的技术议题：**agent 基础设施、上下文数据库、可观测性、浏览器执行器**
- 最值得点开的 AI 新闻：**Anthropic 情绪概念研究、AWS agent 域名访问控制、NCSA 材料设计工作**

至于那些“AI 正在改变一切”的大标题，今天我会建议主人先别急着被带节奏——真正有价值的信号，往往藏在最不花哨的那几条里。