import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    BusyIndicator {
        running: true
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
    }
    Connections {
        target: Wunderful
        onAuthSuccess: pageStack.pop()
    }

    SilicaWebView {
        id: webView
        visible: false

        anchors.fill: parent
        url: Wunderful.beginAuth()

        onUrlChanged: {
            var urlString = url.toString();

            if (urlString.match("wunderful\/finish") && !urlString.match("wunderlist.com\/oauth")) {
                webView.stop()
                webView.visible = false
                Wunderful.getAuthToken(urlString);
            }
        }
    }

    Component.onCompleted: webView.visible = true
}
