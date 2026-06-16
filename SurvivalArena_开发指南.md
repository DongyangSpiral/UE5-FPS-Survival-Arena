# Survival Arena（生存竞技场）— 开发指南

> 基于 UE5 Shooter C++ Template 的多人竞技 PVE FPS

---

## 一、项目结构

```
Source/FPS/
├── FPS.h/.cpp                # 模块入口 + LogFPS 日志类别
├── FPS.Build.cs              # 模块依赖
├── FPSCharacter.h/.cpp       # 基础第一人称角色（模板，不改）
├── FPSGameMode.h/.cpp        # 基础 GameMode（模板，不改）
├── FPSPlayerController.h/.cpp# 玩家控制器（模板，不改）
│
├── Variant_Shooter/          # UE Shooter 模板（不改）
│   ├── ShooterCharacter      # 射击角色（武器/血量/重生）
│   ├── ShooterGameMode       # 射击 GameMode
│   ├── Weapons/              # 武器系统
│   └── AI/                   # AI 敌人
│
├── MyCharacter.h/.cpp        # 玩家：射击(等级子弹) / 死亡重置分数
├── MyPlayerState.h/.cpp      # 个人分数 / WeaponTier / 网络同步
├── MyGameStateBase.h/.cpp    # 胜负判定 / WinnerName
├── MyGameModeBase.h/.cpp     # 绑定 PlayerState 类
└── MyNPC.h/.cpp              # 敌人：死亡给最后一击者加分
```

---

## 二、核心设计

| 项目 | 说明 |
|------|------|
| 游戏模式 | **个人竞技 PVE**，所有玩家竞争击杀 NPC |
| 分数归属 | **PlayerState**（死亡不丢分？方案：死亡重置） |
| 胜利条件 | **个人先到 10 分 → 该玩家获胜** |
| 失败条件 | **场上存活人数 = 0（全员阵亡）** |
| 子弹升级 | 分数越高子弹越强，死亡重置 |
| 排行榜 | 显示所有玩家分数，自己高亮 |
| NPC 击杀归属 | **最后一击者**获得 1 分 |

---

## 三、核心数据流

### NPC 击杀 → 加分 → 升级 → 胜利

```
子弹命中 NPC → TakeDamage(NPC)
  → NPC HP <= 0
  → 追溯 DamageCauser → InstigatorController → PlayerState
  → PlayerState.Score += 1
  → 更新 WeaponTier
  → PlayerState.Score >= 10
    → GameState.Multicast_ShowVictory(PlayerName)
```

### 玩家死亡 → 重置

```
玩家受伤 → HP <= 0
  → MyGameState.PlayerDied(PlayerState)
  → PlayerState.Score = 0, WeaponTier = 0
  → 5 秒计时器 → 重生
```

### 射击 → 按等级选子弹

```
Fire()
  → 获取 PlayerState.WeaponTier
  → 选择对应的 ProjectileClass (T0/T1/T2/T3)
  → SpawnActor 生成子弹
```

---

## 四、子弹等级配置

| Tier | 分数阈值 | ProjectileClass | 伤害 | 效果 |
|------|---------|----------------|------|------|
| 0 | < 3 | BP_Bullet_T0 | 25 | 基础子弹 |
| 1 | ≥ 3 | BP_Bullet_T1 | 40 | 变大 1.2x |
| 2 | ≥ 6 | BP_Bullet_T2 | 60 | 变大 1.5x + 加速 |
| 3 | ≥ 9 | BP_Bullet_T3 | 80 | 双发散射 + 变大 2x |

---

## 五、排行榜 UI

```
┌─────────────────┐
│  排行榜          │
│  1. PlayerA   3  │
│  2. 你        2 ← 高亮
│  3. PlayerB   1  │
│  目标: 10 分    │
└─────────────────┘
```

- 每帧（或分数变化时）遍历所有 PlayerState
- 按 Score 降序排列
- 本地玩家用不同颜色/背景高亮

---

## 六、网络同步清单

| 类型 | 内容 |
|------|------|
| Replicated | PlayerState.Score, PlayerState.WeaponTier, GameState.WinnerName, GameState.bGameFinished |
| Run On Server | TakeDamage, OnNPCDeath, ScoreChange, PlayerDied |
| Multicast | ShowVictory |

---

## 七、与旧版合作模式差异

| 合作模式（旧） | 竞技 PVE（新） |
|---|---|
| 团队共享分数 | **个人分数（PlayerState）** |
| 全员一起赢 | **个人先到 10 分获胜** |
| 全员阵亡才输 | **全员阵亡 = 游戏结束** |
| 分数死亡不丢 | **死亡分数归零** |
| 无子弹升级 | **分数越高子弹越强** |
| 无排行榜 | **实时排行榜** |
