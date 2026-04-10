# UXTools - Static Hand Pose to UE Key Mapping (Design Draft)

本文件记录“**静态手势（Static Hand Pose）映射到 UE 按钮（FKey）**”功能的设计框架与落地建议，用于在开始编码前对齐结构与边界。

> 目标：在运行时识别静态手势，并将其映射为 10 个自定义按键：`Custom_HandPoseKey_1` ~ `Custom_HandPoseKey_10`，供 UE Input/Enhanced Input 绑定 UI/按钮/交互逻辑使用。

> 边界：**手部原始数据统一从 UE 的 `GetMotionControllerData` 接口获取**，不依赖 `UXTools` 其他运行时模块的数据获取链路；本模块应保持自包含，仅在调试/开发文档中说明可参考的现有能力。

---

## 需求拆解（按维度）

### 1) 掌心朝向相机的 6 向 Bitmask

- **配置**：每个手势条目可配置允许的掌心朝向：上/下/左/右/前/后，且允许多选（Bitmask）。
- **判定**：每帧根据 Palm normal 与 Camera 的 `Up/Right/Forward` 做投影，得到离散方向 `CurrentDirBit`，用 bitmask 判断是否允许。

建议枚举（示意）：

- `EUxtPalmCameraDirection`：`Up/Down/Left/Right/Forward/Backward`（`UENUM(meta=(Bitflags,...))`）
- 手势条目字段：`AllowedPalmDirs`（`meta=(Bitmask, BitmaskEnum="EUxtPalmCameraDirection")`）

### 1.1) 手部原始数据输入契约（必须固定）

运行时每帧只从 UE 原生手部追踪接口拉取一次数据，并缓存为统一结构，供所有评估器复用：

- 数据来源：`GetMotionControllerData`
- 不依赖：`UXTools` 现有 Hand Tracker / Input / Interaction 模块
- 单手运行时数据建议：`FUxtTrackedHandFrame`
  - `bIsTracked`
  - `Hand`（Left / Right）
  - `WorldSpaceJointTransforms`（至少覆盖参与识别、PalmDir、PalmWidth 所需全部骨骼）
  - `JointValidMask`
  - `WristTransform`
  - `PalmBasis`（运行时从关节推导，不单独持久化）
  - `PalmWidth`
  - `FrameNumber` / `TimeSeconds`
- 读取规则：子系统在 Tick 开始阶段统一拉取左右手数据，同帧内所有 Binding 共用该快照，避免不同识别器各自取数导致时序不一致。

### 1.2) 手掌坐标系与法向量定义（必须文档化）

- **手部局部坐标系原点**：`Wrist` 骨骼位置
- **掌心法向量**：取“手掌根部到小拇指根部”与“手掌根部到食指根部”两个向量做叉乘得到
- **位置识别参考空间**：统一转换到以 `Wrist` 为基准的手部局部空间
- **PalmDir 判定空间**：推荐在世界空间计算掌心法向量，再与当前相机的 `Up/Right/Forward` 做 dot 后离散为 6 向
- **镜像前提**：上述局部坐标轴方向必须在录入和运行时完全一致，否则镜像结果不可用

### 2) 识别方法（每个手势仅允许一种算法）

每个静态手势条目只允许选择**一种**主要识别算法，不做多算法组合。原因：

- 组合算法会增加运行时评估成本
- 不同算法的数据结构难以自然合并
- 参数调试和录入成本会明显上升

其中**涉及位置/距离**的算法建议都支持“按手掌尺度归一化”，避免大小手（男性/女性/个体差异）导致阈值体验不一致：

- **A. 手指根部旋转相似度（Rotation Similarity）**
  - 配置：关节列表（如 `Metacarpal/Proximal`）、参考旋转（左右手各一套）、阈值（最大角度或最小相似度）。
  - 运行时：计算 `Quat` delta（或矩阵差），聚合（平均/最大/加权）得到分数。

- **B. 指定关节位置范围（Joint Position Bounds）**
  - 配置：关节列表（如 `ThumbTip/IndexTip/Palm/Wrist`）、每个关节的位置容差（盒/球）、参考空间（推荐手部局部空间）、**是否启用手掌尺度归一化**（例如按 PalmWidth）。
  - 运行时：将关节位置变换到手部局部空间后判断是否落入范围；若启用归一化，则用 \( \text{NormalizedPos} = \text{LocalPos} / \text{PalmWidth} \)（或等价方式）再与阈值比较；按规则聚合（全通过 / 允许一定比例）。

