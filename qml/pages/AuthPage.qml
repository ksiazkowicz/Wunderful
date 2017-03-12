import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    SilicaWebView {
        id: webView

        anchors.fill: parent
        url: Wunderful.beginAuth()

        onUrlChanged: {
            var urlString = url.toString();

            if (urlString.match("wunderful\/finish") && !urlString.match("wunderlist.com\/oauth")) {
                webView.stop()
                Wunderful.getAuthToken(urlString);
                pageStack.pop()
            }
        }
    }
}
