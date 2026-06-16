pragma Singleton
import QtQuick
import MotolControlQml


QtObject {
    id: root

    // === TcpClient Manager ===
    property var _realTcpClient: (typeof TcpClient !== "undefined") ? TcpClient : null
    property var tcpImpl: _realTcpClient || Mocks.tcp

    function connectToServer() {
        console.log("[TcpClientManager] connectToServer() 被调用")
        tcpImpl.connectToServer()
    }

    function disconnectFromServer() {
        console.log("[TcpClientManager] disconnectFromServer() 被调用")
        tcpImpl.disconnectFromServer()
    }

    // === WaveChanMgr Manager ===
    property var _realWaveformModel: (typeof WaveChanMgr !== "undefined") ? WaveChanMgr : null
    property var waveformImpl: _realWaveformModel || Mocks.waveform
}
