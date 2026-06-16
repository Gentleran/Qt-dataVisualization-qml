pragma Singleton
import QtQuick

QtObject {
    id: root

    // === TcpClient Mock ===
    property QtObject tcp: QtObject {
        property string hostAddress: "192.168.1.10"
        property int port: 6666
        property bool connected: false
        property bool waveformDataEnabled: false

        function connectToServer() {
            console.log("[MockTcpClient] 连接到", hostAddress, ":", port)
            connected = true
        }

        function disconnectFromServer() {
            console.log("[MockTcpClient] 断开连接")
            connected = false
        }
    }

    // === WaveChanMgr Mock ===
    property QtObject waveform: QtObject {
        property real xMin: 0.0
        property real xMax: 100.0
        property real yMin: -20.0
        property real yMax: 20.0
        property int xTickCount: 10
        property int yTickCount: 5
        property int xSubTickCount: 1
        property int ySubTickCount: 1
        property var samples: []
        property bool running: true

        function generateData() {
            var data = [];
            var xRange = xMax - xMin;
            for (var i = 0; i < 200; i++) {
                var t = xMin + (xRange * i) / 199;
                data.push(Math.sin(t * 0.1));
            }
            samples = data;
        }
    }
}
