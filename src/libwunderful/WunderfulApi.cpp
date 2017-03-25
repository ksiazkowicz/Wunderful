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

    initialLoad = true;
    rootItem = new NestedListModel();
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
    initialLoad = false;
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
    initialLoad = false;
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

    // parse path
    QString type, id;
    QStringList parameters = reply->url().toString().split("?").first().split("/");
    for (int i=0; i<3; i++)
        parameters.removeFirst();
    if (parameters.length() >= 3) {
        type = parameters.at(2);
        type.chop(1);
    }
    if (parameters.length() >= 4)
        id = parameters.at(3);

    if (v == 200 || v == 201) {
        QString content = reply->readAll();
        if (reply->url().toString().contains("/oauth/access_token")) {
            accessTokenCallback(content);
            return;
        }

        QStringList validTypes;
        validTypes << "folder" << "list" << "task" << "subtask" << "file";

        switch (validTypes.indexOf(type)) {
        case WFolder: foldersCallback(content); break;
        case WList: listsCallback(content); break;
        case WTask: tasksCallback(content); break;
        case WSubtask: subtasksCallback(content); break;
        case WFile: filesCallback(content); break;
        default: fallbackCallback(reply->url().toString(), content); break;
        }
    }

    // DELETE callback
    if (v == 204) {
        NestedListModel* item = getObject(type, id);
        allObjects.remove(item->getPath());
        if (item->hasParent()) {
            NestedListModel* parent = (NestedListModel*)item->getParent();
            parent->removeItem(item);
        }
        killOrphans(item);
        delete item;
    }
}

