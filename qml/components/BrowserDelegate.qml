import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    id: delegate
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
            width: parent.width - icon.width - parent.spacing
        }
    }
    onClicked: {
        if (modelData.type === "list" || modelData.type === "folder")
            pageStack.push(Qt.resolvedUrl("../pages/BrowsePage.qml"), {list: modelData});
        else pageStack.push(Qt.resolvedUrl("../pages/TaskPage.qml"), {task: modelData});
    }
}
