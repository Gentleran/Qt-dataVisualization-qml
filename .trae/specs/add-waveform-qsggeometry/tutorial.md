# 波形可视化系统 — 从零搭建教学（QSGGeometry 方案）

> 本文档按步骤从零搭建，每步包含：要改的文件、完整代码、为什么这么写。
> 你只需要按顺序操作，每步完成后都可以编译运行验证。

---

## Step 1: 搭建 ChartPannel QML 骨架

### 目标
在 ChartPannel 中创建上下两个图表区域，为后续填入 WaveformItem 做准备。

### 要修改的文件
- `MotolControlQmlContent/ChartPannel.qml`

### 完整代码

```qml
// ChartPannel.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // ===== 上方：时域波形图 =====
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8

            Label {
                text: "时域波形"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
            }

            // 后续将替换为 WaveformItem
        }

        // ===== 下方：频域波形图 =====
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8

            Label {
                text: "频域波形 (FFT)"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
            }

            // 后续将替换为 WaveformItem
        }
    }
}
```

### 为什么这么写

| 代码 | 含义 |
|------|------|
| `ColumnLayout` | 上下堆叠布局，两个子元素各占一半高度 |
| `Layout.fillHeight: true` | 两个 Rectangle 等分垂直空间 |
| `color: "#1e1e2e"` | 深色背景，波形图通常用深色以突出线条 |
| `Label` | 标题标签，区分时域和频域 |
| `anchors.margins: 10` | 内边距，让图表不贴边 |

### 验证
编译运行，应看到左右布局中右侧区域分为上下两个深色矩形，各带标题。

---

## Step 2: 创建 WaveformItem — QML_ELEMENT + QSGGeometry

### 目标
创建一个 QQuickItem 子类，使用 `QML_ELEMENT` 宏声明式注册，用 QSGGeometry 画一条静态斜线。

**关键变化**：使用 `QML_ELEMENT` 宏代替手动 `qmlRegisterType()`，这是 Qt 6 推荐的方式。配合 `qt_add_qml_module()` 在 CMake 中自动完成 QML 类型注册。

### 要创建的文件
- `App/waveformitem.h`
- `App/waveformitem.cpp`

### waveformitem.h 完整代码

```cpp
#ifndef WAVEFORMITEM_H
#define WAVEFORMITEM_H

#include <QQuickItem>
#include <QVector>

class WaveformItem : public QQuickItem
{
    Q_OBJECT
    // 【关键】QML_ELEMENT 宏：让 Qt 的 moc 自动将此类注册到 QML
    // 不再需要手动调用 qmlRegisterType<WaveformItem>(...)
    // 配合 CMakeLists.txt 中的 qt_add_qml_module() 使用
    QML_ELEMENT

    // samples 属性：存储要显示的数据点
    Q_PROPERTY(QVector<float> samples READ samples WRITE setSamples NOTIFY samplesChanged)

public:
    explicit WaveformItem(QQuickItem *parent = nullptr);

    QVector<float> samples() const;
    void setSamples(const QVector<float> &newSamples);

signals:
    void samplesChanged();

protected:
    // Qt Scene Graph 的核心方法：在 GPU 上绘制内容
    // 每帧需要重绘时，Qt 会自动调用此方法
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

private:
    QVector<float> m_samples;
    bool m_samplesChanged = false;  // 标记数据是否变化，避免每帧都重建几何
};

#endif // WAVEFORMITEM_H
```

### waveformitem.cpp 完整代码

```cpp
#include "waveformitem.h"
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>

WaveformItem::WaveformItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    // 【关键】必须设置这两个 flag，否则 updatePaintNode 不会被调用
    // setFlag(ItemHasContents) 告诉 Qt Scene Graph：这个 item 有可视化内容
    setFlag(ItemHasContents);
}

QVector<float> WaveformItem::samples() const
{
    return m_samples;
}

void WaveformItem::setSamples(const QVector<float> &newSamples)
{
    if (m_samples == newSamples)
        return;
    m_samples = newSamples;
    m_samplesChanged = true;
    emit samplesChanged();
    // 【关键】通知 Qt Scene Graph 需要重绘
    // update() 会在下一帧调用 updatePaintNode()
    update();
}

QSGNode *WaveformItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    QSGGeometryNode *node = static_cast<QSGGeometryNode *>(oldNode);

    if (!node) {
        // ---- 第一次：创建节点 ----
        node = new QSGGeometryNode;

        // 创建材质（纯色）
        QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
        material->setColor(QColor(0, 255, 0));  // 绿色线条
        node->setMaterial(material);

        // 初始几何体（2个顶点的线段）
        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 2);
        geometry->setLineWidth(2.0f);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
        node->setGeometry(geometry);
    }

    // ---- 更新几何数据 ----
    if (m_samplesChanged && !m_samples.isEmpty()) {
        m_samplesChanged = false;

        int count = m_samples.size();
        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), count);
        geometry->setLineWidth(2.0f);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);

        QSGGeometry::Point2D *vertices = geometry->vertexDataAsPoint2D();
        float xStep = width() / qMax(count - 1, 1);

        // 找到数据范围，用于归一化
        float minVal = *std::min_element(m_samples.begin(), m_samples.end());
        float maxVal = *std::max_element(m_samples.begin(), m_samples.end());
        float range = maxVal - minVal;
        if (range < 1e-6f) range = 1.0f;

        for (int i = 0; i < count; ++i) {
            vertices[i].x = i * xStep;
            vertices[i].y = height() - ((m_samples[i] - minVal) / range) * height();
        }

        node->setGeometry(geometry);
    } else if (m_samples.isEmpty() && node->geometry()) {
        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
        node->setGeometry(geometry);
    }

    node->markDirty(QSGNode::DirtyGeometry);
    return node;
}
```

### 为什么这么写

| 代码 | 为什么 |
|------|--------|
| `QML_ELEMENT` | Qt 6 声明式注册宏，配合 `qt_add_qml_module()` 自动完成 QML 类型注册，无需手动调用 `qmlRegisterType()` |
| `setFlag(ItemHasContents)` | **必须设置**，否则 Qt 认为 item 无内容，不会调用 `updatePaintNode()` |
| `update()` | 数据变化后调用，通知 Scene Graph 下一帧需要重绘 |
| `oldNode` 复用 | 避免每帧创建/销毁节点，提高性能 |
| `QSGGeometry::DrawLineStrip` | 连续线段模式，顶点依次连接成折线 |
| `QSGFlatColorMaterial` | 最简单的材质，纯色填充，性能最好 |
| `vertexDataAsPoint2D()` | 获取顶点数组指针，直接写入 GPU 缓冲区 |
| `markDirty(DirtyGeometry)` | 告诉 Scene Graph 几何数据已变，需要重新上传 GPU |
| Y 轴翻转 | 屏幕 Y 轴向下，数据 Y 轴向上，需要 `height() - value` |

### 验证
编译通过即可（此步骤还看不到效果，需要 Step 3 配置 CMake 模块注册）。

---

## Step 3: 使用 qt_add_qml_module 注册 QML 模块

