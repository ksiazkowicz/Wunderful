#include "WunderfulApi.h"
#include <QDebug>
#include <QBuffer>

WunderfulAPI::WunderfulAPI(Settings *appSettings, QObject *parent) : QObject(parent)
{
    apiUrl = "https://a.wunderlist.com";
    oAuthUrl = "https://www.wunderlist.com/oauth";

    settings = appSettings;

    if (settings->authCredentialsPresent()) {
        authToken = settings->getAuthToken();
    }

    mManager = new QNetworkAccessManager(this);
    connect(mManager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));
}


QString WunderfulAPI::getRandomString() const {
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
   const int randomStringLength = 12; // assuming you want random strings of 12 characters

   QString randomString;
   for(int i=0; i<randomStringLength; ++i)
   {
       int index = qrand() % possibleCharacters.length();
       QChar nextChar = possibleCharacters.at(index);
       randomString.append(nextChar);
   }
   return randomString;

}

QString WunderfulAPI::beginAuth() {
    state = getRandomString();
    authToken = "";
    QString authUrl = oAuthUrl + "/authorize?client_id=";
    authUrl += WunderfulSecrets::getSingleton().getClientId();
    authUrl += "&redirect_uri=";
    authUrl += "http://krojony.pl/wunderful/finish";
    authUrl += "&state=";
    authUrl += this->state;
    return authUrl;
}

void WunderfulAPI::getAuthToken(QString callbackUrl) {
    QUrl url(callbackUrl);

    qDebug() << callbackUrl;

    QUrlQuery query = QUrlQuery(url.query());
    QString code = query.queryItemValue("code");
    QString state = query.queryItemValue("state");

    if (state != this->state) {
        qDebug() << "shit request, abort";
        return;
    }

    qDebug() << "got code:" << code;

    QUrlQuery postData;
    postData.addQueryItem("code", code);
    postData.addQueryItem("client_id", WunderfulSecrets::getSingleton().getClientId());
    postData.addQueryItem("client_secret", WunderfulSecrets::getSingleton().getClientSecret());
    // push request
    sendPostRequest(QUrl(oAuthUrl + "/access_token"),postData);
}

void WunderfulAPI::sendPostRequest(const QUrl &url, const QUrlQuery &data){
    QNetworkRequest r(url);
    r.setHeader(QNetworkRequest::ContentTypeHeader,
                       "application/x-www-form-urlencoded");
    if (authToken != "") {
        r.setRawHeader("X-Access-Token", authToken.toUtf8());
        r.setRawHeader("X-Client-ID", WunderfulSecrets::getSingleton().getClientId().toUtf8());
    }
    mManager->post(r, data.toString(QUrl::FullyEncoded).toUtf8());
}

void WunderfulAPI::sendPostRequestJson(const QUrl &url, QString json){
    QNetworkRequest r(url);
    r.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    if (authToken != "") {
        r.setRawHeader("X-Access-Token", authToken.toUtf8());
        r.setRawHeader("X-Client-ID", WunderfulSecrets::getSingleton().getClientId().toUtf8());
    }
    mManager->post(r, json.toUtf8());
}

void WunderfulAPI::sendDeleteRequest(const QUrl &url){
    QNetworkRequest r(url);
    r.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    if (authToken != "") {
        r.setRawHeader("X-Access-Token", authToken.toUtf8());
        r.setRawHeader("X-Client-ID", WunderfulSecrets::getSingleton().getClientId().toUtf8());
    }

    mManager->sendCustomRequest(r, "DELETE");
}

void WunderfulAPI::sendGetRequest(const QUrl &url) {
    QNetworkRequest r(url);
    if (authToken != "") {
        r.setRawHeader("X-Access-Token", authToken.toUtf8());
        r.setRawHeader("X-Client-ID", WunderfulSecrets::getSingleton().getClientId().toUtf8());
    }
    mManager->get(r);
}

void WunderfulAPI::sendPatchRequest(const QUrl &url, QString json) {
    QNetworkRequest r(url);
    r.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    if (authToken != "") {
        r.setRawHeader("X-Access-Token", authToken.toUtf8());
        r.setRawHeader("X-Client-ID", WunderfulSecrets::getSingleton().getClientId().toUtf8());
    }

    QBuffer* buffer = new QBuffer(mManager);
    buffer->setData(json.toUtf8());

    mManager->sendCustomRequest(r, "PATCH", buffer);
}

