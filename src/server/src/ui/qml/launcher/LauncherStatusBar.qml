pragma ComponentBehavior: Bound
import QtQuick
import Vicinae

Item {
    readonly property alias popupAnchor: footer
    implicitHeight: 46

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onWheel: wheel => wheel.accepted = true
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 96
        radius: Config.borderRounding
        gradient: Gradient {
            GradientStop {
                position: 0
                color: Config.withAlpha(Theme.background, 0)
            }
            GradientStop {
                position: 0.65
                color: Theme.background
            }
        }
    }

    Footer {
        id: footer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 16
        anchors.rightMargin: 6
        anchors.bottomMargin: 6
        height: 40
    }
}
