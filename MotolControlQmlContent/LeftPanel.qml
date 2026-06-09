

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: background

    implicitWidth: 300
    implicitHeight: 700

    radius: 10

    // 分割线组件
    component SectionHeader: Item {
        property string titleText: "标题"
        Layout.preferredWidth: parent ? parent.width : 300
        Layout.preferredHeight: 40
        RowLayout {
            anchors.fill: parent
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 2
                Layout.alignment: Qt.AlignVCenter
                width: 2
                color: "gray"
                radius: 1
            }
            Label {
                text: titleText
                font.pixelSize: 18
                font.bold: true
                Layout.alignment: Qt.AlignLeft
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 2
                Layout.alignment: Qt.AlignVCenter
                width: 2
                color: "gray"
                radius: 1
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15

        // --- 第一部分：标题 ---
        SectionHeader {
            Layout.fillWidth: true
            titleText: "IP设置"
        }
        RowLayout {

            anchors.leftMargin: 15 // 内部左边距
            anchors.rightMargin: 15 // 内部右边距
            spacing: 10

            // 左侧文本
            Label {
                text: "连接状态"
                font.pixelSize: 26

                color: "#333333"
                Layout.alignment: Qt.AlignVCenter // 垂直居中
            }

            Item {
                Layout.fillWidth: true // 占据中间剩余空间，把开关推到最右边
            }

            // 右侧开关
            SwitchMy {
                Layout.alignment: Qt.AlignVCenter // 垂直居中
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
