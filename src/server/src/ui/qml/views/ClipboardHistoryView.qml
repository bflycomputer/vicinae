pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import Vicinae

Item {
    id: root
    required property ClipboardHistoryViewHost host

    function moveUp() {
        return listView.moveUp();
    }
    function moveDown() {
        return listView.moveDown();
    }
    function moveSectionUp() {
        return listView.moveSectionUp();
    }
    function moveSectionDown() {
        return listView.moveSectionDown();
    }

    GenericListView {
        id: listView
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        width: root.host.hasDetail ? root.width * 348 / 638 - 24 : root.width - 24
        topMargin: 0
        listModel: root.host.listModel
        model: root.host.listModel
        autoWireModel: true
        selectFirstOnReset: root.host.listModel.selectFirstOnReset

        delegate: Loader {
            id: row
            width: ListView.view.width
            required property int index
            required property bool isSection
            required property string sectionName
            required property string title
            required property string iconSource
            required property bool isPinned
            required property bool isTagged
            required property bool isDraggable
            sourceComponent: isSection ? heading : entry

            Component {
                id: heading
                Item {
                    height: row.index === 0 ? 37 : 41
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 10
                        text: row.sectionName
                        color: Qt.alpha(Theme.foreground, 0.5)
                        font.family: "Onest"
                        font.pixelSize: 13
                    }
                }
            }

            Component {
                id: entry
                SelectableDelegate {
                    width: row.width
                    height: 48
                    selected: listView.currentIndex === row.index
                    onClicked: listView.currentIndex = row.index
                    onActivated: listView.itemActivated(row.index)
                    draggable: row.isDraggable
                    onDragRequested: source => {
                        listView.currentIndex = row.index;
                        root.host.listModel.startDrag(row.index, source);
                    }
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10
                        ViciImage {
                            source: row.iconSource
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                        }
                        Text {
                            text: row.title
                            textFormat: Text.PlainText
                            color: Theme.foreground
                            font.family: "Onest"
                            font.pixelSize: 15
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        ViciImage {
                            visible: row.isPinned || row.isTagged
                            source: Img.icon(row.isPinned ? BuiltinIcon.Pin : BuiltinIcon.Tag).withFillColor(Theme.textMuted)
                            Layout.preferredWidth: 12
                            Layout.preferredHeight: 12
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: divider
        visible: root.host.hasDetail
        x: root.width * 348 / 638
        y: 15
        width: 1
        height: parent.height - 31
        color: Qt.alpha(Theme.foreground, 0.13)
    }

    Item {
        id: detail
        visible: root.host.hasDetail
        anchors.left: divider.right
        anchors.leftMargin: 15
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16

        Item {
            id: preview
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: metadata.top
            anchors.bottomMargin: 12

            Loader {
                anchors.fill: parent
                active: root.host.hasDetailError
                visible: active
                sourceComponent: EmptyView {
                    title: root.host.detailErrorTitle
                    description: root.host.detailErrorDescription
                    icon: Img.icon(BuiltinIcon.Key).withFillColor(Theme.danger)
                }
            }
            ViciImage {
                id: imagePreview
                anchors.centerIn: parent
                width: root.host.detailIsFileIcon ? Math.min(134, parent.width) : parent.width
                height: root.host.detailIsFileIcon ? Math.min(134, parent.height) : parent.height
                source: root.host.detailImageSource
                visible: !root.host.hasDetailError && source !== ""
                fillMode: Image.PreserveAspectFit
                cache: false
                sourceSize: Qt.size(width * Screen.devicePixelRatio, height * Screen.devicePixelRatio)
                layer.enabled: visible && !root.host.detailIsFileIcon
                layer.effect: MultiEffect {
                    maskEnabled: true
                    maskSource: imageMask
                }
            }
            Item {
                id: imageMask
                anchors.fill: imagePreview
                visible: false
                layer.enabled: imagePreview.layer.enabled
                Rectangle {
                    readonly property real imageScale: Math.min(parent.width / Math.max(1, imagePreview.implicitWidth), parent.height / Math.max(1, imagePreview.implicitHeight))
                    anchors.centerIn: parent
                    width: imagePreview.implicitWidth * imageScale
                    height: imagePreview.implicitHeight * imageScale
                    radius: height > width ? 16 : 12
                    color: "white"
                }
            }
            TextViewer {
                id: textPreview
                anchors.fill: parent
                visible: !root.host.hasDetailError && root.host.detailImageSource === "" && root.host.detailTextContent !== ""
                text: root.host.detailTextContent
                highlightTerms: root.host.searchTerms
                padding: 0
                font.family: "Onest"
                font.pixelSize: 13
                lineHeight: 19
                layer.enabled: scrollable && !flickable.atYEnd
                layer.effect: MultiEffect {
                    maskEnabled: true
                    maskSource: fadeMask
                    maskThresholdMin: 0.5
                    maskSpreadAtMin: 1
                }
            }
            Rectangle {
                id: fadeMask
                anchors.fill: parent
                visible: false
                layer.enabled: true
                gradient: Gradient {
                    GradientStop {
                        position: 0.65
                        color: "white"
                    }
                    GradientStop {
                        position: 1
                        color: "transparent"
                    }
                }
            }
            EmptyView {
                anchors.fill: parent
                visible: !root.host.hasDetailError && root.host.detailImageSource === "" && root.host.detailTextContent === ""
                title: root.host.detailType
                description: qsTr("Preview not available for this content type")
            }
        }

        ColumnLayout {
            id: metadata
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            spacing: 3
            RowLayout {
                spacing: 6
                Text {
                    text: root.host.detailTitle
                    textFormat: Text.PlainText
                    color: Theme.foreground
                    font.family: "Onest"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                ViciImage {
                    visible: root.host.detailEncryptionIcon !== ""
                    source: root.host.detailEncryptionIcon
                    Layout.preferredWidth: 12
                    Layout.preferredHeight: 12
                }
            }
            Text {
                text: root.host.detailCopiedAt
                color: Qt.alpha(Theme.foreground, 0.4)
                font.family: "Onest"
                font.pixelSize: 13
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
    }
}
