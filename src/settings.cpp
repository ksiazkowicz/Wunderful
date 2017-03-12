#include "settings.h"

Settings::Settings(QObject *parent) : QObject(parent)
{
    // initialize settings object
    settings = new QSettings("krojony", "Wunderful");
}

bool Settings::authCredentialsPresent() {
    settings->beginGroup("Auth");
    bool result = settings->value("token", "").toString() != "";
    settings->endGroup();
    return result;
}

QString Settings::getAuthToken() {
    settings->beginGroup("Auth");
    QString result = settings->value("token", "").toString();
    settings->endGroup();
    return result;
}

void Settings::setAuthToken(QString token) {
    settings->beginGroup("Auth");
    settings->setValue("token", token);
    settings->endGroup();
}

void Settings::clearAuthCredentials() {
    settings->beginGroup("Auth");
    settings->setValue("token", "");
    settings->setValue("username", "");
    settings->endGroup();
}

QString Settings::getUsername() {
    settings->beginGroup("Auth");
    QString result = settings->value("username", "").toString();
    settings->endGroup();
    return result;
}

void Settings::setUsername(QString username) {
    settings->beginGroup("Auth");
    settings->setValue("username", username);
    settings->endGroup();
}