void WunderfulAPI::replyFinished(QNetworkReply *reply) {
    // if error occured, abort
    if (reply->error() != QNetworkReply::NoError){
        qWarning() << "ERROR:" << reply->errorString();
        qWarning() << reply->readAll();
        if (reply->errorString() == "Host requires authentication")
            emit authInvalid();
        return;
    }

    // get response code
    int v = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (v == 200 || v == 201) {
        QString content = reply->readAll();
        if (reply->url().toString().contains("/oauth/access_token")) {
            accessTokenCallback(content);
            return;
        }

        if (reply->url().toString().contains("/api/v1/folders")) {
            bool update = reply->url().toString().contains("/api/v1/folders/");
            foldersCallback(content, update);
            return;
        }

        if (reply->url().toString().contains("/api/v1/lists")) {
            bool update = reply->url().toString().contains("/api/v1/lists/");
            listsCallback(content, update);
            return;
        }

        if (reply->url().toString().contains("/api/v1/tasks")) {
            bool update = reply->url().toString().contains("/api/v1/tasks/");
            tasksCallback(content, update);
            return;
        }

        if (reply->url().toString().contains("/api/v1/subtasks")) {
            bool update = reply->url().toString().contains("/api/v1/subtasks/");
            subtasksCallback(content, update);
            return;
        }

        if (reply->url().toString().contains("/api/v1/files")) {
            bool update = reply->url().toString().contains("/api/v1/files/");
            filesCallback(content, update);
            return;
        }

        qDebug() << reply->url().toString();
        fallbackCallback(content);
    }

    if (v == 204) {
        // "delete" callback received
        QString url = reply->url().toString();
        if (url.contains("/api/v1/lists")) {
            QString listId = url.split("/api/v1/lists/").last();
            listId = listId.split("?").first();
            NestedListModel* list = (NestedListModel*)lists.value(listId);
            NestedListModel* parent = (NestedListModel*)list->getParent();
            if (root.indexOf((QObject*)list) < 0)
                parent->removeItem((QObject*)list);
            else root.removeOne((QObject*)list);
            lists.remove(listId);
            emit itemsChanged();
            delete list;
        }

        if (url.contains("/api/v1/tasks")) {
            QString taskId = url.split("/api/v1/tasks/").last();
            taskId = taskId.split("?").first();
            NestedListModel* task = (NestedListModel*)tasks.value(taskId);
            NestedListModel* parent = (NestedListModel*)task->getParent();
            parent->removeItem((QObject*)task);
            tasks.remove(taskId);
            emit tasksChanged();
            delete task;
        }

        if (url.contains("/api/v1/subtasks")) {
            QString taskId = url.split("/api/v1/subtasks/").last();
            taskId = taskId.split("?").first();
            NestedListModel* subtask = (NestedListModel*)subtasks.value(taskId);
            NestedListModel* parent = (NestedListModel*)subtask->getParent();
            parent->removeItem((QObject*)subtask);
            subtasks.remove(taskId);
            delete subtask;
        }

        if (url.contains("/api/v1/folders")) {
            QString folderId = url.split("/api/v1/folders/").last();
            folderId = folderId.split("?").first();
            NestedListModel* item = (NestedListModel*)folders.value(folderId);

            QList<QObject*> items = item->getItemsQO();
            for (int i=0; i<items.length(); i++) {
                NestedListModel* list = (NestedListModel*)items.at(i);
                root.append(list);
                list->setParent(0);
            }

            root.removeAll(item);
            folders.remove(folderId);
            delete item;
            emit itemsChanged();
        }
    }
}

void WunderfulAPI::fallbackCallback(QString content) {
    qDebug() << content;
}

void WunderfulAPI::accessTokenCallback(QString content) {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    if (jsonObject.keys().contains("access_token")) {
        authToken = jsonObject["access_token"].toString();
        settings->setAuthToken(authToken);
        emit authSuccess();
    } else {
        emit authFailed("Something happened.");
    }
}

void WunderfulAPI::getFolders() {
    this->sendGetRequest(apiUrl + "/api/v1/folders");
}

void WunderfulAPI::getLists() {
    this->sendGetRequest(apiUrl + "/api/v1/lists");
}

void WunderfulAPI::getTasks(QString listId, bool completed) {
    QString url = apiUrl + "/api/v1/tasks?list_id="+listId;
    if (completed) {
        url += "&completed=true";
    }
    this->sendGetRequest(url);
}