### 目标
使用 `qt_add_qml_module()` 在 CMake 中声明 QML 模块，配合 `QML_ELEMENT` 宏自动完成类型注册。**不再需要**在 `main.cpp` 中手动调用 `qmlRegisterType()`。

**为什么用 qt_add_qml_module？**
- Qt 6 推荐方式，自动生成 `qmldir` 和类型注册代码
- 配合 `QML_ELEMENT` 宏，C++ 类自动暴露到 QML
- 支持QML工具链（qmllint、QML语言服务器等）的类型检查
- 无需在 main.cpp 中手动注册，减少样板代码

### 要修改的文件
- 根 `CMakeLists.txt`

### CMakeLists.txt 修改

在根 `CMakeLists.txt` 中，将 `qt_add_executable` 和 `qt_add_qml_module` 配合使用：

```cmake
qt_add_executable(${CMAKE_PROJECT_NAME}
    App/tcpclient.h App/tcpclient.cpp
    App/ConstantConfig.h
)

qt_add_resources(${CMAKE_PROJECT_NAME} "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf)
)

# 【关键】qt_add_qml_module：将 C++ 类注册为 QML 模块
# - URI "WaveformItem"：QML 中 import WaveformItem 即可使用
# - VERSION 1.0：模块版本号
# - SOURCES：包含 QML_ELEMENT 宏的 C++ 源文件
# - 自动生成 qmldir 和类型注册代码，无需手动 qmlRegisterType
qt_add_qml_module(${CMAKE_PROJECT_NAME}
    URI WaveformItem
    VERSION 1.0
    SOURCES
        App/waveformitem.h App/waveformitem.cpp
)
```

**注意事项**：
- `SOURCES` 中列出的文件如果包含 `QML_ELEMENT` 宏，Qt 会自动将其注册到对应的 QML 模块
- 不要在 `qt_add_executable` 和 `qt_add_qml_module` 中重复添加同一源文件
- `URI` 决定了 QML 中的 `import` 语句：`import WaveformItem`

### main.cpp — 无需修改

使用 `QML_ELEMENT` + `qt_add_qml_module` 后，**不需要**在 `main.cpp` 中添加任何注册代码。原来的 `qmlRegisterType<WaveformItem>(...)` 完全不需要了。

`main.cpp` 保持原样：

```cpp
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "autogen/environment.h"
#include "tcpclient.h"

int main(int argc, char *argv[])
{
    set_qt_environment();
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    TcpClient *tcpClient = new TcpClient(&engine);
    qmlRegisterSingletonInstance<TcpClient>(
        "MotolControlQml", 1, 0, "TcpClient", tcpClient);

    const QUrl url(mainQmlFile);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");
    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
```

### 对比：旧方式 vs 新方式

| 旧方式（手动注册） | 新方式（声明式注册） |
|---|---|
| `qmlRegisterType<WaveformItem>("MotolControlQml", 1, 0, "WaveformItem")` | `QML_ELEMENT` 宏 + `qt_add_qml_module()` |
| 需要在 main.cpp 中为每个类型写注册代码 | 自动注册，无需修改 main.cpp |
| 不支持 qmllint 类型检查 | 支持 qmllint 和 QML 语言服务器 |
| 模块信息分散在 C++ 和 CMake 中 | 模块信息集中在 CMake 中 |

### 验证
编译通过，确认 CMake 配置正确。

---

## Step 4: 在 ChartPannel 中使用 WaveformItem

### 目标
在 ChartPannel 中使用 `import WaveformItem` 导入模块，显示一条静态测试波形。

### 要修改的文件
- `MotolControlQmlContent/ChartPannel.qml`

### ChartPannel.qml 修改

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveformItem   // 通过 qt_add_qml_module 注册的模块

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // ===== 上方：时域波形图 =====
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8
            clip: true

            Label {
                text: "时域波形"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                z: 1
            }

            WaveformItem {
                id: timeDomainChart
                anchors.fill: parent
                anchors.margins: 30

                // 测试数据：正弦波
                samples: {
                    var data = [];
                    for (var i = 0; i < 200; i++) {
                        data.push(Math.sin(i * 0.1) * 100);
                    }
                    return data;
                }
            }
        }

        // ===== 下方：频域波形图 =====
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8
            clip: true

            Label {
                text: "频域波形 (FFT)"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                z: 1
            }

            WaveformItem {
                id: freqDomainChart
                anchors.fill: parent
                anchors.margins: 30

                // 测试数据
                samples: {
                    var data = [];
                    for (var i = 0; i < 200; i++) {
                        data.push(Math.sin(i * 0.3) + Math.sin(i * 0.7) * 0.5);
                    }
                    return data;
                }
            }
        }
    }
}
```

### 为什么这么写

| 代码 | 为什么 |
|------|--------|
| `import WaveformItem` | 导入由 `qt_add_qml_module(URI WaveformItem ...)` 注册的模块 |
| `clip: true` | 裁剪波形超出边界的部分，防止溢出到其他区域 |
| `anchors.margins: 30` | 留出坐标轴/刻度的空间 |
| `z: 1` | 标签在 WaveformItem 之上，不被遮挡 |

### 验证
编译运行，应看到：
- 上方深色区域显示一条绿色正弦波
- 下方深色区域显示一条绿色复合波

---

## Step 5: 为 WaveformItem 添加坐标轴属性

### 目标
为 WaveformItem 添加 X/Y 坐标轴配置属性，支持：
- 自定义 X/Y 轴数值范围（`xMin`, `xMax`, `yMin`, `yMax`）
- 配置坐标轴等分数量（`xTickCount`, `yTickCount`），控制轴上显示几个刻度值
- 坐标轴边距（`axisLeftMargin`, `axisBottomMargin`），为刻度标签留出空间

### 要修改的文件
- `App/waveformitem.h`
- `App/waveformitem.cpp`

### waveformitem.h 修改

在 `Q_PROPERTY(QVector<float> samples ...)` 下方添加：

```cpp
// ===== 坐标轴属性 =====

// X 轴数值范围
Q_PROPERTY(float xMin READ xMin WRITE setXMin NOTIFY xMinChanged)
Q_PROPERTY(float xMax READ xMax WRITE setXMax NOTIFY xMaxChanged)

// Y 轴数值范围
Q_PROPERTY(float yMin READ yMin WRITE setYMin NOTIFY yMinChanged)
Q_PROPERTY(float yMax READ yMax WRITE setYMax NOTIFY yMaxChanged)

// X 轴等分数量（例如 xTickCount=5 表示显示 0%, 20%, 40%, 60%, 80%, 100% 共6个刻度位置）
Q_PROPERTY(int xTickCount READ xTickCount WRITE setXTickCount NOTIFY xTickCountChanged)

// Y 轴等分数量
Q_PROPERTY(int yTickCount READ yTickCount WRITE setYTickCount NOTIFY yTickCountChanged)

// 坐标轴边距（为刻度标签留出空间，单位：像素）
Q_PROPERTY(qreal axisLeftMargin READ axisLeftMargin WRITE setAxisLeftMargin NOTIFY axisLeftMarginChanged)
Q_PROPERTY(qreal axisBottomMargin READ axisBottomMargin WRITE setAxisBottomMargin NOTIFY axisBottomMarginChanged)
Q_PROPERTY(qreal axisTopMargin READ axisTopMargin WRITE setAxisTopMargin NOTIFY axisTopMarginChanged)
Q_PROPERTY(qreal axisRightMargin READ axisRightMargin WRITE setAxisRightMargin NOTIFY axisRightMarginChanged)
```

在 `public:` 区域添加：

```cpp
// X 轴范围
float xMin() const;
void setXMin(float val);
float xMax() const;
void setXMax(float val);