void WunderfulAPI::fallbackCallback(QString url, QString content) {
    qDebug() << url;
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

void WunderfulAPI::getContent(QString type) { getContent(type, false); }
void WunderfulAPI::getContent(QString type, bool completed) { getContent(type, "", "", completed); }
void WunderfulAPI::getContent(QString type, QString idKey, QString id, bool completed) {
    QString url = apiUrl + "/api/v1/"+type+"s";
    if (!idKey.isEmpty() || completed) {
        url += "?";
        if (!idKey.isEmpty())
            url += idKey + "=" + id + "&";
        if (completed)
            url += "&completed=true";
    }

    this->sendGetRequest(url);
}

void WunderfulAPI::updateDueDate(QString taskId, QDate dueDate) {
    NestedListModel* list = getObject("task", taskId);
    int revision = list->getRevision();

    QJsonObject updateRequest;
    updateRequest["revision"] = revision;
    updateRequest["due_date"] = dueDate.toString(Qt::ISODate);
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/tasks/" + taskId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::clearDueDate(QString taskId) {
    NestedListModel* list = getObject("task", taskId);
    int revision = list->getRevision();

    QJsonObject updateRequest;
    updateRequest["revision"] = revision;
    updateRequest["due_date"] = "";
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/tasks/" + taskId;
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
    NestedListModel* item = getObject("folder", folderId);

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

void WunderfulAPI::removeObject(QString type, QString id) {
    int revision = getObjectRevision(type, id);

    QString url = apiUrl + "/api/v1/" + type + "s/" + id + "?revision=" + QString::number(revision);
    this->sendDeleteRequest(url);
}

void WunderfulAPI::killOrphans(NestedListModel *item) {
    QList<QObject*> items = item->getItemsQO();
    for (int i=0; i<items.length(); i++) {
        NestedListModel* list = (NestedListModel*)items.at(i);
        if (list->getType() == "list")
            rootItem->addItem(list);
        else {
            killOrphans(list);
            allObjects.remove(list->getPath());
            item->removeItem(list);
            delete list;
        }
    }
}

void WunderfulAPI::renameObject(QString type, QString id, QString title) {
    QJsonObject updateRequest;
    updateRequest["revision"] = getObjectRevision(type, id);
    updateRequest["title"] = title;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/"+type+"s/" + id;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

void WunderfulAPI::completeTask(QString taskId, bool completed) {
    NestedListModel* item = getObject("task", taskId);
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
    updateRequest["revision"] = getObjectRevision("task", taskId);
    updateRequest["starred"] = starred;
    QJsonDocument doc(updateRequest);

    QString url = apiUrl + "/api/v1/tasks/" + taskId;
    this->sendPatchRequest(url, QString(doc.toJson(QJsonDocument::Compact)));
}

NestedListModel* WunderfulAPI::getObject(QString type, QString id) {
    QString path = type + ":" + id;
    if (allObjects.keys().contains(path))
        return (NestedListModel*)allObjects.value(path);
    return 0;
}

int WunderfulAPI::getObjectRevision(QString type, QString id) {
    return getObject(type, id)->getRevision();
}

void WunderfulAPI::updateSubtask(QString subtaskId, QString title, bool completed) {
    NestedListModel* item = getObject("subtask", subtaskId);
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

QJsonArray WunderfulAPI::arrayFromDocument(QString content) {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(content.toUtf8());
    QJsonArray jsonArray;
    if (jsonResponse.isArray())
        jsonArray = jsonResponse.array();
    else jsonArray.append(jsonResponse.object());
    return jsonArray;
}

NestedListModel* WunderfulAPI::updateOrCreate(QJsonObject *json) {
    // get data about object from json
    QString id = json->value("id").toVariant().toString();
    QString type = json->value("type").toString();

    // check if object already exists
    NestedListModel* item = getObject(type, id);

    // need a new one if doesn't exist
    if (item == 0)
        item = new NestedListModel();

    // set values that we already know are there
    item->setId(id);
    item->setType(type);

    // iterate through all json keys
    for (int i=0; i < json->keys().length(); i++) {
        QString key = json->keys().at(i);
        if (key == "title") item->setTitle(json->value(key).toString());
        if (key == "file_name") item->setTitle(json->value(key).toString());
        if (key == "url") item->setTitle(json->value(key).toString());
        if (key == "due_date") item->setDueDate(QDate::fromString(json->value(key).toString(), Qt::ISODate));
        if (key == "starred") item->setStarred(json->value(key).toBool());
        if (key == "completed_by_id") item->setCompleted(true);
        if (key == "completed") item->setCompleted(json->value(key).toBool());
    }

    return item;
}

void WunderfulAPI::foldersCallback(QString content) {
    QJsonArray jsonArray = arrayFromDocument(content);

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        NestedListModel *item = updateOrCreate(&object);
        item->setRevision(object["revision"].toInt());

        QJsonArray listIds = object["list_ids"].toArray();
        QList<QString> usedIds;
        for (int i=0; i<listIds.count(); i++) {
            QString id = listIds[i].toVariant().toString();
            usedIds.append(id);
            if (allObjects.keys().contains("list:"+id)) {
                NestedListModel* list = getObject("list", id);
                if (!item->hasItem(list))
                    item->addItem(list);
                rootItem->removeItem(list);
            }
        }

        QList<QObject*> items = item->getItemsQO();
        for (int i=0; i<items.length(); i++) {
            NestedListModel* list = (NestedListModel*)items.at(i);
            if (!usedIds.contains(list->getId())) {
                rootItem->addItem(list);
                item->removeItem(list);
            }
        }

        if (!rootItem->hasItem(item)) {
            allObjects.insert(item->getPath(), item);
            rootItem->addItem(item);
        }
    }
    emit itemsChanged();
}

void WunderfulAPI::listsCallback(QString content) {
    QJsonArray jsonArray = arrayFromDocument(content);

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        NestedListModel *item = updateOrCreate(&object);
        int old_revision = item->getRevision();

        item->setRevision(object["revision"].toInt());
        if (object["list_type"] == "inbox")
            inboxListId = item->getId();
        if (!rootItem->hasItem(item)) {
            rootItem->addItem(item);
            allObjects.insert(item->getPath(), item);
        }
        emit itemsChanged();
        if (old_revision < item->getRevision()) {
            this->getContent("task", "list_id", item->getId(), true);  // get completed tasks
            this->getContent("task", "list_id", item->getId(), false); // get other tasks
        }
    }
    if (initialLoad)
        this->getContent("folder");
}

void WunderfulAPI::tasksCallback(QString content) {
    QJsonArray jsonArray = arrayFromDocument(content);

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        NestedListModel *item = updateOrCreate(&object);
        int old_revision = item->getRevision();

        item->setRevision(object["revision"].toInt());
        NestedListModel* list = getObject("list", object["list_id"].toVariant().toString());
        if (!list->hasItem(item)) {
            list->addItem(item);
            allObjects.insert(item->getPath(), item);
            emit tasksChanged();
            if (old_revision < item->getRevision()) {
                this->getContent("subtask", "task_id", item->getId(), true);
                this->getContent("file", "task_id", item->getId(), false);
            }
        }
        if (i == jsonArray.count()-1 && old_revision < item->getRevision())
            propagateChanges(item);
    }
}

void WunderfulAPI::subtasksCallback(QString content) {
    QJsonArray jsonArray = arrayFromDocument(content);

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        NestedListModel *item = updateOrCreate(&object);
        int old_revision = item->getRevision();

        item->setRevision(object["revision"].toInt());
        NestedListModel* list = getObject("task", object["task_id"].toVariant().toString());
        if (!list->hasItem(item)) {
            allObjects.insert(item->getPath(), item);
            list->addItem(item);
        }
        if (i == jsonArray.count()-1 && old_revision < item->getRevision())
            propagateChanges(item);
    }
}

void WunderfulAPI::filesCallback(QString content) {
    QJsonArray jsonArray = arrayFromDocument(content);

    for (int i=0; i<jsonArray.count(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        NestedListModel *item = updateOrCreate(&object);
        int old_revision = item->getRevision();

        item->setRevision(object["revision"].toInt());
        //item->setType(object["content_type"].toString());
        NestedListModel* list = getObject("task", object["task_id"].toVariant().toString());
        if (!list->hasFile(item))
            list->addFile(item);

        if (i == jsonArray.count()-1 && old_revision < item->getRevision())
            propagateChanges(item);
    }
}

void WunderfulAPI::propagateChanges(NestedListModel *item) {
    if (item->hasParent() && !initialLoad) {
        qDebug() << "propagateChanges called for" << item->getType() << item->getId();
        NestedListModel *parent = (NestedListModel*)item->getParent();
        if (parent->getType() != "folder" && parent->getType() != "") {
            qDebug() << "propagating to" << parent->getType() << parent->getId();
            this->sendGetRequest(apiUrl + "/api/v1/" + parent->getType() + "s/" + parent->getId()); // reload data from server
            this->propagateChanges(parent); // do the same 1 level down
        }
    }
}

QList<QObject*> WunderfulAPI::getObjectsByType(QString type) {
    QList<QObject*> list;
    QList<QString> keys = allObjects.keys();
    for (int i=0; i < keys.count(); i++) {
        QString key = keys.at(i);
        if (key.startsWith(type + ":"))
            list.append(allObjects.value(key));
    }
    return list;
}
