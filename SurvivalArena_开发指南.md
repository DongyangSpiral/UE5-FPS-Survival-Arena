# Survival Arena（生存竞技场）— 开发指南

> 基于 UE5 First Person Template 的多人合作 PVE FPS Demo

---

## 一、项目结构

```
FPS/
├── Config/              # 项目配置
├── Content/
│   ├── Maps/            # 地图文件
│   │   └── ArenaMap    # 竞技场地图
│   ├── Blueprints/      # 蓝图
│   │   ├── Player/      # 玩家相关蓝图
│   │   │   ├── BP_FirstPersonCharacter  # 玩家角色
│   │   │   ├── BP_FirstPersonProjectile # 子弹
│   │   │   └── BP_PlayerController      # 玩家控制器
│   │   ├── Enemy/       # 敌人相关蓝图
│   │   │   ├── BP_EnemyCharacter       # 敌人角色
│   │   │   └── BP_EnemySpawner         # 敌人刷新器
│   │   ├── UI/          # UI 蓝图
│   │   │   ├── WBP_HUD                 # 游戏 HUD
│   │   │   ├── WBP_Victory             # 胜利界面
│   │   │   └── WBP_GameOver            # 失败界面
│   │   └── GameMode/    # 游戏模式
│   │       ├── BP_SurvivalGameMode     # 游戏模式
│   │       └── BP_SurvivalGameState    # 游戏状态
│   ├── StarterContent/  # 官方 Starter Content
│   └── ThirdPerson/     # 官方第三人称资源（敌人模型）
├── FPS.uproject         # 项目文件
└── SurvivalArena_开发指南.md  # 本文件
```

---

## 二、核心设计理念（PVE 合作）

| 项目 | 说明 |
|------|------|
| 游戏模式 | **多人合作 PVE**，所有玩家 vs AI 敌人 |
| 分数机制 | **团队共享分数**，存储在 GameState 中 |
| 胜利条件 | 团队总分达到目标值 → **全员胜利** |
| 失败条件 | 全员死亡 → **游戏结束** |
| 玩家死亡 | 可重生复活，继续战斗 |

---

## 三、开发流程（分步实现）

### Step 1：创建项目 & 准备地图

1. 基于 **First Person Template** 创建项目
2. 创建地图 `ArenaMap`
3. 搭建竞技场场景：
   - 地面：大平面（`BSP Box` 或 Landscape）
   - 围墙：环绕四周
   - 掩体：放置若干 `BSP Box` / Cube
   - 光照：`Directional Light` + `Sky Light`
   - 多个 `PlayerStart`（支持多人出生）
   - 多个 `BP_EnemySpawner` 放置点

---

### Step 2：玩家系统（基于模板已有功能扩展）

模板已提供：
- `BP_FirstPersonCharacter` — 第一人称角色（移动 + 视角）
- `BP_FirstPersonProjectile` — 子弹 Actor

#### BP_FirstPersonCharacter

| 变量 | 类型 | Replicated | 默认值 | 说明 |
|------|------|------------|--------|------|
| `Health` | float | 是 | 100 | 玩家血量 |
| `MaxHealth` | float | — | 100 | 最大血量 |
| `WeaponDamage` | float | — | 25 | 武器伤害 |

事件图表：
- `Event AnyDamage` → 扣血 → 判断 `Health <= 0` → `Server_PlayerDie`
- 自定义事件 `Server_PlayerDie` (Run On Server)：
  - 调用 `GameState.Server_PlayerDied`（通知 GameState 该玩家死亡）
  - 一定延迟后重生（`RestartPlayer`）

**网络设置：**
- `bReplicates = true`
- `bReplicateMovement = true`

---

### Step 3：子弹伤害

#### BP_FirstPersonProjectile

在 `OnHit` 事件中：
- `Hit Actor` 类型判断 → 是否 `BP_EnemyCharacter`
- 若是 → `Apply Damage`（伤害值 = WeaponDamage）
- 子弹 `Destroy`（`Destroy Actor`，会自动跨端同步）

---

### Step 4：敌人 AI 系统

#### BP_EnemyCharacter

继承自 `Character` 类。

| 变量 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `Health` | float (Replicated) | 50 | 敌人血量 |
| `MaxHealth` | float | 50 | 最大血量 |
| `AttackDamage` | float | 10 | 近战攻击伤害 |
| `MoveSpeed` | float | 300 | 移动速度 |
| `TargetPlayer` | Character | nullptr | 目标玩家 |

事件图表：
- `Event BeginPlay` → 设置 `Max Walk Speed = MoveSpeed`
- `Event Tick` → 调用 `FindNearestPlayer` + `AI Move To`
- `Event AnyDamage` → 扣血 → 判断 `Health <= 0` → `Server_EnemyDie`
- 自定义事件 `Server_EnemyDie` (Run On Server) → 调用 `GameState.AddScore(1)` → `Destroy`
- 自定义事件 `FindNearestPlayer` → 遍历所有玩家，找最近者
- 碰撞检测 → `OnComponentBeginOverlap` 与玩家重叠 → `Apply Damage` 给玩家（带冷却）

