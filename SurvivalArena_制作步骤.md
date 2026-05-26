# Survival Arena — 制作步骤

## 前置：修复 VS 编译问题

打开 **Visual Studio Installer** → Visual Studio 2022 → **修改** → **工作负荷** 勾选 **使用 C++ 的桌面开发** → 右下角 **修改** → 完成后重启电脑。

## Step 1：新建 C++ 类

VS 修复后，在 UE 编辑器中操作：

| 操作 | 说明 |
|------|------|
| 菜单 → Tools → New C++ Class | 或右键 Content 面板 → New C++ Class |
| 创建 SurvivalGameState | 继承 GameStateBase，Public 类 |
| 创建 SurvivalGameMode | 继承 AShooterGameMode |
| 创建 SurvivalCharacter | 继承 AShooterCharacter |
| 创建 SurvivalNPC | 继承 AShooterNPC |
| 创建 SurvivalAIController | 继承 AShooterAIController |
| 创建 SurvivalNPCSpawner | 继承 AShooterNPCSpawner |

创建完后 UE 会自动生成文件到 `Source/FPS/`，并提示编译。

> **注意：** 如果编译失败，截报错图发给我修。

## Step 2：写 C++ 逻辑（我来写）

新建的类都是空壳，我逐个写代码：

1. **SurvivalGameState** — 团队分数、目标分数、存活人数、游戏结束标志
2. **SurvivalGameMode** — 绑定 GameState，初始化游戏
3. **SurvivalCharacter** — 改造死亡逻辑，通知 GameState
4. **SurvivalNPC** — 改造死亡逻辑，给团队加分
5. **SurvivalNPCSpawner** — 改为持续刷怪模式
6. **SurvivalAIController** — 感知玩家、AI Move To

每个文件你告诉我已生成，我就写代码进去。

## Step 3：配置文件

我直接改你的 `DefaultEngine.ini` 和 `FPS.Build.cs`。

## Step 4：蓝图 UI（编辑器操作）

| 操作 | 说明 |
|------|------|
| 右键 Content → Widget Blueprint | BP_SurvivalHUD |
| 拖入 TextBlock | 绑定血量、分数 |
| 创建 BP_Victory | 显示 YOU WIN |
| 创建 BP_GameOver | 显示 GAME OVER |

## Step 5：搭地图

| 操作 | 说明 |
|------|------|
| File → New Level → Empty | 保存为 Maps/ArenaMap |
| 拉地面 + 围墙 | BSP Box |
| 放 Directional Light | 光照 |
| 放 4 个 PlayerStart | 玩家出生点 |
| 放 3~4 个 SurvivalNPCSpawner | 敌人刷新点 |

## Step 6：测试

编辑器 → Play → Number of Players = 2~4 → Net Mode = Listen Server
