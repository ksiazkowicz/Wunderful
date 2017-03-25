import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

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
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("../components/InputDialog.qml"),
                                                {title: qsTr("New list"), placeholder: qsTr("Name"), label: qsTr("Name")})
                    dialog.accepted.connect(function() {
                        Wunderful.addList(dialog.result)
                    })
                }
            }
            MenuItem {
                text: qsTr("Add a to-do")
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("../components/InputDialog.qml"),
                                                {title: qsTr("Add a to-do"), placeholder: qsTr("Name"), label: qsTr("Name")})
                    dialog.accepted.connect(function() {
                        Wunderful.addTask(Wunderful.getInboxId(), dialog.result)
                    })
                }
            }
        }

        model: Wunderful.items
        delegate: BrowserDelegate {}
        VerticalScrollDecorator {}

    }
    Component.onCompleted: Wunderful.getLists();
}