void WunderfulAPI::getSubtasks(QString taskId, bool completed) {
    NestedListModel* list = (NestedListModel*)tasks.value(taskId);
    if (list) {
        list->clearItems();
        QString url = apiUrl + "/api/v1/subtasks?task_id="+taskId;
        if (completed) {
            url += "&completed=1";
        }
        this->sendGetRequest(url);
    }
}

void WunderfulAPI::getFiles(QString taskId) {
    NestedListModel* list = (NestedListModel*)tasks.value(taskId);
    if (list) {
        list->clearFiles();
        QString url = apiUrl + "/api/v1/files?task_id="+taskId;
        this->sendGetRequest(url);
    }
}

void WunderfulAPI::updateDueDate(QString taskId, QDate dueDate) {
    NestedListModel* list = (NestedListModel*)tasks.value(taskId);
    int revision = list->getRevision();

    QJsonObject updateRequest;
    updateRequest["revision"] = revision;
    updateRequest["due_date"] = dueDate.toString(Qt::ISODate);
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/tasks/" + taskId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::clearDueDate(QString taskId) {
    NestedListModel* list = (NestedListModel*)tasks.value(taskId);
    int revision = list->getRevision();

    QJsonObject updateRequest;
    updateRequest["revision"] = revision;
    updateRequest["due_date"] = "";
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/tasks/" + taskId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::removeList(QString listId) {
    QString url = apiUrl + "/api/v1/lists/" + listId + "?revision=" + QString::number(getListRevision(listId));
    this->sendDeleteRequest(url);
}

void WunderfulAPI::removeFolder(QString folderId) {
    QString url = apiUrl + "/api/v1/folders/" + folderId + "?revision=" + QString::number(getFolderRevision(folderId));
    this->sendDeleteRequest(url);
}

