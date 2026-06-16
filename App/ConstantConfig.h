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

    // ===== 波形数据协议常量 (503 端口) =====
    // 每包时间点数 (samples_per_packet)
    constexpr int WAVEFORM_SAMPLES_PER_PACKET = 20;
    // 通道数 (num_channels)
    constexpr int WAVEFORM_NUM_CHANNELS = 16;
    // 使用通道数 (used_num_channels)
    constexpr int WAVEFORM_USERD_NUM_CHANNELS = 12;
    // 每个 float32 字节数
    constexpr int WAVEFORM_FLOAT_SIZE = 4;
    // 每包总浮点点数 = 时间点数 × 通道数
    constexpr int WAVEFORM_FLOATS_PER_PACKET = WAVEFORM_SAMPLES_PER_PACKET * WAVEFORM_NUM_CHANNELS;
    // 每包字节大小 = 浮点点数 × 每个 float32 字节数
    constexpr int WAVEFORM_PACKET_SIZE_BYTES = WAVEFORM_FLOATS_PER_PACKET * WAVEFORM_FLOAT_SIZE;

}

#endif // CONSTANTCONFIG_H
