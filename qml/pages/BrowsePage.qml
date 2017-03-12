import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

Page {
    id: page
    allowedOrientations: Orientation.All
    property var list

    SilicaListView {
        id: listView
        anchors.fill: parent
        model: list.items
        header: PageHeader { title: list.title }

        PullDownMenu {
            MenuItem {
                visible: list.type === "folder"
                text: qsTr("Ungroup")
            }
            MenuItem {
                visible: list.type === "list"
                text: qsTr("Delete")
            }
            MenuItem {
                visible: list.type === "list"
                text: qsTr("Share")
            }
            MenuItem {
                text: qsTr("Rename")
            }
            MenuItem {
                visible: list.type === "folder"
                text: qsTr("Create list")
            }
            MenuItem {
                visible: list.type === "list"
                text: qsTr("Add a to-do")
            }
        }

        delegate: BrowserDelegate {}
        VerticalScrollDecorator {}
    }
}
