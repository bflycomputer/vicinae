pragma ComponentBehavior: Bound
import QtQuick
import Vicinae

Item {
    id: root

    required property string label
    required property var shortcutTokens
    property bool highlighted: false
    property bool backgrounded: false

    signal clicked

    Accessible.role: Accessible.Button
    Accessible.name: root.label
    Accessible.onPressAction: root.clicked()

    readonly property bool hovered: mouseArea.containsMouse
    readonly property int horizontalPadding: 8
    readonly property int buttonHeight: 37

    implicitWidth: row.implicitWidth + 2 * horizontalPadding
    implicitHeight: buttonHeight

    Rectangle {
        anchors.fill: parent
        visible: root.hovered || root.backgrounded
        radius: height / 2
        color: Config.withAlpha(Theme.foreground, 0.08)
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 6

        Text {
            text: root.label
            color: root.highlighted ? Theme.foreground : Theme.textMuted
            font.family: "Onest"
            font.pixelSize: 13
            anchors.verticalCenter: parent.verticalCenter
        }

        ShortcutBadge {
            visible: root.shortcutTokens && root.shortcutTokens.length > 0
            tokens: root.shortcutTokens
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
