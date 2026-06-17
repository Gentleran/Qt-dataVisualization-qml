#include "wavechanmgr.h"

WaveChanMgr::WaveChanMgr(QObject *parent)
    : QObject(parent)
{
    // connect(&m_timer, &QTimer::timeout, this, &WaveChanMgr::onTimeout);
    // m_timer.setInterval(33);
    // generateData();
    // m_timer.start();
    m_running = false;
}

float WaveChanMgr::xMin() const
{
    return m_xMin;
}

void WaveChanMgr::setXMin(float val)
{
    if (qFuzzyCompare(m_xMin, val))
        return;
    m_xMin = val;
    emit xMinChanged();
}

float WaveChanMgr::xMax() const
{
    return m_xMax;
}

void WaveChanMgr::setXMax(float val)
{
    if (qFuzzyCompare(m_xMax, val))
        return;
    m_xMax = val;
    emit xMaxChanged();
}

float WaveChanMgr::yMin() const
{
    return m_yMin;
}

void WaveChanMgr::setYMin(float val)
{
    if (qFuzzyCompare(m_yMin, val))
        return;
    m_yMin = val;
    emit yMinChanged();
}

float WaveChanMgr::yMax() const
{
    return m_yMax;
}

void WaveChanMgr::setYMax(float val)
{
    if (qFuzzyCompare(m_yMax, val))
        return;
    m_yMax = val;
    emit yMaxChanged();
}

int WaveChanMgr::xTickCount() const
{
    return m_xTickCount;
}

void WaveChanMgr::setXTickCount(int count)
{
    if (m_xTickCount == count)
        return;
    m_xTickCount = count;
    emit xTickCountChanged();
}

int WaveChanMgr::yTickCount() const
{
    return m_yTickCount;
}

void WaveChanMgr::setYTickCount(int count)
{
    if (m_yTickCount == count)
        return;
    m_yTickCount = count;
    emit yTickCountChanged();
}

int WaveChanMgr::xSubTickCount() const
{
    return m_xSubTickCount;
}

void WaveChanMgr::setXSubTickCount(int count)
{
    if (m_xSubTickCount == count)
        return;
    m_xSubTickCount = count;
    emit xSubTickCountChanged();
}

int WaveChanMgr::ySubTickCount() const
{
    return m_ySubTickCount;
}

void WaveChanMgr::setYSubTickCount(int count)
{
    if (m_ySubTickCount == count)
        return;
    m_ySubTickCount = count;
    emit ySubTickCountChanged();
}

bool WaveChanMgr::running() const
{
    return m_running;
}

void WaveChanMgr::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QVector<float> WaveChanMgr::samples() const
{
    return m_samples;
}

void WaveChanMgr::setSamples(const QVector<float> &newSamples)
{
    if (m_samples == newSamples)
        return;
    m_samples = newSamples;
    emit samplesChanged();
}

void WaveChanMgr::onWaveformDataReceived(const QVector<QVector<float>> &channelData)
{
    // if(channelData.size() != 2){
    //     qDebug() << "onWaveformDataReceived: 通道数据数量错误";
    //     return;
    // }
    setSamples(channelData[0]);
    emit samplesChanged();
}

void WaveChanMgr::generateData()
{
    m_samples.resize(200);
    float xRange = m_xMax - m_xMin;
    for (int i = 0; i < 200; i++)
    {
        float t = m_xMin + (xRange * i) / (200-1);
        m_samples[i] = std::sin(t*0.5f + m_timeIndex*0.1f) * (m_yMax - m_yMin)/2*0.8;
    }
    m_timeIndex++;
    emit samplesChanged();

}

void WaveChanMgr::onTimeout()
{
    generateData();
}