- **C. 指定关节距离/捏合距离（Joint Distance / Pinch Distance）**
  - 典型用法：捏合可只比较 `ThumbTip` 与 `IndexTip`（或指定两/多关节）的距离。
  - 问题：绝对距离阈值对不同手掌大小敏感，误差较大。
  - 建议：与手掌宽度（PalmWidth）做比例比较，使用无量纲阈值，例如 \( d / \text{PalmWidth} < T \)；也可扩展为多对距离的加权聚合。

建议抽象：

- `UUxtStaticPoseDefinition`（`UDataAsset`，配置与录入数据载体）
  - `CreateEvaluator()`
- `FUxtStaticPoseEvaluator`（纯 C++ 评估器基类）
  - `Evaluate(CurrentHandData, PoseHandData, OutScore/OutState)`
- 实现类：`UUxtStaticPoseRotationDefinition`、`UUxtStaticPoseBoundsDefinition`、`UUxtStaticPoseDistanceDefinition`
- 评估器实现：`FUxtStaticPoseRotationEvaluator`、`FUxtStaticPoseBoundsEvaluator`、`FUxtStaticPoseDistanceEvaluator`

### 2.1) 统一评分语义

所有识别算法都输出统一语义，便于仲裁、调试和状态机复用：

- `Score`：范围固定为 `0.0 ~ 1.0`
- `bPassed`：是否满足当前算法阈值
- `FailureReason`：调试用途，如 `MissingJoint` / `PalmDirRejected` / `BelowThreshold`
- `PrimaryDebugValue`：算法核心调试值，如最大角度误差、归一化距离、超界关节名等

Binding 层只依赖统一 `Score` 和 `bPassed` 进行仲裁，不感知算法内部细节。

### 3) 静态手势数据（左右手 + 有效性检测 + PIE 录入）

#### 数据载体（推荐 DataAsset）

- `UUxtStaticPoseDefinition : UDataAsset`（推荐主模型：算法 + 数据同类）
  - 内含识别算法实现所需参数与参考数据（如左右手旋转参考、位置参考、距离阈值、PalmWidth 归一化参数）
  - 提供 `CreateEvaluator()`、`IsPoseDataValid()` 等接口
  - 可选：`Version`、`DisplayName/PoseId`
- `UUxtStaticHandPoseDataAsset : UDataAsset`（可选基础数据对象）
  - 若需要在多个定义资产之间复用原始采样数据，可作为被 `UUxtStaticPoseDefinition` 引用的数据源

#### 单手数据结构（建议）

- `FStaticPoseHandData`
  - **旋转参考**：关节到参考旋转（定长数组或 map）
  - **位置参考**：关节到参考位置（手部局部空间）
  - **参考基准**：Wrist/Palm 的参考变换（用于把位置归一化到局部空间）
  - **参考 PalmWidth**：录入时的手掌尺度基准值
  - **PalmWidth 骨骼记录**：用于计算“手掌最宽距离”的两个骨骼及其参考姿态
  - **有效性标记**：是否已完成该手所需全部关节数据录入

#### 有效性规则（运行时强约束）

- **任意一个参与当前手势判定的关节点无效，则整个手势直接无效**
- 不做“缺部分关键点仍继续评估”的容错
- 原因：部分手势只依赖 2~3 个关节的距离/位置关系，缺失任一点都会破坏姿态判定语义
- 评估器应在进入算法前先做关节完整性检查，失败则返回 `bPassed=false` 且 `FailureReason=MissingJoint`

#### PIE 录入

录入建议作为 Editor-only 功能：

- 通过运行时子系统（如 `UUxtStaticHandPoseSubsystem`）统一提供“采样当前手势数据”的接口
- 编辑器模块仅提供一套录入 UI（面板/工具按钮/详情面板扩展）来调用子系统采样接口
- UI 侧负责将采样结果写回目标 `UUxtStaticPoseDefinition`（或其引用的数据对象）并更新有效性标记
- 触发 asset 保存（Editor-only API）

