#include "waveformdatamodel.h"

WaveformDataModel::WaveformDataModel(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &WaveformDataModel::onTimeout);
    m_timer.setInterval(33);
    generateData();
    m_timer.start();
    m_running = true;
}

float WaveformDataModel::xMin() const
{
    return m_xMin;
}

void WaveformDataModel::setXMin(float val)
{
    if (qFuzzyCompare(m_xMin, val))
        return;
    m_xMin = val;
    emit xMinChanged();
}

float WaveformDataModel::xMax() const
{
    return m_xMax;
}

void WaveformDataModel::setXMax(float val)
{
    if (qFuzzyCompare(m_xMax, val))
        return;
    m_xMax = val;
    emit xMaxChanged();
}

float WaveformDataModel::yMin() const
{
    return m_yMin;
}

void WaveformDataModel::setYMin(float val)
{
    if (qFuzzyCompare(m_yMin, val))
        return;
    m_yMin = val;
    emit yMinChanged();
}

float WaveformDataModel::yMax() const
{
    return m_yMax;
}

void WaveformDataModel::setYMax(float val)
{
    if (qFuzzyCompare(m_yMax, val))
        return;
    m_yMax = val;
    emit yMaxChanged();
}

int WaveformDataModel::xTickCount() const
{
    return m_xTickCount;
}

void WaveformDataModel::setXTickCount(int count)
{
    if (m_xTickCount == count)
        return;
    m_xTickCount = count;
    emit xTickCountChanged();
}

int WaveformDataModel::yTickCount() const
{
    return m_yTickCount;
}

void WaveformDataModel::setYTickCount(int count)
{
    if (m_yTickCount == count)
        return;
    m_yTickCount = count;
    emit yTickCountChanged();
}

int WaveformDataModel::xSubTickCount() const
{
    return m_xSubTickCount;
}

void WaveformDataModel::setXSubTickCount(int count)
{
    if (m_xSubTickCount == count)
        return;
    m_xSubTickCount = count;
    emit xSubTickCountChanged();
}

int WaveformDataModel::ySubTickCount() const
{
    return m_ySubTickCount;
}

void WaveformDataModel::setYSubTickCount(int count)
{
    if (m_ySubTickCount == count)
        return;
    m_ySubTickCount = count;
    emit ySubTickCountChanged();
}

bool WaveformDataModel::running() const
{
    return m_running;
}

void WaveformDataModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QVector<float> WaveformDataModel::samples() const
{
    return m_samples;
}

void WaveformDataModel::setSamples(const QVector<float> &newSamples)
{
    if (m_samples == newSamples)
        return;
    m_samples = newSamples;
    emit samplesChanged();
}

void WaveformDataModel::generateData()
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

void WaveformDataModel::onTimeout()
{
    generateData();
}
