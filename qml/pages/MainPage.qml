import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    SilicaListView {
        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Your lists")
        }

        PullDownMenu {
            MenuItem {
                text: qsTr("Settings")
            }
            MenuItem {
                text: qsTr("Refresh")
                onClicked: {
                    Wunderful.resetList();
                    Wunderful.getLists();
                }
            }
            MenuItem {
                text: qsTr("New list")
            }
            MenuItem {
                text: qsTr("New task")
            }
        }

        model: Wunderful.items
        delegate: BackgroundItem {
            id: delegate

            Label {
                x: Theme.horizontalPageMargin
                text: modelData.title
                anchors.verticalCenter: parent.verticalCenter
                color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
            }
            onClicked: pageStack.push(Qt.resolvedUrl("BrowsePage.qml"), {model: modelData.items, title: modelData.title, listId: modelData.id })
        }
        VerticalScrollDecorator {}

        /*contentHeight: column.height

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Wunderful")
            }
            Label {
                x: Theme.horizontalPageMargin
                text: qsTr("Wunderful")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }
        }*/
    }
    Component.onCompleted: Wunderful.getLists();
}
