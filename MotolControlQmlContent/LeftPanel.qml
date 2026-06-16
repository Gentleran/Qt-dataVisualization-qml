

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import MotolControlQml

Rectangle {
    id: background

    implicitWidth: 400
    implicitHeight: 700

    radius: 10

    FolderDialog {
        id: folderDialog
        title: "选择录波存放文件夹"
        currentFolder: savePathText.text
        onAccepted:{
            var path = selectedFolder.toString()
            if(path.startsWith("file:///")) {
                path = path.substring(8)
            }
            else if(path.startsWith("file://")) {
                path = path.replace("file://", "")
            }
            path = path.replace(/\\/g, "\\")

            savePathText.text = path
        }
        
    }

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
        spacing: 15
        // --- 第一部分：标题 ---
        SectionHeader {
            Layout.fillWidth: true
            titleText: "IP设置"
        }

        GridLayout {
            Layout.leftMargin: 8 // 内部左边距
            Layout.rightMargin: 8 // 内部右边距
            Layout.bottomMargin: 10
            Layout.fillWidth: true

            columns: 2
            rowSpacing: 15
            columnSpacing: 10


            // 左侧文本
            Label {
                text: "连接状态"
                font.pixelSize: 26

                color: "#333333"
                Layout.alignment: Qt.AlignVCenter // 垂直居中
            }

            // 右侧开关
            SwitchMy {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                checked: Managers.tcpImpl.connected
                onCheckedChanged: {
                    if(checked){
                        Managers.connectToServer()
                    } else if(!checked){
                        Managers.disconnectFromServer()
                    }
                }

            }

            Label {
                text: "IP地址:"
                font.pixelSize: 26
                color: "#333333"
                Layout.alignment: Qt.AlignVCenter
            }

            TextField {
                id: ipAddressField
                placeholderText: "请输入IP地址"
                text: Managers.tcpImpl.hostAddress
                onTextChanged: {
                    Managers.tcpImpl.hostAddress = text
                }
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                font.pixelSize: 25
                background: Rectangle {
                    color: "white"
                    border.color: "gray"
                    border.width: 3
                    radius: 8
                }
            }
        }

        SectionHeader {
            Layout.fillWidth: true
            titleText: "波形"
        }

        GridLayout{
            Layout.leftMargin: 8 // 内部左边距
            Layout.rightMargin: 8 // 内部右边距
            Layout.bottomMargin: 10
            Layout.fillWidth: true

            columns: 2

            Label {
                text: "波形接收"
                font.pixelSize: 26
                color: "#333333"
                Layout.alignment: Qt.AlignVCenter // 垂直居中
                Layout.fillWidth: true
            }

            SwitchMy {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                checked: Managers.tcpImpl.waveformDataEnabled
                onCheckedChanged: {
                    Managers.tcpImpl.waveformDataEnabled = checked
                }

            }
        }

        SectionHeader {
            Layout.fillWidth: true
            titleText: "录波"
        }

        GridLayout{
            Layout.leftMargin: 8 // 内部左边距
            Layout.rightMargin: 8 // 内部右边距
            Layout.bottomMargin: 10
            Layout.fillWidth: true

            columns: 2
            rowSpacing: 15
            columnSpacing: 10


            Label {
                text: "录波状态"
                font.pixelSize: 26
                color: "#333333"
                Layout.alignment: Qt.AlignVCenter // 垂直居中
                Layout.fillWidth: true
            }

            SwitchMy {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                checked: false
            }

            Label {
                text: "保存路径:"
                font.pixelSize: 26
                color: "#333333"
                Layout.alignment: Qt.AlignVCenter // 垂直居中
                Layout.fillWidth: true
            }

            Button{
                id: browseButton
                text: "浏览"
                Layout.preferredWidth: 100
                Layout.preferredHeight: 50
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                font.pixelSize: 20
                background: Rectangle {
                    color: parent.hovered ? "#5a9cf0" : "#4a8fe0"
                    radius: 8
                }
                onClicked:{
                    folderDialog.open()
                }
            }

            TextField {
                id: savePathText
                text: ""
                font.pixelSize: 20
                Layout.preferredHeight: 50
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.columnSpan: 2
                Layout.topMargin: -8
                background: Rectangle {
                    color: "white"
                    border.color: "gray"
                    border.width: 3
                    radius: 8
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
