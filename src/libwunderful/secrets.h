#ifndef SECRETS_H
#define SECRETS_H
#include <QString>

class WunderfulSecrets
{
public:
    WunderfulSecrets() {
        clientId = "";
        clientSecret = "";
        callbackUrl = "";
    }

    static WunderfulSecrets & getSingleton()
    {
        static WunderfulSecrets secrets;
        return secrets;
    }

    QString getClientId() { return clientId; }
    QString getClientSecret() { return clientSecret; }
    QString getCallbackUrl() { return callbackUrl; }

private:
    QString clientId;
    QString clientSecret;
    QString callbackUrl;
};

#endif // SECRETS_H
