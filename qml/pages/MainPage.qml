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
                text: qsTr("Add a to-do")
            }
        }

        model: Wunderful.items
        delegate: BrowserDelegate {}
        VerticalScrollDecorator {}

    }
    Component.onCompleted: Wunderful.getLists();
}
