# Survival Arena — 制作步骤（竞技 PVE 版）

## 前置：确保 VS 可编译

VS 2022 需要 MSVC 14.44+，打开 **Visual Studio Installer** → 修改 → 勾选 **使用 C++ 的桌面开发** → 安装。

## Step 1：新建 C++ 类

编辑器 → Tools → New C++ Class：

| 类名 | 父类 | 说明 |
|------|------|------|
| MyPlayerState | PlayerState | 个人分数 + WeaponTier |

MyCharacter、MyGameStateBase、MyGameModeBase、MyNPC 这四个类已经存在。

> **注意：** 创建完后如果编译报错，不要慌，截报错图发给我修。

## Step 2：我写 C++ 逻辑

| 文件 | 改动 |
|------|------|
| MyPlayerState.h/.cpp | 新建 → Score、WeaponTier (Replicated)、GetLifetimeReplicatedProps、更新逻辑 |
| MyCharacter.h/.cpp | 删除旧 GunMesh/WeaponMesh 逻辑，Fire() 根据 WeaponTier 选子弹，死亡时重置分数 |
| MyGameStateBase.h/.cpp | 改为个人胜负：WinnerName、Multicast_ShowVictory、移除团队分数逻辑 |
| MyGameModeBase.h/.cpp | 绑定 PlayerStateClass = MyPlayerState |
| MyNPC.h/.cpp | TakeDamage 死亡时找最后一击者 → 该 PlayerState.Score += 1 |

## Step 3：创建子弹蓝图层

Content 中创建 4 个子弹蓝图，都继承自 `AShooterProjectile` 或 `BP_ShooterProjectile_Bullet`：

| 蓝图 | 伤害 (HitDamage) | 大小 (Scale) | 速度 (InitialSpeed) | 备注 |
|------|-----------------|-------------|-------------------|------|
| BP_Bullet_T0 | 25 | 1.0 | 2000 | 基础子弹 |
| BP_Bullet_T1 | 40 | 1.2 | 2500 | 变大加速 |
| BP_Bullet_T2 | 60 | 1.5 | 3000 | 更大更快 |
| BP_Bullet_T3 | 80 | 2.0 | 3500 | 双发散射 |

BP_Bullet_T3 需要额外处理：在 `Fire()` 中一次 `SpawnActor` 两次（左右偏移各 10 度）。

## Step 4：设置项目

| 操作 | 位置 |
|------|------|
| 设 GameMode | Project Settings → Maps & Modes → Default GameMode = BP_SurvivalGameMode |
| 设 GameState | BP_SurvivalGameMode → Classes → Game State Class = BP_SurvivalGameState |
| 设 Pawn | BP_SurvivalGameMode → Classes → Default Pawn Class = BP_SurvivalCharacter |

## Step 5：创建 UI 蓝图

| 蓝图 | 说明 |
|------|------|
| WBP_Leaderboard | 排行榜：遍历所有 PlayerState → 按 Score 排序 → 显示排名 |
| WBP_Victory | 胜利界面：显示 "XXX WINS!" |

排行榜与 HUD 合并为一个 Widget，放在屏幕左上方。

## Step 6：搭地图

| 操作 | 说明 |
|------|------|
| File → New → Level | 保存为 Maps/ArenaMap |
| 地面 + 围墙 | BSP Box |
| Directional Light + Sky Light | 光照 |
| 4 个 PlayerStart | 玩家出生点 |
| 3~4 个 BP_ShooterNPCSpawner | 敌人刷新点 |

## Step 7：测试

编辑器 → Play → Number of Players = 2~4 → Net Mode = Play As Listen Server

| 测试内容 | 方法 |
|---------|------|
| 个人计分 | 击杀 NPC → 自己分数 +1 |
| 分数同步 | 其他玩家能看到你的分数变化 |
| 子弹升级 | 打到 3 分 → 换子弹 → 伤害变高 |
| 死亡重置 | 死后分数归零，子弹降回 T0 |
| 排行榜排序 | 分数高的排上面，自己高亮 |
| 胜利判定 | 有人到 10 分 → 显示 XXX WINS |
