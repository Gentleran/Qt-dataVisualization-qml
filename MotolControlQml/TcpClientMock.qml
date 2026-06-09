pragma Singleton
import QtQuick
import QtQuick.Controls

QtObject {
    id: root

    property string hostAddress: "192.168.1.10"
    property int port: 6666
    property bool connected: false

    function connectToServer() {
        console.log("[MockTcpClient] 连接到", hostAddress, ":", port)
        connected = true
    }

    function disconnectFromServer() {
        console.log("[MockTcpClient] 断开连接")
        connected = false
    }
}
