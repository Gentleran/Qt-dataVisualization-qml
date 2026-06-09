pragma Singleton
import QtQuick
import MotolControlQml

QtObject {
    property var _realClient: (typeof TcpClient !== "undefined" ) ? TcpClient : null
    property var impl:        _realClient || TcpClientMock

    function connectToServer() {
        console.log("[TcpClientManager] connectToServer() 被调用")
        impl.connectToServer()
    }

    function disconnectFromServer() {
        console.log("[TcpClientManager] disconnectFromServer() 被调用")
        impl.disconnectFromServer()
    }

}