// Y 轴范围
float yMin() const;
void setYMin(float val);
float yMax() const;
void setYMax(float val);

// 刻度等分数量
int xTickCount() const;
void setXTickCount(int count);
int yTickCount() const;
void setYTickCount(int count);

// 坐标轴边距
qreal axisLeftMargin() const;
void setAxisLeftMargin(qreal margin);
qreal axisBottomMargin() const;
void setAxisBottomMargin(qreal margin);
qreal axisTopMargin() const;
void setAxisTopMargin(qreal margin);
qreal axisRightMargin() const;
void setAxisRightMargin(qreal margin);
```

在 `signals:` 区域添加：

```cpp
void xMinChanged();
void xMaxChanged();
void yMinChanged();
void yMaxChanged();
void xTickCountChanged();
void yTickCountChanged();
void axisLeftMarginChanged();
void axisBottomMarginChanged();
void axisTopMarginChanged();
void axisRightMarginChanged();
```

在 `private:` 区域添加：

```cpp
// X/Y 轴范围
float m_xMin = 0.0f;
float m_xMax = 1.0f;
float m_yMin = 0.0f;
float m_yMax = 1.0f;

// 刻度等分数量
int m_xTickCount = 5;   // 默认5等分，显示6个刻度值
int m_yTickCount = 5;

// 坐标轴边距
qreal m_axisLeftMargin = 50.0;    // 左侧留空给 Y 轴标签
qreal m_axisBottomMargin = 30.0;  // 底部留空给 X 轴标签
qreal m_axisTopMargin = 10.0;
qreal m_axisRightMargin = 10.0;

bool m_axisChanged = false;  // 标记坐标轴属性是否变化
```

### waveformitem.cpp 修改

添加所有 getter/setter 实现：

```cpp
// ===== X 轴范围 =====
float WaveformItem::xMin() const { return m_xMin; }
void WaveformItem::setXMin(float val) {
    if (qFuzzyCompare(m_xMin, val)) return;
    m_xMin = val;
    m_axisChanged = true;
    emit xMinChanged();
    update();
}

float WaveformItem::xMax() const { return m_xMax; }
void WaveformItem::setXMax(float val) {
    if (qFuzzyCompare(m_xMax, val)) return;
    m_xMax = val;
    m_axisChanged = true;
    emit xMaxChanged();
    update();
}

// ===== Y 轴范围 =====
float WaveformItem::yMin() const { return m_yMin; }
void WaveformItem::setYMin(float val) {
    if (qFuzzyCompare(m_yMin, val)) return;
    m_yMin = val;
    m_axisChanged = true;
    emit yMinChanged();
    update();
}

float WaveformItem::yMax() const { return m_yMax; }
void WaveformItem::setYMax(float val) {
    if (qFuzzyCompare(m_yMax, val)) return;
    m_yMax = val;
    m_axisChanged = true;
    emit yMaxChanged();
    update();
}

// ===== 刻度等分数量 =====
int WaveformItem::xTickCount() const { return m_xTickCount; }
void WaveformItem::setXTickCount(int count) {
    if (m_xTickCount == count) return;
    m_xTickCount = count;
    m_axisChanged = true;
    emit xTickCountChanged();
    update();
}

int WaveformItem::yTickCount() const { return m_yTickCount; }
void WaveformItem::setYTickCount(int count) {
    if (m_yTickCount == count) return;
    m_yTickCount = count;
    m_axisChanged = true;
    emit yTickCountChanged();
    update();
}

// ===== 坐标轴边距 =====
qreal WaveformItem::axisLeftMargin() const { return m_axisLeftMargin; }
void WaveformItem::setAxisLeftMargin(qreal margin) {
    if (qFuzzyCompare(m_axisLeftMargin, margin)) return;
    m_axisLeftMargin = margin;
    m_axisChanged = true;
    emit axisLeftMarginChanged();
    update();
}

qreal WaveformItem::axisBottomMargin() const { return m_axisBottomMargin; }
void WaveformItem::setAxisBottomMargin(qreal margin) {
    if (qFuzzyCompare(m_axisBottomMargin, margin)) return;
    m_axisBottomMargin = margin;
    m_axisChanged = true;
    emit axisBottomMarginChanged();
    update();
}

qreal WaveformItem::axisTopMargin() const { return m_axisTopMargin; }
void WaveformItem::setAxisTopMargin(qreal margin) {
    if (qFuzzyCompare(m_axisTopMargin, margin)) return;
    m_axisTopMargin = margin;
    m_axisChanged = true;
    emit axisTopMarginChanged();
    update();
}