void WunderfulAPI::renameFolder(QString folderId, QString title) {
    QJsonObject updateRequest;
    updateRequest["revision"] = getFolderRevision(folderId);
    updateRequest["title"] = title;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/folders/" + folderId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::newFolder(QString listId, QString title) {
    QJsonObject updateRequest;
    updateRequest["title"] = title;

    // list ids require some more work
    QJsonArray array;
    array.append(QJsonValue(QVariant(listId).toLongLong()));
    updateRequest["list_ids"] = array;

    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/folders";
    this->sendPostRequestJson(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::moveToFolder(QString listId, QString folderId, bool remove) {
    NestedListModel* item = (NestedListModel*)folders.value(folderId);

    // make an array of current list ids
    QJsonArray currentIds;
    QList<QObject*> items = item->getItemsQO();
    for (int i=0; i<items.length(); i++) {
        NestedListModel* list = (NestedListModel*)items.at(i);
        if (list->getId() != listId)
            currentIds.append(QJsonValue(list->getId().toLongLong()));
    }
    // append given id
    if (!remove)
        currentIds.append(QJsonValue(QVariant(listId).toLongLong()));

    // make update request
    QJsonObject updateRequest;
    updateRequest["revision"] = item->getRevision();
    updateRequest["list_ids"] = currentIds;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/folders/" + folderId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::renameList(QString listId, QString title) {
    QJsonObject updateRequest;
    updateRequest["revision"] = getListRevision(listId);
    updateRequest["title"] = title;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/lists/" + listId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::removeTask(QString taskId) {
    QString url = apiUrl + "/api/v1/tasks/" + taskId + "?revision=" + QString::number(getTaskRevision(taskId));
    this->sendDeleteRequest(url);
}

void WunderfulAPI::removeSubtask(QString subtaskId) {
    QString url = apiUrl + "/api/v1/subtasks/" + subtaskId + "?revision=" + QString::number(getSubtaskRevision(subtaskId));
    this->sendDeleteRequest(url);
}

void WunderfulAPI::renameTask(QString taskId, QString title) {
    QJsonObject updateRequest;
    updateRequest["revision"] = getTaskRevision(taskId);
    updateRequest["title"] = title;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/tasks/" + taskId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::completeTask(QString taskId, bool completed) {
    NestedListModel* item = (NestedListModel*)tasks.value(taskId);
    QJsonObject updateRequest;
    updateRequest["revision"] = item->getRevision();
    updateRequest["completed"] = completed;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/tasks/" + taskId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
    item->setCompleted(completed);
}

void WunderfulAPI::starTask(QString taskId, bool starred) {
    QJsonObject updateRequest;
    updateRequest["revision"] = getTaskRevision(taskId);
    updateRequest["starred"] = starred;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/tasks/" + taskId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

int WunderfulAPI::getListRevision(QString listId) {
    NestedListModel* item = (NestedListModel*)lists.value(listId);
    int revision = item->getRevision();
    return revision;
}

int WunderfulAPI::getFolderRevision(QString folderId) {
    NestedListModel* item = (NestedListModel*)folders.value(folderId);
    int revision = item->getRevision();
    return revision;
}

int WunderfulAPI::getTaskRevision(QString taskId) {
    NestedListModel* item = (NestedListModel*)tasks.value(taskId);
    int revision = item->getRevision();
    return revision;
}

int WunderfulAPI::getSubtaskRevision(QString subtaskId) {
    NestedListModel* item = (NestedListModel*)subtasks.value(subtaskId);
    int revision = item->getRevision();
    return revision;
}

void WunderfulAPI::updateSubtask(QString subtaskId, QString title, bool completed) {
    NestedListModel* item = (NestedListModel*)subtasks.value(subtaskId);
    int revision = item->getRevision();

    QJsonObject updateRequest;
    updateRequest["revision"] = revision;
    updateRequest["title"] = title;
    updateRequest["completed"] = completed;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/subtasks/" + subtaskId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::addTask(QString listId, QString title) {
    QJsonObject updateRequest;
    updateRequest["list_id"] = QVariant(listId).toLongLong();
    updateRequest["title"] = title;
    updateRequest["completed"] = false;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/tasks";
    this->sendPostRequestJson(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::addList(QString title) {
    QJsonObject updateRequest;
    updateRequest["title"] = title;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/lists";
    this->sendPostRequestJson(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::addSubtask(QString taskId, QString title) {
    QJsonObject updateRequest;
    updateRequest["task_id"] = QVariant(taskId).toLongLong();
    updateRequest["title"] = title;
    updateRequest["completed"] = false;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/subtasks";
    this->sendPostRequestJson(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::foldersCallback(QString content, bool update) {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(content.toUtf8());
    QJsonArray jsonArray;
    if (jsonResponse.isArray())
        jsonArray = jsonResponse.array();
    else jsonArray.append(jsonResponse.object());

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        QString id = object.value("id").toVariant().toString();

        bool up2 = folders.contains(id);

        NestedListModel *item = 0;
        if (!up2) {
            item = new NestedListModel();
        } else {
            item = (NestedListModel*)folders.value(id);
        }

        if (item == 0) {
            return;
        }

        item->setId(id);
        item->setTitle(object["title"].toString());
        item->setRevision(object["revision"].toInt());
        item->setType(object["type"].toString());

        QJsonArray listIds = object["list_ids"].toArray();
        QList<QString> usedIds;
        for (int i=0; i<listIds.count(); i++) {
            QString id = listIds[i].toVariant().toString();
            usedIds.append(id);
            if (lists.keys().contains(id)) {
                if (item->getItemById(id) == 0) {
                    item->addItem(lists.value(id));
                }
                NestedListModel* list = (NestedListModel*)lists.value(id);
                list->setParent(item);
                root.removeAll(lists.value(id));
            }
        }

        if (up2) {
            QList<QObject*> items = item->getItemsQO();
            for (int i=0; i<items.length(); i++) {
                NestedListModel* list = (NestedListModel*)items.at(i);
                if (!usedIds.contains(list->getId())) {
                    root.append(list);
                    item->removeItem(list);
                    list->setParent(0);
                }
            }
        }

        if (!up2) {
            folders.insert(id, item);
            root.append(item);
        }
    }
    emit itemsChanged();
}

void WunderfulAPI::listsCallback(QString content, bool update) {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(content.toUtf8());
    QJsonArray jsonArray;
    if (jsonResponse.isArray())
        jsonArray = jsonResponse.array();
    else jsonArray.append(jsonResponse.object());

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        QString id = object.value("id").toVariant().toString();
        int old_revision = 0;

        NestedListModel *item = 0;
        if (!update) {
            item = new NestedListModel();
        } else {
            item = (NestedListModel*)lists.value(id);
            old_revision = item->getRevision();
        }

        if (item == 0) {
            return;
        }
        item->setId(id);
        item->setTitle(object["title"].toString());
        item->setType(object["type"].toString());
        item->setRevision(object["revision"].toInt());
        if (object["list_type"] == "inbox")
            inboxListId = item->getId();
        if (!update) {
            root.append(item);
            lists.insert(item->getId(), item);
        }
        emit itemsChanged();
        if (old_revision < item->getRevision()) {
            this->getTasks(item->getId(), true);  // get completed tasks
            this->getTasks(item->getId(), false); // get other tasks
        }
    }
    if (!update) {
        this->getFolders();
    }
}

void WunderfulAPI::tasksCallback(QString content, bool update) {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(content.toUtf8());

    QJsonArray jsonArray;
    if (jsonResponse.isArray())
        jsonArray = jsonResponse.array();
    else jsonArray.append(jsonResponse.object());

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        QString id = object.value("id").toVariant().toString();
        int old_revision = 0;

        NestedListModel *item = 0;
        if (!update) {
            item = new NestedListModel();
        } else {
            item = (NestedListModel*)tasks.value(id);
            old_revision = item->getRevision();
        }

        if (item == 0) {
            return;
        }
        if (!update)
            item->setId(id);
        item->setTitle(object["title"].toString());
        item->setType(object["type"].toString());
        item->setRevision(object["revision"].toInt());
        item->setDueDate(QDate::fromString(object["due_date"].toString(), Qt::ISODate));
        item->setStarred(object["starred"].toBool());
        if (object.keys().contains("completed_by_id"))
            item->setCompleted(true);
        if (!update) {
            NestedListModel* list = (NestedListModel*)lists.value(object["list_id"].toVariant().toString());
            list->addItem(item);
            item->setParent((QObject*)list);
            tasks.insert(id, item);
            emit tasksChanged();
            if (old_revision < item->getRevision()) {
                this->getSubtasks(id, true);
                this->getFiles(id);
            }
        }
        if (i == jsonArray.count()-1 && old_revision < item->getRevision())
            propagateChanges(item);
    }
}

void WunderfulAPI::subtasksCallback(QString content, bool update) {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(content.toUtf8());

    QJsonArray jsonArray;
    if (jsonResponse.isArray())
        jsonArray = jsonResponse.array();
    else jsonArray.append(jsonResponse.object());

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        QString id = object.value("id").toVariant().toString();
        int old_revision = 0;

        update = subtasks.contains(id);

        NestedListModel *item = 0;
        if (!update) {
            item = new NestedListModel();
        } else {
            item = (NestedListModel*)subtasks.value(id);
            old_revision = item->getRevision();
        }

        if (item == 0) {
            return;
        }
        if (!update)
            item->setId(id);

        item->setTitle(object["title"].toString());
        item->setRevision(object["revision"].toInt());
        item->setType("subtask");
        item->setCompleted(object["completed"].toBool());
        NestedListModel* list = (NestedListModel*)tasks.value(object["task_id"].toVariant().toString());
        if (!update) {
            subtasks.insert(id, item);
            list->addItem(item);
            item->setParent((QObject*)list);
        }
        if (i == jsonArray.count()-1 && old_revision < item->getRevision())
            propagateChanges(item);
    }
}

void WunderfulAPI::filesCallback(QString content, bool update) {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(content.toUtf8());

    QJsonArray jsonArray;
    if (jsonResponse.isArray())
        jsonArray = jsonResponse.array();
    else jsonArray.append(jsonResponse.object());

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        QString id = object.value("id").toVariant().toString();

        update = subtasks.contains(id);

        NestedListModel *item = new NestedListModel();
        item->setId(id);
        item->setTitle(object["file_name"].toString());
        item->setRevision(object["revision"].toInt());
        item->setType(object["content_type"].toString());
        item->setUrl(object["url"].toString());
        NestedListModel* list = (NestedListModel*)tasks.value(object["task_id"].toVariant().toString());
        if (!update) {
            subtasks.insert(id, item);
            list->addFile(item);
            item->setParent((QObject*)list);
        }
    }
}

void WunderfulAPI::resetList() {
    root.clear();
    lists.clear();
}

void WunderfulAPI::propagateChanges(NestedListModel *item) {
    qDebug() << "propagateChanges called for" << item->getType() << item->getId();
    if (item->hasParent()) {
        NestedListModel *parent = (NestedListModel*)item->getParent();
        if (parent->getType() != "folder") {
            qDebug() << "propagating to" << parent->getType() << parent->getId();
            this->sendGetRequest(apiUrl + "/api/v1/" + parent->getType() + "s/" + parent->getId()); // reload data from server
            this->propagateChanges(parent); // do the same 1 level down
        }
    }
}
