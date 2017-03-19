import QtQuick 2.0
import Sailfish.Silica 1.0


Page {
    id: page
    property var task

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Remove")
                onClicked: {
                    pageStack.pop();
                    Wunderful.removeTask(task.id);
                }
            }
            MenuItem {
                text: qsTr("Rename")
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("../components/InputDialog.qml"),
                                                {result: task.title, title: qsTr("Enter new name"), placeholder: qsTr("Name"), label: qsTr("Name")})
                    dialog.accepted.connect(function() {
                        Wunderful.renameTask(task.id, dialog.result)
                    })
                }
            }

            MenuItem {
                text: task.starred ? qsTr("Unstar") : qsTr("Star")
                onClicked: Wunderful.starTask(task.id, !task.starred)
            }
            MenuItem {
                text: task.completed ? qsTr("Mark as not done") : qsTr("Mark as done")
                onClicked: Wunderful.completeTask(task.id, !task.completed)
            }
        }

        contentHeight: column.height

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingLarge
            PageHeader {
                title: task.title
            }
            Row {
                ValueButton {
                    id: dueDateField
                    property date selectedDate: task.dueDate
                    width: column.width - Theme.paddingLarge - removeDueDateBtn.width

                    function openDateDialog() {
                        var dialog = pageStack.push("Sailfish.Silica.DatePickerDialog", {
                                        date: selectedDate
                                     })

                        dialog.accepted.connect(function() {
                            value = dialog.date.toLocaleDateString()
                            selectedDate = dialog.date
                            Wunderful.updateDueDate(task.id, selectedDate)
                        })
                    }

                    label: qsTr("Due date")
                    description: task.dueDate < new Date() ? qsTr("Overdue") : ""
                    value: task.dueDate.toLocaleDateString()
                    onClicked: openDateDialog()
                    Component.onCompleted: if (value === "") value = qsTr("Select")
                }

                IconButton {
                    id: removeDueDateBtn
                    visible: dueDateField.value !== qsTr("Select")
                    icon.source: "image://theme/icon-m-clear?" + (pressed
                                 ? Theme.highlightColor
                                 : Theme.primaryColor)
                    onClicked: {
                        Wunderful.clearDueDate(task.id)
                        dueDateField.value = qsTr("Select")
                    }
                }
            }

            SectionHeader {
                text: qsTr("Subtasks")
            }
            SilicaListView {
                id: listView
                anchors { left: parent.left; right: parent.right; }
                model: task.items
                interactive: false
                height: listView.contentHeight
                delegate: ListItem {
                    id: delegate
                    Row {
                        TextSwitch {
                            text: modelData.title
                            checked: modelData.completed
                            onClicked: Wunderful.updateSubtask(modelData.id, modelData.title, checked)
                            width: column.width - Theme.paddingLarge - removeDueDateBtn.width
                        }
                        IconButton {
                            icon.source: "image://theme/icon-m-clear?" + (pressed
                                         ? Theme.highlightColor
                                         : Theme.primaryColor)
                            onClicked: Wunderful.removeSubtask(modelData.id)
                        }
                    }
                }
            }

            Row {
                TextField {
                    width: column.width
                    label: "New subtask"
                    placeholderText: label
                    EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: {
                        Wunderful.addSubtask(task.id, text)
                        text = ""
                    }
                }
            }

            SectionHeader {
                text: qsTr("Notes")
            }
            TextArea {
                width: column.width
                label: "Notes"
                placeholderText: label
            }

            SectionHeader {
                text: qsTr("Comments")
            }

            Row {
                TextArea {
                    id: commentArea
                    width: column.width - Theme.paddingLarge - commentBtn.width
                    label: "Comment"
                    placeholderText: label
                }

                IconButton {
                    id: commentBtn
                    anchors { bottom: commentArea.bottom; bottomMargin: Theme.paddingLarge }
                    icon.source: "image://theme/icon-m-add?" + (pressed
                                 ? Theme.highlightColor
                                 : Theme.primaryColor)
                    onClicked: Wunderful.addComment(task.id, commentArea.text)
                }
            }

            SectionHeader {
                text: qsTr("Files")
            }
            SilicaListView {
                id: listViewFiles
                anchors { left: parent.left; right: parent.right; }
                model: task.files
                interactive: false
                height: listViewFiles.contentHeight
                delegate: ListItem {
                    id: delegateFiles
                    width: parent.width
                    function getIcon() {
                        //if (modelData.type === "list")
                        return "image://theme/icon-m-document"
                    }
                    Row {
                        spacing: Theme.paddingLarge
                        x: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 2*x

                        Image {
                            id: icon
                            anchors.verticalCenter: parent.verticalCenter
                            source: delegateFiles.getIcon() + "?" + (pressed
                                         ? Theme.highlightColor
                                         : Theme.primaryColor)
                        }

                        Label {
                            text: modelData.title
                            color: delegateFiles.highlighted ? Theme.highlightColor : Theme.primaryColor
                            truncationMode: TruncationMode.Fade
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width - icon.width - parent.spacing
                        }
                    }
                    onClicked: Qt.openUrlExternally(modelData.url)

                    menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Delete")

                        }
                    }
                }
            }
            Label {
                visible: listViewFiles.count < 1
                x: Theme.horizontalPageMargin
                text: "No files"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
