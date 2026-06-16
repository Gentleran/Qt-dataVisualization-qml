#ifndef WAVECHANMGR_H
#define WAVECHANMGR_H

#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QVector>
#include "ConstantConfig.h"

class WaveChanMgr : public QObject
{
    Q_OBJECT


    Q_PROPERTY(float xMin READ xMin WRITE setXMin NOTIFY xMinChanged)   // X轴最小值
    Q_PROPERTY(float xMax READ xMax WRITE setXMax NOTIFY xMaxChanged)   // X轴最大值
    Q_PROPERTY(float yMin READ yMin WRITE setYMin NOTIFY yMinChanged)   // Y轴最小值
    Q_PROPERTY(float yMax READ yMax WRITE setYMax NOTIFY yMaxChanged)   // Y轴最大值
    Q_PROPERTY(int xTickCount READ xTickCount WRITE setXTickCount NOTIFY xTickCountChanged)   // X轴刻度数量
    Q_PROPERTY(int yTickCount READ yTickCount WRITE setYTickCount NOTIFY yTickCountChanged)   // Y轴刻度数量
    Q_PROPERTY(int xSubTickCount READ xSubTickCount WRITE setXSubTickCount NOTIFY xSubTickCountChanged)   // X轴子刻度数量
    Q_PROPERTY(int ySubTickCount READ ySubTickCount WRITE setYSubTickCount NOTIFY ySubTickCountChanged)   // Y轴子刻度数量

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)   // 是否正在运行
    Q_PROPERTY(QVector<float> samples READ samples WRITE setSamples NOTIFY samplesChanged)   // 样本数据

public:
    explicit WaveChanMgr(QObject *parent = nullptr);

    float xMin() const;
    void setXMin(float val);

    float xMax() const;
    void setXMax(float val);

    float yMin() const;
    void setYMin(float val);

    float yMax() const;
    void setYMax(float val);

    int xTickCount() const;
    void setXTickCount(int count);

    int yTickCount() const;
    void setYTickCount(int count);

    int xSubTickCount() const;
    void setXSubTickCount(int count);

    int ySubTickCount() const;
    void setYSubTickCount(int count);

    bool running() const;
    void setRunning(bool newRunning);

    QVector<float> samples() const;
    void setSamples(const QVector<float> &newSamples);

    Q_INVOKABLE void generateData();

signals:
    void xMinChanged();
    void xMaxChanged();
    void yMinChanged();
    void yMaxChanged();
    void xTickCountChanged();
    void yTickCountChanged();
    void xSubTickCountChanged();
    void ySubTickCountChanged();

    void runningChanged();

    void samplesChanged();

private slots:
    void onTimeout();

private:
    float m_xMin = ConstantConfig::WAVEFORM_X_MIN;
    float m_xMax = ConstantConfig::WAVEFORM_X_MAX;
    float m_yMin = ConstantConfig::WAVEFORM_Y_MIN;
    float m_yMax = ConstantConfig::WAVEFORM_Y_MAX;
    int m_xTickCount = ConstantConfig::WAVEFORM_X_TICK_COUNT;
    int m_yTickCount = ConstantConfig::WAVEFORM_Y_TICK_COUNT;
    int m_xSubTickCount = ConstantConfig::WAVEFORM_X_SUB_TICK_COUNT;
    int m_ySubTickCount = ConstantConfig::WAVEFORM_Y_SUB_TICK_COUNT;
    bool m_running = false;
    QVector<float> m_samples;

    int m_timeIndex = 0;
    QTimer m_timer;

};

#endif // WAVECHANMGR_H