**AI Move To：**
```
GetAllActorsOfClass(BP_FirstPersonCharacter)
  → 遍历找出最近
  → AI Move To (TargetPlayer)
```

**网络设置：**
- `bReplicates = true`
- `bReplicateMovement = true`

**攻击冷却（防止重叠瞬间多次伤害）：**
- 新增变量 `bCanAttack` (bool)，默认 true
- `OnComponentBeginOverlap` 时检查 `bCanAttack`
- 攻击后 `bCanAttack = false`，启动计时器 1 秒后重置

---

### Step 5：敌人刷新系统

#### BP_EnemySpawner

| 变量 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `SpawnInterval` | float | 5.0 | 刷新间隔（秒） |
| `MaxEnemyCount` | int | 15 | 最大场上敌人数 |
| `EnemyClass` | BP_EnemyCharacter | — | 敌人蓝图类型 |

事件图表：
- `Event BeginPlay` → 延迟 2 秒 → 启动定时器
- 定时器触发 `SpawnEnemy`：
  - `GetAllActorsOfClass(BP_EnemyCharacter)` → 获取当前数量
  - 若 `< MaxEnemyCount` → 随机选一个 Spawner 位置 → `SpawnActor`
- 多个 Spawner 分布在地图各处

**网络：** 全部逻辑在 Server 执行，SpawnActor 自动同步到所有客户端。

---

### Step 6：团队得分系统（核心改动 — 区别于单人计分）

分数是 **团队共享** 的，存储在 GameState 中，而非单个玩家身上。

#### BP_SurvivalGameState

继承自 `GameStateBase`。

| 变量 | 类型 | Replicated | 默认值 | 说明 |
|------|------|------------|--------|------|
| `TeamScore` | int | 是 | 0 | 团队总分 |
| `TargetScore` | int | 是 | 10 | 目标分数 |
| `AlivePlayerCount` | int | 是 | 0 | 存活玩家数 |
| `bGameFinished` | bool | 是 | false | 游戏是否已结束 |

自定义事件：
- `AddScore` (Run On Server) → `TeamScore += Amount` → 判断是否达到目标
- `Server_PlayerDied` (Run On Server) → `AlivePlayerCount -= 1` → 判断是否全部阵亡
- `Server_PlayerRespawned` (Run On Server) → `AlivePlayerCount += 1`
- `Multicast_ShowVictory` (Net Multicast) → 所有客户端显示胜利 UI
- `Multicast_ShowGameOver` (Net Multicast) → 所有客户端显示失败 UI

**胜利判定：**
```
AddScore:
  → TeamScore + 1
  → if TeamScore >= TargetScore and !bGameFinished:
    → bGameFinished = true
    → Multicast_ShowVictory
```

**失败判定：**
```
Server_PlayerDied:
  → AlivePlayerCount - 1
  → if AlivePlayerCount <= 0 and !bGameFinished:
    → bGameFinished = true
    → Multicast_ShowGameOver
```

**注意：** `BeginPlay` 时统计 `AlivePlayerCount = 当前玩家总数`

---

### Step 7：游戏模式

#### BP_SurvivalGameMode

继承自 `GameModeBase`。

| 设置 | 值 |
|------|-----|
| Default Pawn Class | BP_FirstPersonCharacter |
| HUD Class | WBP_HUD |
| Player Controller Class | BP_PlayerController |
| Game State Class | BP_SurvivalGameState |

**逻辑：**
- `Event PostLogin` → 新玩家加入时更新 `GameState.AlivePlayerCount`
- 默认 GameMode 会在玩家死亡后自动处理 `RestartPlayer`

---

### Step 8：UI 系统

#### WBP_HUD（HUD）

每个玩家看到自己的血量和团队总分。

显示内容：
- `TextBlock_HP` — 自己血量
- `TextBlock_TeamScore` — **团队总分**（从 GameState 获取）
- `TextBlock_TargetScore` — 目标分数

**逻辑：**
```
Event Construct / Tick:
  → Get Player Controller → Get Controlled Pawn → Get Health
  → Get Game State → Cast to BP_SurvivalGameState → Get TeamScore
  → 更新 TextBlock
```

#### WBP_Victory（胜利界面）

显示：`YOU WIN` + 团队最终分数

- `Event Construct` → 获取 `GameState.TeamScore` 显示
- 创建方式：`GameState.Multicast_ShowVictory` 触发 → `Create Widget` → `Add to Viewport`

#### WBP_GameOver（失败界面）

显示：`GAME OVER`

- 创建方式：`GameState.Multicast_ShowGameOver` 触发 → `Create Widget` → `Add to Viewport`

---

### Step 9：胜利 / 失败机制

| 条件 | 结果 |
|------|------|
| `TeamScore >= TargetScore` | 全员胜利，显示 YOU WIN |
| `AlivePlayerCount <= 0` | 全员阵亡，显示 GAME OVER |
| 游戏结束后 | 禁用玩家输入，停止刷怪 |

