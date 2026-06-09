#include "tcpclient.h"
#include "ConstantConfig.h"

TcpClient::TcpClient(QObject *parent)
    : QObject{parent}
    , m_hostAddress(ConstantConfig::DEFAULT_IP)
{
    addConnectionPort(ConstantConfig::DEFAULT_PORT_1);
    addConnectionPort(ConstantConfig::DEFAULT_PORT_2);
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
    // 检查所有端口是否已连接
    for(SocketConnect *connect : m_socketConnects){
        if(!connect->connected){
            return false;
        }
    }
    // 所有端口都已连接，返回true
    return !m_socketConnects.isEmpty();
}

void TcpClient::connectToServer()
{
    if (isConnected()){
        qDebug() << "所有端口已已连接-无需重复连接";
        return;
    }
    // 连接所有端口
    for(SocketConnect *connect : m_socketConnects){
        connect->socket->connectToHost(m_hostAddress, connect->port);
    }
}

void TcpClient::disconnectFromServer()
{
    if(!isConnected()){
        qDebug() << "所有端口未未连接-无需断开";
        return;
    }
    // 断开所有端口
    for(SocketConnect *connect : m_socketConnects){
        connect->socket->disconnectFromHost();
    }
}

void TcpClient::addConnectionPort(quint16 port)
{
    // 检查端口是否已存在
    for(SocketConnect *connect : m_socketConnects){
        if(port == connect->port){
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

    qDebug() << "端口" << port << "已添加";

}

void TcpClient::onReadyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if(!socket){
        qDebug() << "onReadyRead: 未知的QTcpSocket";
        return;
    }

    QByteArray data = socket->readAll();
    QString message = QString::fromUtf8(data);
    qDebug() << "[TcpClient:" << socket->socketDescriptor() << "] 接收消息:" << message;
}

void TcpClient::onConnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if(!socket){
        qDebug() << "onConnected: 未知的QTcpSocket";
        return;
    }
    // 标记端口为已连接
    for(SocketConnect *connect : m_socketConnects){
        if(connect->socket == socket){
            connect->connected = true;
            updateConnectedStatus();    // 连接状态更新
            qDebug() << "端口" << connect->port << "已成功连接";
            break;
        }
    }
}

void TcpClient::onDisconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if(!socket){
        qDebug() << "onDisconnected: 未知的QTcpSocket";
        return;
    }
    // 标记端口为未连接
    for(SocketConnect *connect : m_socketConnects){
        if(connect->socket == socket){
            connect->connected = false;
            updateConnectedStatus();    // 连接状态更新
            qDebug() << "端口" << connect->port << "已断开连接";
            break;
        }
    }
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    
}

void TcpClient::updateConnectedStatus()
{
    // 检查所有端口是否已连接
    for(SocketConnect *connect : m_socketConnects){
        if(!connect->connected){
            return;
        }
    }
    // 所有端口都已连接，返回true
    emit connectedChanged();
}
