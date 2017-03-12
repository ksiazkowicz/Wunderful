#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);

    // auth
    QString getAuthToken();
    void setAuthToken(QString token);
    void clearAuthCredentials();
    Q_INVOKABLE bool authCredentialsPresent();

    void setUsername(QString username);
    QString getUsername();


private:
    QSettings* settings;
};

#endif // SETTINGS_H
