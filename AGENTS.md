# Survival Arena — Agent 开发指南

- 每次回答要以'spiral'称呼用户

## 项目概览

| 属性 | 值 |
|------|-----|
| 引擎 | Unreal Engine 5.7 |
| 项目类型 | 多人竞技 PVE FPS |
| 网络模式 | Listen Server（最多 4 人） |
| 开发语言 | **C++ 优先**（仅在必要时用蓝图） |
| 基础模板 | UE5 Shooter C++ Template（Variant_Shooter） |
| 胜利条件 | 个人先到 10 分 |
| 失败条件 | 场上存活人数 = 0 |

## 项目结构

```
Source/FPS/
├── FPS.h/.cpp                 # 模块入口 + LogFPS 日志类别
├── FPS.Build.cs               # 模块依赖（AIModule, StateTreeModule, UMG 等）
├── FPSCharacter.h/.cpp        # 基础第一人称角色（移动/视角/跳跃）
├── FPSGameMode.h/.cpp         # 基础 GameMode（抽象基类）
├── FPSPlayerController.h/.cpp # 输入映射管理
├── FPSCameraManager.h/.cpp    # 相机管理
│
├── Variant_Shooter/           # UE Shooter 模板（不修改）
│   ├── ShooterCharacter       # 射击角色（武器/血量/死亡/重生）
│   ├── ShooterGameMode        # 射击 GameMode（团队计分）
│   ├── Weapons/               # 武器系统（枪械/弹道/拾取）
│   └── AI/
│       ├── ShooterNPC         # NPC（AI 控制的射击角色）
│       ├── ShooterAIController# AI 控制器（感知 + StateTree）
│       └── ShooterNPCSpawner  # 逐个刷怪（死一个刷一个）
│
├── MyCharacter.h/.cpp         # 玩家角色（继承 ShooterCharacter）
├── MyPlayerState.h/.cpp       # 个人分数+武器等级
├── MyGameModeBase.h/.cpp      # PVE GameMode（继承 GameModeBase）
├── MyGameStateBase.h/.cpp     # PVE GameState（胜负判定）
└── MyNPC.h/.cpp               # PVE NPC（继承 ShooterNPC，死时加分）
```

## 类继承链

```
AFPSCharacter (移动/视角)
  └── AShooterCharacter (武器/血量/死亡/重生)
        └── AMyCharacter (PVE 改造)

AShooterNPC (AI 控制的射击角色)
  └── AMyNPC (PVE: 死亡时给最后一击者加分)

AGameModeBase
  └── AMyGameModeBase (PVE: 绑定 GameState/管理玩家)

AGameStateBase
  └── AMyGameStateBase (PVE: 胜负判定/WinnerName)

APlayerState
  └── AMyPlayerState (个人 Score/WeaponTier)
```

## 软件工程原则

### 低耦合 · 高内聚

每个类的职责边界必须清晰，避免跨层直接访问：

| 类 | 职责范围 | 不做什么 |
|---|---|---|
| `AMyGameStateBase` | 胜负判定（Score ≥ TargetScore / AliveCount ≤ 0） | 不碰伤害、不计个人分 |
| `AMyPlayerState` | 持有 Score / WeaponTier | 不参与游戏流程编排 |
| `AMyGameModeBase` | 登录/登出/重生编排 | 不直接计分 |
| `AMyCharacter` | 射击 + 死亡响应 | 不直接修改 GameState |
| `AMyNPC` | 死亡时追溯开枪者 | 不关心加分后的逻辑 |

### 项目中使用的设计模式

| 模式 | 使用位置 | 作用 |
|---|---|---|
| **观察者** | `OnPawnDeath`、`OnDamaged`、`OnTeamScoreChanged` 等 Delegate | 事件驱动，发布者与订阅者解耦 |
| **策略** | `Fire()` 按 `WeaponTier` 选 ProjectileClass | 子弹行为可替换，无需改 Fire() 本身 |
| **工厂** | `GetWorld()->SpawnActor<T>()` | UE 标准创建方式，隐藏构造细节 |
| **中介者** | `AMyGameModeBase` 协调 GameState / PlayerState / Pawn | 类之间不直接引用，由 GameMode 中转 |
| **组件** | UE 原生 `UActorComponent`（Camera / Movement / Weapon） | 功能组合替代深层继承 |
| **状态** | `bGameFinished` 控制流程 / AI StateTree 控制 NPC 行为 | 状态显式化，避免 if-else 泛滥 |

