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
    Q_PROPERTY(QVariant tasks READ getTasksList NOTIFY tasksChanged)
    Q_PROPERTY(QVariant folders READ getFolderList NOTIFY itemsChanged)
    Q_PROPERTY(QVariant rootItem READ getRootItem NOTIFY rootItemChanged)

public:
    explicit WunderfulAPI(Settings *appSettings, QObject *parent = 0);
    QString getRandomString() const;

    enum ContentTypes {
        WFolder = 0,
        WList,
        WTask,
        WSubtask,
        WFile,
    };

signals:
    void error(QString errorString);
    void authFailed(QString reason);
    void authInvalid();
    void authSuccess();
    void itemsChanged();
    void tasksChanged();
    void rootItemChanged();

private slots:
    void replyFinished(QNetworkReply *reply);

public slots:
    Q_INVOKABLE QString beginAuth();
    Q_INVOKABLE void getAuthToken(QString callbackUrl);
    Q_INVOKABLE void getContent(QString type, QString idKey, QString id, bool completed);
    Q_INVOKABLE void getContent(QString type);
    Q_INVOKABLE void getContent(QString type, bool completed);
    Q_INVOKABLE QVariant getRootItem() { return QVariant::fromValue(rootItem); }
    Q_INVOKABLE QVariant getTasksList() { return QVariant::fromValue(getObjectsByType("task")); }
    Q_INVOKABLE QVariant getFolderList() { return QVariant::fromValue(getObjectsByType("folder")); }

    Q_INVOKABLE void addList(QString title);

    Q_INVOKABLE void newFolder(QString listId, QString title);
    Q_INVOKABLE void moveToFolder(QString listId, QString folderId, bool remove);

    Q_INVOKABLE void addTask(QString listId, QString title);
    Q_INVOKABLE void completeTask(QString taskId, bool completed);
    Q_INVOKABLE void starTask(QString taskId, bool starred);

    Q_INVOKABLE void updateDueDate(QString taskId, QDate dueDate);
    Q_INVOKABLE void clearDueDate(QString taskId);

    Q_INVOKABLE void addSubtask(QString taskId, QString title);
    Q_INVOKABLE void updateSubtask(QString subtaskId, QString title, bool completed);

    Q_INVOKABLE void renameObject(QString type, QString id, QString title);
    Q_INVOKABLE void removeObject(QString type, QString id);
    Q_INVOKABLE void killOrphans(NestedListModel* item);

    Q_INVOKABLE QString getInboxId() { return this->inboxListId; }

private:
    QString apiUrl;
    QString oAuthUrl;

    QString state;
    QString authToken = "";

    QList<QObject*> getObjectsByType(QString type);

    void sendPostRequest(const QUrl &url, const QUrlQuery &data);
    void sendPostRequestJson(const QUrl &url, QString json);
    void sendDeleteRequest(const QUrl &url);
    void sendGetRequest(const QUrl &url);
    void sendPatchRequest(const QUrl &url, QString json);

    void accessTokenCallback(QString content);
    void fallbackCallback(QString url, QString content);
    void foldersCallback(QString content);
    void listsCallback(QString content);
    void tasksCallback(QString content);
    void subtasksCallback(QString content);
    void filesCallback(QString content);

    NestedListModel* updateOrCreate(QJsonObject *json);
    QJsonArray arrayFromDocument(QString content);

    int getObjectRevision(QString type, QString id);
    NestedListModel* getObject(QString type, QString id);

    void propagateChanges(NestedListModel *item);

    QString inboxListId;
    bool initialLoad;

    Settings *settings;
    QNetworkAccessManager *mManager;
    NestedListModel *rootItem;
    QMap<QString, QObject*> allObjects;
};

#endif // WUNDERFULAPI_H
