import QtQuick
import CloudMusic

Rectangle {
    width: 100
    height: 100
    anchors.centerIn: parent
    color: "red"

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            cursorShape = Qt.PointingHandCursor;
            parent.color = "blue";
        }
        onExited: {
            cursorShape = Qt.ArrowCursor;
            parent.color = "red";
        }
        onPressed: parent.color = "yellow";
        onReleased: parent.color = "blue";
    }
}