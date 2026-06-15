#include "tcpclient.h"
#include "ConstantConfig.h"

TcpClient::TcpClient(QObject *parent)
    : QObject{parent}
    , m_hostAddress(ConstantConfig::DEFAULT_IP)
    , m_isConnected(false)
    , m_waveformDataEnabled(false)
{
    addConnectionPort(ConstantConfig::DEFAULT_PORT_1);
    addConnectionPort(ConstantConfig::DEFAULT_PORT_2);
}

TcpClient::~TcpClient()
{
    for(const SocketConnect *conn : std::as_const(m_socketConnects)){
        if(conn->socket){
            conn->socket->disconnectFromHost();
            conn->socket->deleteLater();
        }
    }
    m_socketConnects.clear();
}

QString TcpClient::hostAddress() const
{
    qDebug() << "hostAddress" << m_hostAddress;
    return m_hostAddress;
}

void TcpClient::setHostAddress(const QString &newHostAddress)
{
    if (m_hostAddress == newHostAddress)
        return;
    qDebug() << "setHostAddress" << newHostAddress;
    m_hostAddress = newHostAddress;
    emit hostAddressChanged();
}

bool TcpClient::isConnected() const
{
    return m_isConnected;
}

void TcpClient::connectToServer()
{
    // 连接所有端口
    for(const SocketConnect *conn : std::as_const(m_socketConnects)){
        if(conn->socket->state() == QAbstractSocket::UnconnectedState){
            conn->socket->connectToHost(m_hostAddress, conn->port);
        }
    }
}

void TcpClient::disconnectFromServer()
{
    // 断开所有端口
    for(const SocketConnect *conn : std::as_const(m_socketConnects)){
        if(conn->socket->state() == QAbstractSocket::ConnectedState){
            conn->socket->disconnectFromHost();
        }
    }
}

void TcpClient::addConnectionPort(quint16 port)
{
    // 检查端口是否已存在
    for(const SocketConnect *conn : std::as_const(m_socketConnects)){
        if(port == conn->port){
            qDebug() << "端口" << port << "已存在-无需添加";
            return;
        }
    }

    // 创建新的连接对象
    SocketConnect *cnt = new SocketConnect{new QTcpSocket(this), port, false};
    m_socketConnects.append(cnt);

    // 连接信号槽
    connect(cnt->socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(cnt->socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(cnt->socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(cnt->socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &TcpClient::onErrorOccurred);

    qDebug() << "端口" << port << "已添加";

}

void TcpClient::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if(!socket){
        qDebug() << "onReadyRead: 未知的QTcpSocket";
        return;
    }

    quint16 sourcePort;
    for(const SocketConnect *conn : std::as_const(m_socketConnects)){
        if(conn->socket == socket){
            sourcePort = conn->port;
            break;
        }
    }

    if(!m_waveformDataEnabled && ConstantConfig::DEFAULT_PORT_2 == sourcePort){
        QByteArray data = socket->readAll();
        qDebug() << "[TcpClient] 波形接收已关闭，丢弃端口503数据:" << data.size() << "bytes";
        return;
    }

}

void TcpClient::onConnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if(!socket){
        qDebug() << "onConnected: 未知的QTcpSocket";
        return;
    }
    // 标记端口为已连接
    for(SocketConnect *conn : std::as_const(m_socketConnects)){
        if(conn->socket == socket){
            conn->connected = true;
            qDebug() << "端口" << conn->port << "已成功连接";
            break;
        }
    }
    updateConnectedStatus();    // 连接状态更新
}

void TcpClient::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if(!socket){
        qDebug() << "onDisconnected: 未知的QTcpSocket";
        return;
    }
    // 标记端口为未连接
    for(SocketConnect *conn : std::as_const(m_socketConnects)){
        if(conn->socket == socket){
            conn->connected = false;
            qDebug() << "端口" << conn->port << "已断开连接";
            break;
        }
    }
    updateConnectedStatus();    // 连接状态更新

}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        qDebug() << "onErrorOccurred: 未知的QTcpSocket";
        return;
    }

    QString errorMsg = socket->errorString();
    qDebug() << "[TcpClient] Socket error:" << errorMsg;

    for(SocketConnect *conn : std::as_const(m_socketConnects)) {
        if (conn->socket == socket) {
            conn->connected = false;
            emit connectedChanged();
            qDebug() << "端口" << conn->port << "发生错误:" << errorMsg;
            break;
        }
    }
    updateConnectedStatus();

}

void TcpClient::updateConnectedStatus()
{
    bool allConnected = !m_socketConnects.isEmpty();
    for(SocketConnect *conn : std::as_const(m_socketConnects)){
        if (!conn->connected) {
            allConnected = false;
            break;
        }
    }

    if (allConnected != m_isConnected) {
        m_isConnected = allConnected;
        emit connectedChanged();
        qDebug() << "连接状态更新:" << m_isConnected;
    }

}

bool TcpClient::waveformDataEnabled() const
{
    return m_waveformDataEnabled;
}

void TcpClient::setWaveformDataEnabled(bool newWaveformDataEnabled)
{
    if (m_waveformDataEnabled == newWaveformDataEnabled)
        return;
    m_waveformDataEnabled = newWaveformDataEnabled;
    qDebug() << "波形接收已设置为:" << newWaveformDataEnabled;
    emit waveformDataEnabledChanged();
}