qreal WaveformItem::axisRightMargin() const { return m_axisRightMargin; }
void WaveformItem::setAxisRightMargin(qreal margin) {
    if (qFuzzyCompare(m_axisRightMargin, margin)) return;
    m_axisRightMargin = margin;
    m_axisChanged = true;
    emit axisRightMarginChanged();
    update();
}
```

### 为什么这么写

| 代码 | 为什么 |
|------|--------|
| `xMin/xMax/yMin/yMax` | 定义坐标轴数值范围，数据映射到此范围内显示 |
| `xTickCount/yTickCount` | 控制坐标轴等分数量，如 `yTickCount=5` 表示 Y 轴分5段，显示6个刻度值 |
| `axisLeftMargin/axisBottomMargin` | 为坐标轴标签留出空间，避免标签与波形重叠 |
| `m_axisChanged` 标记 | 坐标轴属性变化时标记，避免每帧都重建轴几何 |
| `qFuzzyCompare` | 浮点数比较用模糊比较，避免精度误差导致无限更新 |

### 验证
编译通过。此时属性已添加，但还未绘制坐标轴，QML 中可先设置属性测试编译。

---

## Step 6: 绘制坐标轴线和刻度线

### 目标
在 `updatePaintNode()` 中绘制：
1. 坐标轴边框线（矩形框）
2. X/Y 轴刻度线（短横线/竖线）
3. 网格线（虚线样式的水平/垂直线）
4. 波形数据线（映射到坐标轴范围内）

### 要修改的文件
- `App/waveformitem.cpp`

### 核心概念

```
┌─────────────────────────────────────────┐
│  axisTopMargin                          │
│  ┌──────────────────────────────────┐   │
│  │         绘图区域 (plotArea)       │   │
│ax│  ─ ─ ─ 网格线 ─ ─ ─ ─ ─ ─ ─ ─  │ax│
│is│  ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─  │is│
│L │  ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─  │R │
│e │  ~~~~~~~~~~~~波形线~~~~~~~~~~~~~  │i │
│f │  ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─  │g │
│t │  │  │  │  │  │  │  X轴刻度线    │h │
│M │──┴──┴──┴──┴──┴──┴───────────────┘t │
│a │  axisBottomMargin                    │
│r │  [X轴标签由QML Text元素绘制]         │
│g │                                      │
│i │  [Y轴标签由QML Text元素绘制]         │
│n │                                      │
└──┴──────────────────────────────────────┘
```

### updatePaintNode() 重写

将 `updatePaintNode()` 重写为使用场景图节点树结构：

```cpp
QSGNode *WaveformItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);

    // ============================================================
    // 场景图节点树结构：
    //
    // root (QSGGeometryNode - 不绘制，仅作为容器)
    // ├── axisNode (QSGGeometryNode - 坐标轴边框 + 刻度线 + 网格线)
    // └── waveNode (QSGGeometryNode - 波形数据线)
    //
    // 分离轴线和波形，可以独立更新，提高性能
    // ============================================================

    QSGNode *root = oldNode;
    if (!root) {
        root = new QSGNode;  // 根节点不绘制，仅作为容器
        root->appendChildNode(new QSGGeometryNode);  // axisNode (index 0)
        root->appendChildNode(new QSGGeometryNode);  // waveNode (index 1)
    }

    QSGGeometryNode *axisNode = static_cast<QSGGeometryNode *>(root->childAtIndex(0));
    QSGGeometryNode *waveNode = static_cast<QSGGeometryNode *>(root->childAtIndex(1));

    // 计算绘图区域
    qreal plotX = m_axisLeftMargin;
    qreal plotY = m_axisTopMargin;
    qreal plotW = width() - m_axisLeftMargin - m_axisRightMargin;
    qreal plotH = height() - m_axisTopMargin - m_axisBottomMargin;

    if (plotW <= 0 || plotH <= 0)
        return root;

    // ---- 更新坐标轴几何 ----
    if (m_axisChanged || !axisNode->geometry()) {
        m_axisChanged = false;

        // 计算需要的顶点数：
        // 边框：4条线 = 5个顶点（闭合矩形）
        // Y轴刻度线：yTickCount+1 个刻度 × 2个顶点
        // X轴刻度线：xTickCount+1 个刻度 × 2个顶点
        // Y轴网格线：yTickCount-1 条 × 2个顶点（不含边框）
        // X轴网格线：xTickCount-1 条 × 2个顶点（不含边框）
        int tickCount = (m_yTickCount + 1) + (m_xTickCount + 1);
        int gridCount = (m_yTickCount - 1) + (m_xTickCount - 1);
        if (gridCount < 0) gridCount = 0;
        int totalVertices = 5 + tickCount * 2 + gridCount * 2;

        QSGGeometry *axisGeom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), totalVertices);
        axisGeom->setLineWidth(1.0f);
        axisGeom->setDrawingMode(QSGGeometry::DrawLineStrip);

        QSGGeometry::Point2D *v = axisGeom->vertexDataAsPoint2D();
        int idx = 0;

        // --- 边框线（闭合矩形）---
        v[idx++].set(plotX, plotY);
        v[idx++].set(plotX + plotW, plotY);
        v[idx++].set(plotX + plotW, plotY + plotH);
        v[idx++].set(plotX, plotY + plotH);
        v[idx++].set(plotX, plotY);  // 闭合

        // --- Y轴刻度线 + 网格线 ---
        for (int i = 0; i <= m_yTickCount; ++i) {
            qreal yPos = plotY + (plotH * i) / m_yTickCount;

            // 刻度线（左侧短横线）
            v[idx++].set(plotX - 5, yPos);
            v[idx++].set(plotX, yPos);

            // 网格线（水平虚线，跳过顶部和底部边框线）
            if (i > 0 && i < m_yTickCount) {
                v[idx++].set(plotX, yPos);
                v[idx++].set(plotX + plotW, yPos);
            }
        }

        // --- X轴刻度线 + 网格线 ---
        for (int i = 0; i <= m_xTickCount; ++i) {
            qreal xPos = plotX + (plotW * i) / m_xTickCount;

            // 刻度线（底部短竖线）
            v[idx++].set(xPos, plotY + plotH);
            v[idx++].set(xPos, plotY + plotH + 5);

            // 网格线（垂直虚线，跳过左侧和右侧边框线）
            if (i > 0 && i < m_xTickCount) {
                v[idx++].set(xPos, plotY);
                v[idx++].set(xPos, plotY + plotH);
            }
        }

        axisNode->setGeometry(axisGeom);

        // 坐标轴材质（灰色）
        QSGFlatColorMaterial *axisMat = new QSGFlatColorMaterial;
        axisMat->setColor(QColor(80, 80, 80));  // 深灰色
        axisNode->setMaterial(axisMat);

        axisNode->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    }

    // ---- 更新波形几何 ----
    if (m_samplesChanged || !waveNode->geometry()) {
        // 注意：当窗口缩放时也需要重建波形几何（见 Step 7）

        if (m_samples.isEmpty()) {
            QSGGeometry *emptyGeom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
            emptyGeom->setDrawingMode(QSGGeometry::DrawLineStrip);
            waveNode->setGeometry(emptyGeom);
        } else {
            m_samplesChanged = false;

            int count = m_samples.size();
            QSGGeometry *waveGeom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), count);
            waveGeom->setLineWidth(2.0f);
            waveGeom->setDrawingMode(QSGGeometry::DrawLineStrip);

            QSGGeometry::Point2D *vertices = waveGeom->vertexDataAsPoint2D();

            // 将 samples 数据映射到绘图区域
            // X: 均匀分布在 [plotX, plotX + plotW] 范围
            // Y: 根据 yMin/yMax 映射到 [plotY, plotY + plotH] 范围
            float xRange = m_xMax - m_xMin;
            float yRange = m_yMax - m_yMin;
            if (yRange < 1e-6f) yRange = 1.0f;
            if (xRange < 1e-6f) xRange = 1.0f;

            float xStep = plotW / qMax(count - 1, 1);

            for (int i = 0; i < count; ++i) {
                vertices[i].x = plotX + i * xStep;

                // 将数据值映射到 [yMin, yMax] 范围，再映射到绘图区域
                // 注意：屏幕 Y 向下，数据 Y 向上，需要翻转
                float normalizedY = (m_samples[i] - m_yMin) / yRange;
                normalizedY = qBound(0.0f, normalizedY, 1.0f);  // 裁剪到 [0, 1]
                vertices[i].y = plotY + plotH - normalizedY * plotH;
            }

            waveNode->setGeometry(waveGeom);
        }

        // 波形材质（绿色）
        if (!waveNode->material()) {
            QSGFlatColorMaterial *waveMat = new QSGFlatColorMaterial;
            waveMat->setColor(QColor(0, 255, 0));
            waveNode->setMaterial(waveMat);
        }

        waveNode->markDirty(QSGNode::DirtyGeometry);
    }

    return root;
}
```

### 为什么这么写

| 代码 | 为什么 |
|------|--------|
| 场景图节点树 | 分离轴线和波形，可以独立更新，避免每次数据变化都重建轴几何 |
| `plotX/plotY/plotW/plotH` | 计算绘图区域，留出坐标轴边距空间 |
| `qBound(0.0f, normalizedY, 1.0f)` | 裁剪超出 Y 轴范围的数据，防止波形溢出绘图区域 |
| 灰色轴材质 + 绿色波形材质 | 视觉区分，轴线不抢波形的注意力 |
| 刻度线 `plotX - 5` | 刻度线延伸到轴外侧5像素，视觉上指示刻度位置 |
| 网格线跳过边框 | 边框线已绘制，网格线不需要重复 |

### 验证
编译运行，应看到：
- 波形区域有灰色矩形边框
- Y 轴左侧有短横线刻度
- X 轴底部有短竖线刻度
- 绘图区域内有灰色网格线
- 波形线映射到 yMin/yMax 范围内

---

## Step 7: 处理窗口缩放 — 响应式重绘

### 目标
当用户拖拽窗口改变大小时，波形图和坐标轴应自动适应新的区域大小，而不是保持初始化的显示状态。

### 问题分析

当前代码存在一个问题：`updatePaintNode()` 只在 `m_samplesChanged` 或 `m_axisChanged` 为 true 时重建几何数据。当窗口缩放时，虽然 Qt 会调用 `updatePaintNode()`，但因为这两个标志都是 false，几何数据不会更新，导致波形和坐标轴保持旧的大小。

### 解决方案

重写 `geometryChange()` 方法，当 Item 尺寸变化时标记需要重建几何并触发重绘。

### 要修改的文件
- `App/waveformitem.h`
- `App/waveformitem.cpp`

### waveformitem.h 修改

在 `protected:` 区域添加：

```cpp
// 重写 geometryChange：当 Item 尺寸变化时触发重绘
// 这是实现响应式缩放的关键
void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
```

在 `private:` 区域添加：

```cpp
bool m_geometryChanged = false;  // 标记尺寸是否变化
```

### waveformitem.cpp 修改

添加 `geometryChange()` 实现：

```cpp
void WaveformItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    // 【关键】当窗口缩放导致 Item 尺寸变化时：
    // 1. 标记几何已变化
    // 2. 调用 update() 触发 updatePaintNode() 重绘
    // 这样波形和坐标轴会自动适应新的区域大小

    if (newGeometry.size() != oldGeometry.size()) {
        m_geometryChanged = true;
        m_axisChanged = true;  // 坐标轴也需要重建（因为绘图区域变了）
        update();
    }

    // 必须调用基类实现，否则布局系统可能不正常工作
    QQuickItem::geometryChange(newGeometry, oldGeometry);
}
```

修改 `updatePaintNode()` 中的条件判断，在波形几何更新条件中加入 `m_geometryChanged`：

```cpp
// ---- 更新波形几何 ----
// 增加 m_geometryChanged 条件：窗口缩放时也需要重建波形几何
if (m_samplesChanged || m_geometryChanged || !waveNode->geometry()) {
    m_samplesChanged = false;
    m_geometryChanged = false;  // 重置标志

    // ... 后续代码不变
}
```

### 为什么这么写

| 代码 | 为什么 |
|------|--------|
| `geometryChange()` | Qt 5.10+ 提供的虚函数，当 Item 的位置或尺寸变化时被调用 |
| `newGeometry.size() != oldGeometry.size()` | 只在尺寸变化时触发，位置变化不需要重绘 |
| `m_geometryChanged = true` | 标记尺寸变化，让 `updatePaintNode()` 知道需要重建几何 |
| `m_axisChanged = true` | 绘图区域变了，坐标轴也需要重建 |
| `QQuickItem::geometryChange(...)` | 必须调用基类实现，确保布局系统正常工作 |

### 完整流程

```
用户拖拽窗口
    │
    ▼
