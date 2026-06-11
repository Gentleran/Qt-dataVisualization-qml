#ifndef WAVEFORMITEM_H
#define WAVEFORMITEM_H

#include <QQuickItem>
#include <QVector>

class WaveformItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVector<float> samples READ samples WRITE setSamples NOTIFY samplesChanged)
public:
    explicit WaveformItem(QQuickItem *parent = nullptr);

    QVector<float> samples() const;
    void setSamples(const QVector<float> &newSamples);

signals:

    void samplesChanged();


private:
    QVector<float> m_samples;
    bool m_samplesChanged = false;  // 标记数据是否变化，避免每帧都重建几何

    // QQuickItem interface
protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
};

#endif // WAVEFORMITEM_H
