import QtQuick 2.0
import Sailfish.Silica 1.0

ListItem {
    id: delegate
    width: parent.width

    property bool disableMenu: false
    property bool disableBrowse: false

    function getIcon() {
        if (modelData.type === "list" && modelData.title === "inbox")
            return "image://theme/icon-m-mail-open"
        if (modelData.type === "folder")
            return "image://theme/icon-m-folder";
        if (modelData.type === "list")
            return "image://theme/icon-m-document";
        if (modelData.type === "task")
            return modelData.completed === true ? "image://theme/icon-m-certificates" : "image://theme/icon-m-tabs";
    }
    Row {
        spacing: Theme.paddingLarge
        x: Theme.horizontalPageMargin
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - 2*x

        Image {
            id: icon
            anchors.verticalCenter: parent.verticalCenter
            source: delegate.getIcon() + "?" + (pressed
                         ? Theme.highlightColor
                         : Theme.primaryColor)
        }

        Label {
            text: modelData.title
            color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
            truncationMode: TruncationMode.Fade
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - icon.width - parent.spacing*2 - iconStarred.width
        }

        Image {
            id: iconStarred
            anchors.verticalCenter: parent.verticalCenter
            source: modelData.type === "task" ? (modelData.starred ? "image://theme/icon-m-favorite-selected" : "image://theme/icon-m-favorite") + "?" + (pressed
                                                                          ? Theme.highlightColor
                                                                          : Theme.primaryColor) : ""
        }
    }
    onClicked: {
        if (!disableBrowse) {
            if (modelData.type === "list" || modelData.type === "folder")
                pageStack.push(Qt.resolvedUrl("../pages/BrowsePage.qml"), {list: modelData});
            else pageStack.push(Qt.resolvedUrl("../pages/TaskPage.qml"), {task: modelData});
        }
    }

    menu: ContextMenu {
        enabled: !disableMenu
        MenuItem {
            text: modelData.type === "folder" ? qsTr("Ungroup") : qsTr("Delete")
            onClicked: {
                if (modelData.type === "task")
                    Wunderful.removeTask(modelData.id);
                if (modelData.type === "list")
                    Wunderful.removeList(modelData.id)
                if (modelData.type === "folder")
                    Wunderful.removeFolder(modelData.id)
            }
        }
        MenuItem {
            visible: modelData.type === "list"
            text: qsTr("New folder")
            onClicked: {
                var dialog = pageStack.push(Qt.resolvedUrl("InputDialog.qml"),
                                            {title: qsTr("Enter folder name"), placeholder: qsTr("Name"), label: qsTr("Name")})
                dialog.accepted.connect(function() {
                    Wunderful.newFolder(modelData.id, dialog.result)
                })
            }
        }

        MenuItem {
            text: qsTr("Rename")
            onClicked: {
                var dialog = pageStack.push(Qt.resolvedUrl("InputDialog.qml"),
                                            {result: modelData.title, title: qsTr("Enter new name"), placeholder: qsTr("Name"), label: qsTr("Name")})
                dialog.accepted.connect(function() {
                    if (modelData.type === "task") {
                        Wunderful.renameTask(modelData.id, dialog.result)
                    }
                    if (modelData.type === "list") {
                        Wunderful.renameList(modelData.id, dialog.result)
                    }
                    if (modelData.type === "folder") {
                        Wunderful.renameFolder(modelData.id, dialog.result)
                    }
                })
            }
        }
        MenuItem {
            visible: modelData.type === "task"
            text: qsTr("Complete")
            onClicked: Wunderful.completeTask(modelData.id, !modelData.completed)
        }
        MenuItem {
            visible: modelData.type === "task"
            text: qsTr("Star")
            onClicked: Wunderful.starTask(modelData.id, !modelData.starred)
        }
        MenuItem {
            visible: modelData.type === "list"
            text: qsTr("Move to folder")
            onClicked: {
                var listParent = "";
                if (modelData.hasParent())
                    listParent = modelData.parent.id;

                var dialog = pageStack.push(Qt.resolvedUrl("FolderDialog.qml"),
                                            {result: listParent, title: qsTr("Choose folder to move to")})
                dialog.accepted.connect(function() {
                    if (dialog.result !== "")
                        Wunderful.moveToFolder(modelData.id, dialog.result, false)
                    else if (listParent) Wunderful.moveToFolder(modelData.id, listParent, true)
                })
            }
        }
    }
}