Item 尺寸变化
    │
    ▼
geometryChange() 被调用
    │
    ├─ m_geometryChanged = true
    ├─ m_axisChanged = true
    └─ update() 请求重绘
         │
         ▼
    下一帧 updatePaintNode() 被调用
         │
         ├─ 重建坐标轴几何（使用新的 width/height 计算绘图区域）
         └─ 重建波形几何（映射到新的绘图区域）
              │
              ▼
         GPU 渲染更新后的图形
```

### 验证
编译运行后：
1. 拖拽窗口边缘放大 → 波形和坐标轴自动扩展到新的区域
2. 拖拽窗口边缘缩小 → 波形和坐标轴自动收缩
3. 最大化窗口 → 波形占满整个区域
4. 恢复窗口 → 波形回到原始大小

---

## Step 8: 在 QML 中添加坐标轴标签

### 目标
使用 QML Text 元素在坐标轴旁边显示刻度值。因为 QSGGeometry 无法渲染文字，所以文字标签由 QML 层负责。

### 要修改的文件
- `MotolControlQmlContent/ChartPannel.qml`

### ChartPannel.qml 修改

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveformItem

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // ===== 上方：时域波形图 =====
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8
            clip: true

            Label {
                text: "时域波形 - 通道0"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                z: 1
            }

            WaveformItem {
                id: timeDomainChart
                anchors.fill: parent

                // 坐标轴配置
                xMin: 0
                xMax: 200
                yMin: -100
                yMax: 100
                xTickCount: 5
                yTickCount: 5
                axisLeftMargin: 50
                axisBottomMargin: 30
                axisTopMargin: 10
                axisRightMargin: 10

                // 测试数据：正弦波
                samples: {
                    var data = [];
                    for (var i = 0; i < 200; i++) {
                        data.push(Math.sin(i * 0.1) * 100);
                    }
                    return data;
                }

                // ===== Y 轴刻度标签 =====
                Repeater {
                    model: timeDomainChart.yTickCount + 1
                    delegate: Text {
                        // 计算刻度值
                        property real tickValue: timeDomainChart.yMax -
                            (timeDomainChart.yMax - timeDomainChart.yMin) * modelData / timeDomainChart.yTickCount
                        // 计算Y位置（在绘图区域内）
                        property real yPos: timeDomainChart.axisTopMargin +
                            (timeDomainChart.height - timeDomainChart.axisTopMargin - timeDomainChart.axisBottomMargin) *
                            modelData / timeDomainChart.yTickCount

                        text: tickValue.toFixed(1)
                        color: "#999999"
                        font.pixelSize: 11
                        anchors.right: parent.left
                        anchors.rightMargin: 3
                        y: yPos - height / 2
                    }
                }

                // ===== X 轴刻度标签 =====
                Repeater {
                    model: timeDomainChart.xTickCount + 1
                    delegate: Text {
                        // 计算刻度值
                        property real tickValue: timeDomainChart.xMin +
                            (timeDomainChart.xMax - timeDomainChart.xMin) * modelData / timeDomainChart.xTickCount
                        // 计算X位置（在绘图区域内）
                        property real xPos: timeDomainChart.axisLeftMargin +
                            (timeDomainChart.width - timeDomainChart.axisLeftMargin - timeDomainChart.axisRightMargin) *
                            modelData / timeDomainChart.xTickCount

                        text: tickValue.toFixed(0)
                        color: "#999999"
                        font.pixelSize: 11
                        anchors.top: parent.bottom
                        anchors.topMargin: 3
                        x: xPos - width / 2
                    }
                }
            }
        }

        // ===== 下方：频域波形图 =====
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8
            clip: true

            Label {
                text: "频域波形 (FFT)"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                z: 1
            }

            WaveformItem {
                id: freqDomainChart
                anchors.fill: parent

                xMin: 0
                xMax: 200
                yMin: -2
                yMax: 2
                xTickCount: 5
                yTickCount: 4
                axisLeftMargin: 50
                axisBottomMargin: 30
                axisTopMargin: 10
                axisRightMargin: 10

                // 测试数据
                samples: {
                    var data = [];
                    for (var i = 0; i < 200; i++) {
                        data.push(Math.sin(i * 0.3) + Math.sin(i * 0.7) * 0.5);
                    }
                    return data;
                }

                // Y 轴标签
                Repeater {
                    model: freqDomainChart.yTickCount + 1
                    delegate: Text {
                        property real tickValue: freqDomainChart.yMax -
                            (freqDomainChart.yMax - freqDomainChart.yMin) * modelData / freqDomainChart.yTickCount
                        property real yPos: freqDomainChart.axisTopMargin +
                            (freqDomainChart.height - freqDomainChart.axisTopMargin - freqDomainChart.axisBottomMargin) *
                            modelData / freqDomainChart.yTickCount

                        text: tickValue.toFixed(1)
                        color: "#999999"
                        font.pixelSize: 11
                        anchors.right: parent.left
                        anchors.rightMargin: 3
                        y: yPos - height / 2
                    }
                }

                // X 轴标签
                Repeater {
                    model: freqDomainChart.xTickCount + 1
                    delegate: Text {
                        property real tickValue: freqDomainChart.xMin +
                            (freqDomainChart.xMax - freqDomainChart.xMin) * modelData / freqDomainChart.xTickCount
                        property real xPos: freqDomainChart.axisLeftMargin +
                            (freqDomainChart.width - freqDomainChart.axisLeftMargin - freqDomainChart.axisRightMargin) *
                            modelData / freqDomainChart.xTickCount

                        text: tickValue.toFixed(0)
                        color: "#999999"
                        font.pixelSize: 11
                        anchors.top: parent.bottom
                        anchors.topMargin: 3
                        x: xPos - width / 2
                    }
                }
            }
        }
    }
}
```

