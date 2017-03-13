// NameInputDialog.qml
import QtQuick 2.2
import Sailfish.Silica 1.0

Dialog {
    id: dialog
    property alias result: inputField.text
    property string title
    property string placeholder
    property string label

    Column {
        width: parent.width
        DialogHeader { title: dialog.title }

        TextField {
            id: inputField
            width: parent.width
            placeholderText: dialog.placeholder
            label: dialog.label
        }
    }
}
