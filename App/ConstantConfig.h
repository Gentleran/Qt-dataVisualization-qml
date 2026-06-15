#ifndef CONSTANTCONFIG_H
#define CONSTANTCONFIG_H

#include <QString>
#include <QtGlobal>

namespace ConstantConfig {
    // 默认 IP 地址
    const QString DEFAULT_IP = "127.0.0.1";

    // 默认端口号
    const quint16 DEFAULT_PORT_1 = 502;
    const quint16 DEFAULT_PORT_2 = 503;

    // 连接超时时间 (毫秒)
    const quint32 CONNECTION_TIMEOUT_MS = 5000;

    // 波形默认配置
    constexpr float WAVEFORM_X_MIN = 0.0f;
    constexpr float WAVEFORM_X_MAX = 100.0f;
    constexpr float WAVEFORM_Y_MIN = -10.0f;
    constexpr float WAVEFORM_Y_MAX = 10.0f;
    constexpr int WAVEFORM_X_TICK_COUNT = 10;
    constexpr int WAVEFORM_Y_TICK_COUNT = 5;
    constexpr int WAVEFORM_X_SUB_TICK_COUNT = 1;
    constexpr int WAVEFORM_Y_SUB_TICK_COUNT = 1;
}

#endif // CONSTANTCONFIG_H