### 为什么这么写

| 代码 | 为什么 |
|------|--------|
| `Repeater` + `yTickCount + 1` | 生成 N+1 个标签（N 等分产生 N+1 个刻度点） |
| `tickValue` 计算 | 根据 yMin/yMax 和等分索引计算实际刻度值 |
| `yPos` 计算 | 根据绘图区域边距和等分索引计算像素位置，与 C++ 端刻度线位置对齐 |
| `anchors.right: parent.left` | Y 轴标签在 WaveformItem 左侧 |
| `anchors.top: parent.bottom` | X 轴标签在 WaveformItem 底部 |
| `toFixed(1)` / `toFixed(0)` | 控制小数位数，Y 轴保留1位，X 轴取整 |

### 验证
编译运行，应看到：
- Y 轴左侧显示刻度值（如 -100.0, -60.0, -20.0, 20.0, 60.0, 100.0）
- X 轴底部显示刻度值（如 0, 40, 80, 120, 160, 200）
- 刻度值与刻度线对齐
- 拖拽窗口缩放后，标签位置自动调整

---

## Step 9: 为 WaveformItem 添加样式属性

### 目标
支持自定义线条颜色和线宽，为后续多通道不同颜色显示做准备。

### 要修改的文件
- `App/waveformitem.h`
- `App/waveformitem.cpp`

### waveformitem.h 修改

在坐标轴属性下方添加：

```cpp
Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
Q_PROPERTY(float lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
```

在 `public:` 区域添加：

```cpp
QColor lineColor() const;
void setLineColor(const QColor &color);

float lineWidth() const;
void setLineWidth(float width);
```

在 `signals:` 区域添加：

```cpp
void lineColorChanged();
void lineWidthChanged();
```

在 `private:` 区域添加：

```cpp
QColor m_lineColor = QColor(0, 255, 0);  // 默认绿色
float m_lineWidth = 2.0f;
bool m_colorChanged = false;
```

### waveformitem.cpp 修改

添加 getter/setter 实现：

```cpp
QColor WaveformItem::lineColor() const { return m_lineColor; }
void WaveformItem::setLineColor(const QColor &color) {
    if (m_lineColor == color) return;
    m_lineColor = color;
    m_colorChanged = true;
    emit lineColorChanged();
    update();
}

float WaveformItem::lineWidth() const { return m_lineWidth; }
void WaveformItem::setLineWidth(float width) {
    if (qFuzzyCompare(m_lineWidth, width)) return;
    m_lineWidth = width;
    update();
}
```

修改 `updatePaintNode()` 中波形材质和线宽部分：

```cpp
// 在波形几何更新部分：
waveGeom->setLineWidth(m_lineWidth);

// 在波形材质更新部分：
if (!waveNode->material() || m_colorChanged) {
    m_colorChanged = false;
    QSGFlatColorMaterial *waveMat = new QSGFlatColorMaterial;
    waveMat->setColor(m_lineColor);
    waveNode->setMaterial(waveMat);
    waveNode->markDirty(QSGNode::DirtyMaterial);
}
```

### QML 中使用

```qml
WaveformItem {
    lineColor: "#00ff00"    // 绿色
    lineWidth: 1.5
    yMin: -100
    yMax: 100
    samples: [...]
}
```

### 验证
修改 ChartPannel.qml 中两个 WaveformItem 的 lineColor 为不同颜色，确认颜色生效。

---

## Step 10: 创建 WaveformDataSource — 环形缓冲 + 降采样

### 目标
创建数据源类，负责：
1. 接收原始二进制数据
2. 解析为 16 通道 float 数组
3. 环形缓冲存储
4. Min-Max 降采样
5. 暴露降采样后的数据给 QML

### 要创建的文件
- `App/waveformdatasource.h`
- `App/waveformdatasource.cpp`

### waveformdatasource.h 完整代码

```cpp
#ifndef WAVEFORMDATASOURCE_H
#define WAVEFORMDATASOURCE_H

#include <QObject>
#include <QVector>

class WaveformDataSource : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON  // 全局只有一个数据源

    // 降采样后的通道数据，供 QML 绑定
    Q_PROPERTY(QVector<float> channel0 READ channel0 NOTIFY channel0Changed)
    // TODO: 后续可扩展为 channel1~channel15

public:
    explicit WaveformDataSource(QObject *parent = nullptr);

    // 接收原始二进制数据并解析
    Q_INVOKABLE void appendData(const QByteArray &data, int channelCount = 16, int sampleSize = 4);

    // 获取降采样后的数据
    QVector<float> channel0() const;

    // 设置降采样目标点数（通常 = 屏幕宽度 × 2）
    Q_INVOKABLE void setDisplayPoints(int points);

signals:
    void channel0Changed();

private:
    static const int BUFFER_SIZE = 100000;
    QVector<QVector<float>> m_ringBuffers;
    QVector<int> m_writePos;
    int m_displayPoints = 2000;

    QVector<float> m_channel0Downsampled;

    QVector<float> downsample(const QVector<float> &data, int targetPoints) const;
    void updateDownsampledData();
};

#endif // WAVEFORMDATASOURCE_H
```

### waveformdatasource.cpp 完整代码

