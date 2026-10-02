pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import Vicinae

Item {
    RowLayout {
        anchors.fill: parent
        spacing: 12

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 10

            Text {
                visible: !Launcher.toastActive
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                text: Launcher.navigationTitle
                font.family: "Onest"
                font.pixelSize: 13
                color: Config.withAlpha(Theme.foreground, 0.5)
                elide: Text.ElideRight
            }

            FooterToast {
                visible: Launcher.toastActive
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                constrained: true
            }
        }

        Rectangle {
            visible: Launcher.actionPanel.primaryActionTitle !== "" || Launcher.actionPanel.hasMultipleActions
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: actionRow.implicitWidth + 12
            implicitHeight: 39
            radius: height / 2
            color: Config.withAlpha(Theme.foreground, 0.04)
            border.width: 1
            border.color: Config.withAlpha(Theme.foreground, 0.05)

            RowLayout {
                id: actionRow
                anchors.centerIn: parent
                spacing: 0

                FooterButton {
                    id: primaryButton
                    visible: Launcher.actionPanel.primaryActionTitle !== ""
                    label: Launcher.actionPanel.primaryActionTitle
                    shortcutTokens: Launcher.actionPanel.primaryActionShortcutTokens
                    highlighted: true
                    onClicked: Launcher.actionPanel.executePrimaryAction()
                }

                FooterButton {
                    id: actionsButton
                    visible: Launcher.actionPanel.hasMultipleActions
                    label: qsTr("Actions")
                    shortcutTokens: Keybinds.toggleActionPanelTokens
                    highlighted: Launcher.actionPanel.open
                    backgrounded: Launcher.actionPanel.open
                    onClicked: Launcher.actionPanel.toggle(true)
                }
            }
        }
    }
}