> PIE 录入不新增运行时组件，避免开发与维护冗余；识别与输入输出由运行时子系统负责，编辑器仅做录入交互与资产落盘。

#### PalmWidth 归一化规则（必须统一）

- `PalmWidth` 取**预先固定指定的一对手部骨骼**之间的距离
- 当前建议固定骨骼对：`IndexMetacarpal` 与 `LittleMetacarpal`
- 由于手掌横向也可能弯曲，**录入资产时需要完整记录这两个骨骼的参考姿态**，不能只记录一个标量距离
- 运行时按当前帧这两个固定骨骼的实际距离计算 `PalmWidth`
- 若这两个骨骼任一无效，则当前帧该手势直接无效，不做次级兜底标尺
- 归一化使用无量纲比值，避免不同手掌大小带来的阈值漂移

### 4) 只有单手数据时的镜像补全（初始化自动生成另一只手）

需求：配置允许“另一只手也可用该手势”，但只录入了一只手数据时，初始化静态手势子系统时自动镜像补全。

建议行为：

- 初始化加载手势库时：
  - `LeftValid && !RightValid`：`Right = Mirror(Left)` 并置 `RightValid=true`
  - `RightValid && !LeftValid`：`Left = Mirror(Right)` 并置 `LeftValid=true`

镜像要同时处理：

- **位置镜像**：在手部局部空间沿指定轴取反（轴选择取决于手部局部坐标系定义）。
- **旋转镜像**：建议走矩阵形式 `R_mirror = M * R * M^{-1}` 再转回 `Quat`，避免直接改四元数分量导致错误。
- **关节语义映射**：同名关节在左右手通常枚举不同（或同枚举但 hand 区分），需要确保“左拇指根”映射到“右拇指根”对应项。

建议补充验收样例：

- `ThumbUp` 从左手镜像到右手后，拇指仍应指向外侧，不应转为掌心内扣
- `Pinch` 从左手镜像到右手后，`ThumbTip-IndexTip` 的归一化距离应近似保持一致
- `OpenPalm` 镜像后掌心法向量相对相机的离散方向应符合预期翻转规则

手势条目额外配置：

- `bAllowEitherHand`：是否允许左/右任一手触发（运行时对两只手分别评估，选最满足者）

### 5) 绑定 UE 按键（10 个自定义 FKey）

目标：提供 10 个固定命名的按键，供 Input Mapping / Enhanced Input / UI 绑定。

#### Key 注册

- 在插件输入模块启动时注册：
  - `Custom_HandPoseKey_1` ~ `Custom_HandPoseKey_10`
  - 使用 `EKeys::AddKey(FKeyDetails(...))`（具体类型选择 Button/GamepadKey 语义）

#### 手势到 Key 的绑定表

建议提供可配置资产或项目设置：

- `UUxtStaticHandPoseBindingsAsset`（DataAsset）或 `UUxtStaticHandPoseSettings`（DeveloperSettings）
  - 建议结构：`TMap<EUxtHandPoseKeySlot, FUxtStaticHandPoseBinding> Bindings;`
  - `EUxtHandPoseKeySlot : uint8`
    - `None = 0`
    - `Slot1` ~ `Slot10`
  - Key 语义：`EUxtHandPoseKeySlot` 只表示“插件内部第几个手势按键槽位”，**不直接暴露 `FKey` 给配置资产**
  - 运行时再通过槽位转换函数，把 `EUxtHandPoseKeySlot` 映射到插件注册的 `Custom_HandPoseKey_1` ~ `Custom_HandPoseKey_10`
  - `FUxtStaticHandPoseBinding` 建议字段：
    - `PoseDefinitionAsset`（`UUxtStaticPoseDefinition`，包含算法与该手势所需数据）
    - `AllowedPalmDirs`（第 1 点）
    - `bAllowEitherHand`（第 4 点）
    - `Priority`（可选，默认 0；仅在分数完全相同时作为稳定 tie-breaker）
    - 可选稳定性参数（建议保留，默认关闭即可）：
      - `DebounceDownTime` / `DebounceUpTime`：按下/松开去抖时间（或等价的“连续 N 帧”）
      - `MinHoldTime`：一旦按下至少保持该时长，避免短暂丢帧导致抖动松开
  - 运行时状态：识别到则槽位对应的 `FKey` 置为按下，未识别到则该 `FKey` 置为松开；按下中的 Key 存入运行时 `Set<FKey>`