```cpp
#include "waveformdatasource.h"
#include <algorithm>
#include <cstring>

WaveformDataSource::WaveformDataSource(QObject *parent)
    : QObject(parent)
{
    m_ringBuffers.resize(16);
    m_writePos.resize(16, 0);
    for (int i = 0; i < 16; ++i) {
        m_ringBuffers[i].resize(BUFFER_SIZE);
    }
}

void WaveformDataSource::appendData(const QByteArray &data, int channelCount, int sampleSize)
{
    const float *floatData = reinterpret_cast<const float *>(data.constData());
    int totalFloats = data.size() / sampleSize;
    int samplesPerChannel = totalFloats / channelCount;

    for (int s = 0; s < samplesPerChannel; ++s) {
        for (int ch = 0; ch < channelCount && ch < 16; ++ch) {
            float value = floatData[s * channelCount + ch];
            m_ringBuffers[ch][m_writePos[ch]] = value;
            m_writePos[ch] = (m_writePos[ch] + 1) % BUFFER_SIZE;
        }
    }

    updateDownsampledData();
}

QVector<float> WaveformDataSource::channel0() const
{
    return m_channel0Downsampled;
}

void WaveformDataSource::setDisplayPoints(int points)
{
    if (m_displayPoints == points)
        return;
    m_displayPoints = points;
    updateDownsampledData();
}

QVector<float> WaveformDataSource::downsample(const QVector<float> &data, int targetPoints) const
{
    if (data.size() <= targetPoints)
        return data;

    QVector<float> result;
    result.reserve(targetPoints);

    int groupSize = data.size() / (targetPoints / 2);
    if (groupSize < 1) groupSize = 1;

    for (int i = 0; i < data.size() && result.size() < targetPoints; i += groupSize) {
        float minVal = data[i];
        float maxVal = data[i];
        int end = qMin(i + groupSize, data.size());
        for (int j = i + 1; j < end; ++j) {
            minVal = qMin(minVal, data[j]);
            maxVal = qMax(maxVal, data[j]);
        }
        result.append(minVal);
        result.append(maxVal);
    }

    return result;
}

void WaveformDataSource::updateDownsampledData()
{
    QVector<float> orderedData;
    orderedData.resize(BUFFER_SIZE);
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        orderedData[i] = m_ringBuffers[0][(m_writePos[0] + i) % BUFFER_SIZE];
    }

    m_channel0Downsampled = downsample(orderedData, m_displayPoints);
    emit channel0Changed();
}
```

### 修改 CMakeLists.txt

在 `qt_add_qml_module` 的 `SOURCES` 中添加：

```cmake
qt_add_qml_module(${CMAKE_PROJECT_NAME}
    URI WaveformItem
    VERSION 1.0
    SOURCES
        App/waveformitem.h App/waveformitem.cpp
        App/waveformdatasource.h App/waveformdatasource.cpp
)
```

### 为什么这么写

| 代码 | 为什么 |
|------|--------|
| `QML_SINGLETON` | 配合 `QML_ELEMENT`，声明为 QML 单例，无需在 main.cpp 中手动注册 |
| 环形缓冲区 `m_ringBuffers` | 固定大小，新数据覆盖旧数据，避免内存无限增长 |
| Min-Max 降采样 | 保留极值，不丢失尖峰，比均匀取点更准确 |
| `BUFFER_SIZE = 100000` | 存储 1 秒的原始数据（100kHz 采样） |

### 验证
编译通过即可。

---

## Step 11: 连接数据源到 WaveformItem

### 目标
在 QML 中将 WaveformDataSource 的数据绑定到 WaveformItem，用模拟数据测试实时刷新。

### 要修改的文件
- `MotolControlQmlContent/ChartPannel.qml`

### ChartPannel.qml 修改

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveformItem

