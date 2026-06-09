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

}

#endif // CONSTANTCONFIG_H
