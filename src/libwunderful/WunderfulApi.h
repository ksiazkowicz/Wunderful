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

public:
    explicit WunderfulAPI(Settings *appSettings, QObject *parent = 0);
    QString getRandomString() const;

signals:
    void error(QString errorString);
    void authFailed(QString reason);
    void authSuccess();
    void itemsChanged();

private slots:
    void replyFinished(QNetworkReply *reply);

public slots:
    Q_INVOKABLE QString beginAuth();
    Q_INVOKABLE void getAuthToken(QString callbackUrl);
    Q_INVOKABLE void getFolders();
    Q_INVOKABLE void getLists();
    Q_INVOKABLE void getTasks(QString listId, bool completed);
    Q_INVOKABLE void getSubtasks(QString taskId, bool completed);
    Q_INVOKABLE QVariant getItems() { return QVariant::fromValue(root); }
    Q_INVOKABLE void resetList();
    Q_INVOKABLE void updateDueDate(QString taskId, QDate dueDate);

    Q_INVOKABLE void updateSubtask(QString subtaskId, QString title, bool completed);

private:
    QString apiUrl;
    QString oAuthUrl;

    QString state;
    QString authToken = "";

    void sendPostRequest(const QUrl &url, const QUrlQuery &data);
    void sendGetRequest(const QUrl &url);
    void sendPatchRequest(const QUrl &url, QString json);

    void accessTokenCallback(QString content);
    void fallbackCallback(QString content);
    void foldersCallback(QString content, bool update);
    void listsCallback(QString content, bool update);
    void tasksCallback(QString content, bool update);
    void subtasksCallback(QString content, bool update);

    Settings *settings;
    QNetworkAccessManager *mManager;
    QList<QObject*> root;
    QMap<QString, QObject*> folders;
    QMap<QString, QObject*> lists;
    QMap<QString, QObject*> tasks;
    QMap<QString, QObject*> subtasks;
};

#endif // WUNDERFULAPI_H
