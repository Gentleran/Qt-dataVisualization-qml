pragma Singleton
import QtQuick

QtObject {
    property real xMin: 0.0
    property real xMax: 100.0
    property real yMin: -10.0
    property real yMax: 10.0
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