#### Binding 查询与 Key 转换

建议提供统一查询和转换接口，而不是在外部直接操作 `TMap` 或裸 `FKey`：

- `const FUxtStaticHandPoseBinding* GetBinding(EUxtHandPoseKeySlot Slot) const;`
  - 语义：通过槽位查询绑定配置
  - 行为：内部使用 `Bindings.Find(Slot)` 返回指针；未配置则返回 `nullptr`
- `FKey GetFKeyForSlot(EUxtHandPoseKeySlot Slot);`
  - 语义：把槽位转换为插件内部注册的自定义 `FKey`
  - 行为：`Slot1 -> Custom_HandPoseKey_1`，...，`Slot10 -> Custom_HandPoseKey_10`
- `bool IsValidSlot(EUxtHandPoseKeySlot Slot);`
  - 语义：统一校验槽位是否合法，避免 `None` 或越界值进入运行时状态机

这样设计的原因：

- 配置资产层只关心“第几个手势槽位”，不直接持有 UE 输入系统层的 `FKey`
- 可避免用户在 Details 面板中误选无关键盘/手柄键
- 运行时仍可无损映射到插件内部注册的固定 `FKey`

### 5.1) 多手势仲裁规则（必须固定）

- **同一只手同一帧命中多个手势**：只保留 `Score` 最高的那个 Binding
- **两只手同时命中同一个 Binding**：只保留分数更高的那只手作为该 Binding 的触发来源
- **两只手命中不同 Binding**：允许同时触发不同 Binding
- **分数相同**：按 `Priority` 高者优先；若仍相同，则保持上一帧已稳定按下者优先，避免来回抖动
- 仲裁顺序建议：先做单手内部筛选，再做跨手去重，最后进入去抖/状态机

### 5.2) 时序状态机（建议固定实现）

建议每个 Binding 使用统一状态机，而不是仅靠布尔值切换：

- `Idle`：当前未满足按下条件
- `CandidateDown`：识别通过但尚未满足 `DebounceDownTime`
- `Pressed`：已经按下并输出 Key Down / Hold
- `CandidateUp`：识别丢失但尚未满足 `DebounceUpTime` 或 `MinHoldTime`

状态切换建议：

- `Idle -> CandidateDown`：当前帧通过仲裁且 `bPassed=true`
- `CandidateDown -> Pressed`：连续满足按下条件达到 `DebounceDownTime`
- `Pressed -> CandidateUp`：当前帧未通过或触发源被其他更高分手势抢占
- `CandidateUp -> Idle`：连续不满足达到 `DebounceUpTime`，且已满足 `MinHoldTime`
- 若在 `CandidateUp` 期间重新通过识别，则直接回到 `Pressed`

该状态机同时适用于：

- 边界抖动
- 短时 Tracking 波动
- 左右手在同一 Binding 上切换触发源

---

## 推荐模块/职责划分（高层架构）

### 数据层

- `UUxtStaticPoseDefinition`（DataAsset）：识别配置入口（参数、阈值、关节选择、归一化策略等）
  - 通过基类初始化函数创建纯 C++ 算法代理实例（Evaluator）
  - 负责向算法代理注入外部数据容器引用（或指针/弱引用）
- `BindingsAsset/Settings`：以 `TMap<EUxtHandPoseKeySlot, FUxtStaticHandPoseBinding>` 存储 10 个手势槽位的绑定与参数

### 算法层（可单独文件夹）

- `PalmDirectionUtility`：计算 Palm 相对 Camera 的 6 向离散结果（输出 bit）
- `MirrorUtility`：单手数据镜像生成另一手（位置+旋转+关节映射）
- `Evaluators`（纯 C++ 执行层，和 DataAsset/场景逻辑分离）：
  - 基类示意：`FUxtStaticPoseEvaluator`
  - 工厂入口：由 `UUxtStaticPoseDefinition::CreateEvaluator(...)` 创建具体评估器
  - 数据约束：评估器只保存“外部数据容器指针/引用”，不重复持久化资产数据
  - Rotation Similarity（旋转相似度）
  - Joint Position Bounds（位置范围，可选按 PalmWidth 归一化）
  - Joint Distance / Pinch Distance（距离/捏合距离，建议按 PalmWidth 归一化）
  -（可扩展更多）

