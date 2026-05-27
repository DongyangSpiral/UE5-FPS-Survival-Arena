# Survival Arena（生存竞技场）— 项目策划案

## 一、项目概述

| 项目 | 内容 |
|------|------|
| 项目名称 | Survival Arena（生存竞技场） |
| 游戏类型 | 多人竞技 PVE 第一人称射击 |
| 开发引擎 | Unreal Engine 5.7（C++） |
| 基础模板 | UE5 Shooter C++ Template（Variant_Shooter） |
| 联机模式 | Listen Server（最多 4 人） |
| 背景故事 | 近未来，"奥米加"集团创办全球直播真人秀。参赛者植入"战斗芯片"，击杀怪物积累积分以解锁更强武器。第一个达到 10 分的人活着离开 |

## 二、核心玩法

### 游戏目标
玩家在竞技场中击杀 AI 敌人获得分数，**第一个达到 10 分的玩家获胜**。

### 游戏规则

| 规则 | 说明 |
|------|------|
| 胜利条件 | 个人分数 ≥ 10 分 |
| 玩家死亡 | 5 秒后重生，**个人分数归零**，武器等级重置 |
| 敌人刷新 | 场上保持一定数量敌人，死一个补一个 |
| 得分方式 | 击杀 1 个敌人 → **最后一击者 +1 分** |
| 子弹升级 | 分数越高，子弹效果越强（伤害/速度/散射） |

### 操作方式

| 按键 | 操作 |
|------|------|
| WASD | 移动 |
| 鼠标移动 | 视角控制 |
| 鼠标左键 | 射击 |
| 空格 | 跳跃 |

## 三、系统设计

### 1. 玩家数据 — PlayerState（新建）

| 变量 | 类型 | Replicated | 说明 |
|------|------|------------|------|
| `Score` | int32 | 是 | 个人分数 |
| `WeaponTier` | int32 | 是 | 子弹等级（0~3） |

**规则：**
- 死亡时 `Score = 0`，`WeaponTier = 0`
- 分数变化时更新 `WeaponTier`（如 3 分 → Tier 1，6 分 → Tier 2）

### 2. 游戏状态 — GameState

| 变量 | 类型 | Replicated | 说明 |
|------|------|------------|------|
| `WinnerName` | string | 是 | 获胜者名字 |
| `bGameFinished` | bool | 是 | 是否已结束 |

### 3. 玩家角色 — MyCharacter

| 功能 | 说明 |
|------|------|
| 射击 | 根据 `WeaponTier` 选择对应的 ProjectileClass |
| 死亡 | 分数归零 → 5 秒重生 |
| NPC 最后一击判定 | 通过 `DamageCauser` 追溯开枪者 |

### 4. 敌人 — MyNPC

死亡链条：
```
子弹命中 NPC → TakeDamage → HP <= 0
  → 找出 DamageCauser 的 InstigatorController
  → 找到对应的 PlayerState
  → PlayerState.Score += 1
  → 更新 WeaponTier
  → 判断 PlayerState.Score >= 10
    → GameState.Multicast_ShowVictory(WinnerName)
```

### 5. 子弹升级系统

| 等级 | 触发条件 | 子弹效果 |
|------|---------|---------|
| Tier 0 | Score < 3 | 基础子弹，伤害 25 |
| Tier 1 | Score ≥ 3 | 伤害 40，子弹变大 1.2x |
| Tier 2 | Score ≥ 6 | 伤害 60，子弹变大 1.5x，速度提升 |
| Tier 3 | Score ≥ 9 | 伤害 80，双发散射，子弹变大 2x |

每个等级对应一个 ProjectileClass 蓝班子类，由 `Fire()` 根据 `WeaponTier` 选择。

### 6. 排行榜 UI

```
┌─────────────┐
│ 排行榜      │
│ 1. PlayerA 3 │
│ 2. 你     2 │
│ 3. PlayerB 1 │
│ 目标: 10分   │
└─────────────┘
```

- 左侧排名，显示所有玩家名字和分数
- 自己高亮显示
- 实时更新

### 7. 网络同步

| 类型 | 内容 |
|------|------|
| Replicated 变量 | PlayerState.Score, PlayerState.WeaponTier, GameState.WinnerName, GameState.bGameFinished |
| Run On Server | TakeDamage, OnNPCDeath, AddScore, PlayerDied |
| Net Multicast | ShowVictory |

## 四、文件清单

| 操作 | 文件 | 说明 |
|------|------|------|
| 新建 | MyPlayerState.h/.cpp | 个人分数 + 子弹等级 |
| 修改 | MyCharacter.h/.cpp | 根据 WeaponTier 发射不同子弹，死亡重置分数 |
| 修改 | MyNPC.h/.cpp | 死亡时给最后一击者加分 |
| 修改 | MyGameStateBase.h/.cpp | 改为个人胜负判定，移除团队分数 |
| 新建 | BP_Bullet_T0 (Widget) | 基础子弹蓝图 |
| 新建 | BP_Bullet_T1 (Widget) | Tier 1 子弹蓝图 |
| 新建 | BP_Bullet_T2 (Widget) | Tier 2 子弹蓝图 |
| 新建 | BP_Bullet_T3 (Widget) | Tier 3 子弹蓝图 |
| 新建 | WBP_Leaderboard (Widget) | 排行榜控件 |
| 新建 | WBP_Victory (Widget) | 胜利界面（显示赢家名字） |
