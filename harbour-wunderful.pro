# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-wunderful

CONFIG += sailfishapp

SOURCES += src/main.cpp \
    src/settings.cpp \
    src/libwunderful/WunderfulApi.cpp

HEADERS += src/settings.h \
    src/libwunderful/WunderfulApi.h \
    src/libwunderful/nestedlistmodel.h \
    src/libwunderful/secrets.h

OTHER_FILES += \
    qml/cover/CoverPage.qml \
    qml/pages/SecondPage.qml \
    qml/pages/MainPage.qml \
    qml/pages/WelcomePage.qml \
    rpm/harbour-wunderful.changes.in \
    rpm/harbour-wunderful.spec \
    rpm/harbour-wunderful.yaml \
    translations/*.ts \
    harbour-wunderful.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-wunderful-de.ts

DISTFILES += \
    qml/pages/BrowsePage.qml \
    qml/main.qml \
    qml/pages/TaskPage.qml