### 运行时层（子系统）

建议实现一个运行时子系统（命名示意）：

- `UUxtStaticHandPoseSubsystem`
  - 继承：`UEngineSubsystem`
  - 初始化：加载绑定表、补全镜像数据、准备运行时状态机
  - Tick：建议参考 `UXTools` 现有输入子系统，通过 `FWorldDelegates::OnWorldPreActorTick` 获取当前左右手关节数据 -> 评估每个 binding -> 做仲裁 -> 驱动 Key 状态机
  - 输出：把识别结果注入 UE 输入体系

> 识别、去抖（Debounce/MinHoldTime）、状态机、按键输出集中在 Subsystem，避免散落在组件中导致维护困难。

### 调试层（Runtime，建议新增）

- `UUxtStaticHandPoseDebugFunctionLibrary`（蓝图函数库）
  - 暴露运行时调试入口：查询某只手当前关节数据、单次调用指定算法评估、返回最近一帧 Binding 调试信息、开启/关闭可视化绘制等
  - 所有调试函数以 `BlueprintCallable` / `BlueprintPure` 暴露，便于 PIE 调参与自动化验证
- 可视化建议：
  - 绘制 UE 原生手部追踪返回的关节位置、骨骼连线、Palm normal、Wrist 坐标系、PalmWidth 两关键骨骼连线
  - 显示每个 Binding 的 `Score`、失败原因、当前状态机状态、最终触发的 `FKey`
- 编译控制：
  - 调试绘制和编辑辅助函数统一放在 Runtime 模块中，但受 `WITH_EDITOR` 或显式调试开关控制
  - Shipping 打包时默认关闭调试绘制与高频调试文本，避免性能和安全噪音
- 现有 `UXTools` 可参考能力：
  - 仓内存在 `IUxtHandTracker` 等手部追踪抽象接口，可作为“关节组织方式”的参考
  - **当前未发现可直接复用的现成手部骨骼调试可视化功能**，因此本模块应自行实现运行时可视化，不依赖现有 `UXTools` 模块

### 编辑器录入层（Editor-only）

- `Editor UI/Utility`：通过子系统接口触发 PIE 采样，并写入/保存 DataAsset

---

## 模块拆分建议（运行时识别 vs PIE/编辑器录入）

考虑到你更偏向“配置资产与算法执行分离”，同时编辑器侧需要更强的 UI/录入能力，建议把“静态手势识别”做成**独立模块**，并把 PIE 录入做成**另一个独立（Editor-only）模块**：

- **运行时模块（例如 `UXToolsHandPose` / 或归入 `UXToolsInput` 但逻辑隔离）**
  - `UUxtStaticHandPoseSubsystem`：运行时识别与 Key 状态输出（建议 `UEngineSubsystem`）
  - `UDataAsset`：识别配置与工厂入口（创建纯 C++ Evaluator）
  - 纯 C++ Evaluator：仅执行识别算法，持有外部数据容器指针，不承担资产管理
  - Key 注册与输入注入实现
  - Runtime Debug Function Library 与可视化调试能力

- **编辑器模块（例如 `UXToolsHandPoseEditor`）**
  - PIE 采样录入 UI（面板/工具栏命令/详情面板扩展），通过子系统接口采样
  - `UDataAsset` 资产创建、可视化编辑、验证（数据有效性、镜像补全预览、PalmWidth 归一化调参）
  - 仅在 Editor 构建中编译，打包后自动屏蔽，避免运行时代码冗余与 Editor API 依赖

---

## 关键实现决策（编码前必须统一）

### A) “掌心方向”计算基准

必须明确 Palm normal 的定义与坐标空间（组件/世界/相机空间）。当前固定为：以手掌根部到小拇指根部、手掌根部到食指根部两向量叉乘得到掌心法向量；推荐在世界空间取 Palm normal，再与 Camera Up/Right/Forward 做 dot。

### B) “位置范围”使用的参考空间

强烈建议统一为“手部局部空间”（基于 `Wrist`），避免世界移动与玩家走动影响阈值。

### C) “镜像算法”的坐标系与轴

镜像轴与手部局部坐标系必须固定且一致，否则补全出的另一手会严重偏差；录入与运行时都以 `Wrist` 为局部空间原点。

