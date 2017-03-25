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
                text: list.type === "folder" ? qsTr("Ungroup") : qsTr("Delete")
                onClicked: {
                    pageStack.pop()
                    Wunderful.removeObject(list.type, list.id)
                }
            }
            MenuItem {
                visible: list.type === "list"
                text: qsTr("Share")
            }
            MenuItem {
                text: qsTr("Rename")
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("../components/InputDialog.qml"),
                                                {title: qsTr("Rename"), result: list.title, placeholder: qsTr("Name"), label: qsTr("Name")})
                    dialog.accepted.connect(function() {
                        Wunderful.renameObject(list.type, list.id, dialog.result)
                    })
                }
            }
            MenuItem {
                visible: list.type === "folder"
                text: qsTr("Create list")
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("../components/InputDialog.qml"),
                                                {title: qsTr("New list"), placeholder: qsTr("Name"), label: qsTr("Name")})
                    dialog.accepted.connect(function() {
                        Wunderful.addList(dialog.result)
                    })
                }
            }
            MenuItem {
                visible: list.type === "list"
                text: qsTr("Add a to-do")
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("../components/InputDialog.qml"),
                                                {title: qsTr("Add a to-do"), placeholder: qsTr("Name"), label: qsTr("Name")})
                    dialog.accepted.connect(function() {
                        Wunderful.addTask(list.id, dialog.result)
                    })
                }
            }
        }

        delegate: BrowserDelegate {}
        VerticalScrollDecorator {}
    }
}
