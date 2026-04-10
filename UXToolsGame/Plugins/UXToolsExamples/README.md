# UxtHandPoseAxisActor

这个 Actor 用来监听 `UxtStaticHandPoseSubsystem` 的按键状态变化。

当指定 `Slot` 被按下时：

- Actor 显示出来
- Actor 开启 Tick
- 记录当前手掌在追踪空间下的位置
- 根据后续位移点亮 6 个方向箭头中的一个

当指定 `Slot` 被松开时：

- Actor 隐藏
- Actor 关闭 Tick
- 所有方向取消激活

## 1. 如何创建

你可以用两种方式创建：

### 方式一：直接拖到场景里

1. 在内容浏览器中创建一个继承自 `UxtHandPoseAxisActor` 的蓝图。
2. 把这个蓝图拖到关卡里。
3. 在 Details 面板里设置：
   - `Slot`
   - `MovementThresholdCm`
   - `ArrowLength`

### 方式二：蓝图里动态创建

1. 在蓝图中使用 `Spawn Actor from Class`。
2. Class 选择你基于 `UxtHandPoseAxisActor` 创建的蓝图类。
3. 因为这些变量支持创建时设置，所以可以在 `Spawn Actor from Class` 节点上直接填写：
   - `Slot`
   - `MovementThresholdCm`
   - `ArrowLength`

## 2. 如何绑定事件

这个 Actor 有两个事件：

- `OnAxisActivated`
- `OnAxisDeactivated`

这两个事件都会带一个参数：`Direction`

`Direction` 可能的值有：

- `Up`
- `Down`
- `Left`
- `Right`
- `Forward`
- `Backward`

### 蓝图绑定方法

如果这个 Actor 已经在场景里：

1. 拿到这个 Actor 的引用。
2. 从引用拖一根线。
3. 搜索并选择：
   - `Bind Event to OnAxisActivated`
   - `Bind Event to OnAxisDeactivated`
4. 创建自定义事件。
5. 在自定义事件里读取 `Direction` 参数。

如果这个 Actor 是运行时 `Spawn` 出来的：

1. 用 `Spawn Actor from Class` 创建它。
2. 从返回值拖线。
3. 绑定：
   - `OnAxisActivated`
   - `OnAxisDeactivated`

## 3. 最简单的用法

你可以这样理解：

1. 创建一个 `UxtHandPoseAxisActor` 蓝图。
2. 设置它的 `Slot`。
3. 绑定 `OnAxisActivated`。
4. 当事件触发时，根据 `Direction` 做你的蓝图逻辑。
5. 如果你还需要知道方向何时取消，再绑定 `OnAxisDeactivated`。

## 4. 说明

- 只有当收到匹配 `Slot` 的按下事件时，Actor 才会显示。
- 一次只会激活一个方向。
- 如果位移没有超过阈值，则不会激活任何方向。
