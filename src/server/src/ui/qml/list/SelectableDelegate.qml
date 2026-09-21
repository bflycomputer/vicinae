pragma ComponentBehavior: Bound
import QtQuick
import Vicinae

/// Reusable delegate base for list items. Provides a rounded-rect
/// background that highlights on selection/hover, a MouseArea
/// for click handling, and a content slot for view-specific layouts.
Item {
    id: root

    property bool selected: false
    property bool draggable: false
    readonly property LauncherAppearance appearance: (root.Window.window as LauncherWindow)?.appearance ?? fallbackAppearance
    readonly property bool hovered: mouseArea.containsMouse && HoverActivation.active

    LauncherAppearance {
        id: fallbackAppearance
    }

    default property alias contentData: contentItem.data

    signal clicked
    signal activated
    signal dragRequested(var source)

    DraggableMouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        draggable: root.draggable
        onItemClicked: {
            root.clicked();
            if (Config.activateOnSingleClick)
                root.activated();
        }
        onItemActivated: root.activated()
        onDragRequested: root.dragRequested(root)
    }

    Rectangle {
        anchors.fill: parent
        visible: root.selected || root.hovered
        radius: 12
        color: Qt.rgba(1, 1, 1, 0.06)
    }

    Item {
        id: contentItem
        anchors.fill: parent
    }
}