### D) 输入注入路径

最终方案：**按 UE 传统 VR 控制器按键的实现路径来做自定义手势键**，而不是把结果直接写死到某个 `PlayerController::InputKey` 调用。

- 在模块启动时注册 `FKey`
- 通过独立输入设备或等价消息分发层，把 `Pressed/Released` 事件按 UE 控制器按键上报路径送入输入系统
- 让这些 Key 在 Input Settings / Enhanced Input 中表现得和普通 VR 手柄按钮一致
- 上层项目仍可自由把这些 `FKey` 映射到 `Input Action`

> 结论：底层应尽量贴近 UE 原生控制器按键注入路径，让静态手势键在输入系统中的语义与传统 VR 手柄键保持一致。

### E) 原始数据有效性判定

必须先做关节有效性检查，再进入任意算法评估；**任意一个参与当前手势的关节无效，则整个手势无效**。

### F) 评分与仲裁顺序

必须先统一输出 `Score` / `bPassed`，再做“单手最高分 -> 跨手同 Binding 去重 -> 状态机”的三段式处理，避免不同算法直接耦合到输入状态。

---

## 需要速查的 UE 接口与类型

本节只保留**必须依赖 UE 安装目录源码/头文件才能准确确认**的信息，方便后续按图索骥查阅。

### 1) 手部原始数据获取

已确认使用 UE 的 `GetMotionControllerData` 路径取数，后续需要到 UE 源码中核对以下接口和类型：

- `UHeadMountedDisplayFunctionLibrary::GetMotionControllerData`
- `IXRTrackingSystem::GetMotionControllerData`
- `FXRMotionControllerData`
- `EControllerHand`
- `EHandKeypoint`
- `ETrackingStatus`

当前已确认的关键点：

- 手部原始数据入口走 `GetMotionControllerData`
- 手部关键点索引使用 `EHandKeypoint`
- `FXRMotionControllerData` 中包含 `HandKeyPositions`、`HandKeyRotations`、`HandKeyRadii`
- `PalmWidth` 固定骨骼对建议使用 `IndexMetacarpal` 与 `LittleMetacarpal`

后续需要结合 UE 源码继续核对：

- `FXRMotionControllerData` 在 UE 5.3 下的完整字段和可用性
- `GetMotionControllerData` 的调用时机、`WorldContext` 要求、XR 系统有效性判断
- 不同 XR 后端下手部关键点是否稳定填充

### 2) 子系统与 Tick/生命周期

后续需要结合 UE 与当前插件实现一起核对：

- `UEngineSubsystem` 的生命周期与 Tick 承载方式
- 是否需要额外实现 `FTickableGameObject`
- `UXTools` 中 `UxtDefaultHandTrackerSubsystem` 的 Tick 组织方式可作为参考

当前倾向：

- 继续以 `UUxtStaticHandPoseSubsystem : UEngineSubsystem` 为运行时识别主入口
- 优先参考 `UXTools` 现有做法，通过 `FWorldDelegates::OnWorldPreActorTick` 驱动每帧识别
- 如委托方式不足以承载需求，再补充 Tickable 接口

### 3) 调试绘制与蓝图调试入口

后续需要结合 UE 源码确认最小实现代价和可维护性：

- `UBlueprintFunctionLibrary`
- `DrawDebugLine`
- `DrawDebugPoint`
- `DrawDebugCoordinateSystem`
- `DrawDebugSphere`
- `ULineBatchComponent`
- `WITH_EDITOR`
- `UE_BUILD_SHIPPING`

当前约束：

- 运行时调试绘制在非 Editor Development 包内可保留
- 蓝图函数库只负责查询与一次性评估，不长期持有运行时状态
- 可优先评估通过 `ULineBatchComponent` 以最小工作量渲染 `HandKeyPositions` / `HandKeyRotations`
- 关节点使用点或小球渲染，骨骼使用线条渲染

### 4) UE 安装路径

- `F:\UnrealEngine\UE_5.3`

### 5) 接口速查（待补充原始声明）

本小节用于粘贴**后续实现会频繁对照的 UE 原生声明**，避免每次重新翻 UE 源码。建议保留“函数签名/结构体声明/关键枚举定义”三级信息，而不是只写名称。

