# Survival Arena（生存竞技场）— 项目策划案

## 一、项目概述

| 项目 | 内容 |
|------|------|
| 项目名称 | Survival Arena（生存竞技场） |
| 游戏类型 | 多人合作 PVE 第一人称射击 |
| 开发引擎 | Unreal Engine 5.7（C++） |
| 基础模板 | UE5 Shooter C++ Template（Variant_Shooter） |
| 联机模式 | Listen Server（最多 4 人） |

## 二、核心玩法

### 游戏目标
玩家组队进入竞技场，合作击败不断刷新的 AI 敌人。团队总分达到 **10 分** 即获胜。

### 游戏规则

| 规则 | 说明 |
|------|------|
| 胜利条件 | 团队总分 ≥ 10 分 |
| 失败条件 | 所有玩家同时死亡 |
| 玩家死亡 | 5 秒后自动重生，继续战斗 |
| 敌人刷新 | 场上最多 15 个敌人，间隔 5 秒刷新 |
| 得分方式 | 击杀 1 个敌人 → 团队 +1 分 |

### 操作方式

| 按键 | 操作 |
|------|------|
| WASD | 移动 |
| 鼠标移动 | 视角控制 |
| 鼠标左键 | 射击 |
| 空格 | 跳跃 |
| 数字键 / Q | 切换武器 |

## 三、与现有模板的差异（PVP → PVE）

| 系统 | PVP 原版（Shooter 模板） | PVE 改造后 |
|------|------------------------|-----------|
| 计分 | 敌人击杀玩家 → 敌方加分 | 玩家击杀敌人 → 团队加分 |
| 队伍 | 红蓝两队对战 | 所有玩家同一队伍 |
| 胜负 | 分数高的队伍赢 | 团队分 ≥ 10 → 全员胜利 |
| 失败 | 对方先到目标分 | 全员阵亡 → 游戏结束 |
| 敌人 AI | 与玩家敌对（已可用） | 不变，继续追踪玩家 |
| 玩家重生 | 死亡 5 秒后重生 | 不变 |

## 四、系统设计

### 1. 游戏模式（SurvivalGameMode）

继承 `AShooterGameMode`，核心作为 **Server Only**。

职责：
- 管理团队总分（TeamScore）
- 判定胜利/失败
- 控制游戏流程

### 2. 游戏状态（SurvivalGameState）

新增 `AGameStateBase` 子类，变量全部 `Replicated`。

| 变量 | 类型 | 说明 |
|------|------|------|
| TeamScore | int32 | 团队总分 |
| TargetScore | int32 | 目标分数（10） |
| AlivePlayerCount | int32 | 存活玩家数 |
| bGameFinished | bool | 是否已结束 |

### 3. 玩家角色（SurvivalCharacter）

继承 `AShooterCharacter`，修改 `Die()` 逻辑。

改动：
- 死亡时不增加团队分（原版 PVP 逻辑）
- 死亡时通知 GameState 减少存活人数
- 增加 `Server_PlayerDied` / `Server_PlayerRespawned` RPC

### 4. 敌人（SurvivalNPC）

继承 `AShooterNPC`，修改 `Die()`。

改动：
- 死亡时调用 `GameState.AddScore(1)` → 团队加分
- 死亡广播通知 Spawner 刷新新敌人

### 5. 敌人刷新器（SurvivalNPCSpawner）

继承 `AShooterNPCSpawner`，改为持续刷新模式。

改动：
- 原版：生成 1 个 → 等它死 → 再生成 1 个
- PVE 版：持续生成，保持场上一定数量
- 检查 `bGameFinished`，结束后停止生成

### 6. UI 系统（Widget Blueprint）

#### HUD（WBP_SurvivalHUD）
- 血量条（本地玩家）
- 团队分数 / 目标分数
- 弹药数

#### 胜利界面（WBP_Victory）
- "YOU WIN"
- 最终团队分数

#### 失败界面（WBP_GameOver）
- "GAME OVER"

### 7. 网络同步

| 类型 | 内容 |
|------|------|
| Replicated 变量 | TeamScore, AlivePlayerCount, bGameFinished, Health |
| Run On Server | AddScore, PlayerDied, PlayerRespawned, SpawnNPC |
| Net Multicast | ShowVictory, ShowGameOver |

## 五、文件清单（需新建/修改）

| 操作 | 文件 | 说明 |
|------|------|------|
| 新建 | SurvivalGameMode.h/.cpp | 游戏模式 |
| 新建 | SurvivalGameState.h/.cpp | 游戏状态（团队分数） |
| 新建 | SurvivalCharacter.h/.cpp | 玩家角色（PVE 改造） |
| 新建 | SurvivalNPC.h/.cpp | 敌人（PVE 改造） |
| 新建 | SurvivalNPCSpawner.h/.cpp | 敌人刷新器 |
| 新建 | SurvivalAIController.h/.cpp | AI 控制器 |
| 修改 | FPS.Build.cs | 添加新文件路径 |
| 修改 | DefaultEngine.ini | 设为 SurvivalGameMode |
| 新建 | BP_SurvivalHUD (Widget) | HUD 控件 |
| 新建 | BP_Victory (Widget) | 胜利界面 |
| 新建 | BP_GameOver (Widget) | 失败界面 |
