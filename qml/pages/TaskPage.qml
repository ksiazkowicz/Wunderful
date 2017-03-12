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
            }
            MenuItem {
                text: qsTr("Star")
            }
            MenuItem {
                text: qsTr("Mark as done")
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
            ValueButton {
                property date selectedDate: task.dueDate

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

                label: "Due date"
                value: task.dueDate.toLocaleDateString()
                width: parent.width
                onClicked: openDateDialog()
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
                delegate: BackgroundItem {
                    id: delegate
                    TextSwitch {
                        text: modelData.title
                        checked: modelData.completed
                        onClicked: Wunderful.updateSubtask(modelData.id, modelData.title, checked)
                    }
                }
            }

            Row {
                TextField{
                    id: subtaskArea
                    width: column.width - Theme.paddingLarge - subtaskBtn.width
                    label: "Subtask"
                    placeholderText: label
                }

                IconButton {
                    id: subtaskBtn
                    anchors { bottom: subtaskArea.bottom; bottomMargin: Theme.paddingLarge }
                    icon.source: "image://theme/icon-m-add?" + (pressed
                                 ? Theme.highlightColor
                                 : Theme.primaryColor)
                    onClicked: Wunderful.addSubtask(task.id, subtaskArea.text)
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
            Label {
                x: Theme.horizontalPageMargin
                text: "No files"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
