import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs
import MotolControlQml

Item {
    id: root

    Connections {
        target: WaveformDataModelManager.impl
        function onSamplesChanged() {
            updateChart();
        }
    }

    // Component.onCompleted: {
    //     updateChart();
    // }

    function updateChart() {
        timeSeries.clear();
        var impl = WaveformDataModelManager.impl;
        var samples = impl.samples;
        var count = samples.length;
        if (count === 0) return;

        var xMin = impl.xMin;
        var xMax = impl.xMax;
        var range = xMax - xMin;
        for (var i = 0; i < count; i++) {
            timeSeries.append(xMin + range * i / (count - 1), samples[i]);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8

            Label {
                text: "时域波形"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                z: 1
            }

            GraphsView {
                id: timeDomainChart
                anchors.fill: parent
                anchors.margins: 10
                anchors.topMargin: 40

                axisX: ValueAxis {
                    id: xAxis
                    min: WaveformDataModelManager.impl.xMin
                    max: WaveformDataModelManager.impl.xMax
                    tickInterval: WaveformDataModelManager.impl.xTickCount
                    subTickCount: WaveformDataModelManager.impl.xSubTickCount
                }

                axisY: ValueAxis {
                    id: yAxis
                    min: WaveformDataModelManager.impl.yMin
                    max: WaveformDataModelManager.impl.yMax
                    tickInterval: WaveformDataModelManager.impl.yTickCount
                    subTickCount: WaveformDataModelManager.impl.ySubTickCount
                }

                LineSeries {
                    id: timeSeries
                    color: "#00ff00"
                    width: 2
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1e1e2e"
            radius: 8

            Label {
                text: "频域波形"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                z: 1
            }
        }
    }

}
