pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Vicinae

Item {
    id: root
    implicitWidth: filtered ? selectedRow.implicitWidth + 24 : 40
    implicitHeight: 40
    property Item clipboardContent
    readonly property ClipboardHistoryViewHost host: Launcher.commandViewHost as ClipboardHistoryViewHost
    readonly property bool filtered: (host?.currentKindFilter ?? 0) !== 0
    readonly property var currentItem: host?.kindFilterModel.itemDataById(host.currentKindFilter.toString()) ?? null
    signal popupClosed

    function open() {
        menu.open();
    }

    Binding {
        target: root.clipboardContent
        property: "opacity"
        value: menu.visible ? 0.3 : 1
        when: root.clipboardContent !== null
    }

    Button {
        id: trigger
        anchors.fill: parent
        Accessible.name: qsTr("Filter clipboard")
        onClicked: root.open()
        background: Rectangle {
            radius: 12
            color: root.filtered && (trigger.hovered || trigger.activeFocus) ? Qt.alpha(Theme.foreground, 0.06) : "transparent"
        }
        contentItem: Item {
            Image {
                anchors.fill: parent
                visible: !root.filtered
                source: trigger.hovered || trigger.activeFocus ? "qrc:/icons/clipboard-filter-hover.svg" : "qrc:/icons/clipboard-filter.svg"
            }
            RowLayout {
                id: selectedRow
                anchors.centerIn: parent
                visible: root.filtered
                spacing: 8
                ViciImage {
                    source: root.currentItem?.iconSource ?? ""
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                }
                Text {
                    text: root.currentItem?.displayName ?? ""
                    color: Theme.foreground
                    font.family: "Onest"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                }
            }
        }
        padding: 0
    }

    ViciPopover {
        id: menu
        x: root.width - width + (root.filtered ? 8 : 0)
        y: 0
        width: 165
        height: 217
        padding: 8.5
        nativeWindow: false
        modal: true
        dim: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        onOpened: {
            choices.currentIndex = root.host.kindFilterModel.indexOfItemId(root.host.currentKindFilter.toString());
            choices.forceActiveFocus();
        }
        onClosed: root.popupClosed()
        background: Rectangle {
            color: Qt.tint(Theme.background, Qt.alpha(Theme.foreground, 0.02))
            radius: 12
            border.width: 0.5
            border.color: Qt.alpha(Theme.foreground, 0.15)
            RectangularShadow {
                z: -1
                y: 16
                width: parent.width
                height: parent.height
                radius: 12
                blur: 40
                color: Qt.rgba(0, 0, 0, 0.46)
            }
        }
        contentItem: ListView {
            id: choices
            model: root.host?.kindFilterModel ?? null
            interactive: false
            keyNavigationEnabled: true
            keyNavigationWraps: true
            spacing: 0
            Keys.onReturnPressed: choose(currentIndex)
            Keys.onEnterPressed: choose(currentIndex)
            Keys.onSpacePressed: choose(currentIndex)

            function choose(index: int) {
                const item = root.host.kindFilterModel.itemDataAt(index);
                root.host.setKindFilter(parseInt(item.id));
                menu.close();
            }

            delegate: ItemDelegate {
                id: option
                required property int index
                required property string title
                required property string iconSource
                width: choices.width
                height: 40
                padding: 12
                highlighted: choices.currentIndex === index
                onHoveredChanged: if (hovered)
                    choices.currentIndex = index
                onClicked: choices.choose(index)
                Accessible.name: title
                background: Rectangle {
                    radius: 12
                    color: option.highlighted ? Qt.alpha(Theme.foreground, 0.06) : "transparent"
                }
                contentItem: RowLayout {
                    spacing: 8
                    ViciImage {
                        source: option.iconSource
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                    }
                    Text {
                        text: option.title
                        color: Theme.foreground
                        font.family: "Onest"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
