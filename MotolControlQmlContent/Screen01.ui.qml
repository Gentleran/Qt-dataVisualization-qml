

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MotolControlQml

Rectangle {
    id: rectangle
    width: Constants.width
    height: Constants.height

    color: Constants.backgroundColor

    RowLayout {
        anchors.fill: parent
        spacing: 0
        LeftPanel {
            id: leftPanel
            Layout.fillHeight: true
            Layout.preferredWidth: 400

            Layout.margins: 10
        }
        ChartPannel {
            Layout.fillHeight: true
            Layout.fillWidth: true
            // color: '#238a8a'
        }
    }
}