Item {
    id: root

    // 模拟数据定时器（测试用，后续替换为 TCP 数据）
    Timer {
        id: mockTimer
        interval: 50   // 50ms 刷新一次 = 20 FPS
        running: true
        repeat: true
        property int tick: 0
        onTriggered: {
            tick++
            var data = new ArrayBuffer(16 * 100 * 4);
            var view = new Float32Array(data);
            for (var s = 0; s < 100; s++) {
                for (var ch = 0; ch < 16; ch++) {
                    view[s * 16 + ch] = Math.sin((tick * 100 + s) * 0.05 + ch * 0.5) * 100;
                }
            }
            WaveformDataSource.appendData(
                Qt.bufferFromData(data), 16, 4)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8
            clip: true

            Label {
                text: "时域波形 - 通道0"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                z: 1
            }

            WaveformItem {
                id: timeDomainChart
                anchors.fill: parent

                xMin: 0
                xMax: 100
                yMin: -100
                yMax: 100
                xTickCount: 5
                yTickCount: 5
                axisLeftMargin: 50
                axisBottomMargin: 30
                lineColor: "#00ff00"

                samples: WaveformDataSource.channel0

                // Y 轴标签
                Repeater {
                    model: timeDomainChart.yTickCount + 1
                    delegate: Text {
                        property real tickValue: timeDomainChart.yMax -
                            (timeDomainChart.yMax - timeDomainChart.yMin) * modelData / timeDomainChart.yTickCount
                        property real yPos: timeDomainChart.axisTopMargin +
                            (timeDomainChart.height - timeDomainChart.axisTopMargin - timeDomainChart.axisBottomMargin) *
                            modelData / timeDomainChart.yTickCount
                        text: tickValue.toFixed(1)
                        color: "#999999"
                        font.pixelSize: 11
                        anchors.right: parent.left
                        anchors.rightMargin: 3
                        y: yPos - height / 2
                    }
                }

                // X 轴标签
                Repeater {
                    model: timeDomainChart.xTickCount + 1
                    delegate: Text {
                        property real tickValue: timeDomainChart.xMin +
                            (timeDomainChart.xMax - timeDomainChart.xMin) * modelData / timeDomainChart.xTickCount
                        property real xPos: timeDomainChart.axisLeftMargin +
                            (timeDomainChart.width - timeDomainChart.axisLeftMargin - timeDomainChart.axisRightMargin) *
                            modelData / timeDomainChart.xTickCount
                        text: tickValue.toFixed(0)
                        color: "#999999"
                        font.pixelSize: 11
                        anchors.top: parent.bottom
                        anchors.topMargin: 3
                        x: xPos - width / 2
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8
            clip: true

            Label {
                text: "频域波形 (FFT)"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                z: 1
            }

            WaveformItem {
                id: freqDomainChart
                anchors.fill: parent

                xMin: 0
                xMax: 100
                yMin: -2
                yMax: 2
                xTickCount: 5
                yTickCount: 4
                axisLeftMargin: 50
                axisBottomMargin: 30
                lineColor: "#ff6600"

                samples: []   // FFT 数据后续接入

                // Y 轴标签
                Repeater {
                    model: freqDomainChart.yTickCount + 1
                    delegate: Text {
                        property real tickValue: freqDomainChart.yMax -
                            (freqDomainChart.yMax - freqDomainChart.yMin) * modelData / freqDomainChart.yTickCount
                        property real yPos: freqDomainChart.axisTopMargin +
                            (freqDomainChart.height - freqDomainChart.axisTopMargin - freqDomainChart.axisBottomMargin) *
                            modelData / freqDomainChart.yTickCount
                        text: tickValue.toFixed(1)
                        color: "#999999"
                        font.pixelSize: 11
                        anchors.right: parent.left
                        anchors.rightMargin: 3
                        y: yPos - height / 2
                    }
                }

                // X 轴标签
                Repeater {
                    model: freqDomainChart.xTickCount + 1
                    delegate: Text {
                        property real tickValue: freqDomainChart.xMin +
                            (freqDomainChart.xMax - freqDomainChart.xMin) * modelData / freqDomainChart.xTickCount
                        property real xPos: freqDomainChart.axisLeftMargin +
                            (freqDomainChart.width - freqDomainChart.axisLeftMargin - freqDomainChart.axisRightMargin) *
                            modelData / freqDomainChart.xTickCount
                        text: tickValue.toFixed(0)
                        color: "#999999"
                        font.pixelSize: 11
                        anchors.top: parent.bottom
                        anchors.topMargin: 3
                        x: xPos - width / 2
                    }
                }
            }
        }
    }
}
```

### 验证
编译运行，应看到上方时域波形图实时滚动显示绿色正弦波，带有坐标轴和刻度标签。

---

## Step 12: 修改 TcpClient 将数据传递给 WaveformDataSource

### 目标
TCP 收到的原始二进制数据不再只是打印日志，而是传递给 WaveformDataSource 进行解析和显示。

### 要修改的文件
- `App/tcpclient.h`
- `App/tcpclient.cpp`

### tcpclient.h 修改

添加 include：

```cpp
#include "waveformdatasource.h"
```

在 `private:` 区域添加：

```cpp
WaveformDataSource *m_dataSource = nullptr;
```

在 `public:` 区域添加：

```cpp
void setDataSource(WaveformDataSource *dataSource);
```

### tcpclient.cpp 修改

添加 setDataSource 实现：

```cpp
void TcpClient::setDataSource(WaveformDataSource *dataSource)
{
    m_dataSource = dataSource;
}
```

修改 `onReadyRead()`：

```cpp
void TcpClient::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if(!socket){
        qDebug() << "onReadyRead: 未知的QTcpSocket";
        return;
    }

    QByteArray data = socket->readAll();

    if (m_dataSource) {
        m_dataSource->appendData(data, 16, 4);
    }
}
```

### main.cpp 修改

在创建 `TcpClient` 和 `WaveformDataSource` 之后，连接它们：

```cpp
#include "waveformdatasource.h"

// ... 在 engine 创建后 ...

TcpClient *tcpClient = new TcpClient(&engine);

// WaveformDataSource 已通过 QML_ELEMENT + QML_SINGLETON 自动注册
// 需要在 C++ 端获取其实例来连接 TcpClient
// 方法1：通过 QML 引擎获取
// 方法2：直接创建并手动设置

// 推荐方式：创建实例，同时用于 C++ 连接和 QML 注册
WaveformDataSource *dataSource = new WaveformDataSource(&engine);
tcpClient->setDataSource(dataSource);

// 注意：使用 QML_SINGLETON 时，需要通过 qmlRegisterSingletonInstance 注册实例
// 或者移除 QML_SINGLETON 宏，改为手动注册
qmlRegisterSingletonInstance<WaveformDataSource>(
    "WaveformItem", 1, 0, "WaveformDataSource", dataSource);
```

> **注意**：如果使用 `QML_SINGLETON` 宏，Qt 会自动创建单例实例，但 C++ 端无法直接获取该实例来连接 TcpClient。因此这里更推荐手动注册方式，或者移除 `QML_SINGLETON` 宏改用 `QML_ELEMENT` + 手动 `qmlRegisterSingletonInstance`。

### 验证
1. 启动 TCP 服务器发送模拟数据
2. 连接后，时域波形图应实时显示接收到的数据
3. 断开连接后，波形停止更新

---

## Step 13: FFT 频域图（后续扩展）

此步骤留作后续扩展，基本思路：

1. 集成 FFT 库（推荐 KissFFT，轻量级，MIT 许可）
2. 在 `WaveformDataSource` 中添加 FFT 计算方法
3. 每次收到新数据后，对通道数据执行 FFT
4. 暴露 `Q_PROPERTY(QVector<float> channel0Fft ...)` 给 QML
5. 将 FFT 结果绑定到下方 `WaveformItem` 的 `samples`

---

## 架构总览

```
TCP 服务器
    │
    ▼ 原始二进制数据 (6.4 MB/s)
┌──────────────┐
│  TcpClient   │ onReadyRead() → m_dataSource->appendData()
└──────┬───────┘
       │
       ▼ 16通道 × float 数组
┌─────────────────────┐
│ WaveformDataSource  │  QML_ELEMENT + QML_SINGLETON
│  - 环形缓冲区       │ ← 100,000 点/通道
│  - Min-Max 降采样   │ → ~2000 点/通道
│  - channel0 属性    │ → QML 绑定
└──────┬──────────────┘
       │
       ▼ 降采样后的 float 数组
┌─────────────────────┐
│   WaveformItem      │  QML_ELEMENT + qt_add_qml_module
│  - 坐标轴属性       │ ← xMin/xMax/yMin/yMax/xTickCount/yTickCount
│  - 坐标轴绘制       │ ← QSGGeometry (轴线+刻度+网格)
│  - 波形绘制         │ ← QSGGeometry (数据折线)
│  - 响应式缩放       │ ← geometryChange() → 自动重绘
│  - 刻度标签         │ ← QML Repeater + Text
└─────────────────────┘
       │
       ▼
   屏幕显示带坐标轴的折线图
```

## QML 模块注册方式对比

| 特性 | 旧方式（手动注册） | 新方式（QML_ELEMENT） |
|------|---|---|
| 注册代码 | `qmlRegisterType<WaveformItem>(...)` 在 main.cpp | `QML_ELEMENT` 宏 + `qt_add_qml_module()` |
| main.cpp 修改 | 每添加一个类型都要改 | 无需修改 |
| qmldir 生成 | 手动或自动 | 自动生成 |
| qmllint 支持 | 有限 | 完全支持 |
| QML 类型检查 | 运行时 | 编译时 + 运行时 |
| 推荐程度 | Qt 5 兼容方式 | **Qt 6 推荐方式** |

## 性能预估

| 环节 | 数据量 | 耗时估算 |
|------|--------|---------|
| TCP 接收 | 6.4 MB/s | 网络瓶颈 |
| 二进制解析 | 1.6M float/s | < 1ms |
| 环形缓冲写入 | 1.6M float/s | < 1ms |
| Min-Max 降采样 | 100K → 2K | < 1ms |
| 坐标轴几何构建 | ~50 顶点 | < 0.01ms |
| 波形几何构建 | 2K × 2D | < 0.1ms |
| GPU 顶点上传 | ~2K 顶点 | < 0.1ms |
| GPU 渲染 | 2K 线段 + 轴线 | < 0.1ms |
| **总计** | | **< 5ms/帧** |

目标帧率 30 FPS（33ms/帧），5ms 的处理时间绰绰有余。
