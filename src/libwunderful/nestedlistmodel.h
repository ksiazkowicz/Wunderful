#ifndef USERMODEL_H
#define USERMODEL_H

#include <QObject>
#include <QDebug>
#include <QList>
#include <QDate>
#include <QVariant>

class NestedListModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ getId WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString type READ getType WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QVariant items READ getItems NOTIFY itemsChanged)
    Q_PROPERTY(int order READ getOrder WRITE setOrder NOTIFY orderChanged)
    Q_PROPERTY(bool completed READ getCompleted WRITE setCompleted NOTIFY completedChanged)
    Q_PROPERTY(QString note READ getNote WRITE setNote NOTIFY noteChanged)
    Q_PROPERTY(QVariant comments READ getComments NOTIFY commentsChanged)
    Q_PROPERTY(QVariant files READ getFiles NOTIFY filesChanged)
    Q_PROPERTY(QVariant reminders READ getReminders NOTIFY remindersChanged)
    Q_PROPERTY(QDate dueDate READ getDueDate WRITE setDueDate NOTIFY dueDateChanged)
public:
    explicit NestedListModel(QObject *parent = 0) : QObject(parent) {
    }

    void setTitle(QString title) { this->title = title; emit titleChanged(); }
    void setType(QString type) { this->type = type; emit typeChanged(); }
    void setId(QString id) { this->id = id; emit idChanged(); }
    void setOrder(int order) { this->order = order; emit orderChanged(); }
    void setCompleted(bool completed) { this->completed = completed; emit completedChanged(); }
    void setNote(QString note) { this->note = note; emit noteChanged(); }
    void setDueDate(QDate date) { this->dueDate = date; emit dueDateChanged(); }
    void setRevision(int revision) { this->revision = revision; emit revisionChanged(); }

    const QString getType() { return this->type; }
    const QString getTitle() { return this->title; }
    const QString getId() { return this->id; }
    QVariant getItems() { return QVariant::fromValue(this->items); }

    const int getOrder() { return this->order; }
    const bool getCompleted() { return this->completed; }
    const QString getNote() { return this->note; }
    QVariant getComments() { return QVariant::fromValue(this->comments); }
    QVariant getFiles() { return QVariant::fromValue(this->files); }
    QVariant getReminders() { return QVariant::fromValue(this->reminders); }
    const QDate getDueDate() { return this->dueDate; }
    const int getRevision() { return this->revision; }

    void addItem(QObject* item) {
        this->items.append(item);

        // connect all the signals
        this->connect(item, SIGNAL(idChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(titleChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(typeChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(itemsChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(orderChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(completedChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(noteChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(commentsChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(filesChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(remindersChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(dueDateChanged()), this, SLOT(updateItems()));
        this->connect(item, SIGNAL(revisionChanged()), this, SLOT(updateItems()));

        emit itemsChanged();
    }

    QObject* getItemById(QString id) {
        QObject* item = 0;
        QObject* tmpItem = 0;
        for (int i=0; i < items.count(); i++) {
            tmpItem = items.at(i);
            if (((NestedListModel*)tmpItem)->getId() == id)
                item = tmpItem;
        }
        return item;
    }

    void clearItems() {
        this->items.clear();
        emit itemsChanged();
    }
    void removeItem(QObject* item) {
        this->items.removeAll(item);
        emit itemsChanged();
    }

signals:
    void idChanged();
    void titleChanged();
    void typeChanged();
    void itemsChanged();

    void orderChanged();
    void completedChanged();
    void noteChanged();
    void commentsChanged();
    void filesChanged();
    void remindersChanged();

    void dueDateChanged();
    void revisionChanged();

public slots:
    void updateItems() { emit itemsChanged(); }

private:
    QString id;
    QString title;
    QString type;
    QList<QObject*> items;

    int order;
    int revision;

    // task specific
    bool completed;
    QString note;
    QList<QObject*> comments;
    QList<QObject*> files;
    QList<QObject*> reminders;

    QDate dueDate;
};

#endif // USERMODEL_H