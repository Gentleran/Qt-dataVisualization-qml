#include "waveformitem.h"
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>

WaveformItem::WaveformItem(QQuickItem *parent)
    : QQuickItem(parent)
{
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
    update();   // 触发重绘
}

QSGNode *WaveformItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    // ============================================================
    // 这是 Qt Scene Graph 渲染管线的核心
    //
    // 原理：
    // 1. Qt Scene Graph 是一个场景图（树结构），每个节点是一个 QSGNode
    // 2. QSGGeometryNode = 几何体节点（顶点数据 + 材质）
    // 3. QSGGeometry 定义顶点数据（位置、格式）
    // 4. QSGFlatColorMaterial 定义纯色材质
    // 5. Qt 渲染线程会遍历场景图，调用 GPU API 绘制
    //
    // oldNode 参数：
    // - 第一次调用时为 nullptr，需要创建新节点
    // - 后续调用时返回上一次的节点，可以复用（修改几何数据即可）
    // ============================================================

    QSGGeometryNode *node = static_cast<QSGGeometryNode *>(oldNode);

    if (!node) {
        // ---- 第一次：创建节点 ----

        // 创建几何体节点
        node = new QSGGeometryNode;

        // 创建材质（纯色）
        QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
        material->setColor(QColor(0, 255, 0));  // 绿色线条
        node->setMaterial(material);

        // 初始几何体（2个顶点的线段）
        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 2);
        geometry->setLineWidth(2.0f);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);  // 连续线段
        node->setGeometry(geometry);
    }

    // ---- 更新几何数据 ----
    if (m_samplesChanged && !m_samples.isEmpty()) {
        m_samplesChanged = false;

        int count = m_samples.size();
        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), count);
        geometry->setLineWidth(2.0f);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);

        // 将 samples 数据映射为 2D 坐标
        // x: 均匀分布在 [0, width] 范围
        // y: 根据 samples 值映射到 [0, height] 范围
        QSGGeometry::Point2D *vertices = geometry->vertexDataAsPoint2D();
        float xStep = width() / qMax(count - 1, 1);

        // 找到数据范围，用于归一化
        float minVal = *std::min_element(m_samples.begin(), m_samples.end());
        float maxVal = *std::max_element(m_samples.begin(), m_samples.end());
        float range = maxVal - minVal;
        if (range < 1e-6f) range = 1.0f;  // 防止除零

        for (int i = 0; i < count; ++i) {
            vertices[i].x = i * xStep;
            // 归一化到 [0, height]，翻转 Y 轴（屏幕 Y 向下，数据 Y 向上）
            vertices[i].y = height() - ((m_samples[i] - minVal) / range) * height();
        }

        // 替换旧几何体（旧几何体会被自动删除）
        node->setGeometry(geometry);
    } else if (m_samples.isEmpty() && node->geometry()) {
        // 无数据时清空
        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
        node->setGeometry(geometry);
    }

    // 标记节点需要更新
    node->markDirty(QSGNode::DirtyGeometry);

    return node;
}
