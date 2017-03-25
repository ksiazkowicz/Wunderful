#ifndef WUNDERFULAPI_H
#define WUNDERFULAPI_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDate>
#include "secrets.h"
#include "../settings.h"
#include "nestedlistmodel.h"

class WunderfulAPI : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant items READ getItems NOTIFY itemsChanged)
    Q_PROPERTY(QVariant tasks READ getTasksList NOTIFY tasksChanged)
    Q_PROPERTY(QVariant folders READ getFolderList NOTIFY itemsChanged)

public:
    explicit WunderfulAPI(Settings *appSettings, QObject *parent = 0);
    QString getRandomString() const;

signals:
    void error(QString errorString);
    void authFailed(QString reason);
    void authInvalid();
    void authSuccess();
    void itemsChanged();
    void tasksChanged();

private slots:
    void replyFinished(QNetworkReply *reply);

public slots:
    Q_INVOKABLE QString beginAuth();
    Q_INVOKABLE void getAuthToken(QString callbackUrl);
    Q_INVOKABLE void getFolders();
    Q_INVOKABLE void getLists();
    Q_INVOKABLE void getTasks(QString listId, bool completed);
    Q_INVOKABLE void getFiles(QString taskId);
    Q_INVOKABLE void getSubtasks(QString taskId, bool completed);
    Q_INVOKABLE QVariant getItems() { return QVariant::fromValue(root); }
    Q_INVOKABLE QVariant getTasksList() { return QVariant::fromValue(tasks.values()); }
    Q_INVOKABLE QVariant getFolderList() { return QVariant::fromValue(folders.values()); }
    Q_INVOKABLE void resetList();

    Q_INVOKABLE void addList(QString title);
    Q_INVOKABLE void removeList(QString listId);
    Q_INVOKABLE void renameList(QString listId, QString title);

    Q_INVOKABLE void newFolder(QString listId, QString title);
    Q_INVOKABLE void removeFolder(QString folderId);
    Q_INVOKABLE void renameFolder(QString folderId, QString title);
    Q_INVOKABLE void moveToFolder(QString listId, QString folderId, bool remove);

    Q_INVOKABLE void addTask(QString listId, QString title);
    Q_INVOKABLE void removeTask(QString taskId);
    Q_INVOKABLE void renameTask(QString taskId, QString title);
    Q_INVOKABLE void completeTask(QString taskId, bool completed);
    Q_INVOKABLE void starTask(QString taskId, bool starred);

    Q_INVOKABLE void updateDueDate(QString taskId, QDate dueDate);
    Q_INVOKABLE void clearDueDate(QString taskId);

    Q_INVOKABLE void addSubtask(QString taskId, QString title);
    Q_INVOKABLE void updateSubtask(QString subtaskId, QString title, bool completed);
    Q_INVOKABLE void removeSubtask(QString subtaskId);

    Q_INVOKABLE QString getInboxId() { return this->inboxListId; }


private:
    QString apiUrl;
    QString oAuthUrl;

    QString state;
    QString authToken = "";

    void sendPostRequest(const QUrl &url, const QUrlQuery &data);
    void sendPostRequestJson(const QUrl &url, QString json);
    void sendDeleteRequest(const QUrl &url);
    void sendGetRequest(const QUrl &url);
    void sendPatchRequest(const QUrl &url, QString json);

    void accessTokenCallback(QString content);
    void fallbackCallback(QString content);
    void foldersCallback(QString content, bool update);
    void listsCallback(QString content, bool update);
    void tasksCallback(QString content, bool update);
    void subtasksCallback(QString content, bool update);
    void filesCallback(QString content, bool update);

    int getListRevision(QString listId);
    int getTaskRevision(QString taskId);
    int getSubtaskRevision(QString subtaskId);
    int getFolderRevision(QString folderId);

    QString inboxListId;

    Settings *settings;
    QNetworkAccessManager *mManager;
    QList<QObject*> root;
    QMap<QString, QObject*> folders;
    QMap<QString, QObject*> lists;
    QMap<QString, QObject*> tasks;
    QMap<QString, QObject*> subtasks;
};

#endif // WUNDERFULAPI_H
