// NameInputDialog.qml
import QtQuick 2.2
import Sailfish.Silica 1.0

Dialog {
    id: dialog
    property var result
    property string title

    SilicaListView {
        anchors.fill: parent

        header: DialogHeader { title: dialog.title }

        footer: IconButton {
            icon.source: "image://theme/icon-m-delete"
            onClicked: result = ""
        }

        model: Wunderful.folders
        delegate: BrowserDelegate {
            disableBrowse: true
            disableMenu: true
            highlighted: modelData.id === result
            onClicked: {
                result = modelData.id
            }
        }
        VerticalScrollDecorator {}
    }
}