### UE C++ 设计要点

- **头文件最小化**：能用前置声明就不要 `#include`，避免编译链爆炸
- **RAII**：利用 UE 的 `UPROPERTY()` 管理引用计数，避免手动 Delete
- **const 正确性**：只读方法标记 `const`，输入参数传 `const&`
- **网络透明**：`Replicated` 变量自动同步，Server 端修改后客户端自动更新，无需手动 RPC
- **UClass 反射**：`UPROPERTY` / `UFUNCTION` 不仅决定 GC，还决定序列化、网络复制和蓝图暴露

## 编码规范

### C++ 优先原则
- 所有游戏逻辑用 C++ 实现，蓝图仅用于 UI、动画蓝图、数据资产
- 新增功能优先在 `My*` 类中扩展，不修改 `Variant_Shooter/` 模板代码

### 命名规范
- 类名：`AFPS` / `AMy` 前缀（Actor），`U` 前缀（UObject），`F` 前缀（struct）
- 变量：`PascalCase`
- 函数：`PascalCase`
- 布尔变量：`b` 前缀（`bGameFinished`、`bIsDead`）
- 成员变量不要加 `m_` 或 `_` 前缀，直接 PascalCase

### UE 规范
- 所有 UCLASS 宏中标记 `abstract` 的基类不可直接实例化
- 网络变量必须标记 `UPROPERTY(Replicated)` 并在 `GetLifetimeReplicatedProps` 注册
- 服务器执行函数标记 `UFUNCTION(Server, Reliable)`，多播标记 `NetMulticast`
- 日志用 `UE_LOG(LogFPS, Warning/Error, TEXT(...))`

## 网络规则

### RPC 类型选择
| 类型 | 使用场景 |
|------|---------|
| `Server, Reliable` | 玩家输入、计分、死亡通知 |
| `NetMulticast, Reliable` | UI 通知（胜利/失败） |
| `Client, Reliable` | 客户端特定通知 |

### 关键原则
- **服务器权威**：所有游戏逻辑（伤害、计分、刷怪）在 Server 端执行
- `Replicated` 变量自动同步到所有客户端
- AI 和刷怪逻辑只在 Server 运行（`HasAuthority()` 检查）
- `SpawnActor` 在 Server 调用后会自动在客户端同步生成

### RPC 调用链（核心流程）
```
子弹命中 NPC → Server 端 TakeDamage → NPC 死亡
  → 追溯 DamageCauser → InstigatorController → PlayerState
  → PlayerState.Score += 1 (Server Only)
  → Score 自动同步到所有客户端
  → Score >= TargetScore → Multicast_ShowVictory(WinnerName)
```

## PVE 核心逻辑规则

### GameState（AMyGameStateBase）
- `WinnerName`：获胜者名字，`Replicated`
- `TargetScore`：目标分数（默认 10），`EditDefaultsOnly`
- `AlivePlayerCount`：存活玩家数，`Replicated`
- `bGameFinished`：游戏是否结束，`Replicated`
- 胜利：有人 Score >= TargetScore && !bGameFinished
- 失败：AlivePlayerCount <= 0 && !bGameFinished

### PlayerState（AMyPlayerState）
- `Score`：个人分数，`Replicated`
- `WeaponTier`：武器等级（0~3），`Replicated`
- 分数变化时更新 WeaponTier
- 死亡时 Score = 0, WeaponTier = 0

### GameMode（AMyGameModeBase）
- 设置 `GameStateClass = AMyGameStateBase`
- 设置 `PlayerStateClass = AMyPlayerState`
- `PostLogin`：调用 `GameState.OnPlayerRespawned()`
- `Logout`：调用 `GameState.OnPlayerDied()`
- `BeginPlay`：统计初始存活玩家数

