# 波形可视化系统 Spec（QQuickItem + QSGGeometry 教学式）

## Why
项目需要实时显示 100kHz 采样率、16 通道、每点 4 字节的 TCP 数据波形，以及对应的 FFT 频域波形。Qt Charts / Canvas 等方案无法承受 6.4 MB/s 的数据吞吐量，必须使用 GPU 加速渲染（QSGGeometry）。同时需要完整的坐标轴系统（X/Y 轴、刻度、网格线）和窗口缩放自适应能力。

## What Changes
- 新增 C++ 类 `WaveformItem`（继承 QQuickItem），使用 `QML_ELEMENT` 宏声明式注册，用 QSGGeometry 在 GPU 上绘制折线图 + 坐标轴
- 新增 C++ 类 `WaveformDataSource`，使用 `QML_ELEMENT` + `QML_SINGLETON` 注册，负责接收 TCP 数据、环形缓冲、降采样
- 使用 `qt_add_qml_module()` 替代手动 `qmlRegisterType()`，自动完成 QML 模块注册
- `WaveformItem` 支持坐标轴属性：`xMin/xMax/yMin/yMax`（值范围）、`xTickCount/yTickCount`（等分数量）、`axisLeftMargin/axisBottomMargin`（边距）
- `WaveformItem` 在 `updatePaintNode()` 中绘制坐标轴边框、刻度线、网格线
- `WaveformItem` 重写 `geometryChange()` 实现窗口缩放时自动重绘
- QML 层使用 `Repeater` + `Text` 绘制坐标轴刻度标签
- 修改 `ChartPannel.qml`，上下堆叠两个带坐标轴的 `WaveformItem`（时域 + 频域）
- 修改 `TcpClient`，在 `onReadyRead` 中将原始二进制数据传递给 `WaveformDataSource`
- 修改 `CMakeLists.txt`，使用 `qt_add_qml_module()` 注册 QML 模块

## Impact
- Affected code: `App/tcpclient.h/cpp`, `MotolControlQmlContent/ChartPannel.qml`, `App/CMakeLists.txt`, 根 `CMakeLists.txt`
- 新增文件: `App/waveformitem.h`, `App/waveformitem.cpp`, `App/waveformdatasource.h`, `App/waveformdatasource.cpp`

---

## ADDED Requirements

### Requirement: QML 模块声明式注册

系统 SHALL 使用 `QML_ELEMENT` 宏和 `qt_add_qml_module()` 完成 QML 类型注册，不再使用手动 `qmlRegisterType()`。

#### Scenario: WaveformItem 自动注册到 QML
- **WHEN** `WaveformItem` 类声明中包含 `QML_ELEMENT` 宏，且 `qt_add_qml_module()` 的 SOURCES 包含该文件
- **THEN** QML 中 `import WaveformItem` 后可直接使用 `WaveformItem { }` 组件
- **AND** `main.cpp` 中无需手动调用 `qmlRegisterType<WaveformItem>()`

#### Scenario: WaveformDataSource 单例注册
- **WHEN** `WaveformDataSource` 类声明中包含 `QML_ELEMENT` + `QML_SINGLETON` 宏
- **THEN** QML 中可直接访问 `WaveformDataSource` 单例
- **AND** C++ 端可通过 `qmlRegisterSingletonInstance()` 注入预创建实例

### Requirement: WaveformItem — GPU 加速折线图组件

系统 SHALL 提供一个名为 `WaveformItem` 的 QQuickItem 子类，使用 QSGGeometry 在 Qt Scene Graph 中渲染折线图。

#### Scenario: 显示静态测试数据
- **WHEN** QML 中创建 `WaveformItem { }` 并设置 `samples` 属性
- **THEN** 组件区域显示对应折线

#### Scenario: 显示实时数据
- **WHEN** `WaveformDataSource` 收到新数据并通知更新
- **THEN** `WaveformItem` 在下一帧重绘折线，帧率不低于 30 FPS

### Requirement: WaveformItem — 坐标轴系统

系统 SHALL 在 `WaveformItem` 中提供完整的坐标轴系统，包括：

1. **坐标轴属性**：
   - `xMin`/`xMax`：X 轴数值范围（默认 0.0 / 1.0）
   - `yMin`/`yMax`：Y 轴数值范围（默认 0.0 / 1.0）
   - `xTickCount`：X 轴等分数量（默认 5，表示 6 个刻度位置）
   - `yTickCount`：Y 轴等分数量（默认 5）
   - `axisLeftMargin`/`axisBottomMargin`/`axisTopMargin`/`axisRightMargin`：坐标轴边距（为标签留空间）

2. **坐标轴绘制**（QSGGeometry）：
   - 绘图区域边框线（矩形）
   - X/Y 轴刻度线（短横线/竖线）
   - 网格线（水平/垂直线）

3. **坐标轴标签**（QML Text）：
   - Y 轴左侧显示刻度数值
   - X 轴底部显示刻度数值

