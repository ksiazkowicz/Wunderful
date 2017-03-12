import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    property alias model: listView.model
    property string title
    property int listId
    property string type

    SilicaListView {
        id: listView
        anchors.fill: parent

        header: PageHeader {
            title: page.title
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

        delegate: BackgroundItem {
            id: delegate

            Label {
                x: Theme.horizontalPageMargin
                text: modelData.title
                anchors.verticalCenter: parent.verticalCenter
                color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
            }
            onClicked: {
                if (modelData.type === "list" || modelData.type === "folder")
                    pageStack.push(Qt.resolvedUrl("BrowsePage.qml"), {model: modelData.items, title: modelData.title, listId: modelData.id });
                else pageStack.push(Qt.resolvedUrl("TaskPage.qml"), {model: modelData.items, title: modelData.title, task: modelData });
            }
        }
        VerticalScrollDecorator {}
    }
}
