pragma ComponentBehavior: Bound
import org.kde.layershell as LayerShell
import Vicinae

LauncherWindow {
    id: root
    shadowPadding: Config.shadowSize
    LayerShell.Window.exclusionZone: -1
    LayerShell.Window.margins.top: root.isRootScreen ? Math.max(0, root.panelTop - root.shadowPadding) : 0

    LayerShell.Window.anchors: root.isRootScreen ? LayerShell.Window.AnchorTop : LayerShell.Window.AnchorNone
    LayerShell.Window.scope: "vicinae"
    LayerShell.Window.wantsToBeOnActiveScreen: true
    LayerShell.Window.layer: Launcher.lsLayer
    LayerShell.Window.keyboardInteractivity: Launcher.lsKeyboardInteractivity
}