建议补回以下内容：

#### A. 手部数据获取函数

需要的符号：

- `IXRTrackingSystem::GetMotionControllerData`


需要明确的使用方式：

- 函数完整签名
- `WorldContext` 的要求
- `EControllerHand` 传 `Left` / `Right` 的调用方式
- 输出参数 `FXRMotionControllerData&` 的填充行为
- 调用前后是否需要先判断 `GEngine->XRSystem` 有效

- XRSystem 转发调用的关键实现
```c++
void UHeadMountedDisplayFunctionLibrary::GetMotionControllerData(UObject* WorldContext, const EControllerHand Hand, FXRMotionControllerData& MotionControllerData)
{
	MotionControllerData.bValid = false;
	
	IXRTrackingSystem* TrackingSys = GEngine->XRSystem.Get();
	if (TrackingSys)
	{
		TrackingSys->GetMotionControllerData(WorldContext, Hand, MotionControllerData);
	}
}
```
#### B. 手部数据结构

需要的符号：

- `FXRMotionControllerData`

需要重点关注的字段：

- `bValid`
- `HandIndex`
- `TrackingStatus`
- `PalmPosition`
- `PalmRotation`
- `HandKeyPositions`
- `HandKeyRotations`
- `HandKeyRadii`
- 其他能够帮助判断手部追踪状态的字段

需要明确的使用方式：

- 哪些字段可直接作为当前帧手部快照来源
- 哪些字段在手部未追踪时仍会保留旧值
- `HandKeyPositions` / `HandKeyRotations` 的索引是否严格对应 `EHandKeypoint`

- `FXRMotionControllerData` 的完整声明
```c++
USTRUCT(BlueprintType)
struct FXRMotionControllerData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadOnly, Category = "XR")
	bool bValid = false;
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	FName DeviceName;
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	FGuid ApplicationInstanceID;
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	EXRVisualType DeviceVisualType = EXRVisualType::Controller;

	UPROPERTY(BlueprintReadOnly, Category = "XR")
	EControllerHand HandIndex = EControllerHand::Left;

	UPROPERTY(BlueprintReadOnly, Category = "XR")
	ETrackingStatus TrackingStatus = ETrackingStatus::NotTracked;

	// Vector representing an object being held in the player's hand
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	FVector GripPosition = FVector(0.0f);
	// Quaternion representing an object being held in the player's hand
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	FQuat GripRotation = FQuat(EForceInit::ForceInitToZero);

	// For handheld controllers, gives a vector for pointing at objects
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	FVector AimPosition = FVector(0.0f);
	// For handheld controllers, gives a quaternion for pointing at objects
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	FQuat AimRotation = FQuat(EForceInit::ForceInitToZero);

	// For handheld controllers, gives a vector for representing the hand
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	FVector PalmPosition = FVector(0.0f);
	// For handheld controllers, gives a quaternion for representing the hand
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	FQuat PalmRotation = FQuat(EForceInit::ForceInitToZero);

	// The indices of this array are the values of EHandKeypoint (Palm, Wrist, ThumbMetacarpal, etc).
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	TArray<FVector> HandKeyPositions;
	// The indices of this array are the values of EHandKeypoint (Palm, Wrist, ThumbMetacarpal, etc).
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	TArray<FQuat> HandKeyRotations;
	// The indices of this array are the values of EHandKeypoint (Palm, Wrist, ThumbMetacarpal, etc).
	UPROPERTY(BlueprintReadOnly, Category = "XR")
	TArray<float> HandKeyRadii;

	UPROPERTY(BlueprintReadOnly, Category = "XR")
	bool bIsGrasped = false;
};

```
#### C. 关键枚举定义

需要的符号：

- `EControllerHand`

- `EHandKeypoint`
- `ETrackingStatus`

需要明确的使用方式：

- 左右手对应的枚举值
- `Wrist`、`Palm`、`IndexMetacarpal`、`LittleMetacarpal` 等关键骨骼项的精确枚举名
- `TrackingStatus` 哪些值应视为“可参与识别”

