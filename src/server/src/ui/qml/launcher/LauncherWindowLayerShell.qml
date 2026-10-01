pragma ComponentBehavior: Bound
import QtQuick
import org.kde.layershell as LayerShell
import Vicinae

LauncherWindow {
    id: root
    shadowPadding: Config.shadowSize
    width: Screen.width
    height: Screen.height
    panelX: Math.round((width - root._w) / 2)
    panelY: root.isRootScreen ? Math.max(shadowPadding, panelTop) : Math.round((height - expandedHeight) / 2) + shadowPadding
    LayerShell.Window.exclusionZone: -1

    LayerShell.Window.anchors: LayerShell.Window.AnchorTop | LayerShell.Window.AnchorBottom | LayerShell.Window.AnchorLeft | LayerShell.Window.AnchorRight
    LayerShell.Window.scope: "vicinae"
    LayerShell.Window.wantsToBeOnActiveScreen: true
    LayerShell.Window.layer: Launcher.lsLayer
    LayerShell.Window.keyboardInteractivity: Launcher.lsKeyboardInteractivity

    MouseArea {
        anchors.fill: parent
        z: -1
        acceptedButtons: Qt.AllButtons
        onClicked: mouse => {
            if (mouse.x < root.panelX || mouse.x >= root.panelX + root._w || mouse.y < root.panelY || mouse.y >= root.panelY + root._contentH)
                Launcher.nav.closeWindow();
        }
    }
}