**实现：**
- GameState 中 `bGameFinished = true` 后，所有执行路径先检查此标志
- Spawner 的定时器判断 `bGameFinished`，若为 true 停止生成
- 玩家角色判断 `bGameFinished`，禁止移动和射击

---

### Step 10：多人联机设置

#### 项目设置

`Edit → Project Settings → Maps & Modes`：
- Default GameMode: `BP_SurvivalGameMode`
- Default GameState: `BP_SurvivalGameState`

#### 网络同步检查清单

| 蓝图对象 | 需同步内容 |
|----------|-----------|
| BP_FirstPersonCharacter | Health, Movement, bGameFinished |
| BP_EnemyCharacter | Health, Movement, Overlap events |
| BP_FirstPersonProjectile | Movement, OnHit |
| BP_EnemySpawner | Server Only 执行 |
| BP_SurvivalGameState | TeamScore, AlivePlayerCount, bGameFinished |

**RPC 使用原则：**

| RPC 类型 | 使用场景 |
|----------|---------|
| Run On Server | AddScore, PlayerDied, PlayerRespawned, SpawnEnemy |
| Multicast | ShowVictory, ShowGameOver |

#### 注意：核心同步流程

```
[敌人被子弹击中]           → Server 端执行 AnyDamage
[敌人死亡]                → Server 端调用 GameState.AddScore(1)
[GameState.AddScore]     → Server 端 TeamScore + 1 → 自动同步到所有客户端
[判断胜利]                → Server 端 Multicast_ShowVictory
[所有客户端]              → 显示 YOU WIN

[玩家死亡]                → Server 端 GameState.Server_PlayerDied
[GameState 判断]          → AlivePlayerCount <= 0 → Multicast_ShowGameOver
[所有客户端]              → 显示 GAME OVER
```

#### 启动 Listen Server

编辑器：
- `Play → Number of Players = 2~4` → `Net Mode = Play As Listen Server`

---

## 四、关键蓝图节点速查

### 敌人死亡 → 团队加分

```
Enemy AnyDamage (Event) → Health - Damage
  → Branch (Health <= 0)
    → Server_EnemyDie (Run On Server)
      → Get GameState → Cast to BP_SurvivalGameState
      → AddScore (1)
      → Destroy Actor
```

### 团队加分 → 胜利判断

```
AddScore (Custom Event, Run On Server):
  → TeamScore + Amount
  → Branch (TeamScore >= TargetScore && !bGameFinished)
    → bGameFinished = true
    → Multicast_ShowVictory (Net Multicast)
```

### 玩家死亡 → 失败判断

```
Server_PlayerDied (Custom Event, Run On Server):
  → AlivePlayerCount - 1
  → Branch (AlivePlayerCount <= 0 && !bGameFinished)
    → bGameFinished = true
    → Multicast_ShowGameOver (Net Multicast)
```

### 敌人 AI 追踪

```
Event Tick (只在 Server 执行):
  → GetAllActorsOfClass (BP_FirstPersonCharacter)
  → 遍历 → 计算距离 → 选最近
  → AI Move To (Target = TargetPlayer)
```

---

## 五、与单人竞技模式的关键差异

| 单人竞技（原方案） | 多人合作 PVE（现方案） |
|---|---|
| 分数在 Player 身上 | **分数在 GameState 上（团队共享）** |
| 每个玩家独立计分 | 所有玩家共享一个 TeamScore |
| 自己达到 10 分就赢 | **团队达到 10 分，全员一起赢** |
| 自己死亡就 GAME OVER | **全员死亡才 GAME OVER** |
| 单人游戏 | 2~4 人合作 |
| 敌人追踪最近的"单个玩家" | 敌人追踪最近的"任意玩家"（逻辑相同） |

---

## 六、调试与测试

| 测试内容 | 方法 |
|----------|------|
| 合作模式 | Play → Number of Players = 2~4 |
| 分数同步 | 击杀敌人 → 所有客户端分数同时增加 |
| 全员阵亡 | 让所有玩家死亡 → 应显示 GAME OVER |
| 团队胜利 | 团队分数达到目标 → 应显示 YOU WIN |
| 网络同步 | 打开 `Stat Replication` 查看同步状态 |
| AI 行为 | 查看 AI Move To 是否正确追踪最近玩家 |

---

## 七、常见问题

| 问题 | 解决方法 |
|------|---------|
| 分数不同步 | 确认 `TeamScore` 设为 Replicated，`AddScore` 设为 Run On Server |
| 胜利不触发 | 确认 `Multicast_ShowVictory` 设为 Net Multicast |
| 某个玩家死亡就 Game Over | 确认失败条件是 `AlivePlayerCount <= 0`，不是单个玩家死亡 |
| 敌人不打人 | 检查 `OnComponentBeginOverlap` 和攻击冷却变量 |
| 重生后分数归零 | 分数是团队分数，在 GameState 上，不会随重生重置 |
| 新加入玩家没有 HUD | 检查 HUD Class 是否在 GameMode 中设置 |