建议在这里贴回：
- `EControllerHand` 的完整定义
```c++
/** Defines the controller hands for tracking.  Could be expanded, as needed, to facilitate non-handheld controllers */
UENUM(BlueprintType)
enum class EControllerHand : uint8
{
	Left,
	Right,
	AnyHand,
	Pad,
	ExternalCamera,
	Gun,
	HMD,
	Chest,
	LeftShoulder,
	RightShoulder,
	LeftElbow,
	RightElbow,
	Waist,
	LeftKnee,
	RightKnee,
	LeftFoot,
	RightFoot,
	Special,

	ControllerHand_Count UMETA(Hidden, DisplayName = "<INVALID>"),
};

```
- `EHandKeypoint` 的完整定义
```c++
/**
 * Transforms that are tracked on the hand.
 * Matches the enums from WMR to make it a direct mapping
 */
UENUM(BlueprintType)
enum class EHandKeypoint : uint8
{
	Palm,
	Wrist,
	ThumbMetacarpal,
	ThumbProximal,
	ThumbDistal,
	ThumbTip,
	IndexMetacarpal,
	IndexProximal,
	IndexIntermediate,
	IndexDistal,
	IndexTip,
	MiddleMetacarpal,
	MiddleProximal,
	MiddleIntermediate,
	MiddleDistal,
	MiddleTip,
	RingMetacarpal,
	RingProximal,
	RingIntermediate,
	RingDistal,
	RingTip,
	LittleMetacarpal,
	LittleProximal,
	LittleIntermediate,
	LittleDistal,
	LittleTip
};

```
- `ETrackingStatus` 的完整定义
```c++

UENUM(BlueprintType)
enum class ETrackingStatus : uint8
{
	NotTracked,
	InertialOnly,
	Tracked,
};
```
## 下一步（你确认框架后再做）

- 以“尽量与 `UXTools` 现有模块解耦”为硬约束，给出独立模块化实现方案：
  - 新增独立 Runtime 模块与 Editor 模块（算法与类型自包含）
  - 新增/修改文件清单与放置路径（优先新模块内闭环）
  - 具体类名/接口名/UPROPERTY 暴露策略（避免引用 `UXTools` 现有内部类型）
  - Key 注册位置与模块启动时机（基于独立模块）
  - 输入注入所需的 `IInputDevice` / 消息分发实现方式（按 UE 控制器按键路径）
  - PIE 录入的 Editor-only 实现方式（仅依赖新 Editor 模块）
  - Runtime 调试函数库与手部骨骼可视化方案

---

## 当前已声明的空类与文件路径（便于后续实现）

运行时模块 `UXToolsHandPose` 下：

- `Public/UxtStaticPoseDefinition.h`
  - `EUxtPalmCameraDirection`（掌心相机方向 Bitmask 枚举）
  - `UUxtStaticPoseDefinition : UDataAsset`（识别配置基类，空实现）
  - `UUxtStaticPoseRotationDefinition : UUxtStaticPoseDefinition`
  - `UUxtStaticPoseBoundsDefinition : UUxtStaticPoseDefinition`
  - `UUxtStaticPoseDistanceDefinition : UUxtStaticPoseDefinition`

- `Public/UxtStaticPoseEvaluator.h`
  - `FUxtStaticPoseEvaluator`（纯 C++ Evaluator 基类，空实现）
  - `FUxtStaticPoseRotationEvaluator : FUxtStaticPoseEvaluator`
  - `FUxtStaticPoseBoundsEvaluator : FUxtStaticPoseEvaluator`
  - `FUxtStaticPoseDistanceEvaluator : FUxtStaticPoseEvaluator`

- `Public/UxtStaticHandPoseSubsystem.h` + `Private/UxtStaticHandPoseSubsystem.cpp`
  - `UUxtStaticHandPoseSubsystem : UEngineSubsystem`（Initialize/Deinitialize 空实现）

- `Public/UxtStaticHandPoseBindings.h`
  - `FUxtStaticHandPoseBinding`（占位 USTRUCT）
  - `UUxtStaticHandPoseBindingsAsset : UDataAsset`（占位资产）

- `Public/UxtStaticHandPoseDebugFunctionLibrary.h`
  - `UUxtStaticHandPoseDebugFunctionLibrary : UBlueprintFunctionLibrary`（调试入口，待创建）

- `Public/UxtStaticHandPoseUtilities.h`
  - `FUxtPalmDirectionUtility`（占位工具结构）
  - `FUxtHandPoseMirrorUtility`（占位工具结构）
