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
                description: task.dueDate < new Date() ? qsTr("Overdue") : ""
                value: task.dueDate.toLocaleDateString()
                width: parent.width
                onClicked: openDateDialog()
                Component.onCompleted: if (value === "") value = qsTr("Select")
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
                    TextSwitch {
                        text: modelData.title
                        checked: modelData.completed
                        onClicked: Wunderful.updateSubtask(modelData.id, modelData.title, checked)
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
            Label {
                x: Theme.horizontalPageMargin
                text: "No files"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