#### Scenario: 配置坐标轴范围
- **WHEN** QML 中设置 `WaveformItem { yMin: -100; yMax: 100; yTickCount: 5 }`
- **THEN** Y 轴显示 6 个刻度值：-100.0, -60.0, -20.0, 20.0, 60.0, 100.0
- **AND** 波形数据映射到 [-100, 100] 范围内显示

#### Scenario: 配置坐标轴等分数量
- **WHEN** QML 中设置 `xTickCount: 10`
- **THEN** X 轴显示 11 个刻度位置，将 X 轴分为 10 等分

#### Scenario: 坐标轴边距
- **WHEN** QML 中设置 `axisLeftMargin: 50; axisBottomMargin: 30`
- **THEN** 绘图区域左侧留出 50px（给 Y 轴标签），底部留出 30px（给 X 轴标签）
- **AND** 波形和网格线仅在绘图区域内绘制

### Requirement: WaveformItem — 响应式缩放

系统 SHALL 在窗口缩放时自动重绘波形和坐标轴，使其始终填满可用区域。

#### Scenario: 窗口放大
- **WHEN** 用户拖拽窗口边缘放大
- **THEN** 波形和坐标轴自动扩展到新的区域，绘图区域按比例增大
- **AND** 刻度标签位置自动调整

#### Scenario: 窗口缩小
- **WHEN** 用户拖拽窗口边缘缩小
- **THEN** 波形和坐标轴自动收缩，不超出可视区域

#### Scenario: 最大化/恢复
- **WHEN** 用户最大化窗口后再恢复
- **THEN** 波形和坐标轴先扩展到全屏，再恢复到原始大小

### Requirement: WaveformDataSource — 数据源管理

系统 SHALL 提供一个名为 `WaveformDataSource` 的 QObject 子类，负责：
- 接收 TCP 原始二进制数据
- 解析为 16 通道 float 数组
- 环形缓冲存储最近 N 个采样点
- Min-Max 降采样到屏幕可显示的点数
- 通知 UI 更新

#### Scenario: 接收并解析 TCP 数据
- **WHEN** TCP 收到一帧数据（16 通道 × M 个采样点 × 4 字节）
- **THEN** 数据被解析为 16 个 float 数组，存入环形缓冲区

#### Scenario: 降采样
- **WHEN** UI 请求显示数据，屏幕宽度为 W 像素
- **THEN** 对每个通道执行 Min-Max 降采样，将数据压缩到约 2W 个点（保留极值）

### Requirement: ChartPannel UI 布局

系统 SHALL 在 `ChartPannel.qml` 中提供上下堆叠的两个图表区域：
- 上方：时域波形图（WaveformItem + 坐标轴 + 标签）
- 下方：频域波形图（WaveformItem + 坐标轴 + 标签）

#### Scenario: 界面显示
- **WHEN** 应用启动
- **THEN** ChartPannel 显示两个等高的图表区域，各有标题标签、坐标轴、刻度标签

### Requirement: 教学式分步实现

文档 SHALL 按以下顺序逐步引导开发者：
1. Step 1: 搭建 ChartPannel QML 骨架（两个空白图表区域）
2. Step 2: 创建 WaveformItem 骨架（QML_ELEMENT + QSGGeometry 绘制静态线）
3. Step 3: 使用 qt_add_qml_module 注册 QML 模块
4. Step 4: 在 ChartPannel 中使用 WaveformItem
5. Step 5: 为 WaveformItem 添加坐标轴属性
6. Step 6: 绘制坐标轴线和刻度线
7. Step 7: 处理窗口缩放 — 响应式重绘
8. Step 8: 在 QML 中添加坐标轴标签
9. Step 9: 为 WaveformItem 添加样式属性（lineColor, lineWidth）
10. Step 10: 创建 WaveformDataSource（环形缓冲 + 降采样）
11. Step 11: 连接数据源到 WaveformItem
12. Step 12: 修改 TcpClient 将数据传递给 WaveformDataSource
13. Step 13: FFT 频域图（后续扩展）

每一步 SHALL 包含：
- 要创建/修改的文件列表
- 完整代码（可直接复制使用）
- 代码含义解释（为什么这么写）

## MODIFIED Requirements

### Requirement: QML 类型注册方式
**变更**：从手动 `qmlRegisterType()` 改为 `QML_ELEMENT` + `qt_add_qml_module()` 声明式注册。
**原因**：Qt 6 推荐方式，支持 qmllint 类型检查，减少 main.cpp 样板代码。

### Requirement: WaveformItem 数据映射
**变更**：波形数据映射从自动缩放（基于数据 min/max）改为基于 `yMin/yMax` 属性的固定范围映射。
**原因**：配合坐标轴系统，数据需要映射到用户指定的范围内显示，超出范围的数据裁剪到边界。

## REMOVED Requirements

### Requirement: 手动 qmlRegisterType 注册
**Reason**: 已被 `QML_ELEMENT` + `qt_add_qml_module()` 声明式注册替代。
**Migration**: 移除 main.cpp 中的 `qmlRegisterType<WaveformItem>(...)` 调用，改用 `QML_ELEMENT` 宏。
