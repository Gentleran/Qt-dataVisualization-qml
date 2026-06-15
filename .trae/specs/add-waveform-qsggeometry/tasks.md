# Tasks

- [ ] Step 1: 搭建 ChartPannel QML 骨架 — 上下两个图表区域
  - [ ] 1.1 修改 `ChartPannel.qml`，添加 ColumnLayout + 两个 Rectangle 占位区域 + 标题标签
  - [ ] 1.2 验证：运行后看到上下两个带标题的空白区域

- [ ] Step 2: 创建 WaveformItem 骨架 — QML_ELEMENT + QSGGeometry 绘制静态线
  - [ ] 2.1 创建 `App/waveformitem.h`，声明 WaveformItem 类（继承 QQuickItem），添加 `QML_ELEMENT` 宏
  - [ ] 2.2 创建 `App/waveformitem.cpp`，实现 `updatePaintNode()` 绘制一条静态斜线
  - [ ] 2.3 验证：编译通过

- [ ] Step 3: 使用 qt_add_qml_module 注册 QML 模块
  - [ ] 3.1 修改根 `CMakeLists.txt`，配置 `qt_add_qml_module(URI WaveformItem VERSION 1.0 SOURCES ...)`
  - [ ] 3.2 确认 `main.cpp` 中无需手动 `qmlRegisterType<WaveformItem>()`
  - [ ] 3.3 验证：编译通过，QML 可 `import WaveformItem`

- [ ] Step 4: 在 ChartPannel 中使用 WaveformItem
  - [ ] 4.1 修改 `ChartPannel.qml`，添加 `import WaveformItem`，用 `WaveformItem` 替换占位区域
  - [ ] 4.2 验证：运行后上下区域各显示一条绿色测试波形

- [ ] Step 5: 为 WaveformItem 添加坐标轴属性
  - [ ] 5.1 添加 `Q_PROPERTY(float xMin/xMax/yMin/yMax ...)` 坐标轴范围属性
  - [ ] 5.2 添加 `Q_PROPERTY(int xTickCount/yTickCount ...)` 等分数量属性
  - [ ] 5.3 添加 `Q_PROPERTY(qreal axisLeftMargin/axisBottomMargin/axisTopMargin/axisRightMargin ...)` 边距属性
  - [ ] 5.4 实现所有 getter/setter，属性变化时设置 `m_axisChanged` 标记并调用 `update()`
  - [ ] 5.5 验证：编译通过，QML 中可设置坐标轴属性

- [ ] Step 6: 绘制坐标轴线和刻度线
  - [ ] 6.1 重写 `updatePaintNode()`，使用场景图节点树（root → axisNode + waveNode）
  - [ ] 6.2 在 axisNode 中绘制：边框线、Y轴刻度线、X轴刻度线、网格线
  - [ ] 6.3 修改波形数据映射：使用 `yMin/yMax` 范围替代自动缩放，映射到绘图区域（考虑 axisMargin）
  - [ ] 6.4 验证：运行后看到灰色边框、刻度线、网格线，波形映射到 yMin/yMax 范围

- [ ] Step 7: 处理窗口缩放 — 响应式重绘
  - [ ] 7.1 在 `waveformitem.h` 中声明 `geometryChange()` 重写和 `m_geometryChanged` 标记
  - [ ] 7.2 实现 `geometryChange()`：尺寸变化时设置 `m_geometryChanged = true` + `m_axisChanged = true` + `update()`
  - [ ] 7.3 修改 `updatePaintNode()` 波形几何更新条件：加入 `m_geometryChanged`
  - [ ] 7.4 验证：拖拽窗口放大/缩小，波形和坐标轴自动适应新区域

- [ ] Step 8: 在 QML 中添加坐标轴标签
  - [ ] 8.1 修改 `ChartPannel.qml`，在 WaveformItem 内添加 Y 轴 Repeater + Text 标签
  - [ ] 8.2 添加 X 轴 Repeater + Text 标签
  - [ ] 8.3 验证：Y 轴左侧显示刻度数值，X 轴底部显示刻度数值，与刻度线对齐

- [ ] Step 9: 为 WaveformItem 添加样式属性（lineColor, lineWidth）
  - [ ] 9.1 添加 `Q_PROPERTY(QColor lineColor ...)` 和 `Q_PROPERTY(float lineWidth ...)`
  - [ ] 9.2 修改 `updatePaintNode()` 中波形材质和线宽设置
  - [ ] 9.3 验证：QML 中设置不同 lineColor，两个波形显示不同颜色

- [ ] Step 10: 创建 WaveformDataSource — 环形缓冲 + 降采样
  - [ ] 10.1 创建 `App/waveformdatasource.h`，声明类（QObject，`QML_ELEMENT` + `QML_SINGLETON`，16 通道环形缓冲区）
  - [ ] 10.2 创建 `App/waveformdatasource.cpp`，实现环形缓冲区写入和 Min-Max 降采样
  - [ ] 10.3 添加 `Q_PROPERTY` 暴露降采样后的数据给 QML
  - [ ] 10.4 修改 `qt_add_qml_module` 的 SOURCES 添加新文件
  - [ ] 10.5 验证：编译通过

- [ ] Step 11: 连接数据源到 WaveformItem
  - [ ] 11.1 修改 `ChartPannel.qml`，添加模拟数据 Timer，调用 `WaveformDataSource.appendData()`
  - [ ] 11.2 将 `WaveformDataSource.channel0` 绑定到 WaveformItem 的 `samples`
  - [ ] 11.3 验证：运行后时域波形图实时滚动显示绿色正弦波

- [ ] Step 12: 修改 TcpClient 将数据传递给 WaveformDataSource
  - [ ] 12.1 修改 `tcpclient.h/cpp`，添加 `setDataSource()` 方法和 `m_dataSource` 指针
  - [ ] 12.2 修改 `onReadyRead()`，将二进制数据传递给 `m_dataSource->appendData()`
  - [ ] 12.3 修改 `main.cpp`，创建 WaveformDataSource 实例并连接到 TcpClient
  - [ ] 12.4 验证：TCP 连接后，时域波形图实时显示接收到的数据

- [ ] Step 13: FFT 频域图（后续扩展，本次不实现）
  - [ ] 13.1 集成 FFT 库（FFTW 或 KissFFT）
  - [ ] 13.2 在 WaveformDataSource 中添加 FFT 计算方法
  - [ ] 13.3 将 FFT 结果绑定到下方 WaveformItem
  - [ ] 13.4 验证：下方图表显示频域波形

# Task Dependencies
- [Step 2] depends on [Step 1]（先有 UI 骨架再填入组件）
- [Step 3] depends on [Step 2]（先有 C++ 类再配置模块注册）
- [Step 4] depends on [Step 3]（先注册模块再在 QML 中使用）
- [Step 5] depends on [Step 4]（先能显示波形再添加坐标轴属性）
- [Step 6] depends on [Step 5]（先有属性再绘制坐标轴）
- [Step 7] depends on [Step 6]（先有坐标轴绘制再处理缩放）
- [Step 8] depends on [Step 7]（先有坐标轴绘制再添加标签）
- [Step 9] depends on [Step 8]（先有完整坐标轴再添加样式）
- [Step 10] depends on [Step 9]（先有显示组件再构建数据源）
- [Step 11] depends on [Step 10]（先有数据源再连接）
- [Step 12] depends on [Step 11]（先有数据通道再接 TCP）
- [Step 13] depends on [Step 12]（先有时域再扩展频域）
