

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls

Switch {
    id: control

    // 组件缩放比例
    property real scaleFactor: 1.3
    // 圆边距
    property real circleMargin: 2 * scaleFactor
    // 选中颜色
    property color checkedColor: "#4cd964"
    // 未选中颜色
    property color uncheckedColor: "#e5e5ea"
    // 动画时长
    property int animationDuration: 135

    implicitWidth: 70   * scaleFactor
    implicitHeight: 31.5  * scaleFactor

    indicator: Rectangle {
        id: indicator

        width: control.width
        height: control.height
        radius: height / 2

        color: control.checked ? control.checkedColor : control.uncheckedColor

        Rectangle {
            id: circle
            width: indicator.height - circleMargin * 2
            height: width
            radius: width / 2
            x: control.checked ? indicator.width - width - circleMargin : circleMargin

            anchors.verticalCenter: parent.verticalCenter 

            color: "white"

            Behavior on x {
                PropertyAnimation {
                    duration: control.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }

        }


    }

}
