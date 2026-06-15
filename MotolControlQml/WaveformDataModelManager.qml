pragma Singleton
import QtQuick
import MotolControlQml

QtObject {
    property var _realModel: (typeof WaveformDataModel !== "undefined") ? WaveformDataModel : null
    property var impl: _realModel || WaveformDataModelMock
}
