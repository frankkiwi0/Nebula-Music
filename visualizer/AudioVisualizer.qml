import QtQuick

Row {
    id: visualizerRoot
    property var levels: [] 
    spacing: 2
    height: 30
    anchors.bottom: parent.bottom

    Repeater {
        model: 32 
        Rectangle {
            width: (visualizerRoot.width / 32) - 2
           
            height: (visualizerRoot.levels.length > index) ? (visualizerRoot.levels[index] * visualizerRoot.height) + 2 : 2
            color: Qt.rgba(1, 1, 1, 0.6)
            radius: 2
            anchors.bottom: parent.bottom

            Behavior on height {
                NumberAnimation { duration: 80; easing.type: Easing.OutQuad }
            }
        }
    }
}