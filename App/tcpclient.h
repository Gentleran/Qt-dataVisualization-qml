#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QList>

class TcpClient : public QObject
{
    Q_OBJECT    // 使类可被QML访问

    Q_PROPERTY(QString hostAddress READ hostAddress WRITE setHostAddress NOTIFY hostAddressChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool waveformDataEnabled READ waveformDataEnabled WRITE setWaveformDataEnabled NOTIFY waveformDataEnabledChanged)
public:
    explicit TcpClient(QObject *parent = nullptr);       
    ~TcpClient();   



    QString hostAddress() const;                           
    void setHostAddress(const QString &newHostAddress);     

    bool isConnected() const;

    Q_INVOKABLE void connectToServer();        // 连接到服务器
    Q_INVOKABLE void disconnectFromServer();   // 断开与服务器的连接

    void addConnectionPort(quint16 port);

    bool waveformDataEnabled() const;
    void setWaveformDataEnabled(bool newWaveformDataEnabled);

signals:
    void hostAddressChanged();               

    void connectedChanged();

    void waveformDataEnabledChanged();

private slots:
    void onReadyRead();             // 处理可读数据
    void onConnected();             // 处理连接成功
    void onDisconnected();          // 处理断开连接
    void onErrorOccurred(QAbstractSocket::SocketError socketError); // 处理错误发生

private:
    struct SocketConnect{
        QTcpSocket *socket;
        quint16 port;
        bool connected;
    };

    QString m_hostAddress;                  // 服务器IP地址
    QList<SocketConnect*> m_socketConnects; // 连接对象列表
    bool m_isConnected;                     // 是否已连接

    void updateConnectedStatus();

    bool m_waveformDataEnabled;
};

#endif // TCPCLIENT_H
