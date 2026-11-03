pragma ComponentBehavior: Bound
import QtQuick
import Vicinae

GenericListView {
    id: searchListView
    required property var cmdModel
    required property var host
    // Rest and selected tile fills. Flat tints, the card gradient shows through them.
    readonly property var tileFills: ({
            "pond-hazel": [Qt.alpha("#FFF0C5", 0.08), Qt.alpha("#F7E3AA", 0.2)],
            "pond-wisteria": [Qt.alpha("#F0D7FF", 0.1), Qt.alpha("#F0D7FF", 0.2)],
            "pond-north": [Qt.alpha("#E0F1FB", 0.08), Qt.alpha("#E0F1FB", 0.14)]
        })[Theme.id] ?? [Qt.rgba(1, 1, 1, 0.06), Qt.rgba(1, 1, 1, 0.10)]
    readonly property bool pondTheme: Theme.id.startsWith("pond-")
    // Enter launches the first tile before any arrow key, so Pond lights it until the pointer is on the tiles.
    readonly property int litTile: pondTheme && host.recentIndex < 0 && !tileHover.hovered ? 0 : host.recentIndex
    showResults: !host.queryEmpty
    suppressEmpty: host.queryEmpty || Launcher.isLoading

    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        y: 16
        spacing: 16
        visible: searchListView.host.queryEmpty
        HoverHandler {
            id: tileHover
        }
        Repeater {
            model: searchListView.host.recentApps
            delegate: Rectangle {
                id: recentCard
                required property var modelData
                required property int index
                readonly property bool highlighted: pointer.containsMouse || searchListView.litTile === index
                width: (parent.width - 32) / 3
                height: 144
                radius: 10
                color: searchListView.tileFills[highlighted ? 1 : 0]
                ViciImage {
                    x: 20
                    y: 20
                    width: 20
                    height: 20
                    source: recentCard.modelData.icon
                }
                Text {
                    x: 20
                    y: 102
                    width: parent.width - 40
                    height: 28
                    text: recentCard.modelData.title
                    font.family: "Pond Gramercy"
                    font.styleName: "Book"
                    font.pixelSize: 20
                    verticalAlignment: Text.AlignVCenter
                    color: Theme.foreground
                    opacity: searchListView.pondTheme ? 0.8 : recentCard.highlighted ? 1 : 0.6
                    elide: Text.ElideRight
                }
                MouseArea {
                    id: pointer
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: searchListView.host.launchRecent(recentCard.index)
                }
            }
        }
    }
    model: cmdModel
    listModel: cmdModel
    autoWireModel: true
    selectFirstOnReset: cmdModel.selectFirstOnReset
    topMargin: 16
    implicitHeight: Math.max(170, contentHeight + topMargin)

    delegate: Loader {
        id: delegateLoader
        width: ListView.view.width

        required property int index
        required property bool isSelectable
        required property string itemType
        required property string title
        required property string subtitle
        required property string iconSource
        required property string alias
        required property var shortcutTokens
        required property bool isActive
        required property string accessoryText
        required property string accessoryColor
        required property bool isCalculator
        required property string calcQuestion
        required property string calcQuestionUnit
        required property string calcAnswer
        required property string calcAnswerUnit
        required property bool isFile
        required property bool isDraggable

        sourceComponent: isCalculator ? calculatorComponent : itemComponent

        Component {
            id: calculatorComponent
            CalculatorResultDelegate {
                width: delegateLoader.width
                calcQuestion: delegateLoader.calcQuestion
                calcQuestionUnit: delegateLoader.calcQuestionUnit
                calcAnswer: delegateLoader.calcAnswer
                calcAnswerUnit: delegateLoader.calcAnswerUnit
                selected: searchListView.currentIndex === delegateLoader.index
                onClicked: searchListView.currentIndex = delegateLoader.index
                onActivated: searchListView.itemActivated(delegateLoader.index)
            }
        }

        Component {
            id: itemComponent
            ListItemDelegate {
                width: delegateLoader.width
                height: 48
                itemTitle: delegateLoader.title
                itemSubtitle: delegateLoader.subtitle
                itemIconSource: delegateLoader.iconSource
                itemAlias: delegateLoader.alias
                itemShortcutTokens: delegateLoader.shortcutTokens
                itemIsActive: delegateLoader.isActive
                itemAccessory: delegateLoader.accessoryText
                itemAccessoryColor: delegateLoader.accessoryColor
                selected: searchListView.currentIndex === delegateLoader.index
                draggable: delegateLoader.isDraggable
                onClicked: searchListView.currentIndex = delegateLoader.index
                onActivated: searchListView.itemActivated(delegateLoader.index)
                onDragRequested: function (source) {
                    searchListView.currentIndex = delegateLoader.index;
                    searchListView.cmdModel.startDrag(delegateLoader.index, source);
                }
            }
        }
    }
}
