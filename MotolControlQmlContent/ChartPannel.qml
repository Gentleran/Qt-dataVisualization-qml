import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveformItem

Item {
    id: root

    ColumnLayout{
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Rectangle{
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
            }

            WaveformItem {
                id: timeDomainChart
                anchors.fill: parent
                anchors.margins: 30  // 留出坐标轴空间

                // 测试数据：正弦波
                samples: {
                    var data = [];
                    for (var i = 0; i < 200; i++) {
                        data.push(Math.sin(i * 0.1) * 100);
                    }
                    return data;
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
            }

            // 后续将替换为 WaveformItem
        }
}
}