### 角色（AMyCharacter）
- `TakeDamage`：扣血 → 死亡 → `GameState.OnPlayerDied()`，分数归零
- `Fire()`：根据 PlayerState.WeaponTier 选择对应子弹
- 死亡后 DisableInput + 计时器 5 秒 → `OnRespawn` → `Destroy()`
- GameMode 自动 RestartPlayer 生成新 Pawn

### NPC（AMyNPC）
- 继承 `AShooterNPC`
- `TakeDamage`：扣血 → 死亡 → 追溯最后一击者 PlayerState → Score += 1
- 不修改 `Variant_Shooter` 模板代码

## Unity → UE 对照表

| Unity | Unreal Engine |
|-------|---------------|
| GameObject | AActor |
| MonoBehaviour | UActorComponent（或 AActor 的 `BeginPlay`/`Tick`）|
| Transform | USceneComponent（`SetActorLocation`/`SetActorRotation`）|
| Instantiate | `GetWorld()->SpawnActor<>()` |
| Destroy(obj) | `obj->Destroy()` |
| Start() | `BeginPlay()` |
| Update() | `Tick(float DeltaTime)` |
| GetComponent<T>() | `FindComponentByClass<T>()` 或 Cast |
| SceneManager.LoadScene | `GetWorld()->ServerTravel()` |
| NetworkBehaviour | `AActor` + `bReplicates = true` |
| [Command] | `UFUNCTION(Server, Reliable)` |
| [ClientRpc] | `UFUNCTION(NetMulticast, Reliable)` |
| [SyncVar] | `UPROPERTY(Replicated)` |
| Input.GetAxis | Enhanced Input + UInputAction |
| Physics.Raycast | `GetWorld()->LineTraceSingleByChannel()` |
| OnCollisionEnter | `OnHit` / `OnComponentBeginOverlap` |
| public SerializeField | `UPROPERTY(EditAnywhere, BlueprintReadWrite)` |
| Coroutine | `FTimerHandle` + `GetWorld()->GetTimerManager()` |
| ScriptableObject | UDataAsset（继承 UDataAsset）|
| Transform.position | `GetActorLocation()` |
| transform.rotation | `GetActorRotation()` |

## 构建与测试

## 编译方式

1. **日常编译（推荐）：** 在 UE 编辑器中按 **Alt+Ctrl+F11**（Live Compile），无需重启编辑器
2. **完整编译：** 只在 **Live Compile 失败** 时，才关闭编辑器执行以下命令：

```powershell
# 生成 VS 工程文件
D:\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe -projectfiles -project="$(pwd)\FPS.uproject" -game -engine

# 编译
"D:\UE_5.7\Engine\Build\BatchFiles\Build.bat" FPSEditor Win64 Development "$(pwd)\FPS.uproject" -waitmutex
```

编辑器启动 / Listen Server 测试：

```powershell
# 从编辑器启动
start "" "D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "$(pwd)\FPS.uproject"

# Listen Server 测试
# 编辑器 → Play → Number of Players = 2~4 → Net Mode = Play As Listen Server
```

## AI 助手行为准则

- 每次回答必须以 `spiral` 称呼用户开头


- 优先阅读 `SurvivalArena_策划案.md`、`SurvivalArena_开发指南.md`、`SurvivalArena_制作步骤.md` 了解完整需求
- 所有新 C++ 类应在 `My*` 命名空间下，不修改 `Variant_Shooter/` 模板代码
- 修改代码前先检查 `MyCharacter` / `MyGameStateBase` / `MyGameModeBase` / `MyNPC` 是否已有相关逻辑，优先扩展现有类
- 网络变量必须标记 `Replicated` 并在 `GetLifetimeReplicatedProps` 注册
- Server Only 逻辑必须先检查 `HasAuthority()`
- 修改 `DefaultEngine.ini` 需要确保 GameMode 路径正确
- 添加模块依赖时同步更新 `FPS.Build.cs` 和 `FPS.uproject`
- 遇到 UE 特有概念（StateTree、EnhancedInput）先查模板代码中的使用方式
- 注释保持简洁（仅当代码意图不清晰时加注释）
- 修改完成后运行编译确保无报错
- 不要提交文件到 git，除非用户明确要求
